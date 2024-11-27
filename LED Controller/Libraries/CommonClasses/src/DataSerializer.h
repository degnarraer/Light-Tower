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
			m_SerializeDocLock = xSemaphoreCreateMutex();
			if (m_SerializeDocLock == nullptr)
			{
				ESP_LOGE("WebSocketDataProcessor", "ERROR! Failed to create semaphore.");
			}
			m_DeSerializeDocLock = xSemaphoreCreateMutex();
			if (m_DeSerializeDocLock == nullptr)
			{
				ESP_LOGE("WebSocketDataProcessor", "ERROR! Failed to create semaphore.");
			}
	  	}
		virtual ~DataSerializer()
		{
			if (m_SerializeDocLock)
			{
				vSemaphoreDelete(m_SerializeDocLock);
				m_SerializeDocLock = nullptr;
			}
			if (m_DeSerializeDocLock)
			{
				vSemaphoreDelete(m_DeSerializeDocLock);
				m_DeSerializeDocLock = nullptr;
			}
		}
		virtual void SetDataSerializerDataItems(DataItem_t& DataItem, size_t DataItemCount)
		{
			m_DataItems = &DataItem;
			m_DataItemsCount = DataItemCount;
		}
		
		virtual String SerializeDataItemToJson(String Name, DataType_t DataType, void* Object, size_t Count, size_t ChangeCount)
		{
			if (xSemaphoreTakeRecursive(m_SerializeDocLock, pdMS_TO_TICKS(100)) == pdTRUE)
			{
				if (Object == nullptr)
				{
					ESP_LOGW("SerializeDataItemToJson", "Object is null, serialization skipped");
					xSemaphoreGiveRecursive(m_SerializeDocLock);
					return "";
				}

				int32_t CheckSum = 0;
				size_t ObjectByteCount = GetSizeOfDataType(DataType);

				if (DataType >= DataType_Undef)
				{
					ESP_LOGW("SerializeDataItemToJson", "Invalid DataType index: %d", DataType);
					xSemaphoreGiveRecursive(m_SerializeDocLock);
					return "";
				}

				JSONVar data;
				m_SerializeDoc[m_NameTag] = Name;
				m_SerializeDoc[m_CountTag] = Count;
				m_SerializeDoc[m_DataTypeTag] = DataTypeStrings[DataType];
				m_SerializeDoc[m_TotalByteCountTag] = ObjectByteCount * Count;
				m_SerializeDoc[m_ChangeCountTag] = ChangeCount;

				char ByteHexValue[3];
				for (int i = 0; i < Count; ++i)
				{
					String BytesString = "";
					for (int j = 0; j < ObjectByteCount; ++j)
					{
						uint8_t DecValue = ((uint8_t*)Object)[i * ObjectByteCount + j];
						sprintf(ByteHexValue, "%02X", DecValue);
						BytesString += String(ByteHexValue);
						CheckSum += DecValue;
					}
					data[i] = BytesString;
				}
				
				m_SerializeDoc[m_DataTag] = data;
				m_SerializeDoc[m_CheckSumTag] = CheckSum;

				try
				{
					String result = JSON.stringify(m_SerializeDoc);
					xSemaphoreGiveRecursive(m_SerializeDocLock);
					return result;
				}
				catch (const std::exception& e)
				{
					ESP_LOGE("SerializeDataItemToJson", "ERROR! Error during JSON serialization: %s", e.what() );
					xSemaphoreGiveRecursive(m_SerializeDocLock);
					return "{}";
				}
				catch (...)
				{
					ESP_LOGE("SerializeDataItemToJson", "ERROR! Unknown error during JSON serialization.");
					xSemaphoreGiveRecursive(m_SerializeDocLock);
					return "{}";
				}
			}
			else
			{
				ESP_LOGW("Semaphore Take Failure", "WARNING! Failed to take Semaphore");
			}
		}

		virtual bool DeSerializeJsonToNamedObject(String json, NamedObject_t &NamedObject)
		{
			bool deserialized = false;
			if (xSemaphoreTakeRecursive(m_DeSerializeDocLock, pdMS_TO_TICKS(100)) == pdTRUE)
			{
				ESP_LOGD("DeSerializeJsonToNamedObject", "JSON String: %s", json.c_str());
				// Parse the JSON
				m_DeserializeDoc = JSON.parse(json);

				if (JSON.typeof(m_DeserializeDoc) == "undefined")
				{
					++m_FailCount;
					NamedObject.Object = nullptr;
					ESP_LOGW("DeSerializeJsonToNamedObject", "WARNING! Parsing failed for Input: %s", json.c_str());
					xSemaphoreGiveRecursive(m_DeSerializeDocLock);
					return deserialized;
				}
				else
				{
					if (AllTagsExist(m_DeserializeDoc)) // Check all tags before proceeding
					{
						// Extract necessary values
						const String Name = m_DeserializeDoc[m_NameTag];
						ESP_LOGD("DeSerializeJsonToNamedObject", "Name: %s", Name.c_str());
						NamedObject.Name = Name;

						size_t CheckSumCalc = 0;
						size_t CheckSumIn = m_DeserializeDoc[m_CheckSumTag];
						ESP_LOGD("DeSerializeJsonToNamedObject", "Sum: %i", CheckSumIn);

						size_t CountIn = m_DeserializeDoc[m_CountTag];
						ESP_LOGD("DeSerializeJsonToNamedObject", "Object Count: %i", CountIn);

						size_t ChangeCountIn = m_DeserializeDoc[m_ChangeCountTag];
						ESP_LOGD("DeSerializeJsonToNamedObject", "Change Count: %i", ChangeCountIn);
						NamedObject.ChangeCount = ChangeCountIn;

						size_t ByteCountIn = m_DeserializeDoc[m_TotalByteCountTag];
						ESP_LOGD("DeSerializeJsonToNamedObject", "Byte Count: %i", ByteCountIn);

						DataType_t DataType = GetDataTypeFromString(m_DeserializeDoc[m_DataTypeTag]);
						ESP_LOGD("DeSerializeJsonToNamedObject", "DataType: %i", DataType);

						size_t ActualDataCount = m_DeserializeDoc[m_DataTag].length();
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
							xSemaphoreGiveRecursive(m_DeSerializeDocLock);
							return deserialized;
						}

						// Ensure the deserialized data is valid
						if (ActualDataCount == CountIn && ByteCountIn == ActualDataCount * ObjectByteCount)
						{
							// Process the data and calculate checksum
							for (int j = 0; j < CountIn; ++j)
							{
								String BytesString = m_DeserializeDoc[m_DataTag][j];
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
				xSemaphoreGiveRecursive(m_DeSerializeDocLock);
			}
			else
			{
				ESP_LOGW("Semaphore Take Failure", "WARNING! Failed to take Semaphore");
			}
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
		JSONVar m_SerializeDoc;
		SemaphoreHandle_t m_SerializeDocLock;
		JSONVar m_DeserializeDoc;
		SemaphoreHandle_t m_DeSerializeDocLock;
		DataItem_t* m_DataItems;
		size_t m_DataItemsCount = 0;
		//Tags
		String m_NameTag = "Name";
		String m_CheckSumTag = "Sum";
		String m_CountTag = "Count";
		String m_ChangeCountTag = "Change Count";
		String m_DataTag = "Data";
		String m_DataTypeTag = "Type";
		String m_TotalByteCountTag = "Bytes";
};