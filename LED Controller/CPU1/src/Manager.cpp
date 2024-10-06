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
                , StatisticalEngine &StatisticalEngine
                , Bluetooth_Sink &BT_In
                , I2S_Device &Mic_In
                , I2S_Device &I2S_Out )
                : NamedItem(Title)
                , m_StatisticalEngine(StatisticalEngine)
                , m_BT_In(BT_In)
                , m_Mic_In(Mic_In) 
                , m_I2S_Out(I2S_Out)
{
}
Manager::~Manager()
{
  vTaskDelete(m_Manager_20mS_Task);
  vTaskDelete(m_Manager_1000mS_Task);
  vTaskDelete(m_Manager_300000mS_Task);
}

void Manager::Setup()
{
  SetupAllSetupCallees();
  SetupSerialPortManager();
  SetupBlueTooth();
  SetupI2S();
  SetupStatisticalEngine();
  SetupTasks();
}

void Manager::SetupBlueTooth()
{
  //Set Bluetooth Power to Max
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT, ESP_PWR_LVL_P9);
  m_BT_In.Setup();
  m_BT_In.RegisterForConnectionStateChangedCallBack(this);
}

void Manager::SetupSerialPortManager()
{
  m_CPU1SerialPortMessageManager.SetupSerialPortMessageManager();
  m_CPU3SerialPortMessageManager.SetupSerialPortMessageManager();
}

void Manager::SetupI2S()
{
  m_Mic_In.Setup();
  m_I2S_Out.Setup();
  m_Mic_In.SetCallback(this); 
}

void Manager::SetupStatisticalEngine()
{
  m_StatisticalEngine.RegisterForSoundStateChangeNotification(this);
}

void Manager::SetupTasks()
{
  xTaskCreatePinnedToCore( Static_Manager_20mS_TaskLoop,     "Manager_20mS_Task",      5000,  NULL,   configMAX_PRIORITIES - 1,  &m_Manager_20mS_Task,     0 );
  xTaskCreatePinnedToCore( Static_Manager_1000mS_TaskLoop,   "Manager_1000mS_rTask",   5000,  NULL,   configMAX_PRIORITIES - 3,  &m_Manager_1000mS_Task,   0 );
  xTaskCreatePinnedToCore( Static_Manager_300000mS_TaskLoop, "Manager_300000mS_Task",  5000,  NULL,   configMAX_PRIORITIES - 3,  &m_Manager_300000mS_Task, 0 );
}

void Manager::SoundStateChange(SoundState_t SoundState)
{
}

void Manager::Static_Manager_20mS_TaskLoop(void * parameter)
{
  Manager* manager = static_cast<Manager*>(parameter);
  manager->ProcessEventQueue20mS();
}
void Manager::ProcessEventQueue20mS()
{
  const TickType_t xFrequency = 10;
  TickType_t xLastWakeTime = xTaskGetTickCount();
  while(true)
  {
    vTaskDelayUntil( &xLastWakeTime, xFrequency );
    MoveDataToStatisticalEngine();
  }
}

void Manager::Static_Manager_1000mS_TaskLoop(void * parameter)
{
  Manager* manager = static_cast<Manager*>(parameter);
  manager->ProcessEventQueue1000mS();
}
void Manager::ProcessEventQueue1000mS()
{
  
  const TickType_t xFrequency = 1000;
  TickType_t xLastWakeTime = xTaskGetTickCount();
  while(true)
  {
    vTaskDelayUntil( &xLastWakeTime, xFrequency );
  }
}

void Manager::Static_Manager_300000mS_TaskLoop(void * parameter)
{
  Manager* manager = static_cast<Manager*>(parameter);
  manager->ProcessEventQueue300000mS();
}

void Manager::ProcessEventQueue300000mS()
{
  const TickType_t xFrequency = 300000;
  TickType_t xLastWakeTime = xTaskGetTickCount();
  while(true)
  {
    vTaskDelayUntil( &xLastWakeTime, xFrequency );
    
  }
}

