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

SerialDataLinkCore::SerialDataLinkCore(String Title, HardwareSerial &hSerial): m_Title(Title)
																			 , m_hSerial(hSerial)
																			 , DataSerializer()
{
	//ESP_LOGV("Function Debug", "%s, ", __func__);
}
SerialDataLinkCore::~SerialDataLinkCore()
{
	//ESP_LOGV("Function Debug", "%s, ", __func__);
}

void SerialDataLinkCore::ProcessDataRXEventQueue()
{
  //ESP_LOGV("Function Debug", "%s, ", __func__);
  int32_t ByteCount = m_hSerial.available();
  for(int i = 0; i < ByteCount; ++i)
  {
	m_InboundStringData += (char)m_hSerial.read();
	if ( m_InboundStringData.length() >= m_Startinator.length() )
	{	
		if( m_Terminator.equals(m_InboundStringData.substring(0, m_Terminator.length())))
		{
			ESP_LOGW("Serial_Datalink", "WARNING! Serial Port EOM without SOM: %s", m_InboundStringData.c_str());
			m_InboundStringData.clear();
		}
		else if( m_Startinator.equals(m_InboundStringData.substring(0, m_Startinator.length())))
		{
			if ( m_InboundStringData.length() > m_Startinator.length() && m_Startinator.equals(m_InboundStringData.substring(m_InboundStringData.length() - m_Startinator.length())) ) 
			{
				ESP_LOGW("Serial_Datalink", "WARNING! Serial Port SOM without EOM: %s", m_InboundStringData.c_str());
				m_InboundStringData.clear();
				m_InboundStringData += m_Startinator;
			}
			else if ( m_Terminator.equals(m_InboundStringData.substring(m_InboundStringData.length() - m_Terminator.length())) ) 
			{
			  m_InboundStringData.trim();
			  m_InboundStringData = m_InboundStringData.substring(m_Startinator.length(), m_InboundStringData.length() - m_Terminator.length());
			  ESP_LOGD("Serial_Datalink", "RX: %s", m_InboundStringData.c_str());
			  DeSerialize(m_InboundStringData);
			  m_InboundStringData.clear();
			  break; //Only process 1 message at a time
			}
			if(m_InboundStringData.length() > SERIAL_RX_LENGTH_LIMIT)
			{
				ESP_LOGW("Serial_Datalink", "WARNING! Serial Port RX Overflow: %s", m_InboundStringData.c_str());
				m_InboundStringData.clear();
			}
		}
		else
		{
			m_InboundStringData.clear();
		}
	}
  }
}

void SerialDataLinkCore::ProcessDataTXEventQueue()
{
  //ESP_LOGV("Function Debug", "%s, ", __func__);
  if(NULL != m_DataItems)
  {
    for(int i = 0; i < m_DataItemsCount; ++i)
    {
      if(NULL != m_DataItems[i].QueueHandle_TX)
      {
        if(uxQueueMessagesWaiting(m_DataItems[i].QueueHandle_TX) > 0)
		{
			ESP_LOGV("Serial_Datalink", "%s Queue Count : %i", m_Title, uxQueueMessagesWaiting(m_DataItems[i].QueueHandle_TX));
			ProcessTXData(m_DataItems[i]);
        }
      }
    } 
  }
}
