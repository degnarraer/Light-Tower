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

void I2C_Datalink_Master::SetupMaster( uint16_t MaxResponseLength, uint32_t Freq, uint8_t RequestAttempts, uint8_t RequestTimeout )
{
  m_MaxResponseLength = MaxResponseLength;
  m_Freq = Freq;
  m_RequestAttempts = RequestAttempts;
  m_RequestTimeout = RequestTimeout;
  if(true == m_TwoWire->begin(m_SDA_PIN, m_SCL_PIN))
  {
    ESP_LOGI("I2C_Datalink", "I2C Master Device Named \"%s\" joined I2C bus", GetTitle().c_str());    
  }
  else
  {
    ESP_LOGE("I2C_Datalink", "I2C Master Device Named \"%s\" Setup Failed", GetTitle().c_str());
  }
}

String I2C_Datalink_Master::ReadDataFromSlave(uint8_t SlaveAddress, uint32_t count)
{
	WireSlaveRequest slaveReq(*m_TwoWire, SlaveAddress, count);
	slaveReq.setRetryDelay(m_RequestTimeout);
	slaveReq.setAttempts(m_RequestAttempts);
	String Result;
	if (true == slaveReq.request()) 
	{
		uint32_t available = slaveReq.available();
		Serial << available << "\n";
		while( 0 < available ) 
		{
			char c = (char)slaveReq.read();
			Result += c;
			Serial << c;
			available = slaveReq.available();
		}
	}
	else 
	{
		ESP_LOGE("I2C_Datalink", "I2C Master Device Named \"%s\" Read Data Request Error: %s", GetTitle().c_str(), slaveReq.lastStatusToString().c_str());
	}
	return Result;
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

void I2C_Datalink_Slave::SetupSlave( uint8_t My_Address, uint16_t MaxResponseLength, TwoWireSlaveNotifiee *TwoWireSlaveNotifiee )
{
  m_MaxResponseLength = MaxResponseLength;
  m_I2C_Address = My_Address;
  m_TwoWireSlave->RegisterForNotification(TwoWireSlaveNotifiee);
  ESP_LOGW("I2C_Datalink", "I2C Slave Device Named \"%s\" Registered for Notifications", GetTitle().c_str());
  if(true == m_TwoWireSlave->begin(m_SDA_PIN, m_SCL_PIN, m_I2C_Address))
  {
    ESP_LOGW("I2C_Datalink", "I2C Slave Device Named \"%s\" joined I2C bus with addr #%d", GetTitle().c_str(), m_I2C_Address);
  }
  else
  {
    ESP_LOGE("I2C_Datalink", "I2C Slave Device Named \"%s\" Setup Failed!", GetTitle().c_str());
  }
}

void I2C_Datalink_Slave::UpdateI2C()
{
	if(NULL != m_TwoWireSlave)
	{
		m_TwoWireSlave->update();
	}
}


//Callbacks
void AudioStreamSender::ReceiveEvent(int howMany)
{
  String Result;
  while (0 < m_TwoWireSlave->available())
  {
      char c = m_TwoWireSlave->read();
	  Result += c;
  }
  m_RequestCount = Result.toInt();
  Serial << "Receive Event: " << m_RequestCount << "\n";
}

void AudioStreamSender::RequestEvent()
{
  static int y = 0;
  String result = "A B C D E F G H I J K L M N O P Q R S T U V W X Y Z " + String(++y) + ".\n";
  m_TwoWireSlave->print(result.c_str());
  Serial << "Sent Data: " << result;
}