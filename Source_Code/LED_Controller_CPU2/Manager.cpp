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
                , SerialPortMessageManager &CPU1SerialPortMessageManager
                , SerialPortMessageManager &CPU3SerialPortMessageManager
                , Bluetooth_Source &BT_Out
                , I2S_Device &I2S_In
                , ContinuousAudioBuffer<AUDIO_BUFFER_SIZE> &AudioBuffer )
                : NamedItem(Title)
                , m_SoundProcessor(SoundProcessor)
                , m_CPU1SerialPortMessageManager(CPU1SerialPortMessageManager)
                , m_CPU3SerialPortMessageManager(CPU3SerialPortMessageManager)
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
  m_Preferences.begin("Manager Settings", false);
  InitializeNVM(m_Preferences.getBool("NVM Reset", false));
  LoadFromNVM();
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT, ESP_PWR_LVL_P9); //Set Bluetooth Power to Max
  m_AudioBuffer.Initialize();
  m_I2S_In.StartDevice();
  m_BT_Out.RegisterForConnectionStatusChangedCallBack(this);
  m_BT_Out.RegisterForActiveDeviceUpdate(this);
  m_BT_Out.StartDevice( m_SourceSSID.c_str(), m_SourceADDRESS.c_str() );
  
  xTaskCreatePinnedToCore( Static_TaskLoop_20mS,     "Manager_20mS_Task",      5000,   this,   configMAX_PRIORITIES - 1,   &m_Manager_20mS_Task,       1 );
  xTaskCreatePinnedToCore( Static_TaskLoop_1000mS,   "Manager_1000mS_Task",    5000,   this,   configMAX_PRIORITIES - 3,   &m_Manager_1000mS_Task,     1 );
}

void Manager::InitializeNVM(bool Reset)
{
  if(true == Reset || false == m_Preferences.getBool("NVM Initialized", false))
  {
    if(true == Reset) m_Preferences.clear();
    m_Preferences.putString("Source SSID", "");
    m_Preferences.putString("Source ADDRESS", "");
    m_Preferences.putBool("Source BT Reset", true);
    m_Preferences.putBool("BT NVS Init", true);
    m_Preferences.putBool("Src ReConnect", false);
    m_Preferences.putBool("SSP Enabled", false);
    m_Preferences.putBool("NVM Initialized", true);
    m_Preferences.putBool("NVM Reset", false);
  }
}

void Manager::LoadFromNVM()
{
  //Reload NVM Values
  m_SourceSSID = m_Preferences.getString("Source SSID", "");
  m_SourceADDRESS = m_Preferences.getString("Source ADDRESS", "");
  
  m_SourceBTReset = m_Preferences.getBool("Source BT Reset", true);
  m_BT_Out.Set_Reset_BLE(m_SourceBTReset);
  
  m_NVSInit = m_Preferences.getBool("BT NVS Init", false);
  m_BT_Out.Set_NVS_Init(m_NVSInit);
  
  m_SourceBTReConnect = m_Preferences.getBool("Src ReConnect", true);
  m_BT_Out.Set_Auto_Reconnect(m_SourceBTReConnect);
  
  m_SSP_Enabled = m_Preferences.getBool("SSP Enabled", false);
  m_BT_Out.Set_SSP_Enabled(m_SSP_Enabled);
}

void Manager::Static_TaskLoop_20mS(void * parameter)
{
  Manager *aManager = (Manager*)parameter;
  aManager->TaskLoop_20mS();
}

void Manager::Static_TaskLoop_1000mS(void * parameter)
{
  Manager *aManager = (Manager*)parameter;
  aManager->TaskLoop_1000mS();
}

void Manager::TaskLoop_20mS()
{
  const TickType_t xFrequency = 20;
  TickType_t xLastWakeTime = xTaskGetTickCount();
  while(true)
  {
    vTaskDelayUntil( &xLastWakeTime, xFrequency );
    m_I2S_In.ProcessEventQueue();
  }
}

void Manager::TaskLoop_1000mS()
{
  const TickType_t xFrequency = 1000;
  TickType_t xLastWakeTime = xTaskGetTickCount();
  while(true)
  {
    vTaskDelayUntil( &xLastWakeTime, xFrequency );
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
  assert(0 == ByteReceived % sizeof(uint32_t)); 
  size_t FrameCount = ByteReceived / sizeof(uint32_t);
  m_AudioBuffer.Push((Frame_t*)Data, FrameCount);
  return ByteReceived;
}

//BluetoothConnectionStatusCallee Callback 
void Manager::BluetoothConnectionStatusChanged(ConnectionStatus_t ConnectionStatus)
{
  
}

//BluetoothActiveDeviceUpdatee Callback 
void Manager::BluetoothActiveDeviceListUpdated(const std::vector<ActiveCompatibleDevice_t> &Devices)
{
  for(int i = 0; i < Devices.size(); ++i)
  {  
    m_SSIDWLUT = SSID_Info_With_LastUpdateTime_t( Devices[i].SSID.c_str()
                                                , Devices[i].ADDRESS.c_str()
                                                , millis()-Devices[i].LastUpdateTime
                                                , Devices[i].RSSI );
  }
}
