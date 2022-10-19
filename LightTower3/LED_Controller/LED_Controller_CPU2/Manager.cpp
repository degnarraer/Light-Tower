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
                , Bluetooth_Source &BT_Out
                , I2S_Device &I2S_OUT ): NamedItem(Title)
                                       , m_SoundProcessor(SoundProcessor)
                                       , m_SerialDataLink(SerialDataLink)
                                       , m_BT_Out(BT_Out)
                                       , m_I2S_Out(I2S_OUT)
{ 
  ESP_LOGV("Manager", "%s, ", __func__);
}
Manager::~Manager()
{
  FreeMemory();
  ESP_LOGV("Manager", "%s, ", __func__);
}

void Manager::AllocateMemory()
{
}
void Manager::FreeMemory()
{
}

void Manager::Setup()
{
  AllocateMemory();
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT, ESP_PWR_LVL_P9); //Set Bluetooth Power to Max
  m_SoundProcessor.SetupSoundProcessor();
  m_I2S_Out.ResgisterForDataBufferRXCallback(this);
  m_I2S_Out.StartDevice();
  m_BT_Out.StartDevice();
  m_AudioBuffer.Initialize();
  m_AudioStreamSlave.Setup();
}

void Manager::Loop()
{
}

void Manager::ProcessEventQueue()
{
  ESP_LOGV("Function Debug", "%s, ", __func__);
  //UpdateNotificationRegistrationStatus();
  //m_I2S_Out.ProcessEventQueue();
}

void Manager::UpdateNotificationRegistrationStatus()
{
  bool IsConnected = m_BT_Out.IsConnected();
  if(true == IsConnected)
  {
  }
  else
  {
  }
}
//I2S_Device_Callback
void Manager::DataBufferModifyRX(String DeviceTitle, uint8_t* DataBuffer, size_t ByteCount, size_t SampleCount)
{
  //ESP_LOGV("Manager", "%s, ", __func__);
  //if( DeviceTitle == m_I2S_In.GetTitle() && ByteCount > 0)
  {
  }
}
void Manager::RightChannelDataBufferModifyRX(String DeviceTitle, uint8_t* DataBuffer, size_t ByteCount, size_t SampleCount)
{
  //ESP_LOGV("Manager", "%s, ", __func__);
  //if( DeviceTitle == m_I2S_In.GetTitle() && ByteCount > 0)
  {
    QueueHandle_t Queue1 = m_SoundProcessor.GetQueueHandleRXForDataItem("R_PSD_IN");
    QueueHandle_t Queue2 = m_SoundProcessor.GetQueueHandleRXForDataItem("R_FFT_IN");
    if(NULL != Queue1 && NULL != Queue2)
    {
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
  //if( DeviceTitle == m_I2S_In.GetTitle() )
  {
    QueueHandle_t Queue1 = m_SoundProcessor.GetQueueHandleRXForDataItem("L_PSD_IN");
    QueueHandle_t Queue2 = m_SoundProcessor.GetQueueHandleRXForDataItem("L_FFT_IN");
    if(NULL != Queue1 && NULL != Queue2)
    {
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
  size_t BytesRequested = channel_len * sizeof(Frame_t);
  size_t FramesAvailable = m_AudioStreamSlave.GetFrameCount();
  size_t FramesRead = m_AudioStreamSlave.GetAudioFrames((Frame_t*)frame, channel_len);
  assert(FramesRead <= channel_len);
  size_t BytesRead = FramesRead * sizeof(Frame_t);

  //ESP_LOGE("Manager", "%i | %i | %i | %i", channel_len, FramesAvailable, BytesRead, FramesRead);
  //m_I2S_Out.SetSoundBufferData((uint8_t*)RXBuffer, BytesRead);

  /*
  int loopcount = floor(m_FrameBuffer.size() / I2S_SAMPLE_COUNT);
  for(int i = 0; i < loopcount; ++i)
  {
    int32_t ActualSampleReadCount = m_FrameBuffer.Read(m_LinearFrameBuffer, sizeof(m_LinearFrameBuffer)/sizeof(m_LinearFrameBuffer[0]));
    assert(ActualSampleReadCount <= sizeof(m_LinearFrameBuffer)/sizeof(m_LinearFrameBuffer[0]));
    assert(ActualSampleReadCount <= sizeof(m_RightDataBuffer)/sizeof(m_RightDataBuffer[0]));
    assert(ActualSampleReadCount <= sizeof(m_LeftDataBuffer)/sizeof(m_LeftDataBuffer[0]));
    memset(m_RightDataBuffer, 0, sizeof(m_RightDataBuffer)/sizeof(m_RightDataBuffer[0]));
    memset(m_LeftDataBuffer, 0, sizeof(m_LeftDataBuffer)/sizeof(m_LeftDataBuffer[0]));
    for(int j = 0; j < ActualSampleReadCount; ++j)
    {
      m_RightDataBuffer[j] = m_LinearFrameBuffer[j].channel1 << 16;
      m_LeftDataBuffer[j] = m_LinearFrameBuffer[j].channel2 << 16;
    }
    //RightChannelDataBufferModifyRX(m_I2S_In.GetTitle(), ((uint8_t*)m_RightDataBuffer), ActualSampleReadCount * sizeof(m_RightDataBuffer[0]), ActualSampleReadCount);
    //LeftChannelDataBufferModifyRX(m_I2S_In.GetTitle(), ((uint8_t*)m_LeftDataBuffer), ActualSampleReadCount * sizeof(m_LeftDataBuffer[0]), ActualSampleReadCount);
  }
  ESP_LOGV("Manager", "Samples Requested: %i\tBytes Read: %i\tSamples Read: %i", channel_len, BytesRead, SamplesRead);
  */
  return FramesRead;
}
