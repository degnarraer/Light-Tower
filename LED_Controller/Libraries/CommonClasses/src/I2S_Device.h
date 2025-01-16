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

#include <Arduino.h>
#include <DataTypes.h>
#include <Helpers.h>
#include <memory>
#include <mutex>
#include "driver/i2s.h"
#include "Streaming.h"
#include "BitDepthConverter.h"

#define TIME_TO_WAIT_FOR_SOUND portMAX_DELAY
//#define TIME_TO_WAIT_FOR_SOUND SEMAPHORE_LONG_BLOCK

extern "C" { size_t i2s_get_buffered_data_len(i2s_port_t i2s_num);}

class I2S_Device: public NamedItem
                , public CommonUtils
                , public QueueController
                , public BitDepthConverter

{
  public:
    I2S_Device( std::string Title
              , i2s_port_t i2S_PORT
              , i2s_mode_t Mode
              , int SampleRate
              , i2s_bits_per_sample_t BitsPerSampleIn
              , i2s_bits_per_sample_t BitsPerSampleOut
              , i2s_channel_fmt_t i2s_Channel_Fmt
              , i2s_comm_format_t CommFormat
              , i2s_channel_t i2s_channel
			        , bool Use_APLL
              , bool fixedClock
              , size_t BufferCount
              , size_t BufferSize
              , int SerialClockPin
              , int WordSelectPin
              , int SerialDataInPin
              , int SerialDataOutPin );
    virtual ~I2S_Device();

    void Setup();
    bool IsInitialized();
    void StartDevice();
    void StopDevice();
    DeviceState_t GetDeviceState();
    String GetDeviceStateString();
    bool IsRunning();
    size_t WriteSoundBufferData(uint8_t *SoundBufferData, size_t ByteCount);
    size_t ReadSoundBufferData(uint8_t *SoundBufferData, size_t ByteCount);

  private:
    DataItemConfig_t* m_ItemConfig = NULL;
    const i2s_port_t m_I2S_PORT;
    const int m_SampleRate;
    const i2s_mode_t m_i2s_Mode;
    const i2s_bits_per_sample_t m_BitsPerSampleIn;
    const i2s_bits_per_sample_t m_BitsPerSampleOut;
    const i2s_comm_format_t m_CommFormat;
    const i2s_channel_fmt_t m_Channel_Fmt;
    const i2s_channel_t m_i2s_channel;
	  const bool m_Use_APLL;
	  const bool m_FixedClock;
    const int m_BufferCount;
    const int m_BufferSize;
    const int m_I2SClockPin;
    const int m_I2SWordSelectPin;
    const int m_I2SDataInPin;
    const int m_I2SDataOutPin;
    QueueHandle_t m_i2s_event_queueHandle = nullptr;
    TaskHandle_t m_TaskHandle = nullptr;
    size_t m_SampleCount;
    size_t m_ChannelSampleCount;
    size_t m_BytesPerSample;
    size_t m_TotalBytesToRead;
    size_t m_ChannelBytesToRead;

    //Device Installation
    DeviceState m_DeviceState = DeviceState_t::Uninstalled;
    void InstallDevice();
    void UninstallDevice();

    //Process Task
    void CreateTask();
    void DestroyTask();
    static void Static_ProcessEventQueue(void * parameter);
    void ProcessEventQueue();
    bool ESP_Process(const char* subject, esp_err_t result);

    //Read & Write Data
    size_t ReadSamples();
    size_t WriteSamples(uint8_t *samples, size_t ByteCount);
};

#endif
