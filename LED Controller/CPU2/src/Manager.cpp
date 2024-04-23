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
                , ContinuousAudioBuffer<AUDIO_BUFFER_SIZE> &AudioBuffer
                , Preferences& preferences)
                : NamedItem(Title)
                , m_Preferences(preferences)
                , m_CPU1SerialPortMessageManager(CPU1SerialPortMessageManager)
                , m_CPU3SerialPortMessageManager(CPU3SerialPortMessageManager)
                , m_SoundProcessor(SoundProcessor)
                , m_AudioBuffer(AudioBuffer)
                , m_I2S_In(I2S_In)
                , m_BT_Out(BT_Out)
{
}
Manager::~Manager()
{
}

void Manager::Setup()
{
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT, ESP_PWR_LVL_P9); //Set Bluetooth Power to Max
  m_AudioBuffer.Initialize();
  m_BT_Out.RegisterForConnectionStateChangedCallBack(this);
  m_BT_Out.RegisterForActiveDeviceUpdate(this);
  if( xTaskCreatePinnedToCore( Static_TaskLoop_20mS, "Manager_20mS_Task", 10000, this, THREAD_PRIORITY_MEDIUM, &m_Manager_20mS_Task, 1 ) != pdTRUE )
  {
    ESP_LOGE("Setup", "Error creating task!");
  }
}

void Manager::StartBluetooth()
{
  ESP_LOGI("Manager::ConnectToTargetDevice", "Starting Bluetooth!" );
  m_BT_Out.Set_Auto_Reconnect(m_BluetoothSourceAutoReConnect.GetValue());
  m_BT_Out.Set_Reset_BLE(m_BluetoothReset.GetValue());
  m_BT_Out.StartDevice();
}

void Manager::StopBluetooth()
{
  ESP_LOGI("Manager::ConnectToTargetDevice", "Stopping Bluetooth!" );
  m_BT_Out.StopDevice();
}

void Manager::Static_TaskLoop_20mS(void * parameter)
{
  Manager *aManager = (Manager*)parameter;
  aManager->TaskLoop_20mS();
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

//BluetoothConnectionStateCallee Callback 
void Manager::BluetoothConnectionStateChanged(const esp_a2d_connection_state_t connectionState)
{
  ConnectionStatus_t newValue = static_cast<ConnectionStatus_t>(connectionState);
  if(m_ConnectionStatus.GetValue() != newValue)
  {
    m_ConnectionStatus.SetValue(newValue);
    ESP_LOGI("Manager: BluetoothConnectionStatusChanged", "Connection Status Changed to %s", ConnectionStatusStrings[connectionState]);
  }
}

//BluetoothActiveDeviceUpdatee Callback 
void Manager::BluetoothActiveDeviceListUpdated(const std::vector<ActiveCompatibleDevice_t> &Devices)
{
  unsigned long currentMillis = millis();
  for(int i = 0; i < Devices.size(); ++i)
  {
    unsigned long elapsedTime;
    unsigned long previousMillis = Devices[i].lastUpdateTime;
    if (currentMillis >= previousMillis) { elapsedTime = currentMillis - previousMillis; } 
    else { elapsedTime = (ULONG_MAX - previousMillis) + currentMillis + 1; }
    ESP_LOGD("Manager: BluetoothActiveDeviceListUpdated", "Active Device List Item %i: Name: \"%s\": Address: \"%s\" Last Update Time: \"%i\" RSSI: \"%i\""
            , i
            , Devices[i].name
            , Devices[i].address
            , elapsedTime
            , Devices[i].rssi );
    ActiveCompatibleDevice_t ActiveDevice = ActiveCompatibleDevice_t( Devices[i].name
                                                                    , Devices[i].address
                                                                    , Devices[i].rssi
                                                                    , Devices[i].lastUpdateTime
                                                                    , elapsedTime );
    m_ScannedDevice.SetValue(&ActiveDevice, 1);                                            
  }
}








