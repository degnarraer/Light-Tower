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

#define I2C_MAX_BYTES 124
#define FRAMES_PER_PACKET 128

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
	size_t ReadDataFromSlave(uint8_t SlaveAddress, unsigned char *data, size_t count);
    void WriteDataToSlave(uint8_t SlaveAddress, unsigned char *data, size_t ByteCount);
  protected:
    void SetupMaster(uint32_t Freq, uint8_t RequestAttempts, uint8_t RequestTimeout);
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
	void SetupSlave( uint8_t My_Address, TwoWireSlaveNotifiee *TwoWireSlaveNotifiee );
    TwoWireSlave *m_TwoWireSlave = NULL;
};

class AudioBuffer: public NamedItem
{
	public:
		AudioBuffer( String Title)
				   : NamedItem(Title)
				   {}
		virtual ~AudioBuffer(){}
		void Initialize()
		{			
			if(0 != pthread_mutex_init(&m_Lock, NULL))
			{
			   ESP_LOGE("AudioBuffer", "Failed to Create Lock");
			}
			ClearAudioBuffer();
		}
		size_t GetFrameCapacity();
		bool ClearAudioBuffer();
		size_t GetFrameCount();
		size_t GetAvailableCount();
		size_t WriteAudioFrames( Frame_t *FrameBuffer, size_t FrameCount );
		bool WriteAudioFrame( Frame_t &FrameBuffer );
		size_t ReadAudioFrames(Frame_t *FrameBuffer, size_t FrameCount);
		bfs::optional<Frame_t> ReadAudioFrame();
	private:
		bfs::CircleBuf<Frame_t, 2048> m_CircularAudioBuffer;
		pthread_mutex_t m_Lock;
};

class AudioStreamRequester: public NamedItem
						  , public I2C_Datalink_Master
{
	public:
		AudioStreamRequester( String Title
							, AudioBuffer &AudioBuffer
							, TwoWire &TwoWire
							, uint32_t Freq
							, uint8_t RequestAttempts
							, uint8_t RequestTimeout
							, uint8_t SDA_Pin
							, uint8_t SCL_Pin )
							: NamedItem(Title)
							, m_AudioBuffer(AudioBuffer)
							, I2C_Datalink_Master( Title + "_I2C", TwoWire, SDA_Pin, SCL_Pin )
							, m_Freq(Freq)
							, m_RequestAttempts(RequestAttempts)
							, m_RequestTimeout(RequestTimeout){}
		virtual ~AudioStreamRequester(){}
		void SetupAudioStreamRequester();
		size_t BufferMoreAudio(uint8_t SourceAddress);
		size_t GetFrameCount();
		size_t GetAudioFrames(Frame_t *FrameBuffer, size_t FrameCount);
	private:
		uint32_t m_Freq;
		uint8_t m_RequestAttempts;
		uint8_t m_RequestTimeout;
		AudioBuffer &m_AudioBuffer;
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
						 , uint8_t SDA_Pin
						 , uint8_t SCL_Pin )
						 : NamedItem(Title)
						 , m_AudioBuffer(AudioBuffer)
						 , I2C_Datalink_Slave( Title + "_I2C", TwoWireSlave, SDA_Pin, SCL_Pin )
						 , m_I2C_Address(I2C_Address){}
		virtual ~AudioStreamSender(){}
		void SetupAudioStreamSender();
		void UpdateStreamSender();
		
		//TwoWireSlaveNotifiee Callbacks
		void ReceiveEvent(int howMany);
		void RequestEvent();
	private:
		uint8_t m_I2C_Address;
		AudioBuffer &m_AudioBuffer;
};


#endif