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
#define SERIAL_RX_LENGTH_LIMIT 1000
#include <HardwareSerial.h>
#include "DataSerializer.h"

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
  String m_Startinator = "<SOM>";
  String m_Terminator = "<EOM>";
  
  void EncodeAndTransmitData(String Name, DataType_t DataType, void* Object, size_t Count)
  {
	  String DataToSend = m_Startinator + SerializeDataToJson(Name, DataType, Object, Count) + m_Terminator;
	  ESP_LOGD("Serial_Datalink", "TX: %s", DataToSend.c_str());
	  m_hSerial.print(DataToSend.c_str());
  }
  
  void ProcessTXData(DataItem_t DataItem)
  {
	if(NULL != DataItem.QueueHandle_TX)
	{
		size_t MessageCount = uxQueueMessagesWaiting(DataItem.QueueHandle_TX);
		if(MessageCount > 0)
		{
			byte Buffer[GetSizeOfDataType(DataItem.DataType) * DataItem.Count];
			if ( xQueueReceive(DataItem.QueueHandle_TX, Buffer, 0) == pdTRUE )
			{
				EncodeAndTransmitData(DataItem.Name, DataItem.DataType, Buffer, DataItem.Count);
			}
		}
	}
  }
};

#endif
