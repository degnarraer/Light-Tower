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
#define SERIAL_RX_DEBUG false

#include <HardwareSerial.h>
#include <Arduino.h>
#include <Helpers.h>
#include "Streaming.h"
#include <ArduinoJson.h>

class DataSerializer: public CommonUtils
{
	public:
		DataSerializer(DataItem_t* DataItem, size_t& DataItemCount): m_DataItem(DataItem)
																   , m_DataItemCount(DataItemCount){}
		virtual ~DataSerializer(){}
		void SetDataItems(DataItem_t* DataItem, size_t& DataItemCount)
		{
			m_DataItem = DataItem;
			m_DataItemCount = DataItemCount;
		}
		
		String Serialize(String Name, String DataType, void* Object, size_t ByteCount)
		{
			docOut.clear();
			docOut["Name"] = Name;
			docOut["Count"] = ByteCount;
			docOut["DataType"] = DataType;
			JsonArray data = docOut.createNestedArray("data");
			for(int i = 0; i < ByteCount; ++i)
			{
				char Out[2];
				uint8_t Value = ((uint8_t*)Object)[i];
				sprintf(Out, "%02x", Value);
				data.add(String(Out));
			}
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
				Serial << "deserializeJson() failed\n";
				//Serial << error.f_str() << "\n";
				return;
			}
			else
			{
				if(NULL != m_DataItem)
				{
					for(int i = 0; i < m_DataItemCount; ++i)
					{
						String ItemName = (m_DataItem[i]).Name;
						String DocName = docIn["Name"];
						if(true == ItemName.equals(DocName))
						{
							Serial << "Received: " << m_DataItem[i].Name << "\n";
						}
					}
				}
			}
		}
	private:
		StaticJsonDocument<1000> docIn;
		StaticJsonDocument<1000> docOut;
		DataItem_t* m_DataItem = NULL;
		size_t& m_DataItemCount;
};

class SerialDataLinkCore: public NamedItem
						, public DataSerializer
{
  public:
    SerialDataLinkCore(String Title);
    virtual ~SerialDataLinkCore();
	virtual DataItemConfig_t* GetConfig() = 0;
	virtual size_t GetConfigCount() = 0;
	
    void Setup();
    void CheckForNewSerialData();
    void ProcessDataTXEventQueue();
	QueueHandle_t GetQueueHandleRXForDataItem(String Name);
	QueueHandle_t GetQueueHandleTXForDataItem(String Name);
  private:
  DataItem_t* m_DataItem = NULL;
  size_t m_DataItemCount = 0;
  HardwareSerial &hSerial = Serial2;
  String m_InboundStringData = "";
  
  void EncodeAndTransmitData(String Name, String DataType, void* Object, size_t ByteCount)
  {
	  String DataToSend = Serialize(Name, DataType, Object, ByteCount);
	  if(true == SERIAL_TX_DEBUG) Serial.println(DataToSend);
	  hSerial.println(DataToSend);
  }
  
  void DecodeAndStoreData(String Data)
  {
	  DeSerialize(Data);
  }
  
  template <class T>
  void ProcessTXData(DataItem_t DataItem)
  {
	  size_t ByteCount = sizeof(T) * DataItem.Count;
	T* DataBuffer = (T*)malloc(ByteCount);
	if ( xQueueReceive(DataItem.QueueHandle_TX, DataBuffer, portMAX_DELAY) != pdTRUE ){Serial.println("Error Reading Queue");}
	memcpy(DataItem.Object, DataBuffer, ByteCount);
	EncodeAndTransmitData(DataItem.Name, DataTypeStrings[DataItem.DataType], DataItem.Object, ByteCount);
	delete DataBuffer;
  }
};

#endif
