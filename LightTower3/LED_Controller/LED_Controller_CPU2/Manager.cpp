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
                , I2S_Device &I2S_In
                , I2S_Device &I2S_OUT ): NamedItem(Title)
                                       , m_SoundProcessor(SoundProcessor)
                                       , m_SerialDataLink(SerialDataLink)
                                       , m_BT_Out(BT_Out)
                                       , m_I2S_In(I2S_In)
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
  int FrameBufferByteCount = sizeof(bfs::CircleBuf<Frame_t, m_CircularBufferSize>);
  ESP_LOGE("Manager", "Frame Buffer Byte Count: %d", FrameBufferByteCount);
  m_FrameBuffer = (bfs::CircleBuf<Frame_t, m_CircularBufferSize>*)ps_malloc(FrameBufferByteCount);
}
void Manager::FreeMemory()
{
  free(m_FrameBuffer);
}

void Manager::Setup()
{
  ESP_LOGV("Manager", "%s, ", __func__);
  AllocateMemory();
  m_SoundProcessor.SetupSoundProcessor();
  m_I2S_In.ResgisterForDataBufferRXCallback(this);
  m_I2S_Out.ResgisterForDataBufferRXCallback(this);
  m_I2S_In.StartDevice();  
  m_I2S_Out.StartDevice();
  m_BT_Out.StartDevice();
}

void Manager::ProcessEventQueue()
{
  ESP_LOGV("Function Debug", "%s, ", __func__);
  UpdateNotificationRegistrationStatus();
  m_I2S_In.ProcessEventQueue();
  m_I2S_Out.ProcessEventQueue();
}

void Manager::UpdateNotificationRegistrationStatus()
{
  bool IsConnected = m_BT_Out.IsConnected();
  if(true == IsConnected)
  {
    m_I2S_In.DeResgisterForDataBufferRXCallback(this);
  }
  else
  {
    m_I2S_In.ResgisterForDataBufferRXCallback(this);
  }
}
//I2S_Device_Callback
void Manager::DataBufferModifyRX(String DeviceTitle, uint8_t* DataBuffer, size_t ByteCount, size_t SampleCount)
{
  //ESP_LOGV("Manager", "%s, ", __func__);
  if( DeviceTitle == m_I2S_In.GetTitle() && ByteCount > 0)
  {
    size_t ChannelSampleCount = SampleCount/2;
    m_I2S_Out.SetSoundBufferData(DataBuffer, ByteCount);
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
  int32_t BytesRead = 0;
  int32_t BytesRequested = channel_len * m_32BitFrameByteCount;
  assert(BytesRequested <= m_I2S_RXBuffer_Count*sizeof(m_I2S_RXBuffer));
  BytesRead = m_I2S_In.GetSoundBufferData(m_I2S_RXBuffer, BytesRequested);
  assert(BytesRead <= m_I2S_RXBuffer_Count*sizeof(m_I2S_RXBuffer));
  m_I2S_Out.SetSoundBufferData(m_I2S_RXBuffer, BytesRead);
  int32_t SamplesRead = BytesRead / m_32BitFrameByteCount;
  for(int i = 0; i < SamplesRead; ++i)
  {
    Frame aFrame;
    aFrame.channel1 = ((int32_t*)m_I2S_RXBuffer)[2*i] >> 16;
    aFrame.channel2 = ((int32_t*)m_I2S_RXBuffer)[2*i + 1] >> 16;
    //m_FrameBuffer->Write( (Frame_t&)(aFrame) );
    frame[i] = aFrame;
  }
  for(int i = 0; i < floor(m_FrameBuffer->size() / I2S_SAMPLE_COUNT); ++i)
  {
    int32_t ActualSampleReadCount = m_FrameBuffer->Read(m_LinearFrameBuffer, I2S_SAMPLE_COUNT);
    assert(ActualSampleReadCount <= m_RightDataBuffer_Count);
    assert(ActualSampleReadCount <= m_LeftDataBuffer_Count);
    for(int i = 0; i < ActualSampleReadCount; ++i)
    {
      m_RightDataBuffer[i] = m_LinearFrameBuffer[i].channel1 << 16;
      m_LeftDataBuffer[i] = m_LinearFrameBuffer[i].channel2 << 16;
    }
    RightChannelDataBufferModifyRX(m_I2S_In.GetTitle(), ((uint8_t*)m_RightDataBuffer), ActualSampleReadCount * sizeof(m_RightDataBuffer[0]), ActualSampleReadCount);
    LeftChannelDataBufferModifyRX(m_I2S_In.GetTitle(), ((uint8_t*)m_LeftDataBuffer), ActualSampleReadCount * sizeof(m_RightDataBuffer[0]), ActualSampleReadCount);
  }
  ESP_LOGV("Manager", "Samples Requested: %i\tBytes Read: %i\tSamples Read: %i", channel_len, BytesRead, SamplesRead);
  return SamplesRead;
}
