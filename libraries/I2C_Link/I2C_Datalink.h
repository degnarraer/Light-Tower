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

#ifndef I2C_Datalink_H
#define I2C_Datalink_H
#include <Wire.h>
#include <WireSlaveRequest.h>
#include <WireSlave.h>
#include <Helpers.h>

class I2C_Datalink
{
  public:
    I2C_Datalink( uint8_t SDA_Pin, uint8_t SCL_Pin )
                : m_SDA_PIN(SDA_Pin)
                , m_SCL_PIN(SCL_Pin){}
    virtual ~I2C_Datalink(){}
  protected:
    uint8_t m_SDA_PIN;
    uint8_t m_SCL_PIN;
    uint8_t m_I2C_Address;
    uint16_t m_MaxResponseLength;
    uint32_t m_Freq;
    uint8_t m_RequestAttempts;
    uint8_t m_RequestTimeout;
};

class I2C_Datalink_Master: public NamedItem
                         , public I2C_Datalink
{
  public:
    I2C_Datalink_Master( String Title, TwoWire &TwoWire, uint8_t SDA_Pin, uint8_t SCL_Pin )
                       : NamedItem(Title)
                       , I2C_Datalink(SDA_Pin, SCL_Pin)
                       , m_TwoWire(&TwoWire){}
    virtual ~I2C_Datalink_Master(){}

    //Master Functions
	String ReadDataFromSlave(uint8_t SlaveAddress, uint32_t count);
    void WriteDataToSlave(uint8_t SlaveAddress, String Data);
  protected:
    void SetupMaster(uint16_t MaxResponseLength, uint32_t Freq, uint8_t RequestAttempts, uint8_t RequestTimeout);
  private:  
	TwoWire *m_TwoWire = NULL;
};

class I2C_Datalink_Slave: public NamedItem
                        , public I2C_Datalink
{
  public:
    I2C_Datalink_Slave( String Title, TwoWireSlave &TwoWireSlave, uint8_t SDA_Pin, uint8_t SCL_Pin )
                      : NamedItem(Title)
                      , I2C_Datalink(SDA_Pin, SCL_Pin)
                      , m_TwoWireSlave(&TwoWireSlave){}
    virtual ~I2C_Datalink_Slave(){}
    void UpdateI2C();
  protected:
	void SetupSlave( uint8_t My_Address, uint16_t MaxResponseLength, TwoWireSlaveNotifiee *TwoWireSlaveNotifiee );
    TwoWireSlave *m_TwoWireSlave = NULL;
};

class AudioStreamRequester: public NamedItem
						  , public I2C_Datalink_Master
{
	public:
		AudioStreamRequester( String Title
							, TwoWire &TwoWire
							, uint16_t MaxResponseLength
							, uint32_t Freq
							, uint8_t RequestAttempts
							, uint8_t RequestTimeout
							, uint8_t SDA_Pin
							, uint8_t SCL_Pin )
							: NamedItem(Title)
							, I2C_Datalink_Master( Title + "_I2C", TwoWire, SDA_Pin, SCL_Pin )
							{
								SetupMaster(MaxResponseLength, Freq, RequestAttempts, RequestTimeout);
							}
		virtual ~AudioStreamRequester(){}
		void RequestAudioStream(uint32_t count)
		{
			
		}			
};

class AudioStreamSender: public NamedItem
					   , public I2C_Datalink_Slave
					   , public TwoWireSlaveNotifiee
{
	public:
		AudioStreamSender( String Title
						 , TwoWireSlave &TwoWireSlave
						 , uint8_t I2C_Address
						 , uint16_t MaxResponseLength
						 , uint8_t SDA_Pin
						 , uint8_t SCL_Pin )
						 : NamedItem(Title)
						 , I2C_Datalink_Slave( Title + "_I2C", TwoWireSlave, SDA_Pin, SCL_Pin )
						 , m_I2C_Address(I2C_Address)
						 , m_MaxResponseLength(MaxResponseLength){}
		virtual ~AudioStreamSender(){}
		void SetupAudioStreamSender()
		{
			SetupSlave(m_I2C_Address, m_MaxResponseLength, this);
		}
		
		//TwoWireSlaveNotifiee Callbacks
		void RequestEvent();
		void ReceiveEvent(int HowMany);
	private:
		uint16_t m_RequestCount;
		uint8_t m_I2C_Address;
		uint16_t m_MaxResponseLength;
};


#endif