/*
    Light Tower by Rob Shockency
    Copyright (C) 2021 Rob Shockency degnarraer@yahoo.com

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version of the License, or
    (at your option) any later version. 3

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#pragma once

#include "Arduino_JSON.h"
#include "Helpers.h"
#include "Streaming.h"
#include "DataTypes.h"
#include <mutex>

class DataSerializer: public CommonUtils
					, public DataTypeFunctions

{
	public:
		DataSerializer()
		{
			
	  	}
		virtual ~DataSerializer()
		{
			
		}
		virtual void SetDataSerializerDataItems(DataItem_t& DataItem, size_t DataItemCount)
		{
			m_DataItems = &DataItem;
			m_DataItemsCount = DataItemCount;
		}
		
		virtual String SerializeDataItemToJson(String Name, DataType_t DataType, void* Object, size_t Count, size_t ChangeCount)
		{
			if (Object == nullptr)
			{
				ESP_LOGW("SerializeDataItemToJson", "Object is null, serialization skipped");
				return "";
			}

			int32_t CheckSum = 0;
			size_t ObjectByteCount = GetSizeOfDataType(DataType);

			if (DataType >= DataType_Undef)
			{
				ESP_LOGW("SerializeDataItemToJson", "Invalid DataType index: %d", DataType);
				return "";
			}

			JSONVar data;
			JSONVar serializeDoc;
			serializeDoc[m_NameTag] = Name;
			serializeDoc[m_CountTag] = Count;
			serializeDoc[m_DataTypeTag] = DataTypeStrings[DataType];
			serializeDoc[m_TotalByteCountTag] = ObjectByteCount * Count;
			serializeDoc[m_ChangeCountTag] = ChangeCount;

			// Buffer to hold hex values for each byte
			char ByteHexValue[3]; // For hex byte conversion

			// Use an array of fixed-length strings or memory buffers to hold byte data
			String BytesString;
			BytesString.reserve(ObjectByteCount * 2); // Reserve enough space for hex values

			for (size_t i = 0; i < Count; ++i)
			{
				BytesString = "";  // Reset BytesString for each frame

				// Construct each frame's hex string
				for (size_t j = 0; j < ObjectByteCount; ++j)
				{
					uint8_t DecValue = ((uint8_t*)Object)[i * ObjectByteCount + j];
					sprintf(ByteHexValue, "%02X", DecValue);  // Convert byte to hex
					BytesString += ByteHexValue;  // Append hex byte value
					CheckSum += DecValue;  // Accumulate checksum
				}

				data[i] = BytesString;  // Store hex data in JSON object
			}

			serializeDoc[m_DataTag] = data;
			serializeDoc[m_CheckSumTag] = CheckSum;

			// Use try-catch for error handling during serialization
			try
			{
				String result = JSON.stringify(serializeDoc);
				return result;
			}
			catch (const std::exception& e)
			{
				ESP_LOGE("SerializeDataItemToJson", "ERROR! Error during JSON serialization: %s", e.what());
				return "{}";
			}
			catch (...)
			{
				ESP_LOGE("SerializeDataItemToJson", "ERROR! Unknown error during JSON serialization.");
				return "{}";
			}
			return "";
		}


		virtual bool DeSerializeJsonToNamedObject(String json, NamedObject_t &NamedObject)
		{
			bool deserialized = false;
			ESP_LOGD("DeSerializeJsonToNamedObject", "JSON String: %s", json.c_str());
			
			JSONVar deserializeDoc;
			deserializeDoc = JSON.parse(json);

			if (JSON.typeof(deserializeDoc) == "undefined")
			{
				++m_FailCount;
				NamedObject.Object = nullptr;
				ESP_LOGW("DeSerializeJsonToNamedObject", "WARNING! Parsing failed for Input: %s", json.c_str());
				return deserialized;
			}
			else
			{
				if (AllTagsExist(deserializeDoc)) // Check all tags before proceeding
				{
					// Extract necessary values
					const String Name = deserializeDoc[m_NameTag];
					ESP_LOGD("DeSerializeJsonToNamedObject", "Name: %s", Name.c_str());
					NamedObject.Name = Name;

					size_t CheckSumCalc = 0;
					size_t CheckSumIn = deserializeDoc[m_CheckSumTag];
					ESP_LOGD("DeSerializeJsonToNamedObject", "Sum: %i", CheckSumIn);

					size_t CountIn = deserializeDoc[m_CountTag];
					ESP_LOGD("DeSerializeJsonToNamedObject", "Object Count: %i", CountIn);

					size_t ChangeCountIn = deserializeDoc[m_ChangeCountTag];
					ESP_LOGD("DeSerializeJsonToNamedObject", "Change Count: %i", ChangeCountIn);
					NamedObject.ChangeCount = ChangeCountIn;

					size_t ByteCountIn = deserializeDoc[m_TotalByteCountTag];
					ESP_LOGD("DeSerializeJsonToNamedObject", "Byte Count: %i", ByteCountIn);

					DataType_t DataType = GetDataTypeFromString(deserializeDoc[m_DataTypeTag]);
					ESP_LOGD("DeSerializeJsonToNamedObject", "DataType: %i", DataType);

					size_t ActualDataCount = deserializeDoc[m_DataTag].length();
					ESP_LOGD("DeSerializeJsonToNamedObject", "Actual Count: %i", ActualDataCount);

					size_t ObjectByteCount = GetSizeOfDataType(DataType);
					ESP_LOGD("DeSerializeJsonToNamedObject", "Actual Byte Count: %i", ObjectByteCount);

					// Allocate buffer to store deserialized data
					uint8_t *Buffer = (uint8_t *)malloc(sizeof(uint8_t) * ByteCountIn); // Raw pointer allocation
					if (Buffer == nullptr)
					{
						ESP_LOGW("DeSerializeJsonToNamedObject", "Memory allocation failed for Buffer");
						NamedObject.Object = nullptr; // Set Object to nullptr if allocation fails
						++m_FailCount;
						return deserialized;
					}

					// Ensure the deserialized data is valid
					if (ActualDataCount == CountIn && ByteCountIn == ActualDataCount * ObjectByteCount)
					{
						// Process the data and calculate checksum
						for (int j = 0; j < CountIn; ++j)
						{
							String BytesString = deserializeDoc[m_DataTag][j];
							for (int k = 0; k < ObjectByteCount; ++k)
							{
								size_t startIndex = 2 * k;
								char hexArray[2];
								strcpy(hexArray, BytesString.substring(startIndex, startIndex + 2).c_str());
								long decValue = strtol(String(hexArray).c_str(), NULL, 16);
								CheckSumCalc += decValue;
								Buffer[j * ObjectByteCount + k] = decValue;
							}
						}
						ESP_LOGD("DeSerializeJsonToNamedObject", "Calculated Checksum: %i", CheckSumCalc);

						// Check if the calculated checksum matches the expected checksum
						if (CheckSumCalc == CheckSumIn)
						{
							ESP_LOGD("DeSerializeJsonToNamedObject", "Checksum matched, setting buffer");
							NamedObject.Object = Buffer; // Assign the buffer to NamedObject
							deserialized = true;
							ESP_LOGD("DeSerializeJsonToNamedObject", "Buffer set successfully");
						}
						else
						{
							ESP_LOGD("DeSerializeJsonToNamedObject", "Checksum mismatch!");
							free(Buffer); // Free buffer if checksum does not match
							NamedObject.Object = nullptr;
							++m_FailCount;
							ESP_LOGW("DeSerializeJsonToNamedObject", "Checksum error: (%i != %i)", CheckSumCalc, CheckSumIn);
						}
					}
					else
					{
						// Byte count or object count mismatch
						++m_FailCount;
						NamedObject.Object = nullptr;
						ESP_LOGW("DeSerializeJsonToNamedObject", "Byte count or object count mismatch.");
					}
				}
				else
				{
					// Missing or invalid tags
					++m_FailCount;
					NamedObject.Object = nullptr;
					ESP_LOGW("DeSerializeJsonToNamedObject", "Missing or invalid tags in input: %s", json.c_str());
				}
			}
			FailPercentage();
			return deserialized;
		}

		bool isHexadecimal(const String &str)
		{
			for (char c : str)
			{
				if (!isxdigit(c))
					return false;
			}
			return true;
		}

		void FailPercentage()
		{
			m_CurrentTime = millis();
			++m_TotalCount;
			if (m_CurrentTime - m_FailCountTimer >= m_FailCountDuration)
			{
				m_FailCountTimer = m_CurrentTime;
				ESP_LOGD("Serial_Datalink", "Deserialization Failure Percentage: %f", 100.0 * (float)m_FailCount / (float)m_TotalCount);
				m_FailCount = 0;
				m_TotalCount = 0;
			}
		}

		bool AllTagsExist(JSONVar &doc)
		{
			// Check if all required tags are present and valid
			if (!doc.hasOwnProperty(m_NameTag) || doc[m_NameTag] == nullptr)
			{
				ESP_LOGW("AllTagsExist", "Missing or invalid %s", m_NameTag.c_str());
				return false;
			}

			if (!doc.hasOwnProperty(m_CheckSumTag) || doc[m_CheckSumTag] == nullptr)
			{
				ESP_LOGW("AllTagsExist", "Missing or invalid %s", m_CheckSumTag.c_str());
				return false;
			}

			if (!doc.hasOwnProperty(m_CountTag) || doc[m_CountTag] == nullptr)
			{
				ESP_LOGW("AllTagsExist", "Missing or invalid %s", m_CountTag.c_str());
				return false;
			}

			if (!doc.hasOwnProperty(m_ChangeCountTag) || doc[m_ChangeCountTag] == nullptr)
			{
				ESP_LOGW("AllTagsExist", "Missing or invalid %s", m_ChangeCountTag.c_str());
				return false;
			}

			if (!doc.hasOwnProperty(m_TotalByteCountTag) || doc[m_TotalByteCountTag] == nullptr)
			{
				ESP_LOGW("AllTagsExist", "Missing or invalid %s", m_TotalByteCountTag.c_str());
				return false;
			}

			if (!doc.hasOwnProperty(m_DataTag) || doc[m_DataTag] == nullptr)
			{
				ESP_LOGW("AllTagsExist", "Missing or invalid %s", m_DataTag.c_str());
				return false;
			}

			// If all tags are valid
			return true;
		}


	private:
		size_t m_TotalCount = 0;
		size_t m_FailCount = 0;
		uint64_t m_CurrentTime = 0;
		uint64_t m_FailCountTimer = 0;
		uint64_t m_FailCountDuration = 5000;
		DataItem_t* m_DataItems;
		size_t m_DataItemsCount = 0;
		//Tags
		String m_NameTag = "N";
		String m_CheckSumTag = "S";
		String m_CountTag = "C";
		String m_ChangeCountTag = "I";
		String m_DataTag = "D";
		String m_DataTypeTag = "T";
		String m_TotalByteCountTag = "B";
};