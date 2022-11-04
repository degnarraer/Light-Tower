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
                , I2S_Device &I2S_In ): NamedItem(Title)
                                       , m_SoundProcessor(SoundProcessor)
                                       , m_SerialDataLink(SerialDataLink)
                                       , m_BT_Out(BT_Out)
                                       , m_I2S_In(I2S_In)
{
}
Manager::~Manager()
{
}

void Manager::Setup()
{
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT, ESP_PWR_LVL_P9); //Set Bluetooth Power to Max
  m_SoundProcessor.SetupSoundProcessor();
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
void Manager::I2SDataReceived(String DeviceTitle, uint8_t *data, uint32_t length)
{
}

//Bluetooth Source Callback
int32_t Manager::SetBTTxData(uint8_t *Data, int32_t channel_len)
{
  size_t ByteReceived = m_I2S_In.ReadSoundBufferData(Data, channel_len);
  assert(0 == ByteReceived % sizeof(Frame_t)); 
  size_t FrameCount = ByteReceived / sizeof(Frame_t);

  QueueHandle_t FFTQueue = m_SoundProcessor.GetQueueHandleRXForDataItem("FFT_Frames");
  QueueHandle_t AmplitudeQueue = m_SoundProcessor.GetQueueHandleRXForDataItem("Amplitude_Frames");

  if(FFTQueue != NULL && AmplitudeQueue != NULL)
  {
    size_t FFTFrameCount = m_SoundProcessor.GetSampleCountForDataItem("FFT_Frames");
    size_t AmplitudeFrameCount = m_SoundProcessor.GetSampleCountForDataItem("Amplitude_Frames");
    assert(FFTFrameCount < channel_len / sizeof(Frame_t));
    assert(AmplitudeFrameCount < channel_len / sizeof(Frame_t));
    bool FFTPushError = false;
    PushValueToQueue(Data, FFTQueue, false, "Manager BT Received: FFT", FFTPushError);
    bool AmplitudePushError = false;
    PushValueToQueue(Data, FFTQueue, false, "Manager BT Received: Amplitude", AmplitudePushError);
  }
  return ByteReceived;
}
