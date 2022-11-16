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
			return Result.c_str();
		}
		
		void DeSerializeJsonToMatchingDataItem(String json)
		{
			DeserializationError error = deserializeJson(doc, json.c_str());
			Serial << json.c_str() << "\n";
			// Test if parsing succeeds.
			if (error)
			{
				ESP_LOGE("Serial_Datalink", "WARNING! Deserialize failed: %s. For String: %s", error.c_str(), json.c_str());
				return;
			}
			else
			{
				if(NULL != m_DataItems)
				{
					for(int i = 0; i < m_DataItemsCount; ++i)
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
								if(CheckSumCalc == CheckSumIn)
								{
									PushValueToQueue(Buffer, m_DataItems[i].QueueHandle_RX, false, ItemName.c_str(), m_DataItems[i].DataPushHasErrored);
								}
								else
								{
									ESP_LOGE("Serial_Datalink", "WARNING! Deserialize failed: Checksum Error (%i != %i) for String: %s", CheckSumCalc, CheckSumIn, json.c_str());
								}
							}
							else
							{
								ESP_LOGE("Serial_Datalink", "WARNING! Deserialize failed: Byte Count Error for String: %s", json.c_str());
							}
							return;
						}
					}
				}
			}
		}
	private:
		StaticJsonDocument<20000> doc;
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
		String m_Terminator = "<PACKET_END>";
};


#endif