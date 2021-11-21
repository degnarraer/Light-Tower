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
#define DEBUG_DATA false
#define DEBUG_QUEUE true

#include "Task.h"
#include "driver/i2s.h"
#include "Streaming.h"
#include "EventSystem.h"

enum Device_Type_t
{
  Device_Type_Receiver,
  Device_Type_Transmitter,
};

struct SampledData_t
{
  int32_t *Samples;
  int Count;
};

class I2S_Device: public Task
                , public EventSystemCaller
{
  public:
    I2S_Device( String Title
              , DataManager &DataManager
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
              , String Notification_RX
              , String Notification_TX );
    virtual ~I2S_Device();
    
    void StartDevice();
    void StopDevice();
    int32_t* GetSoundBufferData(){return m_SoundBufferData;}
    int32_t* GetRightSoundBufferData(){return m_RightChannel_SoundBufferData;}
    int32_t* GetLeftSoundBufferData(){return m_LeftChannel_SoundBufferData;}
    
    //Task Interface
    void Setup();
    bool CanRunMyTask();
    void RunMyTask();
  private:    
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
    const i2s_port_t m_I2S_PORT;
    const String m_Notification_RX;
    const String m_Notification_TX;
    QueueHandle_t m_i2s_event_queue = NULL;
    int ReadSamples();
    int WriteSamples(int32_t *samples);
    void InstallDevice();
    void ProcessEventQueue();
};

#endif
