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
}
SerialDataLinkCore::~SerialDataLinkCore()
{
}

void SerialDataLinkCore::GetRXData()
{
  for(int i = 0; i < m_hSerial.available(); ++i)
  {
	byte ch;
	ch = m_hSerial.read();
	m_InboundStringData += (char)ch;
	if ( m_InboundStringData.substring(m_InboundStringData.length() - 2) == "\r\n" ) 
	{
	  m_InboundStringData.trim();
	  if(true == SERIAL_RX_DEBUG) Serial << m_InboundStringData << "\n";
	  DeSerialize(m_InboundStringData);
	  m_InboundStringData.clear();
	  break; //Only process 1 message at a time
	}
	if(m_InboundStringData.length() > SERIAL_RX_LENGTH_LIMIT)
	{
		//Serial << "Warning! Serial Port RX Overflow\n";
		m_InboundStringData.clear();
	}
  }
}

void SerialDataLinkCore::ProcessDataTXEventQueue()
{
  if(NULL != m_DataItems)
  {
    for(int i = 0; i < m_DataItemsCount; ++i)
    {
      if(NULL != m_DataItems[i].QueueHandle_TX)
      {
        if(uxQueueMessagesWaiting(m_DataItems[i].QueueHandle_TX) > 0)
		{
			if(true == QUEUE_DEBUG) Serial << m_Title << " Queue Count : " << uxQueueMessagesWaiting(m_DataItems[i].QueueHandle_TX) << "\n";
			ProcessTXData(m_DataItems[i]);
        }
      }
    } 
  }
}
