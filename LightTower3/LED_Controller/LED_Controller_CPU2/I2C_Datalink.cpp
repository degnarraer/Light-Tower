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
#include "I2C_Datalink.h"


//Callbacks
void I2C_Datalink::ReceiveEvent(int howMany)
{
}

void I2C_Datalink::RequestEvent()
{
  static byte y = 0;
  WireSlave.print("y is ");
  WireSlave.write(y++);
}


//Master Functions
void I2C_Datalink::SetupAsMaster( uint16_t MaxResponseLength, uint32_t Freq )
{
  m_MaxResponseLength = MaxResponseLength;
  m_Freq = Freq;
  if(true == m_TwoWire->begin(m_SDA_PIN, m_SCL_PIN))
  {
    WireSlave.onReceive(ReceiveEvent);
    WireSlave.onRequest(RequestEvent);
    ESP_LOGI("I2C_Datalink", "I2C Master Device Named \"%s\" joined I2C bus", GetTitle().c_str());    
  }
  else
  {
    ESP_LOGE("I2C_Datalink", "I2C Master Device Named \"%s\" Setup Failed", GetTitle().c_str());
  }
}

char I2C_Datalink::ReadDataFromSlave(uint8_t SlaveAddress)
{
  WireSlaveRequest slaveReq(*m_TwoWire, SlaveAddress, m_MaxResponseLength);
  slaveReq.setRetryDelay(5);
  if (true == slaveReq.request()) 
  {
    while( 0 < slaveReq.available() ) 
    {
      char c = (char)slaveReq.read();
      Serial << c;
    }   
  }
  else 
  {
    ESP_LOGE("I2C_Datalink", "I2C Master Device Named \"%s\" Read Data Request Error: %s", GetTitle().c_str(), slaveReq.lastStatusToString().c_str());
  } 
}

//Slave Functions
void I2C_Datalink::SetupAsSlave( uint8_t My_Address, uint16_t MaxResponseLength )
{
  m_MaxResponseLength = MaxResponseLength;
  if(true == WireSlave.begin(m_SDA_PIN, m_SCL_PIN, My_Address))
  {
    WireSlave.onReceive(ReceiveEvent);
    WireSlave.onRequest(RequestEvent);
    ESP_LOGI("I2C_Datalink", "I2C Slave Device Named \"%s\" joined I2C bus with addr #%d", GetTitle().c_str(), My_Address);
  }
  else
  {
    ESP_LOGE("I2C_Datalink", "I2C Slave Device Named \"%s\" Setup Failed!", GetTitle().c_str());
  }
}

void I2C_Datalink::UpdateI2C()
{
  WireSlave.update();
}
