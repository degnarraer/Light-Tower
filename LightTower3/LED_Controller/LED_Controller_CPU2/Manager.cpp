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

//I2S_Device_Callback
void Manager::I2SDataReceived(String DeviceTitle, const uint8_t *data, uint32_t length)
{

}

//Bluetooth Source Callback
int32_t Manager::SetBTTxData(uint8_t *Data, int32_t channel_len)
{
  if(channel_len <= 0 || Data == NULL)
  {
    return 0;
  }
  
  return 0;
}
