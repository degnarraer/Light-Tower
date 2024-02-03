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
                , m_SoundProcessor(SoundProcessor)
                , m_CPU1SerialPortMessageManager(CPU1SerialPortMessageManager)
                , m_CPU3SerialPortMessageManager(CPU3SerialPortMessageManager)
                , m_BT_Out(BT_Out)
                , m_I2S_In(I2S_In)
                , m_AudioBuffer(AudioBuffer)
                , m_Preferences(preferences)
{
}
Manager::~Manager()
{
}

void Manager::Setup()
{
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT, ESP_PWR_LVL_P9); //Set Bluetooth Power to Max
  m_AudioBuffer.Initialize();
  m_I2S_In.StartDevice();
  m_BT_Out.RegisterForConnectionStateChangedCallBack(this);
  m_BT_Out.RegisterForActiveDeviceUpdate(this);
  RegisterForDataItemCallBacks();
  if( xTaskCreatePinnedToCore( Static_TaskLoop_20mS, "Manager_20mS_Task", 10000, this, THREAD_PRIORITY_MEDIUM, &m_Manager_20mS_Task, 1 ) != pdTRUE )
  {
    ESP_LOGE("Setup", "Error creating task!");
  }
}

void Manager::StartBluetooth()
{
  ESP_LOGI("Manager::ConnectToTargetDevice", "Starting Bluetooth!" );
  m_BT_Out.StartDevice( "", "" );
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
void Manager::BluetoothConnectionStateChanged(const esp_a2d_connection_state_t ConnectionState)
{
  ESP_LOGI("Manager: BluetoothConnectionStatusChanged", "Connection Status Changed to %i", ConnectionState );
  /*
  ConnectionStatus_t currentValue;
  m_ConnectionStatus.GetValue(&currentValue, 1);
  if(currentValue != ConnectionStatus)
  {
    m_ConnectionStatus.SetValue(&ConnectionStatus, 1);
    ESP_LOGI("Manager: BluetoothConnectionStatusChanged", "Connection Status Changed to %s", String(ConnectionStatusStrings[ConnectionStatus]).c_str());
  }
  */
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

void Manager::RegisterForDataItemCallBacks()
{
  m_OuputSourceConnect_CallbackArgs = {this};
  m_OuputSourceConnect_Callback = { m_OuputSourceConnect.GetName().c_str(), &OuputSourceConnect_ValueChanged, & m_OuputSourceConnect_CallbackArgs};
  m_OuputSourceConnect.RegisterNamedCallback(&m_OuputSourceConnect_Callback);
  
  m_OuputSourceDisconnect_CallbackArgs = {&m_BT_Out};
  m_OuputSourceDisconnect_Callback = { m_OuputSourceDisconnect.GetName().c_str(), &OuputSourceDisconnect_ValueChanged, & m_OuputSourceDisconnect_CallbackArgs};
  m_OuputSourceDisconnect.RegisterNamedCallback(&m_OuputSourceDisconnect_Callback);
  
  m_BluetoothSourceEnable_CallbackArgs = {&m_BT_Out};
  m_BluetoothSourceEnable_Callback = {m_BluetoothSourceEnable.GetName().c_str(), &BluetoothSourceEnable_ValueChanged, &m_BluetoothSourceEnable_CallbackArgs};
  m_BluetoothSourceEnable.RegisterNamedCallback(&m_BluetoothSourceEnable_Callback);

  m_BluetoothSourceAutoReConnect_CallbackArgs = {&m_BT_Out};
  m_BluetoothSourceAutoReConnect_Callback = {m_BluetoothSourceAutoReConnect.GetName().c_str(), &BluetoothSourceAutoReConnect_ValueChanged, &m_BluetoothSourceAutoReConnect_CallbackArgs};
  m_BluetoothSourceAutoReConnect.RegisterNamedCallback(&m_BluetoothSourceAutoReConnect_Callback);

  m_BluetoothReset_CallbackArgs = {&m_BT_Out};
  m_BluetoothReset_Callback = {m_BluetoothReset.GetName().c_str(), &BluetoothReset_ValueChanged, &m_BluetoothReset_CallbackArgs};
  m_BluetoothReset.RegisterNamedCallback(&m_BluetoothReset_Callback);

  m_TargetCompatibleDevice_CallbackArgs = {&m_BT_Out};
  m_TargetCompatibleDevice_Callback = {m_TargetCompatibleDevice.GetName().c_str(), &TargetCompatibleDevice_ValueChanged, &m_TargetCompatibleDevice_CallbackArgs};
  m_TargetCompatibleDevice.RegisterNamedCallback(&m_TargetCompatibleDevice_Callback);

  m_SoundOutputSource_CallbackArgs = {&m_BT_Out};
  m_SoundOutputSource_Callback = {m_SoundOutputSource.GetName(), &SoundOutputSource_ValueChanged, &m_SoundOutputSource_CallbackArgs};
  m_SoundOutputSource.RegisterNamedCallback(&m_SoundOutputSource_Callback);
}

void Manager::OuputSourceConnect_ValueChanged(const String &Name, void* object, void* arg)
{
  ESP_LOGI("OuputSourceConnect_ValueChanged", "Ouput Source Connect Value Changed ");
  if(arg)
  {
    CallbackArguments* arguments = static_cast<CallbackArguments*>(arg);
    if(arguments->arg1 && object)
    {
      Manager *manager = static_cast<Manager*>(arguments->arg1);
      if(manager)
      {
        manager->StartBluetooth();
      }
    }
  }
}

void Manager::OuputSourceDisconnect_ValueChanged(const String &Name, void* object, void* arg)
{
  ESP_LOGI("OuputSourceDisconnect_ValueChanged", "Ouput Source Disconnect Value Changed ");
  if(arg)
  {
    CallbackArguments* arguments = static_cast<CallbackArguments*>(arg);
    if(arguments->arg1 && object)
    {
      Bluetooth_Source *BT_Out = static_cast<Bluetooth_Source*>(arguments->arg1);
      if(BT_Out)
      {
        BT_Out->Disconnect();
      }
    }
  }
}

void Manager::BluetoothSourceEnable_ValueChanged(const String &Name, void* object, void* arg)
{
  ESP_LOGI("Manager::TargetCompatibleDeviceValueChanged", "Bluetooth Source Enable Value Changed Value Changed");
  CallbackArguments* arguments = static_cast<CallbackArguments*>(arg);
}

void Manager::BluetoothSourceAutoReConnect_ValueChanged(const String &Name, void* object, void* arg)
{
  if(arg)
  {
    CallbackArguments* arguments = static_cast<CallbackArguments*>(arg);
    if(arguments->arg1 && object)
    {
      Bluetooth_Source& BT_Out = *static_cast<Bluetooth_Source*>(arguments->arg1);
      bool autoReconnect = *static_cast<bool*>(object);
      ESP_LOGI("Manager::BluetoothSourceAutoReConnect_ValueChanged", "Bluetooth Source Auto Reconnect Value Changed Value Changed: %i", autoReconnect);
      BT_Out.Set_Auto_Reconnect(autoReconnect);
    }
    else
    {
      ESP_LOGE("BluetoothSourceAutoReConnect_ValueChanged", "Invalid Pointer!");
    }
  }
  else
  {
    ESP_LOGE("BluetoothSourceAutoReConnect_ValueChanged", "Invalid arg Pointer!");
  }
}

void Manager::BluetoothReset_ValueChanged(const String &Name, void* object, void* arg)
{
  if(arg)
  {
    CallbackArguments* arguments = static_cast<CallbackArguments*>(arg);
    if(arguments->arg1 && object)
    {
      Bluetooth_Source& BT_Out = *static_cast<Bluetooth_Source*>(arguments->arg1);
      bool resetBLE = *static_cast<bool*>(object);
      ESP_LOGI("Manager::BluetoothSourceAutoReConnect_ValueChanged", "Bluetooth Source Reset Value Changed Value Changed: %i", resetBLE);
      BT_Out.Set_Reset_BLE(resetBLE);
    }
    else
    {
      ESP_LOGE("BluetoothSourceAutoReConnect_ValueChanged", "Invalid Pointer!");
    }
  }
  else
  {
    ESP_LOGE("BluetoothSourceAutoReConnect_ValueChanged", "Invalid arg Pointer!");
  }
}

void Manager::BluetoothResetNVS_ValueChanged(const String &Name, void* object, void* arg)
{
  if(arg)
  {
    CallbackArguments* arguments = static_cast<CallbackArguments*>(arg);
    if(arguments->arg1 && object)
    {
      Bluetooth_Source& BT_Out = *static_cast<Bluetooth_Source*>(arguments->arg1);
      bool resetNVS = *static_cast<bool*>(object);
      ESP_LOGI("Manager::BluetoothSourceResetNVS_ValueChanged", "Bluetooth Source Reset NVS Value Changed: %i", resetNVS);
      BT_Out.Set_NVS_Init(resetNVS);
    }
    else
    {
      ESP_LOGE("BluetoothSourceResetNVS_ValueChanged", "Invalid Pointer!");
    }
  }
  else
  {
    ESP_LOGE("BluetoothSourceResetNVS_ValueChanged", "Invalid arg Pointer!");
  }
}

void Manager::TargetCompatibleDevice_ValueChanged(const String &Name, void* object, void* arg)
{
  ESP_LOGI("Manager::TargetCompatibleDeviceValueChanged", "Target Compatible Device Value Changed Value Changed");
  CallbackArguments* arguments = static_cast<CallbackArguments*>(arg);
  CompatibleDevice_t* targetCompatibleDevice = static_cast<CompatibleDevice_t*>(object);
  Bluetooth_Source* BT_Out = static_cast<Bluetooth_Source*>(arguments->arg1);
  if(BT_Out && targetCompatibleDevice)
  {
    BT_Out->SetNameToConnect(targetCompatibleDevice->name, targetCompatibleDevice->address);
  }
  else
  {
    ESP_LOGE("Manager::TargetCompatibleDeviceValueChanged", "Invalid Pointer!");
  }
}

void Manager::SoundOutputSource_ValueChanged(const String &Name, void* object, void* arg)
{
  ESP_LOGI("Manager::SoundOutputSourceValueChanged", "Sound Output Source Value Changed");
}
