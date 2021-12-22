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
		
		template <typename T>
		String Serialize(String Name, String DataType, void* Object, size_t Count)
		{
			docOut.clear();
			docOut["Name"] = Name;
			docOut["Count"] = Count;
			docOut["DataType"] = DataType;
			JsonArray data = docOut.createNestedArray("data");
			for(int i = 0; i < Count; ++i)
			{
				data.add(((T*)Object)[i]);
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
				Serial << "deserializeJson() failed: ";
				//Serial << error.f_str() << "\n";
				return;
			}
			else
			{
				//Serial << "deserializeJson() Success: ";
				String Name = docIn["Name"];
				int Count = docIn["Count"];
				String DataTypeString = docIn["DataType"];
				DataType_t DataType = GetDataTypeFromString(DataTypeString);
				//Serial << Name << "|" << Count << "|" << DataTypeString << "\n";
			}
		}
	private:
		StaticJsonDocument<1000> docIn;
		StaticJsonDocument<1000> docOut;
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
    void ProcessDataSendEventQueue();
	QueueHandle_t GetQueueHandleForDataItem(String Name);
  private:
  DataItem_t* m_DataItem = NULL;
  size_t m_DataItemCount = 0;
  HardwareSerial &hSerial = Serial2;
  String m_InboundStringData = "";
  
  template <class T>
  void EncodeAndTransmitData(String Name, String DataType, void* Object, size_t Count)
  {
	  String DataToSend = Serialize<T>(Name, DataType, Object, Count);
	  if(true == SERIAL_TX_DEBUG) Serial.println(DataToSend);
	  hSerial.println(DataToSend);
  }
  
  void DecodeAndStoreData(String Data)
  {
	  DeSerialize(Data);
  }
  
  template <class T>
  void ProcessData(DataItem_t DataItem)
  {
	T* DataBuffer = (T*)malloc(sizeof(T) * DataItem.Count);
	if ( xQueueReceive(DataItem.QueueHandle, DataBuffer, portMAX_DELAY) != pdTRUE ){Serial.println("Error Reading Queue");}
	memcpy(DataItem.Object, DataBuffer, sizeof(T) * DataItem.Count);
	EncodeAndTransmitData<T>(DataItem.Name, DataTypeStrings[DataItem.DataType], DataItem.Object, DataItem.Count);
	delete DataBuffer;
  }
};

#endif
