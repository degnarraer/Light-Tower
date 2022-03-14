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
  ESP_LOGV("Function Debug", "%s, ", __func__);
}
Manager::~Manager()
{
  ESP_LOGV("Function Debug", "%s, ", __func__);
}

void Manager::Setup()
{
  ESP_LOGV("Function Debug", "%s, ", __func__);
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
  ESP_LOGV("Function Debug", "%s, ", __func__);
  if( DeviceTitle == m_I2S_In.GetTitle() )
  {
    assert(m_I2S_Out.GetBytesToRead() == ByteCount);
    assert(m_I2S_Out.GetSampleCount() == SampleCount);
    if(DeviceTitle == m_I2S_In.GetTitle() && ByteCount > 0)
    {
      m_I2S_Out.SetSoundBufferData(DataBuffer, ByteCount);
      if(true == m_BT_Source.is_connected())
      {
        int16_t *DatBuffer16 = (int16_t*)malloc(ByteCount/2);
        for(int i = 0; i < SampleCount; ++i)
        {
          DatBuffer16[i] = ((int32_t)DataBuffer[i]) >> 16;
        }
        SoundData *BT_Sound_Data = new TwoChannelSoundData((Frame*)DatBuffer16, ByteCount/2);
        m_BT_Source.write_data(BT_Sound_Data);
        delete BT_Sound_Data;
      }
    }
  }
}
void Manager::RightChannelDataBufferModifyRX(String DeviceTitle, uint8_t* DataBuffer, size_t ByteCount, size_t SampleCount)
{
  ESP_LOGV("Function Debug", "%s, ", __func__);
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
      PushValueToQueue(DataBuffer, Queue1, false, false);
      PushValueToQueue(DataBuffer, Queue2, false, false);
    }
  }
}
void Manager::LeftChannelDataBufferModifyRX(String DeviceTitle, uint8_t* DataBuffer, size_t ByteCount, size_t SampleCount)
{
  ESP_LOGV("Function Debug", "%s, ", __func__);
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
      PushValueToQueue(DataBuffer, Queue1, false, false);
      PushValueToQueue(DataBuffer, Queue2, false, false);
    } 
  }
}
