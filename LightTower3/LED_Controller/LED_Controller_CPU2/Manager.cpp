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
                , SPIDataLinkMaster &SPIDataLinkMaster
                , Bluetooth_Source &BT_Out
                , I2S_Device &I2S_In
                , ContinuousAudioBuffer<AUDIO_BUFFER_SIZE> AudioBuffer )
                : NamedItem(Title)
                , m_SoundProcessor(SoundProcessor)
                , m_SPIDataLinkMaster(SPIDataLinkMaster)
                , m_BT_Out(BT_Out)
                , m_I2S_In(I2S_In)
                , m_AudioBuffer(AudioBuffer)
{
}
Manager::~Manager()
{
}

void Manager::Setup()
{
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT, ESP_PWR_LVL_P9); //Set Bluetooth Power to Max
  m_SoundProcessor.SetupSoundProcessor();
  m_AmplitudeAudioBuffer.Initialize();
  m_FFTAudioBuffer.Initialize();
  m_AudioBuffer.Initialize();
  m_I2S_In.StartDevice();
  m_BT_Out.StartDevice();
}

void Manager::ProcessEventQueue()
{
  UpdateNotificationRegistrationStatus();
  m_I2S_In.ProcessEventQueue();
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
void Manager::I2SDataReceived(String DeviceTitle, uint8_t *Data, uint32_t channel_len)
{
}

//Bluetooth Source Callback
int32_t Manager::SetBTTxData(uint8_t *Data, int32_t channel_len)
{
  size_t ByteReceived = m_I2S_In.ReadSoundBufferData(Data, channel_len);
  assert(0 == ByteReceived % sizeof(Frame_t)); 
  size_t FrameCount = ByteReceived / sizeof(Frame_t);
  m_AmplitudeAudioBuffer.WriteAudioFrames((Frame_t*)Data, FrameCount);
  m_FFTAudioBuffer.WriteAudioFrames((Frame_t*)Data, FrameCount);
  m_AudioBuffer.Push((Frame_t*)Data, FrameCount);
 
  QueueHandle_t FFTQueue = m_SoundProcessor.GetQueueHandleRXForDataItem("FFT_Frames");
  QueueHandle_t AmplitudeQueue = m_SoundProcessor.GetQueueHandleRXForDataItem("Amplitude_Frames");

  if(FFTQueue != NULL && AmplitudeQueue != NULL)
  {
    size_t RequiredAmplitudeFrameCount = m_SoundProcessor.GetSampleCountForDataItem("Amplitude_Frames");
    size_t AvailableAmplitudeFrameCount = m_AmplitudeAudioBuffer.GetFrameCount();
    size_t RequiredFFTFrameCount = m_SoundProcessor.GetSampleCountForDataItem("FFT_Frames");
    size_t AvailableFFTFrameCount = m_FFTAudioBuffer.GetFrameCount();
    size_t AmplitudeLoopCount = AvailableAmplitudeFrameCount  / RequiredAmplitudeFrameCount;
    size_t FFTLoopCount = AvailableFFTFrameCount / RequiredFFTFrameCount;
    
    for(int i = 0; i < AmplitudeLoopCount; ++i)
    {
      size_t FramesRead = m_AmplitudeAudioBuffer.ReadAudioFrames(m_AmplitudeFrameBuffer, RequiredAmplitudeFrameCount);
      assert(FramesRead == RequiredAmplitudeFrameCount);
      static bool AmplitudePushError = false;
      PushValueToQueue(m_AmplitudeFrameBuffer, AmplitudeQueue, false, "Manager Amplitude Buffer", AmplitudePushError);
    }
    for(int i = 0; i < FFTLoopCount; ++i)
    {
      size_t FramesRead = m_FFTAudioBuffer.ReadAudioFrames(m_FFTFrameBuffer, RequiredFFTFrameCount);
      assert(FramesRead == RequiredFFTFrameCount);
      static bool FFTPushError = false;
      PushValueToQueue(m_FFTFrameBuffer, FFTQueue, false, "Manager FFT Buffer", FFTPushError);
    }
  }
  return ByteReceived;
}
