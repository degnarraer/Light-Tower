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
#include <Serial_Datalink_Core.h>
#include "circle_buf.h"

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
	uint32_t ReadDataFromSlave(uint8_t SlaveAddress, uint8_t *data, uint32_t count);
    void WriteDataToSlave(uint8_t SlaveAddress, uint8_t *data, uint32_t ByteCount);
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
  protected:
    void UpdateI2C();
	void SetupSlave( uint8_t My_Address, uint16_t MaxResponseLength, TwoWireSlaveNotifiee *TwoWireSlaveNotifiee );
    TwoWireSlave *m_TwoWireSlave = NULL;
};

class AudioBuffer: public NamedItem
{
	public:
		AudioBuffer( String Title)
				   : NamedItem(Title){}
		virtual ~AudioBuffer(){}
		size_t GetFrameCount()
		{
			size_t size = 0;
			if(xSemaphoreTake(m_FrameBufferBinarySemaphore, portMAX_DELAY) == pdTRUE)
			{
				ESP_LOGW("AudioBuffer", "Getting Audio Buffer Size");
				size = m_AudioBuffer.size();
				xSemaphoreGive(m_FrameBufferBinarySemaphore);
			}
			else
			{
				ESP_LOGW("AudioBuffer", "Failed to get Semaphore");
			}
			return size;
		}
		void WriteAudioFrame( Frame_t Frame )
		{
			if(xSemaphoreTake(m_FrameBufferBinarySemaphore, portMAX_DELAY) == pdTRUE)
			{
				ESP_LOGW("AudioBuffer", "Writing Audio Buffer");
				m_AudioBuffer.Write((Frame_t&)Frame);
				xSemaphoreGive(m_FrameBufferBinarySemaphore);
			}
			else
			{
				ESP_LOGW("AudioBuffer", "Failed to get Semaphore");
			}
		}
		void ReadAudioFrames(Frame_t *DataBuffer, size_t FrameCount)
		{
			Frame_t Frame;
			if(xSemaphoreTake(m_FrameBufferBinarySemaphore, portMAX_DELAY) == pdTRUE)
			{
				ESP_LOGW("AudioBuffer", "Reading Audio Buffer");
				m_AudioBuffer.Read(DataBuffer, FrameCount);
				xSemaphoreGive(m_FrameBufferBinarySemaphore);
			}
		}
	private:
		bfs::CircleBuf<Frame_t, 2048> m_AudioBuffer;
		SemaphoreHandle_t m_FrameBufferBinarySemaphore = xSemaphoreCreateBinary();		
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
							, m_MaxResponseLength(MaxResponseLength)
							, m_Freq(Freq)
							, m_RequestAttempts(RequestAttempts)
							, m_RequestTimeout(RequestTimeout){}
		virtual ~AudioStreamRequester(){}
		void SetupAudioStreamRequester()
		{
			SetupMaster(m_MaxResponseLength, m_Freq, m_RequestAttempts, m_RequestTimeout);
		}
		uint32_t RequestAudioStream(uint8_t SourceAddress, uint8_t *SoundBufferData, uint32_t ByteCount)
		{
			return ReadDataFromSlave(SourceAddress, SoundBufferData, ByteCount);
		}
	private:
		uint16_t m_MaxResponseLength;
		uint32_t m_Freq;
		uint8_t m_RequestAttempts;
		uint8_t m_RequestTimeout;
};

class AudioStreamSender: public NamedItem
					   , public I2C_Datalink_Slave
					   , public TwoWireSlaveNotifiee
{
	public:
		AudioStreamSender( String Title
						 , AudioBuffer &AudioBuffer
						 , TwoWireSlave &TwoWireSlave
						 , uint8_t I2C_Address
						 , uint16_t MaxResponseLength
						 , uint8_t SDA_Pin
						 , uint8_t SCL_Pin )
						 : NamedItem(Title)
						 , I2C_Datalink_Slave( Title + "_I2C", TwoWireSlave, SDA_Pin, SCL_Pin )
						 , m_AudioBuffer(AudioBuffer)
						 , m_I2C_Address(I2C_Address)
						 , m_MaxResponseLength(MaxResponseLength ){}
		virtual ~AudioStreamSender(){}
		void SetupAudioStreamSender()
		{
			SetupSlave(m_I2C_Address, m_MaxResponseLength, this);
		}
		void UpdateStreamSender()
		{
			ESP_LOGW("AudioBuffer", "Updating I2C");
			UpdateI2C();
		}
		//TwoWireSlaveNotifiee Callbacks
		void ReceiveEvent(int howMany)
		{
		  
		}

		void RequestEvent()
		{
			Frame_t FrameBuffer[30];
			if(30 <= m_AudioBuffer.GetFrameCount())
			{
				m_AudioBuffer.ReadAudioFrames(FrameBuffer, 30);
				for(int i = 0; i < 30; ++i)
				{
					for(int j = 0; j < sizeof(Frame_t); ++j)
					{
						m_TwoWireSlave->write(((uint8_t*)FrameBuffer)[4*i+j]);
					}
				}
			}
		}
	private:
		uint8_t m_I2C_Address;
		uint16_t m_MaxResponseLength;
		AudioBuffer &m_AudioBuffer;
};


#endif