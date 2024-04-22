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

#ifndef DataSerializer_H
#define DataSerializer_H
#include <stdlib.h>
#include <Arduino.h>
#include <Helpers.h>
#include "Streaming.h"
#include "Arduino_JSON.h"

class DataSerializer: public CommonUtils
					, public QueueController

{
	public:
		DataSerializer(){}
		virtual ~DataSerializer(){}
		void SetDataSerializerDataItems(DataItem_t& DataItem, size_t DataItemCount)
		{
			m_DataItems = &DataItem;
			m_DataItemsCount = DataItemCount;
		}
		String SerializeDataToJson(String Name, DataType_t DataType, void* Object, size_t Count)
		{
			String Result = "";
			int32_t CheckSum = 0;
			size_t ObjectByteCount = GetSizeOfDataType(DataType);
			JSONVar serializeDoc;
			JSONVar data;
			serializeDoc[m_NameTag] = Name;
			serializeDoc[m_CountTag] = Count;
			serializeDoc[m_DataTypeTag] = DataTypeStrings[DataType];
			serializeDoc[m_TotalByteCountTag] = ObjectByteCount * Count;
			for(int i = 0; i < Count; ++i)
			{
				String BytesString = "";
				for(int j = 0; j < ObjectByteCount; ++j)
				{
					uint8_t DecValue = ((uint8_t*)Object)[i*ObjectByteCount + j];
					char ByteHexValue[2];
					sprintf(ByteHexValue,"%02X", DecValue);
					BytesString += String(ByteHexValue);
					CheckSum += DecValue;
				}
				data[i] = BytesString;
			}
			serializeDoc[m_DataTag] = data;
			serializeDoc[m_CheckSumTag] = CheckSum;
			Result =  JSON.stringify(serializeDoc);
			return Result;
		}
		void DeSerializeJsonToNamedObject(String json, NamedObject_t &NamedObject)
		{
			ESP_LOGD("DeSerializeJsonToNamedObject", "JSON String: %s", json.c_str());
			
			JSONVar jsonObject = JSON.parse(json);
			if (JSON.typeof(jsonObject) == "undefined")
			{
				++m_FailCount;
				NamedObject.Object = nullptr;
				ESP_LOGE("DeSerializeJsonToNamedObject", "Parsing failed for Input: %s", json.c_str());
			}
			else
			{
				if(AllTagsExist(jsonObject))
				{
					const String DocName = jsonObject[m_NameTag];
					NamedObject.Name = DocName;
					size_t CheckSumCalc = 0;
					size_t CheckSumIn = jsonObject[m_CheckSumTag];
					size_t CountIn = jsonObject[m_CountTag];
					size_t ByteCountIn = jsonObject[m_TotalByteCountTag];
					size_t ActualDataCount = jsonObject[m_DataTag].length();
					DataType_t DataType = GetDataTypeFromString(jsonObject[m_DataTypeTag]);
					size_t ObjectByteCount = GetSizeOfDataType(DataType);
					//This memory needs deleted by caller of function.
					uint8_t *Buffer = (uint8_t*)heap_caps_malloc(sizeof(uint8_t)* ByteCountIn, MALLOC_CAP_SPIRAM);								
					if( ActualDataCount == CountIn && ByteCountIn == ActualDataCount * ObjectByteCount )
					{
						for(int j = 0; j < CountIn; ++j)
						{
							String BytesString = jsonObject[m_DataTag][j];
							for(int k = 0; k < ObjectByteCount; ++k)
							{
								size_t startIndex = 2*k;
								char hexArray[2];
								strcpy(hexArray, BytesString.substring(startIndex,startIndex+2).c_str());
								long decValue = strtol(String(hexArray).c_str(), NULL, 16);
								CheckSumCalc += decValue;
								Buffer[j * ObjectByteCount + k] = decValue;
							}
						}
						if(CheckSumCalc == CheckSumIn)
						{
							NamedObject.Object = Buffer;
						}
						else
						{
							heap_caps_free(Buffer);
							NamedObject.Object = nullptr;
							++m_FailCount;
							ESP_LOGE("DeSerializeJsonToNamedObject", "WARNING! Deserialize failed: \"Checksum Error\" Value: \"(%i != %i)\"", CheckSumCalc, CheckSumIn);
						}
					}
					else
					{
						++m_FailCount;
						NamedObject.Object = nullptr;
						ESP_LOGE("DeSerializeJsonToNamedObject", "WARNING! Deserialize failed: Byte Count Error.");
					}
				}
				else
				{
					++m_FailCount;
					NamedObject.Object = nullptr;
					ESP_LOGE("DeSerializeJsonToNamedObject", "WARNING! Deserialize failed: \"Missing Tags\" Input: \"%s\"", json.c_str());
				}
			}
			FailPercentage();
		}
		void FailPercentage()
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
		bool AllTagsExist(JSONVar &jsonObject)
		{
			const String tags[] = {m_NameTag, m_CheckSumTag, m_CountTag, m_DataTag, m_DataTypeTag, m_TotalByteCountTag};
			bool result = true;
			for (const String& tag : tags) {
				if (!jsonObject.hasOwnProperty(tag)) {
					ESP_LOGE("AllTagsExist", "Missing Tag: \"%s\"", tag.c_str());
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


#endif