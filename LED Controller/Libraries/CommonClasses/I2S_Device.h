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

#ifndef I2S_Device_H
#define I2S_Device_H 

//DEBUGGING
#define DATA_RX_DEBUG false
#define DATA_TX_DEBUG false
#define QUEUE_DEBUG false
#define QUEUE_INDEPTH_DEBUG false

#include <Arduino.h>
#include <DataTypes.h>
#include <Helpers.h>
#include "driver/i2s.h"
#include "Streaming.h"

class I2S_Device_Callback
{
	public:
		I2S_Device_Callback(){}
		virtual ~I2S_Device_Callback(){}
				//Callbacks called by this class
		virtual void I2SDataReceived(String DeviceTitle, uint8_t *data, uint32_t length) = 0;
};

class I2S_Device: public NamedItem
				, public CommonUtils
				, public QueueController
{
  public:
    I2S_Device( String Title
              , i2s_port_t i2S_PORT
              , i2s_mode_t Mode
              , int SampleRate
              , i2s_bits_per_sample_t BitsPerSample
              , i2s_channel_fmt_t i2s_Channel_Fmt
              , i2s_comm_format_t CommFormat
              , i2s_channel_t i2s_channel
			        , bool Use_APLL
              , size_t BufferCount
              , size_t BufferSize
              , int SerialClockPin
              , int WordSelectPin
              , int SerialDataInPin
              , int SerialDataOutPin );
    virtual ~I2S_Device();
    void SetCallback(I2S_Device_Callback* callee){ m_Callee = callee; }
    void StartDevice();
    void StopDevice();

    size_t WriteSoundBufferData(uint8_t *SoundBufferData, size_t ByteCount);
    size_t ReadSoundBufferData(uint8_t *SoundBufferData, size_t ByteCount);
	
    void Setup();
    void ProcessEventQueue();
	
  private:
	  I2S_Device_Callback* m_Callee = NULL;
    DataItemConfig_t* m_ItemConfig = NULL;
    const i2s_port_t m_I2S_PORT;
    const int m_SampleRate;
    const i2s_mode_t m_i2s_Mode;
    const i2s_bits_per_sample_t m_BitsPerSample;
    const i2s_comm_format_t m_CommFormat;
    const i2s_channel_fmt_t m_Channel_Fmt;
    const i2s_channel_t m_i2s_channel;
	  const bool m_Use_APLL;
    const size_t m_BufferCount;
    const size_t m_BufferSize;
    const int m_SerialClockPin;
    const int m_WordSelectPin;
    const int m_SerialDataInPin;
    const int m_SerialDataOutPin;

	  bool m_Is_Running = false;
    QueueHandle_t m_i2s_event_queue = NULL;
    size_t m_SampleCount;
    size_t m_ChannelSampleCount;
    size_t m_BytesPerSample;
    size_t m_TotalBytesToRead;
    size_t m_ChannelBytesToRead;

    size_t ReadSamples();
    size_t WriteSamples(uint8_t *samples, size_t ByteCount);
    void InstallDevice();
    void UninstallDevice();
};

#endif
