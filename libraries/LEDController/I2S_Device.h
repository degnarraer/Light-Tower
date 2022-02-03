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
		virtual void DataBufferModifyRX(String DeviceTitle, uint8_t* DataBuffer, size_t Count) = 0;
		virtual void RightChannelDataBufferModifyRX(String DeviceTitle, uint8_t* DataBuffer, size_t Count) = 0;
		virtual void LeftChannelDataBufferModifyRX(String DeviceTitle, uint8_t* DataBuffer, size_t Count) = 0;
};

class I2S_Device: public NamedItem
				, public CommonUtils
                , public QueueManager
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
              , int BufferCount
              , int BufferSize
			  , size_t OutputQueueCount
              , int SerialClockPin
              , int WordSelectPin
              , int SerialDataInPin
              , int SerialDataOutPin );
    virtual ~I2S_Device();
    void ResgisterForDataBufferRXCallback(I2S_Device_Callback* callee){ m_Callee = callee; }
    void StartDevice();
    void StopDevice();
	size_t GetBytesPerSample() { return m_BytesPerSample; }

	int32_t GetDataBufferValue(uint8_t* DataBuffer, size_t index);
	void SetDataBufferValue(uint8_t* DataBuffer, size_t index, int32_t value);

    void SetSoundBufferData(uint8_t *SoundBufferData, size_t ByteCount);
    size_t GetSampleCount() { return m_SampleCount; }
    size_t GetChannelSampleCount() { return m_ChannelSampleCount; }
    size_t GetBytesToRead() {return m_TotalBytesToRead; }
    size_t GetChannelBytesToRead() {return m_ChannelBytesToRead; }
    int GetSampleRate() { return m_SampleRate; }
    void Setup();
    void ProcessEventQueue();
	
    //QueueManager Interface
    DataItemConfig_t* GetDataItemConfig() { return m_ItemConfig; }
    size_t GetDataItemConfigCount() { return m_ConfigCount; }
  private:
	I2S_Device_Callback* m_Callee = NULL;
    size_t m_ConfigCount = 0;
    DataItemConfig_t* m_ItemConfig = NULL;
	uint8_t* m_SoundBufferData;
	uint8_t* m_RightChannel_SoundBufferData;
	uint8_t* m_LeftChannel_SoundBufferData;
    size_t m_SampleCount;
    size_t m_ChannelSampleCount;
    size_t m_BytesPerSample;
    size_t m_TotalBytesToRead;
    size_t m_ChannelBytesToRead;
    const int m_SampleRate;
    const i2s_mode_t m_i2s_Mode;
    const i2s_bits_per_sample_t m_BitsPerSample;
    const i2s_comm_format_t m_CommFormat;
    const i2s_channel_fmt_t m_Channel_Fmt;
    const i2s_channel_t m_i2s_channel;
	const bool m_Use_APLL;
    const int m_BufferCount;
    const int m_BufferSize;
    size_t m_OutputQueueCount;
    const int m_SerialClockPin;
    const int m_WordSelectPin;
    const int m_SerialDataInPin;
    const int m_SerialDataOutPin;
	bool m_Is_Running = false;
    const i2s_port_t m_I2S_PORT;
    QueueHandle_t m_i2s_event_queue = NULL;

    int ReadSamples();
    int WriteSamples(uint8_t *samples, size_t ByteCount);
    void InstallDevice();
	void AllocateMemory();
	void FreeMemory();
};

#endif
