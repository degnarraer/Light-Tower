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

Manager::Manager( std::string Title
                , Sound_Processor &SoundProcessor
                , SerialPortMessageManager &CPU1SerialPortMessageManager
                , SerialPortMessageManager &CPU3SerialPortMessageManager
                , Bluetooth_Source &BT_Out
                , I2S_Device &I2S_In
                , FFT_Computer &r_FFT
                , ContinuousAudioBuffer<AMPLITUDE_AUDIO_BUFFER_SIZE> &Amplitude_AudioBuffer
                , IPreferences& preferencesInterface)
                : NamedItem(Title)
                , m_PreferencesInterface(preferencesInterface)
                , m_CPU1SerialPortMessageManager(CPU1SerialPortMessageManager)
                , m_CPU3SerialPortMessageManager(CPU3SerialPortMessageManager)
                , m_SoundProcessor(SoundProcessor)
                , m_FFT_Computer(r_FFT)
                , m_Amplitude_AudioBuffer(Amplitude_AudioBuffer)
                , m_I2S_In(I2S_In)
                , m_BT_Out(BT_Out)
{
}
Manager::~Manager()
{
}

void Manager::Setup()
{
  m_BT_Out.ResgisterForCallbacks(this);
  SetupAllSetupCallees();
}

void Manager::StartBluetooth()
{
  ESP_LOGI("StartBluetooth", "Starting Bluetooth!" );
  m_BT_Out.Set_Auto_Reconnect(m_BluetoothSourceAutoReConnect.GetValue());
  m_BT_Out.Set_Reset_BLE(m_BluetoothReset.GetValue());
  m_I2S_In.StartDevice();
  m_BT_Out.StartDevice();
}

void Manager::StopBluetooth()
{
  ESP_LOGI("StopBluetooth", "Stopping Bluetooth!" );
  m_BT_Out.StopDevice();
  m_I2S_In.StopDevice();
}

//I2S_Device_Callback
void Manager::I2SDataReceived(String DeviceTitle, uint8_t *Data, uint32_t channel_len)
{
  ESP_LOGV("I2SDataReceived", "I2S Data: %i bytes received.", channel_len);
}

//Bluetooth Source Callback
int32_t Manager::SetBTTxData(uint8_t *Data, int32_t channel_len)
{
  ESP_LOGV("SetBTTxData", "BT Tx Data: %i bytes requested.", channel_len);
  size_t ByteReceived = m_I2S_In.ReadSoundBufferData(Data, channel_len);
  ESP_LOGV("SetBTTxData", "BT Tx Data: %i bytes received.", ByteReceived);
  size_t FrameCount = ByteReceived / ( sizeof(Frame_t) );
  m_FFT_Computer.PushFrames((Frame_t*)Data, FrameCount);
  //m_Amplitude_AudioBuffer.WriteAudioFrames((Frame_t*)Data, FrameCount);
  return ByteReceived;
}

void Manager::Discovery_Mode_Changed(esp_bt_gap_discovery_state_t discoveryMode)
{
  switch(discoveryMode)
  {
    case ESP_BT_GAP_DISCOVERY_STOPPED:
      ESP_LOGI("DiscoveryModeChanged", "Discovery Mode Stopped");
      m_Bluetooth_Discovery_Mode_t.SetValue(Bluetooth_Discovery_Mode_t::Discovery_Mode_Stopped);
    break;
    case ESP_BT_GAP_DISCOVERY_STARTED:
      ESP_LOGI("DiscoveryModeChanged", "Discovery Mode Started");
      m_Bluetooth_Discovery_Mode_t.SetValue(Bluetooth_Discovery_Mode_t::Discovery_Mode_Started);
    break;
    default:
      ESP_LOGI("DiscoveryModeChanged", "Unknown Discovery Mode");
      m_Bluetooth_Discovery_Mode_t.SetValue(Bluetooth_Discovery_Mode_t::Discovery_Mode_Unknown);
  }
}

void Manager::BluetoothConnectionStateChanged(const esp_a2d_connection_state_t connectionState, void* object)
{
  switch(connectionState)
  {
    case ESP_A2D_CONNECTION_STATE_DISCONNECTED:
      //ESP_LOGI("BluetoothConnectionStatusChanged", "Connection State Changed to Disconnected.");
      m_ConnectionStatus.SetValue(ConnectionStatus_t::Disconnected);
      break;
    case ESP_A2D_CONNECTION_STATE_CONNECTING:
      //ESP_LOGI("BluetoothConnectionStatusChanged", "Connection State Changed to Connecting.");
      m_ConnectionStatus.SetValue(ConnectionStatus_t::Connecting);
      break;
    case ESP_A2D_CONNECTION_STATE_CONNECTED:
      //ESP_LOGI("BluetoothConnectionStatusChanged", "Connection State Changed to Connected.");
      m_ConnectionStatus.SetValue(ConnectionStatus_t::Connected);
      break;
    case ESP_A2D_CONNECTION_STATE_DISCONNECTING:
      //ESP_LOGI("BluetoothConnectionStatusChanged", "Connection State Changed to Disconnecting.");
      m_ConnectionStatus.SetValue(ConnectionStatus_t::Disconnecting);
      break;
    default:
      //ESP_LOGW("BluetoothConnectionStatusChanged", "WARNING! Unhandled Connection State Change. Changed to Disconnected.");
      m_ConnectionStatus.SetValue(ConnectionStatus_t::Unknown);
    break;
  }
}

void Manager::BluetoothActiveDeviceListUpdated(const std::vector<ActiveBluetoothDevice_t> Devices)
{
  unsigned long currentMillis = millis();
  for(int i = 0; i < Devices.size(); ++i)
  {
    unsigned long elapsedTime;
    unsigned long previousMillis = Devices[i].lastUpdateTime;
    if (currentMillis >= previousMillis)
    { 
      elapsedTime = currentMillis - previousMillis;
    } 
    else
    { 
      elapsedTime = (ULONG_MAX - previousMillis) + currentMillis + 1;
    }
    ESP_LOGD("BluetoothActiveDeviceListUpdated", "Device: %i Name: \"%s\" Address: \"%s\"  RSSI: \"%i\" Last Update Time: \"%i\""
            , i
            , Devices[i].name
            , Devices[i].address
            , Devices[i].rssi 
            , elapsedTime );
    BT_Device_Info_With_Time_Since_Update ActiveDevice = BT_Device_Info_With_Time_Since_Update( Devices[i], elapsedTime );
    m_ScannedDevice.SetValue(ActiveDevice);                                          
  }
}








