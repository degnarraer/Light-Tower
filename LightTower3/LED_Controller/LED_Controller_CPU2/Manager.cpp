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
                , AudioBuffer<441> &AudioBufferAmplitude
                , AudioBuffer<512> &AudioBufferFFT
                , Bluetooth_Source &BT_Out
                , I2S_Device &I2S_In ): NamedItem(Title)
                                       , m_SoundProcessor(SoundProcessor)
                                       , m_SerialDataLink(SerialDataLink)
                                       , m_AudioBufferAmplitude(AudioBufferAmplitude)
                                       , m_AudioBufferFFT(AudioBufferFFT)
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
  ESP_LOGV("Function Debug", "%s, ", __func__);
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
  uint8_t Buffer[channel_len];
  size_t ByteReceived = m_I2S_In.ReadSoundBufferData(Buffer, channel_len);
  assert(0 == ByteReceived % sizeof(Frame_t)); 
  size_t FramesReceived = ByteReceived / sizeof(Frame_t);
  memcpy(Data, Buffer, ByteReceived);
  for(int i = 0; i < FramesReceived; ++i)
  {
    m_AudioBufferAmplitude.Push(((Frame_t*)Buffer)[i]);
  }
  return ByteReceived;
}
