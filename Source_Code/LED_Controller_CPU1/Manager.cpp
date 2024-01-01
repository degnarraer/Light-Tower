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
}

void Manager::Setup()
{
  InitializePreferences();
  m_CPU1SerialPortMessageManager.SetupSerialPortMessageManager();
  m_CPU3SerialPortMessageManager.SetupSerialPortMessageManager();
  //Set Bluetooth Power to Max
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT, ESP_PWR_LVL_P9);
  m_BT_In.Setup();
  m_BT_In.RegisterForConnectionStatusChangedCallBack(this);
  m_Mic_In.Setup();
  m_I2S_Out.Setup();
  m_Mic_In.SetCallback(this);
  m_StatisticalEngine.RegisterForSoundStateChangeNotification(this);
  //SetInputType(InputType_Bluetooth);
  SetInputType(InputType_Microphone);
}


void Manager::InitializePreferences()
{
  m_Preferences.begin("Settings", false);
  if(m_Preferences.getBool("Pref_Reset", false)) m_Preferences.clear();
  m_Preferences.putBool("Pref_Reset", false);
}


void Manager::SoundStateChange(SoundState_t SoundState)
{
  //SoundState_RX(SoundState);
}

void Manager::ProcessEventQueue20mS()
{
  MoveDataToStatisticalEngine();
}

void Manager::ProcessEventQueue1000mS()
{
  //SoundState_TX();
}

void Manager::ProcessEventQueue300000mS()
{
}

void Manager::SetInputType(InputType_t Type)
{
  m_InputType = Type;
  switch(m_InputType)
  {
    case InputType_Microphone:
      //m_BT_In.StopDevice();
      //m_Mic_In.StartDevice();
      //m_I2S_Out.StartDevice();
    break;
    case InputType_Bluetooth:
      //m_BT_In.StartDevice(m_SinkSSID.c_str(), m_SinkReConnect);
      //m_Mic_In.StopDevice();
      //m_I2S_Out.StopDevice();
    break;
    default:
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
  switch(m_InputType)
  {
    case InputType_Microphone:
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
    case InputType_Bluetooth:
    {
      m_I2S_Out.WriteSoundBufferData((uint8_t *)data, length);
    }
    break;
    default:
    break;
  }
}

void Manager::MoveDataToStatisticalEngine()
{
}

//BluetoothConnectionStateCallee Callback
void Manager::BluetoothConnectionStatusChanged(ConnectionStatus_t ConnectionStatus)
{
  if(m_BluetoothConnectionStatus.GetValue() != ConnectionStatus)
  {
    m_BluetoothConnectionStatus.SetValue(ConnectionStatus);
    switch(ConnectionStatus)
    {
      case ConnectionStatus_t::Disconnected:
        Serial << "Bluetooth Connection Status: Disconnected\n";
      break;
      case ConnectionStatus_t::Searching:
        Serial << "Bluetooth Connection Status: Searching\n";
      break;
      case ConnectionStatus_t::Waiting:
         Serial << "Bluetooth Connection Status: Waiting\n";
      break;
      case ConnectionStatus_t::Pairing:
         Serial << "Bluetooth Connection Status: Pairing\n";
      break;
      case ConnectionStatus_t::Paired:
         Serial << "Bluetooth Connection Status: Paired\n";
      break;
    }
  }
}

  /*
void Manager::SinkSSID_RX()
{
  String DatalinkValue;
  char Buffer[m_SPIDataLinkSlave.GetQueueByteCountForDataItem("Sink SSID")];
  static bool SinkSSIDPullErrorHasOccured = false;
  if(true == m_SPIDataLinkSlave.GetValueFromRXQueue(&Buffer, "Sink SSID", false, 0, SinkSSIDPullErrorHasOccured))
  {
    DatalinkValue = String(Buffer);
    m_SinkSSID = m_Preferences.getString("Sink SSID", "LED Tower of Power").c_str();
    Serial << "RX Datalink Value: " << DatalinkValue.c_str() << "\n";
    Serial << "RX NVMValue Value: " << m_SinkSSID.c_str() << "\n";
    if(!m_SinkSSID.equals(DatalinkValue))
    {
      Serial << "Sink SSID Value Changed\n";
      m_SinkSSID = DatalinkValue;
      m_Preferences.putString("Sink SSID", m_SinkSSID);
      m_BT_In.StartDevice(m_SinkSSID.c_str(), m_SinkReConnect);
      SinkSSID_TX();
    }
  }
}
  */

    
