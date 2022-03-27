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
#define SERIAL_TX_DEBUG true
#define SERIAL_RX_DEBUG true
#define SERIAL_FAIL_DEBUG true
#define SERIAL_RX_LENGTH_LIMIT 1000
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
			docOut[m_NameTag] = Name;
			docOut[m_CountTag] = Count;
			docOut[m_DataTypeTag] = DataTypeStrings[DataType];
			JsonArray data = docOut.createNestedArray(m_DataTag);
			docOut[m_TotalByteCountTag] = GetSizeOfDataType(DataType) * Count;
			for(int i = 0; i < docOut[m_TotalByteCountTag]; ++i)
			{
				uint8_t Value = ((uint8_t*)Object)[i];
				CheckSum += Value;
				data.add(Value);
			}
			docOut[m_CheckSumTag] = CheckSum;
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
				ESP_LOGW("Serial_Datalink", "WARNING! Deserialize failed: %s", error.f_str());
				return;
			}
			else
			{
				if(NULL != m_DataItems)
				{
					for(int i = 0; i < m_DataItemsCount; ++i)
					{
						const String ItemName = (m_DataItems[i]).Name;
						const String DocName = docIn[m_NameTag];
						if(true == ItemName.equals(DocName))
						{
							int CheckSumCalc = 0;
							int CheckSumIn = docIn[m_CheckSumTag];
							int CountIn = docIn[m_CountTag];
							int ByteCountIn = docIn[m_TotalByteCountTag];
							int ByteCountInCalc = CountIn * GetSizeOfDataType((DataType_t)GetDataTypeFromString(docIn[m_DataTypeTag]));
							int DataByteCount = docIn[m_DataTag].size();
							uint8_t* Buffer = (uint8_t*)malloc(DataByteCount);
							if(ByteCountInCalc == DataByteCount)
							{
								for(int j = 0; j < DataByteCount; ++j)
								{
									CheckSumCalc += (uint8_t)(docIn[m_DataTag][j]);
									Buffer[j] = (uint8_t)(docIn[m_DataTag][j]);
								}
								if(CheckSumCalc == CheckSumIn)
								{
									PushValueToQueue(Buffer, m_DataItems[i].QueueHandle_RX, false, ItemName.c_str(), m_DataItems[i].DataPushHasErrored);
								}
								else
								{
									ESP_LOGW("Serial_Datalink", "WARNING! Deserialize failed: Checksum Error.");
								}
							}
							else
							{
								ESP_LOGW("Serial_Datalink", "WARNING! Deserialize failed: Byte Count Error.");
							}
							delete Buffer;
							return;
						}
					}
				}
			}
		}
	private:
		StaticJsonDocument<5000> docIn;
		StaticJsonDocument<5000> docOut;
		DataItem_t* m_DataItems;
		size_t m_DataItemsCount = 0;
		
		//Tags
		String m_NameTag = "N";
		String m_CheckSumTag = "S";
		String m_CountTag = "C";
		String m_DataTag = "D";
		String m_DataTypeTag = "T";
		String m_TotalByteCountTag = "B";
};

class SerialDataLinkCore: public DataSerializer
{
  public:
    SerialDataLinkCore(String Title, HardwareSerial &hSerial);
    virtual ~SerialDataLinkCore();
	
    void ProcessDataRXEventQueue();
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
  String m_Terminator = "\r\n";
  
  void EncodeAndTransmitData(String Name, DataType_t DataType, void* Object, size_t Count)
  {
	  String DataToSend = Serialize(Name, DataType, Object, Count);
	  ESP_LOGV("Serial_Datalink", "Encoded Data: %d", DataToSend);
	  m_hSerial.print(DataToSend);
	  m_hSerial.print(m_Terminator);
  }
  
  void ProcessTXData(DataItem_t DataItem)
  {
	if(NULL != DataItem.QueueHandle_TX)
	{
		size_t MessageCount = uxQueueMessagesWaiting(DataItem.QueueHandle_TX);
		if(MessageCount > 0)
		{
			size_t ByteCount = GetSizeOfDataType(DataItem.DataType) * DataItem.Count;
			if ( xQueueReceive(DataItem.QueueHandle_TX, DataItem.DataBuffer, 0) == pdTRUE )
			{
				EncodeAndTransmitData(DataItem.Name, DataItem.DataType, DataItem.DataBuffer, DataItem.Count);
			}
		}
	}
  }
};

#endif
