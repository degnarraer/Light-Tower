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
                , Bluetooth_Sink &bluetooth_Sink
                , I2S_Device &microphone
                , I2S_Device &i2S_Out )
                : NamedItem(Title)
                , m_StatisticalEngine(StatisticalEngine)
                , m_Bluetooth_Sink(bluetooth_Sink)
                , m_Microphone(microphone)
                , m_I2S_Out(i2S_Out)
{
}
Manager::~Manager()
{
  if(m_Manager_10mS_TaskHandle)
  {
    vTaskDelete(m_Manager_10mS_TaskHandle);
    m_Manager_10mS_TaskHandle = nullptr;
  }
  if(m_Manager_10mS_TaskHandle)
  {
    vTaskDelete(m_Manager_1000mS_TaskHandle);
    m_Manager_1000mS_TaskHandle = nullptr;
  }
  if(m_Manager_300000mS_TaskHandle)
  {
    vTaskDelete(m_Manager_300000mS_TaskHandle);
    m_Manager_300000mS_TaskHandle = nullptr;
  }
}

void Manager::Setup()
{
  m_PreferencesWrapper.Setup();
  SetupDevices();
  SetupSerialPortManager();
  SetupAllSetupCallees();
  SetupStatisticalEngine();
  SetupTasks();
}

void Manager::SetupDevices()
{
  m_Bluetooth_Sink.ResgisterForCallbacks(this);
  m_Bluetooth_Sink.Setup();

  m_Microphone.SetDataReceivedCallback(this);
  m_Microphone.Setup();

  m_I2S_Out.SetDataReceivedCallback(this);
  m_I2S_Out.Setup();
}

void Manager::SetupSerialPortManager()
{
  m_CPU1SerialPortMessageManager.Setup();
  m_CPU3SerialPortMessageManager.Setup();
}

void Manager::SetupStatisticalEngine()
{
  m_StatisticalEngine.RegisterForSoundStateChangeNotification(this);
}

void Manager::SetupTasks()
{
  if(xTaskCreatePinnedToCore( Static_Manager_10mS_TaskLoop,     "Manager_10mS_Task",      10000,  NULL,   THREAD_PRIORITY_HIGH,  &m_Manager_10mS_TaskHandle,     0 ) != pdTRUE )
  {
    ESP_LOGE("Setup", "ERROR! Unable to create task.");
  }
  if(xTaskCreatePinnedToCore( Static_Manager_1000mS_TaskLoop,   "Manager_1000mS_rTask",   10000,  NULL,   THREAD_PRIORITY_HIGH,  &m_Manager_1000mS_TaskHandle,   0 ) != pdTRUE )
  {
    ESP_LOGE("Setup", "ERROR! Unable to create task.");
  }
  if(xTaskCreatePinnedToCore( Static_Manager_300000mS_TaskLoop, "Manager_300000mS_Task",  10000,  NULL,   THREAD_PRIORITY_HIGH,  &m_Manager_300000mS_TaskHandle, 0 ) != pdTRUE )
  {
    ESP_LOGE("Setup", "ERROR! Unable to create task.");
  }
}

void Manager::SoundStateChange(SoundState_t SoundState)
{
}

void Manager::Static_Manager_10mS_TaskLoop(void * parameter)
{
  Manager* manager = static_cast<Manager*>(parameter);
  manager->ProcessEventQueue10mS();
}
void Manager::ProcessEventQueue10mS()
{
  const TickType_t xFrequency = 10;
  TickType_t xLastWakeTime = xTaskGetTickCount();
  while(true)
  {
    vTaskDelayUntil( &xLastWakeTime, xFrequency );
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
      m_Bluetooth_Sink.StopDevice();
      m_I2S_Out.StartDevice();
      m_Microphone.StartDevice();
    break;
    case SoundInputSource_t::Bluetooth:
    {
      ESP_LOGI("Manager::SetInputType", "Setting Sound Input Type to \"Bluetooth.\"");
      m_Microphone.StopDevice();
      m_I2S_Out.StartDevice();
      m_Bluetooth_Sink.StartDevice();
      m_Bluetooth_Sink.Connect(m_SinkName.GetValuePointer(), m_SinkAutoReConnect.GetValue());
    }
    break;
    case SoundInputSource_t::OFF:
    default:
      ESP_LOGI("Manager::SetInputType", "Setting Sound Input Type to \"OFF.\"");
      m_Bluetooth_Sink.StopDevice();
      m_Microphone.StopDevice();
      m_I2S_Out.StopDevice();
    break;
  }
}

//I2S_Device_Callback
void Manager::I2SDataReceived(String DeviceTitle, uint8_t *data, uint32_t length)
{
  ESP_LOGV("I2SDataReceived", "I2S Data: %i bytes received.", length);
  switch(m_SoundInputSource.GetValue())
  {
    case SoundInputSource_t::Microphone:
    {
      size_t newBufferLength = 0;
      uint8_t *newbuffer = m_I2S_Out.ConvertBitDepth( data 
                                                    , length
                                                    , m_Microphone.GetBitDepth()
                                                    , m_I2S_Out.GetBitDepth()
                                                    , newBufferLength );
      m_I2S_Out.WriteSoundBufferData(newbuffer, newBufferLength);
      free(newbuffer);
    }
    break;
    case SoundInputSource_t::Bluetooth:
    {
      //m_I2S_Out.WriteSoundBufferData((uint8_t *)data, length);
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
void Manager::BluetoothConnectionStateChanged(const esp_a2d_connection_state_t connectionState, void* object)
{
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
      ESP_LOGW("BluetoothConnectionStatusChanged", "WARNING! Unhandled Connection State Change. Changed to Disconnected.");
      m_BluetoothSinkConnectionStatus.SetValue(ConnectionStatus_t::Unknown);
    break;
  }
}