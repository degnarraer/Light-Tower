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

enum Mute_State_t
{
  Mute_State_Un_Muted = 0,
  Mute_State_Muted,
};

struct SampledData_t
{
  int32_t *Samples;
  int Count;
};

class I2S_Device_Callback
{
	public:
		I2S_Device_Callback(){}
		virtual ~I2S_Device_Callback(){}
		virtual void DataBufferModifyRX(String DeviceTitle, int32_t* DataBuffer, size_t Count) = 0;
		virtual void RightChannelDataBufferModifyRX(String DeviceTitle, int32_t* DataBuffer, size_t Count) = 0;
		virtual void LeftChannelDataBufferModifyRX(String DeviceTitle, int32_t* DataBuffer, size_t Count) = 0;
};

class I2S_Device: public NamedItem
				, public CommonUtils
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
              , int BufferCount
              , int BufferSize
              , int SerialClockPin
              , int WordSelectPin
              , int SerialDataInPin
              , int SerialDataOutPin
              , int MutePin );
    virtual ~I2S_Device();
    void ResgisterForDataBufferRXCallback(I2S_Device_Callback* callee){ m_Callee = callee; }
    void StartDevice();
    void StopDevice();
    QueueHandle_t GetDataBufferQueue() { return m_i2s_Data_Buffer_Queue; }
    QueueHandle_t GetRightDataBufferQueue() { return m_i2s_Right_Data_Buffer_queue; }
    QueueHandle_t GetLeftDataBufferQueue() { return m_i2s_Left_Data_Buffer_queue; }

    void SetSoundBufferData(int32_t *SoundBufferData);
    void SetMuteState(Mute_State_t MuteState);
    size_t GetSampleCount() { return m_SampleCount; }
    size_t GetChannelSampleCount() { return m_ChannelSampleCount; }
    size_t GetBytesToRead() {return m_TotalBytesToRead; }
    size_t GetChannelBytesToRead() {return m_ChannelBytesToRead; }
    int GetSampleRate() { return m_SampleRate; }
    void Setup();
    void ProcessEventQueue();
  private:
	I2S_Device_Callback* m_Callee = NULL;
    size_t m_SampleCount;
    size_t m_ChannelSampleCount;
    size_t m_BytesPerSample;
    size_t m_TotalBytesToRead;
    size_t m_ChannelBytesToRead;
    int32_t *m_SoundBufferData;
    int32_t *m_LeftChannel_SoundBufferData;
    int32_t *m_RightChannel_SoundBufferData;
    const int m_SampleRate;
    const i2s_mode_t m_i2s_Mode;
    const i2s_bits_per_sample_t m_BitsPerSample;
    const i2s_comm_format_t m_CommFormat;
    const i2s_channel_fmt_t m_Channel_Fmt;
    const i2s_channel_t m_i2s_channel;
    const int m_BufferCount;
    const int m_BufferSize;
    const int m_SerialClockPin;
    const int m_WordSelectPin;
    const int m_SerialDataInPin;
    const int m_SerialDataOutPin;
    const int m_MutePin;
    Mute_State_t m_MuteState = Mute_State_Un_Muted;
    const i2s_port_t m_I2S_PORT;
    QueueHandle_t m_i2s_event_queue = NULL;
    QueueHandle_t m_i2s_Data_Buffer_Queue = NULL;
    QueueHandle_t m_i2s_Right_Data_Buffer_queue = NULL;
    QueueHandle_t m_i2s_Left_Data_Buffer_queue = NULL;
    int ReadSamples();
    int WriteSamples(int32_t *samples);
    void InstallDevice();
};

#endif
