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

#ifndef SerialDataLink_H
#define SerialDataLink_H
#define QUEUE_SIZE 10
#define QUEUE_DEBUG false
#define SERIAL_TX_DEBUG false
#define SERIAL_RX_DEBUG false
#define SERIAL_FAIL_DEBUG false

#include <HardwareSerial.h>
#include <Arduino.h>
#include <Helpers.h>
#include "Streaming.h"
#include <ArduinoJson.h>

class DataSerializer: public CommonUtils

{
	public:
		DataSerializer(){}
		virtual ~DataSerializer(){}
		void SetDataSerializerDataItems(DataItem_t& DataItem, size_t DataItemCount)
		{
			m_DataItems = &DataItem;
			m_DataItemsCount = DataItemCount;
		}
		
		String Serialize(String Name, DataType_t DataType, void* Object, size_t Count)
		{
			int32_t CheckSum = 0;
			docOut.clear();
			docOut["Name"] = Name;
			docOut["Count"] = Count;
			docOut["DataType"] = DataTypeStrings[DataType];
			JsonArray data = docOut.createNestedArray("Data");
			docOut["TotalByteCount"] = GetSizeOfDataType(DataType) * Count;
			for(int i = 0; i < docOut["TotalByteCount"]; ++i)
			{
				uint8_t Value = ((uint8_t*)Object)[i];
				CheckSum += Value;
				data.add(Value);
			}
			docOut["CheckSum"] = CheckSum;
			String Result;
			serializeJson(docOut, Result);
			return Result;
		}
		void DeSerialize(String json)
		{
			DeserializationError error = deserializeJson(docIn, json);
			// Test if parsing succeeds.
			if (error)
			{
				if(true == SERIAL_FAIL_DEBUG)Serial << "Deserialize failed: " << error.f_str() << "\n";
				return;
			}
			else
			{
				if(NULL != m_DataItems)
				{
					for(int i = 0; i < m_DataItemsCount; ++i)
					{
						String ItemName = (m_DataItems[i]).Name;
						String DocName = docIn["Name"];
						if(true == ItemName.equals(DocName))
						{
							int CheckSumCalc = 0;
							int CheckSumIn = docIn["CheckSum"];
							int CountIn = docIn["Count"];
							int ByteCountIn = docIn["TotalByteCount"];
							int ByteCountInCalc = CountIn * GetSizeOfDataType((DataType_t)GetDataTypeFromString(docIn["DataType"]));
							int DataByteCount = docIn["Data"].size();
							uint8_t* Buffer = (uint8_t*)malloc(DataByteCount);
							if(ByteCountInCalc == DataByteCount)
							{
								for(int j = 0; j < DataByteCount; ++j)
								{
									CheckSumCalc += (uint8_t)(docIn["Data"][j]);
									Buffer[j] = (uint8_t)(docIn["Data"][j]);
								}
								if(CheckSumCalc == CheckSumIn)
								{
									PushValueToQueue(Buffer, m_DataItems[i].QueueHandle_RX, false, false);
								}
								else
								{
									if(true == SERIAL_FAIL_DEBUG)Serial << "Deserialize failed: Checksum Error\n";
								}
							}
							else
							{
								if(true == SERIAL_FAIL_DEBUG)Serial << "Deserialize failed: Byte Count Error\n";
							}
							delete Buffer;
							return;
						}
					}
				}
			}
		}
	private:
		StaticJsonDocument<3000> docIn;
		StaticJsonDocument<3000> docOut;
		DataItem_t* m_DataItems;
		size_t m_DataItemsCount = 0;
};

class SerialDataLinkCore: public DataSerializer
{
  public:
    SerialDataLinkCore(String Title, HardwareSerial &hSerial);
    virtual ~SerialDataLinkCore();
	
    void GetRXData();
    void ProcessDataTXEventQueue();
	void SetSerialDataLinkDataItems(DataItem_t& DataItem, size_t Count) 
	{ 
		m_DataItems = &DataItem;
		m_DataItemsCount = Count;
		SetDataSerializerDataItems(DataItem, Count);
	}

  private:
  String m_Title;
  DataItem_t* m_DataItems;
  size_t m_DataItemsCount = 0;
  HardwareSerial &m_hSerial;
  String m_InboundStringData = "";
  
  void EncodeAndTransmitData(String Name, DataType_t DataType, void* Object, size_t Count)
  {
	  String DataToSend = Serialize(Name, DataType, Object, Count);
	  if(true == SERIAL_TX_DEBUG) Serial.println(DataToSend);
	  m_hSerial.println(DataToSend);
  }
  
  void ProcessTXData(DataItem_t DataItem)
  {
	if(NULL != DataItem.QueueHandle_TX)
	{
		size_t MessageCount = uxQueueMessagesWaiting(DataItem.QueueHandle_TX);
		if(MessageCount > 0)
		{
			size_t ByteCount = GetSizeOfDataType(DataItem.DataType) * DataItem.Count;
			uint8_t* DataBuffer = (uint8_t*)malloc(ByteCount);
			if ( xQueueReceive(DataItem.QueueHandle_TX, DataBuffer, portMAX_DELAY) != pdTRUE )
			{
				Serial.println("Error Reading Queue");
			}
			else
			{
				memcpy(DataItem.Object, DataBuffer, ByteCount);
				EncodeAndTransmitData(DataItem.Name, DataItem.DataType, DataItem.Object, DataItem.Count);
			}
			delete DataBuffer;
		}
	}
  }
};

#endif
