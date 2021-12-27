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

#include <Serial_Datalink_Core.h>

SerialDataLinkCore::SerialDataLinkCore(String Title, HardwareSerial &hSerial): NamedItem(Title)
													, DataSerializer(m_DataItem, m_DataItemCount)
													, m_hSerial(hSerial)
{
}
SerialDataLinkCore::~SerialDataLinkCore()
{

}

void SerialDataLinkCore::Setup()
{
  Serial << GetTitle() << ": Allocating Memory\n";
  DataItemConfig_t* ConfigFile = GetConfig();
  m_DataItemCount = GetConfigCount();
  size_t ConfigBytes = sizeof(DataItem_t) * m_DataItemCount;
  Serial << GetTitle() << ": Allocating " << m_DataItemCount << " DataItem's for a total of " << ConfigBytes << " bytes of Memory\n";
  m_DataItem = new DataItem_t[m_DataItemCount];
  for(int i = 0; i < m_DataItemCount; ++i)
  {
    void* Object;
	size_t bytes = 0;
	
    switch(ConfigFile[i].DataType)
    {
      case DataType_Int16_t:
      {
        bytes = sizeof(int16_t) * ConfigFile[i].Count;
		Object = malloc(bytes);
      }
      break;
      case DataType_Int32_t:
      {
        bytes = sizeof(int32_t) * ConfigFile[i].Count;
		Object = malloc(bytes);
      }
      break;
      case DataType_Uint16_t:
      {
        bytes = sizeof(uint16_t) * ConfigFile[i].Count;
		Object = malloc(bytes);
      }
      break;
      case DataType_Uint32_t:
      {
        bytes = sizeof(uint32_t) * ConfigFile[i].Count;
		Object = malloc(bytes);
      }
      break;
      case DataType_String:
      {
        bytes = sizeof(String) * ConfigFile[i].Count;
		Object = malloc(bytes);
      }
      break;
      case DataType_Float:
      {
        bytes = sizeof(float) * ConfigFile[i].Count;
		Object = malloc(bytes);
      }
      break;
      default:
        Serial << GetTitle() << ": Error, unsupported data type";
      break;
    }
	CreateQueue(m_DataItem[i].QueueHandle_RX, bytes, QUEUE_SIZE, true);
	CreateQueue(m_DataItem[i].QueueHandle_TX, bytes, QUEUE_SIZE, true);
    Serial << GetTitle() << ": Try Configuring DataItem " << i+1 << " of " << m_DataItemCount << "\n"; 
	m_DataItem[i].Name = ConfigFile[i].Name;
    m_DataItem[i].DataType = ConfigFile[i].DataType;
    m_DataItem[i].Count = ConfigFile[i].Count;
    m_DataItem[i].TransceiverConfig = ConfigFile[i].TransceiverConfig;
    m_DataItem[i].Object = Object;
	SetDataItems(m_DataItem, m_DataItemCount);
    Serial << GetTitle() << ": Successfully Configured DataItem " << i+1 << " of " << m_DataItemCount << "\n";
  }  
}

QueueHandle_t SerialDataLinkCore::GetQueueHandleRXForDataItem(String Name)
{
	if(NULL != m_DataItem)
	{
		for(int i = 0; i < m_DataItemCount; ++i)
		{
			if(Name == m_DataItem[i].Name)
			{
				return m_DataItem[i].QueueHandle_RX;
			}
		}
	}
	return NULL;
}

QueueHandle_t SerialDataLinkCore::GetQueueHandleTXForDataItem(String Name)
{
	if(NULL != m_DataItem)
	{
		for(int i = 0; i < m_DataItemCount; ++i)
		{
			if(Name == m_DataItem[i].Name)
			{
				return m_DataItem[i].QueueHandle_TX;
			}
		}
	}
	return NULL;
}
	
size_t SerialDataLinkCore::GetByteCountForDataItem(String Name)
{
	if(NULL != m_DataItem)
	{
		for(int i = 0; i < m_DataItemCount; ++i)
		{
			if(Name == m_DataItem[i].Name)
			{
				return m_DataItem[i].Count;
			}
		}
	}
	return NULL;
}

void SerialDataLinkCore::CheckForNewSerialData()
{
  for(int i = 0; i < m_hSerial.available(); ++i)
  {
	byte ch;
	ch = m_hSerial.read();
	m_InboundStringData += (char)ch;
	if (ch=='\n') 
	{
	  m_InboundStringData.trim();
	  if(true == SERIAL_RX_DEBUG) Serial << m_InboundStringData << "\n";
	  DecodeAndStoreData(m_InboundStringData);
	  m_InboundStringData.clear();
	}
  }
}

void SerialDataLinkCore::ProcessDataTXEventQueue()
{
  if(NULL != m_DataItem)
  {
    for(int i = 0; i < m_DataItemCount; ++i)
    {
      if(NULL != m_DataItem[i].QueueHandle_TX)
      {
        if(uxQueueMessagesWaiting(m_DataItem[i].QueueHandle_TX) > 0)
		{
			if(true == QUEUE_DEBUG) Serial << GetTitle() << " Queue Count : " << uxQueueMessagesWaiting(m_DataItem[i].QueueHandle_TX) << "\n";
			switch(m_DataItem[i].DataType)
			{
				case DataType_Int16_t:
				{
					ProcessTXData<int16_t>(m_DataItem[i]);
				}
				break;
				case DataType_Int32_t:
				{	
					ProcessTXData<int32_t>(m_DataItem[i]);
				}
				break;
				case DataType_Uint16_t:
				{
					ProcessTXData<uint16_t>(m_DataItem[i]);
				}
				break;
				case DataType_Uint32_t:
				{
					ProcessTXData<uint32_t>(m_DataItem[i]);
				}
				break;
				case DataType_String:
					//EncodeAndTransmitData<String>(m_DataItem[i].Name, m_DataItem[i].Object, m_DataItem[i].Count);
				break;
				case DataType_Float:
				{
					ProcessTXData<float>(m_DataItem[i]);
				}
				break;
				default:
				  Serial << "Error, unsupported data type";
				break;
			}
        }
      }
    } 
  }
}
