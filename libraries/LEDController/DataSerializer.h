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
#include <ArduinoJson.h>
#include <Helpers.h>
#include "Streaming.h"

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
			int32_t CheckSum = 0;
			size_t ObjectByteCount = GetSizeOfDataType(DataType);
			
			doc.clear();
			doc[m_NameTag] = Name.c_str();
			doc[m_CountTag] = Count;
			doc[m_DataTypeTag] = DataTypeStrings[DataType];
			JsonArray data = doc.createNestedArray(m_DataTag);
			doc[m_TotalByteCountTag] = ObjectByteCount * Count;
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
				data.add(BytesString);
			}
			doc[m_CheckSumTag] = CheckSum;
			String Result;
			serializeJson(doc, Result);
			return Result;
		}
		
		void DeSerializeJsonToMatchingDataItem(String json, bool DebugMessage = false)
		{
			m_CurrentTime = millis();
			++m_TotalCount;
			DeserializationError error = deserializeJson(doc, json.c_str());
			// Test if parsing succeeds.
			if (error)
			{
				++m_FailCount;
				ESP_LOGD("Serial_Datalink", "WARNING! Deserialize failed: %s.", error.c_str());
				return;
			}
			else
			{
				if(true == DebugMessage)
				{
					Serial << "JSON String: " << json.c_str() << "\n";
				}
				if(NULL != m_DataItems)
				{
					for(int i = 0; i < m_DataItemsCount; ++i)
					{
						if(AllTagsExist())
						{
							const String ItemName = (m_DataItems[i]).Name;
							const String DocName = doc[m_NameTag];
							if(true == ItemName.equals(DocName))
							{
								size_t CheckSumCalc = 0;
								size_t CheckSumIn = doc[m_CheckSumTag];
								size_t CountIn = doc[m_CountTag];
								size_t ByteCountIn = doc[m_TotalByteCountTag];
								size_t ActualDataCount = doc[m_DataTag].size();
								DataType_t DataType = GetDataTypeFromString(doc[m_DataTypeTag]);
								size_t ObjectByteCount = GetSizeOfDataType(DataType);
								uint8_t Buffer[ByteCountIn];							
								if( ActualDataCount == CountIn && ByteCountIn == ActualDataCount * ObjectByteCount )
								{
									for(int j = 0; j < CountIn; ++j)
									{
										String BytesString = doc[m_DataTag][j];
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
								}
								else
								{
									++m_FailCount;
									ESP_LOGD("Serial_Datalink", "WARNING! Deserialize failed: Byte Count Error.");
								}
								
								if(CheckSumCalc == CheckSumIn)
								{
									if(NULL != m_DataItems[i].QueueHandle_RX)
									{
										PushValueToQueue(Buffer, m_DataItems[i].QueueHandle_RX, false, ItemName.c_str(), m_DataItems[i].DataPushHasErrored);
									}
									else
									{
										++m_FailCount;
										ESP_LOGD("Serial_Datalink", "WARNING! Deserialize failed: No matching DataItem RX Handle");
									}
								}
								else
								{
									++m_FailCount;
									ESP_LOGD("Serial_Datalink", "WARNING! Deserialize failed: Checksum Error (%i != %i)", CheckSumCalc, CheckSumIn);
								}
								FailPercentage();
								return;
							}
						}
						else
						{
							ESP_LOGD("Serial_Datalink", "WARNING! Deserialize failed: Missing Tags");
						}
					}
					
				}
			}
			FailPercentage();
		}
		void FailPercentage()
		{
			if(m_CurrentTime - m_FailCountTimer >= m_FailCountDuration)
			{
				m_FailCountTimer = m_CurrentTime;
				ESP_LOGE("Serial_Datalink", "Deserialization Failure Percentage: %f", 100.0 * (float)m_FailCount / (float)m_TotalCount);
				m_FailCount = 0;
				m_TotalCount = 0;
			}	
		}
		bool AllTagsExist()
		{
			JsonVariant FoundObject1 = doc[m_NameTag];
			JsonVariant FoundObject2 = doc[m_CheckSumTag];
			JsonVariant FoundObject3 = doc[m_CountTag];
			JsonVariant FoundObject4 = doc[m_DataTag];
			JsonVariant FoundObject5 = doc[m_DataTypeTag];
			JsonVariant FoundObject6 = doc[m_TotalByteCountTag];
			bool result = !(FoundObject1.isNull()) &&
						  !(FoundObject2.isNull()) &&
						  !(FoundObject3.isNull()) &&
						  !(FoundObject4.isNull()) &&
						  !(FoundObject5.isNull()) &&
						  !(FoundObject6.isNull());
			return result;
		}
	private:
		size_t m_TotalCount = 0;
		size_t m_FailCount = 0;
		uint64_t m_CurrentTime = 0;
		uint64_t m_FailCountTimer = 0;
		uint64_t m_FailCountDuration = 5000;
		
		StaticJsonDocument<10000> doc;
		DataItem_t* m_DataItems;
		size_t m_DataItemsCount = 0;
		//Tags
		String m_Startinator = "<PACKET_START>";
		String m_NameTag = "N";
		String m_CheckSumTag = "S";
		String m_CountTag = "C";
		String m_DataTag = "D";
		String m_DataTypeTag = "T";
		String m_TotalByteCountTag = "B";
		String m_TotalStringLengthTag = "L";
		String m_Terminator = "<PACKET_END>";
};


#endif