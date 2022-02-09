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

#include "Manager.h"

Manager::Manager( String Title
                , Sound_Processor &SoundProcessor
                , SerialDataLink &SerialDataLink
                , I2S_Device &I2S_In
                , I2S_Device &I2S_OUT ): NamedItem(Title)
                                      , m_SoundProcessor(SoundProcessor)
                                      , m_SerialDataLink(SerialDataLink)
                                      , m_I2S_In(I2S_In)
                                      , m_I2S_Out(I2S_OUT)
{
}
Manager::~Manager()
{
}

void Manager::Setup()
{
  if(true == EVENT_HANDLER_DEBUG) Serial << "Setup i2s Event Handler\n"; 
  m_SoundProcessor.SetupSoundProcessor(m_I2S_In.GetChannelBytesToRead(), m_I2S_In.GetSampleRate(), FFT_LARGE_SIZE, FFT_SMALL_SIZE);
  m_I2S_In.ResgisterForDataBufferRXCallback(this);
  m_I2S_Out.ResgisterForDataBufferRXCallback(this);
  m_I2S_In.StartDevice();  
  m_I2S_Out.StartDevice(); 
}

void Manager::ProcessEventQueue()
{
  m_I2S_In.ProcessEventQueue();
  m_I2S_Out.ProcessEventQueue();
}

//I2S_Device_Callback
void Manager::DataBufferModifyRX(String DeviceTitle, uint8_t* DataBuffer, size_t ByteCount, size_t SampleCount)
{
  if( DeviceTitle == m_I2S_In.GetTitle() )
  {
    for(int i = 0; i < SampleCount; ++i)
    {
      if(true == PRINT_DATA_DEBUG)
      {
        Serial.println(((int32_t*)DataBuffer)[i]);
      }
      if(true == PRINT_DATA_DEBUG) Serial.println(m_I2S_In.GetDataBufferValue(DataBuffer, i));
    }
    if(DeviceTitle == m_I2S_In.GetTitle())
    {
      assert(m_I2S_Out.GetBytesToRead() == ByteCount);
      assert(m_I2S_Out.GetSampleCount() == SampleCount);
      m_I2S_Out.SetSoundBufferData(DataBuffer, ByteCount);
    }
  }
}
void Manager::RightChannelDataBufferModifyRX(String DeviceTitle, uint8_t* DataBuffer, size_t ByteCount, size_t SampleCount)
{
  if( DeviceTitle == m_I2S_In.GetTitle() )
  {
    for(int i = 0; i < SampleCount; ++i)
    {
      if(true == PRINT_RIGHT_CHANNEL_DATA_DEBUG)
      {
        Serial.println(((int32_t*)DataBuffer)[i]);
      }
    }
    QueueHandle_t Queue32 = m_SoundProcessor.GetQueueHandleRXForDataItem("R_RAW32_IN");
    size_t Queue32ByteCount = m_SoundProcessor.GetQueueByteCountForDataItem("R_RAW32_IN");
    if(NULL != Queue32)
    {
      assert(Queue32ByteCount == ByteCount);
      PushValueToQueue(DataBuffer, Queue32, false, false);
    } 
  }
}
void Manager::LeftChannelDataBufferModifyRX(String DeviceTitle, uint8_t* DataBuffer, size_t ByteCount, size_t SampleCount)
{
    for(int i = 0; i < SampleCount; ++i)
    {
      if(true == PRINT_LEFT_CHANNEL_DATA_DEBUG)
      {
        
        Serial.println(((int32_t*)DataBuffer)[i]);
      }
    }
  if( DeviceTitle == m_I2S_In.GetTitle() )
  {
    QueueHandle_t Queue32 = m_SoundProcessor.GetQueueHandleRXForDataItem("L_RAW32_IN");
    size_t Queue32ByteCount = m_SoundProcessor.GetQueueByteCountForDataItem("L_RAW32_IN");
    if(NULL != Queue32)
    {
      assert(Queue32ByteCount == ByteCount);
      PushValueToQueue(DataBuffer, Queue32, false, false);
    } 
  }
}
