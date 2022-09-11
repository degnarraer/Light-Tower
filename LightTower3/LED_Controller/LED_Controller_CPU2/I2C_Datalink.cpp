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

void I2C_Datalink_Master::SetupMaster( uint16_t MaxResponseLength, uint32_t Freq )
{
  m_MaxResponseLength = MaxResponseLength;
  m_Freq = Freq;
  if(true == m_TwoWire->begin(m_SDA_PIN, m_SCL_PIN))
  {
    ESP_LOGI("I2C_Datalink", "I2C Master Device Named \"%s\" joined I2C bus", GetTitle().c_str());    
  }
  else
  {
    ESP_LOGE("I2C_Datalink", "I2C Master Device Named \"%s\" Setup Failed", GetTitle().c_str());
  }
}

void I2C_Datalink_Master::ReadDataFromSlave(uint8_t SlaveAddress, uint32_t count)
{
  WireSlaveRequest slaveReq(*m_TwoWire, SlaveAddress, count);
  slaveReq.setRetryDelay(3);
  slaveReq.setAttempts(3);
  if (true == slaveReq.request()) 
  {
    Serial << "Received Data: ";
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
void I2C_Datalink_Master::WriteDataToSlave(uint8_t SlaveAddress, String Data)
{
  Serial << "Writing: " << Data.c_str() << "\n";
  WirePacker packer;
  packer.write(Data.c_str());
  packer.end();
  m_TwoWire->beginTransmission(SlaveAddress);
  while (packer.available())
  {
    m_TwoWire->write(packer.read());
  }
  m_TwoWire->endTransmission();
}

void I2C_Datalink_Slave::SetupSlave( uint8_t My_Address, uint16_t MaxResponseLength )
{
  m_MaxResponseLength = MaxResponseLength;
  m_TwoWireSlave->RegisterForNotification(this);
  ESP_LOGW("I2C_Datalink", "I2C Slave Device Named \"%s\" Registered for Notifications", GetTitle().c_str());
  if(true == m_TwoWireSlave->begin(m_SDA_PIN, m_SCL_PIN, My_Address))
  {
    ESP_LOGW("I2C_Datalink", "I2C Slave Device Named \"%s\" joined I2C bus with addr #%d", GetTitle().c_str(), My_Address);
  }
  else
  {
    ESP_LOGE("I2C_Datalink", "I2C Slave Device Named \"%s\" Setup Failed!", GetTitle().c_str());
  }
}

void I2C_Datalink_Slave::UpdateI2C()
{
  m_TwoWireSlave->update();
}

//Callbacks
void I2C_Datalink_Slave::ReceiveEvent(int howMany)
{
  Serial.println("Receive Event");
  while (0 < m_TwoWireSlave->available())
  {
      char c = m_TwoWireSlave->read();
      Serial.print(c);
  }
}

void I2C_Datalink_Slave::RequestEvent()
{
  static int y = 0;
  String result = "The value for y is " + String(++y) + ".\n";
  m_TwoWireSlave->print(result);
  Serial << "Sent Data: " << result;
}
