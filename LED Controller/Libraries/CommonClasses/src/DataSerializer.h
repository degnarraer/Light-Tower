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

class DataSerializer: public CommonUtils
					, public DataTypeFunctions

{
	public:
		DataSerializer(){}
		virtual ~DataSerializer(){}
		virtual void SetDataSerializerDataItems(DataItem_t& DataItem, size_t DataItemCount)
		{
			m_DataItems = &DataItem;
			m_DataItemsCount = DataItemCount;
		}
		virtual String SerializeDataToJson(String Name, DataType_t DataType, void* Object, size_t Count)
		{
			int32_t CheckSum = 0;
			size_t ObjectByteCount = GetSizeOfDataType(DataType);
			
			JSONVar data;

			m_SerializeDoc[m_NameTag] = Name;
			m_SerializeDoc[m_CountTag] = Count;
			m_SerializeDoc[m_DataTypeTag] = DataTypeStrings[DataType];
			m_SerializeDoc[m_TotalByteCountTag] = ObjectByteCount * Count;
			
			char ByteHexValue[3];
			for(int i = 0; i < Count; ++i)
			{
				String BytesString = "";
				for(int j = 0; j < ObjectByteCount; ++j)
				{
					uint8_t DecValue = ((uint8_t*)Object)[i*ObjectByteCount + j];
					sprintf(ByteHexValue,"%02X", DecValue);
					BytesString += String(ByteHexValue);
					CheckSum += DecValue;
				}
				data[i] = BytesString;
			}
			m_SerializeDoc[m_DataTag] = data;
			m_SerializeDoc[m_CheckSumTag] = CheckSum;
			return JSON.stringify(m_SerializeDoc);
		}
		virtual bool DeSerializeJsonToNamedObject(String json, NamedObject_t &NamedObject)
		{
			ESP_LOGD("DeSerializeJsonToNamedObject", "JSON String: %s", json.c_str());
			bool deserialized = false;
			m_DeserializeDoc = JSON.parse(json);
			if (JSON.typeof(m_DeserializeDoc) == "undefined")
			{
				++m_FailCount;
				NamedObject.Object = nullptr;
				ESP_LOGW("DeSerializeJsonToNamedObject", "WARNING! Parsing failed for Input: %s", json.c_str());
				return deserialized;
			}
			else
			{
				if(AllTagsExist(m_DeserializeDoc))
				{
					const String Name = m_DeserializeDoc[m_NameTag];
					ESP_LOGD("DeSerializeJsonToNamedObject", "Name: %s", Name.c_str());
					NamedObject.Name = Name;
					size_t CheckSumCalc = 0;
					size_t CheckSumIn = m_DeserializeDoc[m_CheckSumTag];
					ESP_LOGD("DeSerializeJsonToNamedObject", "Sum: %i", CheckSumIn);
					size_t CountIn = m_DeserializeDoc[m_CountTag];
					ESP_LOGD("DeSerializeJsonToNamedObject", "Object Count: %i", CountIn);
					size_t ByteCountIn = m_DeserializeDoc[m_TotalByteCountTag];
					ESP_LOGD("DeSerializeJsonToNamedObject", "Byte Count: %i", ByteCountIn);
					DataType_t DataType = GetDataTypeFromString(m_DeserializeDoc[m_DataTypeTag]);
					ESP_LOGD("DeSerializeJsonToNamedObject", "DataType: %i", DataType);
					size_t ActualDataCount = m_DeserializeDoc[m_DataTag].length();
					ESP_LOGD("DeSerializeJsonToNamedObject", "Actual Count: %i", ActualDataCount);
					size_t ObjectByteCount = GetSizeOfDataType(DataType);
					ESP_LOGD("DeSerializeJsonToNamedObject", "Actual Byte Count: %i", ObjectByteCount);
					//This memory needs deleted by caller of function.
					uint8_t *Buffer = (uint8_t*)malloc(sizeof(uint8_t)* ByteCountIn);								
					if( ActualDataCount == CountIn && ByteCountIn == ActualDataCount * ObjectByteCount )
					{
						for(int j = 0; j < CountIn; ++j)
						{
							String BytesString = m_DeserializeDoc[m_DataTag][j];
							for(int k = 0; k < ObjectByteCount; ++k)
							{
								size_t startIndex = 2*k;
								char hexArray[2];
								strcpy(hexArray, BytesString.substring(startIndex,startIndex+2).c_str());
								long decValue = strtol(String(hexArray).c_str(), NULL, 16);
								CheckSumCalc += decValue;
								Buffer[j * ObjectByteCount + k] = decValue;
							}
							ESP_LOGD("DeSerializeJsonToNamedObject", "Actual Sum: %i", CheckSumCalc);
						}
						if(CheckSumCalc == CheckSumIn)
						{
							ESP_LOGD("DeSerializeJsonToNamedObject", "Setting Buffer");
							NamedObject.Object = Buffer;
							deserialized = true;
							ESP_LOGD("DeSerializeJsonToNamedObject", "Buffer Set");
						}
						else
						{
							ESP_LOGD("DeSerializeJsonToNamedObject", "CheckSumCalc Did Not Match!");
							free(Buffer);
							NamedObject.Object = nullptr;
							++m_FailCount;
							ESP_LOGW("DeSerializeJsonToNamedObject", "WARNING! Deserialize failed: \"Checksum Error\" Value: \"(%i != %i)\"", CheckSumCalc, CheckSumIn);
						}
					}
					else
					{
						++m_FailCount;
						NamedObject.Object = nullptr;
						ESP_LOGW("DeSerializeJsonToNamedObject", "WARNING! Deserialize failed: Byte Count Error.");
					}
				}
				else
				{
					++m_FailCount;
					NamedObject.Object = nullptr;
					ESP_LOGW("DeSerializeJsonToNamedObject", "WARNING! Deserialize failed: \"Missing Tags\" Input: \"%s\"", json.c_str());
				}
			}
			FailPercentage();
			return deserialized;
		}
		virtual void FailPercentage()
		{
			m_CurrentTime = millis();
			++m_TotalCount;
			if(m_CurrentTime - m_FailCountTimer >= m_FailCountDuration)
			{
				m_FailCountTimer = m_CurrentTime;
				ESP_LOGD("Serial_Datalink", "Deserialization Failure Percentage: %f", 100.0 * (float)m_FailCount / (float)m_TotalCount);
				m_FailCount = 0;
				m_TotalCount = 0;
			}	
		}
		virtual bool AllTagsExist(JSONVar &jsonObject)
		{
			const String tags[] = {m_NameTag, m_CheckSumTag, m_CountTag, m_DataTag, m_DataTypeTag, m_TotalByteCountTag};
			bool result = true;
			for (const String& tag : tags) {
				if (!jsonObject.hasOwnProperty(tag)) {
					ESP_LOGW("AllTagsExist", "WARNING! Missing Tag: \"%s\"", tag.c_str());
					result = false;
				}
			}
			return result;
		}
	private:
		size_t m_TotalCount = 0;
		size_t m_FailCount = 0;
		uint64_t m_CurrentTime = 0;
		uint64_t m_FailCountTimer = 0;
		uint64_t m_FailCountDuration = 5000;
		JSONVar m_SerializeDoc;
		JSONVar m_DeserializeDoc;
		DataItem_t* m_DataItems;
		size_t m_DataItemsCount = 0;
		//Tags
		String m_NameTag = "Name";
		String m_CheckSumTag = "Sum";
		String m_CountTag = "Count";
		String m_DataTag = "Data";
		String m_DataTypeTag = "Type";
		String m_TotalByteCountTag = "Bytes";
};