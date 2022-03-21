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
                , BluetoothA2DPSource &BT_Source
                , I2S_Device &I2S_In
                , I2S_Device &I2S_OUT ): NamedItem(Title)
                                       , m_SoundProcessor(SoundProcessor)
                                       , m_SerialDataLink(SerialDataLink)
                                       , m_BT_Source(BT_Source)
                                       , m_I2S_In(I2S_In)
                                       , m_I2S_Out(I2S_OUT)
{ 
  ESP_LOGV("Manager", "%s, ", __func__);
}
Manager::~Manager()
{
  ESP_LOGV("Manager", "%s, ", __func__);
}

void Manager::Setup()
{
  ESP_LOGV("Manager", "%s, ", __func__);
  m_SoundProcessor.SetupSoundProcessor();
  m_I2S_In.ResgisterForDataBufferRXCallback(this);
  m_I2S_Out.ResgisterForDataBufferRXCallback(this);
  m_I2S_In.StartDevice();  
  m_I2S_Out.StartDevice();
}

void Manager::ProcessEventQueue()
{
  ESP_LOGV("Function Debug", "%s, ", __func__);
  m_I2S_In.ProcessEventQueue();
  m_I2S_Out.ProcessEventQueue();
}


//I2S_Device_Callback
void Manager::DataBufferModifyRX(String DeviceTitle, uint8_t* DataBuffer, size_t ByteCount, size_t SampleCount)
{
  //ESP_LOGV("Manager", "%s, ", __func__);
  if( DeviceTitle == m_I2S_In.GetTitle() )
  {
    size_t ChannelSampleCount = SampleCount/2;
    assert(m_I2S_Out.GetBytesToRead() == ByteCount);
    assert(m_I2S_Out.GetSampleCount() == SampleCount);
    if(DeviceTitle == m_I2S_In.GetTitle() && ByteCount > 0)
    {
      m_I2S_Out.SetSoundBufferData(DataBuffer, ByteCount);
      if(true == m_BT_Source.is_connected())
      {
        for(int i = 0; i < ChannelSampleCount; ++i)
        {
          m_DataFrameRX[i].channel1 = ((int32_t*)DataBuffer)[2*i] >> 16;
          m_DataFrameRX[i].channel2 = ((int32_t*)DataBuffer)[2*i+1] >> 16;
          size_t space = m_FrameBuffer.capacity() - m_FrameBuffer.size();
          if(0 == space)
          {
            m_FrameBuffer.Clear();
          }
          m_FrameBuffer.Write(m_DataFrameRX[i]);
        }
      }
    }
  }
}
void Manager::RightChannelDataBufferModifyRX(String DeviceTitle, uint8_t* DataBuffer, size_t ByteCount, size_t SampleCount)
{
  //ESP_LOGV("Manager", "%s, ", __func__);
  if( DeviceTitle == m_I2S_In.GetTitle() && ByteCount > 0)
  {
    QueueHandle_t Queue1 = m_SoundProcessor.GetQueueHandleRXForDataItem("R_PSD_IN");
    QueueHandle_t Queue2 = m_SoundProcessor.GetQueueHandleRXForDataItem("R_FFT_IN");
    if(NULL != Queue1 && NULL != Queue2)
    {
      assert(m_I2S_Out.GetChannelBytesToRead() == ByteCount);
      assert(m_I2S_Out.GetChannelSampleCount() == SampleCount);
      assert(m_SoundProcessor.GetQueueByteCountForDataItem("R_PSD_IN") == ByteCount);
      assert(m_SoundProcessor.GetQueueByteCountForDataItem("R_FFT_IN") == ByteCount);
      static bool R_PSD_IN_Push_Successful = true;
      PushValueToQueue(DataBuffer, Queue1, false, "Right Channel Data: R_PSD_IN", R_PSD_IN_Push_Successful);
      static bool R_FFT_IN_Push_Successful = true;
      PushValueToQueue(DataBuffer, Queue2, false, "Right Channel Data: R_FFT_IN", R_FFT_IN_Push_Successful);
    }
  }
}
void Manager::LeftChannelDataBufferModifyRX(String DeviceTitle, uint8_t* DataBuffer, size_t ByteCount, size_t SampleCount)
{
  //ESP_LOGV("Manager", "%s, ", __func__);
  if( DeviceTitle == m_I2S_In.GetTitle() )
  {
    QueueHandle_t Queue1 = m_SoundProcessor.GetQueueHandleRXForDataItem("L_PSD_IN");
    QueueHandle_t Queue2 = m_SoundProcessor.GetQueueHandleRXForDataItem("L_FFT_IN");
    if(NULL != Queue1 && NULL != Queue2)
    {
      assert(m_I2S_Out.GetChannelBytesToRead() == ByteCount);
      assert(m_I2S_Out.GetChannelSampleCount() == SampleCount);
      assert(m_SoundProcessor.GetQueueByteCountForDataItem("L_PSD_IN") == ByteCount);
      assert(m_SoundProcessor.GetQueueByteCountForDataItem("L_FFT_IN") == ByteCount);
      static bool L_PSD_IN_Push_Successful = true;
      PushValueToQueue(DataBuffer, Queue1, false, "Left Channel Data: L_PSD_IN", L_PSD_IN_Push_Successful);
      static bool L_FFT_IN_Push_Successful = true;
      PushValueToQueue(DataBuffer, Queue2, false, "Left Channel Data: L_FFT_IN", L_FFT_IN_Push_Successful);
    } 
  }
}

//Bluetooth Source Callback
int32_t Manager::get_data_channels(Frame *frame, int32_t channel_len)
{
  //ESP_LOGD("Manager",  "Channel Length Needed: %i\tTotal Samples Available: %i",channel_len, m_FrameBuffer.size());
  int32_t SamplesToRead = min((int32_t)m_FrameBuffer.size(), channel_len);
  m_FrameBuffer.Read( (Frame_t*)frame, SamplesToRead );
  //ESP_LOGD("Manager",  "SamplesRead: %i", SamplesToRead);
  return SamplesToRead;
}
