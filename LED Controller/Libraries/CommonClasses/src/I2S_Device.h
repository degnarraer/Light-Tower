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
#include <mutex>
#include "driver/i2s.h"
#include "Streaming.h"

extern "C" { size_t i2s_get_buffered_data_len(i2s_port_t i2s_num);}

class I2S_Device_Callback
{
	public:
		I2S_Device_Callback(){}
		virtual ~I2S_Device_Callback(){}
		virtual void I2SDataReceived(String DeviceTitle, uint8_t *data, uint32_t length) = 0;
};

class I2S_Device: public NamedItem
				, public CommonUtils
				, public QueueController
{
  enum class DeviceState
  {
    Installed,
    Uninstalled,
    Running,
    Stopped
  };

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

    void Setup();
    bool IsInitialized();
    void StartDevice();
    void StopDevice();
    size_t WriteSoundBufferData(uint8_t *SoundBufferData, size_t ByteCount);
    size_t ReadSoundBufferData(uint8_t *SoundBufferData, size_t ByteCount);
    void SetDataReceivedCallback(I2S_Device_Callback* callee)
    { 
      m_Callee = callee;
    }

    i2s_bits_per_sample_t GetBitDepth()
    {
      return m_BitsPerSample;
    }

    uint8_t* ConvertBitDepth( const uint8_t* inputData
                            , size_t inputSize
                            , i2s_bits_per_sample_t bitDepthIn
                            , i2s_bits_per_sample_t bitDepthOut
                            , size_t& outputSize )
    {
      size_t sampleCount = inputSize * 8 / bitDepthIn;
      outputSize = sampleCount * bitDepthOut / 8;

      uint8_t* outputData = (uint8_t*)ps_malloc(outputSize);
      if (!outputData)
      {
        return nullptr;
      }

      for (size_t i = 0; i < sampleCount; ++i)
      {
        int32_t sample = 0;

        switch (bitDepthIn)
        {
          case I2S_BITS_PER_SAMPLE_8BIT:
            sample = static_cast<int8_t>(inputData[i]);
            break;
          case I2S_BITS_PER_SAMPLE_16BIT:
            sample = static_cast<int16_t>(inputData[i * 2] | (inputData[i * 2 + 1] << 8));
            break;
          case I2S_BITS_PER_SAMPLE_24BIT:
            sample = (inputData[i * 3] | (inputData[i * 3 + 1] << 8) | (inputData[i * 3 + 2] << 16));
            sample = (sample << 8) >> 8;
            break;
          case I2S_BITS_PER_SAMPLE_32BIT:
            sample = static_cast<int32_t>( inputData[i * 4] | (inputData[i * 4 + 1] << 8) | (inputData[i * 4 + 2] << 16) | (inputData[i * 4 + 3] << 24) );
            break;
        }
        switch (bitDepthOut)
        {
          case I2S_BITS_PER_SAMPLE_8BIT:
            outputData[i] = static_cast<uint8_t>(sample >> (bitDepthIn - 8));
            break;
          case I2S_BITS_PER_SAMPLE_16BIT:
            outputData[i * 2] = static_cast<uint8_t>(sample);
            outputData[i * 2 + 1] = static_cast<uint8_t>(sample >> 8);
            break;
          case I2S_BITS_PER_SAMPLE_24BIT:
            outputData[i * 3] = static_cast<uint8_t>(sample);
            outputData[i * 3 + 1] = static_cast<uint8_t>(sample >> 8);
            outputData[i * 3 + 2] = static_cast<uint8_t>(sample >> 16);
            break;
          case I2S_BITS_PER_SAMPLE_32BIT:
            outputData[i * 4] = static_cast<uint8_t>(sample);
            outputData[i * 4 + 1] = static_cast<uint8_t>(sample >> 8);
            outputData[i * 4 + 2] = static_cast<uint8_t>(sample >> 16);
            outputData[i * 4 + 3] = static_cast<uint8_t>(sample >> 24);
            break;
        }
      }
      return outputData;
    }
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
    DeviceState m_DeviceState = DeviceState::Uninstalled;
    void InstallDevice();
    void UninstallDevice();

    //Process Task
    void CreateTask();
    void DestroyTask();
    static void Static_ProcessEventQueue(void * parameter);
    void ProcessEventQueue();

    //Read & Write Data
    size_t ReadSamples();
    size_t WriteSamples(uint8_t *samples, size_t ByteCount);
};

#endif