void Manager::SetInputSource(SoundInputSource_t Type)
{
  switch(Type)
  {
    case SoundInputSource_t::Microphone:
      ESP_LOGI("Manager::SetInputType", "Setting Sound Input Type to \"Microphone.\"");
      m_BT_In.StopDevice();
      m_Mic_In.StartDevice();
      m_I2S_Out.StartDevice();
    break;
    case SoundInputSource_t::Bluetooth:
    {
      ESP_LOGI("Manager::SetInputType", "Setting Sound Input Type to \"Bluetooth.\"");
      m_Mic_In.StopDevice();
      m_I2S_Out.StopDevice();
      m_BT_In.StartDevice();
      m_BT_In.Connect(m_SinkName.GetValuePointer(), m_SinkAutoReConnect.GetValue());
      break;
    }
    case SoundInputSource_t::OFF:
    default:
      ESP_LOGI("Manager::SetInputType", "Setting Sound Input Type to \"OFF.\"");
      m_BT_In.StopDevice();
      m_Mic_In.StopDevice();
      m_I2S_Out.StopDevice();
    break;
  }
}

//Bluetooth_Callback
void Manager::BTDataReceived(uint8_t *data, uint32_t length)
{
}

//I2S_Device_Callback
void Manager::I2SDataReceived(String DeviceTitle, uint8_t *data, uint32_t length)
{  
  switch(m_SoundInputSource.GetValue())
  {
    case SoundInputSource_t::Microphone:
    {
      uint16_t Buffer[length];
      for(int i = 0; i < length / sizeof(uint32_t); ++i)
      {
        uint32_t Value32 = ((uint32_t*)data)[i];
        uint16_t Value16 = Value32 >> 16;
        Buffer[i] = Value16;
      }
      m_I2S_Out.WriteSoundBufferData((uint8_t *)Buffer, length); 
    }
    break;
    case SoundInputSource_t::Bluetooth:
    {
      m_I2S_Out.WriteSoundBufferData((uint8_t *)data, length);
    }
    break;
    case SoundInputSource_t::OFF:
    default:
    break;
  }
}

void Manager::MoveDataToStatisticalEngine()
{
}

//BluetoothConnectionStateCallee Callback
void Manager::BluetoothConnectionStateChanged(const esp_a2d_connection_state_t connectionState)
{
  ESP_LOGI("Manager: BluetoothConnectionStatusChanged", "Connection State Change Callback");
  switch(connectionState)
  {
    case ESP_A2D_CONNECTION_STATE_DISCONNECTED:
      ESP_LOGI("BluetoothConnectionStatusChanged", "Connection State Changed to Disconnected.");
      m_BluetoothSinkConnectionStatus.SetValue(ConnectionStatus_t::Disconnected);
      break;
    case ESP_A2D_CONNECTION_STATE_CONNECTING:
      ESP_LOGI("BluetoothConnectionStatusChanged", "Connection State Changed to Connecting.");
      m_BluetoothSinkConnectionStatus.SetValue(ConnectionStatus_t::Connecting);
      break;
    case ESP_A2D_CONNECTION_STATE_CONNECTED:
      ESP_LOGI("BluetoothConnectionStatusChanged", "Connection State Changed to Connected.");
      m_BluetoothSinkConnectionStatus.SetValue(ConnectionStatus_t::Connected);
      break;
    case ESP_A2D_CONNECTION_STATE_DISCONNECTING:
      ESP_LOGI("BluetoothConnectionStatusChanged", "Connection State Changed to Disconnecting.");
      m_BluetoothSinkConnectionStatus.SetValue(ConnectionStatus_t::Disconnecting);
      break;
    default:
      ESP_LOGW("BluetoothConnectionStatusChanged", "Unhandled Connection State Change. Changed to Disconnected.");
      m_BluetoothSinkConnectionStatus.SetValue(ConnectionStatus_t::Unknown);
    break;
  }
  
}