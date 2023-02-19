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
                , SPIDataLinkSlave &SPIDataLinkSlave
                , Bluetooth_Sink &BT_In
                , I2S_Device &Mic_In
                , I2S_Device &I2S_Out )
                : NamedItem(Title)
                , m_StatisticalEngine(StatisticalEngine)
                , m_SPIDataLinkSlave(SPIDataLinkSlave)
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
  m_Preferences.begin("My Settings", false);
  InitializeNVM(true); //m_Preferences.getBool("NVM Reset", false));
  //Set Bluetooth Power to Max
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT, ESP_PWR_LVL_P9);
  m_BT_In.Setup();
  m_Mic_In.Setup();
  m_I2S_Out.Setup();
  m_Mic_In.SetCallback(this);
  m_StatisticalEngine.RegisterForSoundStateChangeNotification(this);
  SetInputType(InputType_Bluetooth);
  //SetInputType(InputType_Microphone);
}

void Manager::InitializeNVM(bool Reset)
{
  if(true == Reset || false == m_Preferences.getBool("NVM Initialized", false))
  {
    Serial << "Initializing NVM\n";
    if(true == Reset)
    {
      m_Preferences.clear();
      Serial << "NVM Cleared\n";
    }
    m_Preferences.putString("Sink SSID", "LED Tower of Power");
    m_Preferences.putBool("Sink BT Reset", true);
    m_Preferences.putBool("Sink ReConnect", true);
    m_Preferences.putBool("NVM Initialized", true);
    m_Preferences.putBool("NVM Reset", false);
  }
}

void Manager::SaveToNVM()
{
    m_Preferences.putString("Sink SSID", m_SinkSSID);
    m_Preferences.putBool("Sink BT Reset", m_SinkReset);
    m_Preferences.putBool("Sink ReConnect", m_SinkReConnect);
    m_Preferences.putBool("NVM Initialized", true);
    m_Preferences.putBool("NVM Reset", false);
}

void Manager::LoadFromNVM()
{
  m_SinkSSID = m_Preferences.getString("Sink SSID", "LED Tower of Power");
  m_SinkReset = m_Preferences.getBool("Sink Reset", true);
  m_SinkReConnect = m_Preferences.getBool("Sink ReConnect", true);
}

void Manager::SoundStateChange(SoundState_t SoundState)
{
  m_SoundState = SoundState;
  SoundState_TX(m_SoundState);
}

void Manager::ProcessEventQueue20mS()
{
  ProcessI2S_And_BT();
  MoveDataToStatisticalEngine();
  SinkSSID_RX();
}

void Manager::ProcessEventQueue1000mS()
{
  ProcessBluetoothConnectionStatus(true);
  SoundState_TX(m_SoundState);
  SinkSSID_TX();
}

void Manager::SetInputType(InputType_t Type)
{
  m_InputType = Type;
  switch(m_InputType)
  {
    case InputType_Microphone:
      m_BT_In.StopDevice();
      m_Mic_In.StartDevice();
      m_I2S_Out.StartDevice();
    break;
    case InputType_Bluetooth:
      m_BT_In.StartDevice(m_Preferences.getString("Sink SSID", "LED Tower of Power").c_str());
      m_Mic_In.StopDevice();
      m_I2S_Out.StopDevice();
    break;
    default:
      m_BT_In.StopDevice();
      m_Mic_In.StopDevice();
      m_I2S_Out.StopDevice();
    break;
  }
}
void Manager::ProcessI2S_And_BT()
{    
  switch(m_InputType)
  {
    case InputType_Microphone:
      m_Mic_In.ProcessEventQueue();
      m_I2S_Out.ProcessEventQueue();
    break;
    case InputType_Bluetooth:
    {  
      m_I2S_Out.ProcessEventQueue();
      ProcessBluetoothConnectionStatus(false);
    }
    break;
    default:
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
  const uint8_t count = 6;
  String Signals[count] = { "Processed_Frame"
                          , "R_BANDS"
                          , "L_BANDS"
                          , "R_MAXBAND"
                          , "L_MAXBAND"
                          , "L_MAJOR_FREQ" };
                                      
  for(int i = 0; i < count; ++i)
  {
    MoveDataFromQueueToQueue( "Move Data Between CPU1 And Statistical Engine: " + Signals[i]
                            , m_SPIDataLinkSlave.GetQueueHandleRXForDataItem(Signals[i].c_str())
                            , m_StatisticalEngine.GetQueueHandleTXForDataItem(Signals[i].c_str())
                            , m_SPIDataLinkSlave.GetTotalByteCountForDataItem(Signals[i].c_str())
                            , 0
                            , false );
  }
}

void Manager::ProcessBluetoothConnectionStatus(bool ForceUpdate)
{
  if(InputType_Bluetooth == m_InputType)
  {
    bool SendUpdate = false;
    bool IsConnected = m_BT_In.IsConnected();
    if(m_BluetoothIsConnected != IsConnected)
    {
      m_BluetoothIsConnected = IsConnected;
      SendUpdate = true;
      if(true == m_BluetoothIsConnected)
      {
        ESP_LOGI("Manager", "BT Sink Connected!");
      }
      else
      {
        ESP_LOGI("Manager", "BT Sink Disconnected!");
      }
    }
    if(true == ForceUpdate || true == SendUpdate)
    {
      static bool SinkIsConnectedValuePushError = false;
      PushValueToQueue( &m_BluetoothIsConnected
                      , m_SPIDataLinkSlave.GetQueueHandleTXForDataItem("Sink Connected")
                      , 0
                      , "Sink Is Connected"
                      , SinkIsConnectedValuePushError );
    }
  }
}

void Manager::SoundState_TX(SoundState_t SoundState)
{
  if(m_SoundState != SoundState)
  {
      m_SoundState = SoundState;
      static bool SoundStateValuePushError = false;
      PushValueToQueue( &m_SoundState
                      , m_SPIDataLinkSlave.GetQueueHandleTXForDataItem("Sound State")
                      , 0
                      , "Sound State"
                      , SoundStateValuePushError );
  }
  
}

void Manager::SinkSSID_RX()
{
  String DatalinkValue;
  static bool MySSIDPullErrorHasOccured = false;
  if(true == m_SPIDataLinkSlave.GetValueFromRXQueue(&DatalinkValue, "Sink SSID", false, 0, MySSIDPullErrorHasOccured))
  {
    m_SinkSSID = m_Preferences.getString("Sink SSID", "LED Tower of Power").c_str();
    Serial << "RX Datalink Value: " << DatalinkValue.c_str() << "\n";
    Serial << "RX NVMValue Value: " << m_SinkSSID.c_str() << "\n";
    if(!m_SinkSSID.equals(DatalinkValue))
    {
      Serial << "Sink SSID Value Changed\n";
      m_SinkSSID = DatalinkValue;
      m_Preferences.putString("Sink SSID", m_SinkSSID);
      m_BT_In.StartDevice(m_SinkSSID.c_str());
      SinkSSID_TX();
    }
  }
}

void Manager::SinkSSID_TX()
{
  m_SinkSSID = m_Preferences.getString("Sink SSID", "LED Tower of Power").c_str();
  Wifi_Info_t WifiInfo = Wifi_Info_t(m_SinkSSID);
  static bool SinkSSIDPushErrorHasOccured = false;
  Serial << String(WifiInfo.SSID) << "\n";
  m_SPIDataLinkSlave.PushValueToTXQueue(&WifiInfo, "Sink SSID", 0, SinkSSIDPushErrorHasOccured );
}


void Manager::SinkBluetoothReset_RX()
{
  bool DatalinkValue;
  static bool SinkBluetoothResetPullErrorHasOccured = false;
  if(true == m_SPIDataLinkSlave.GetValueFromRXQueue(&DatalinkValue, "Sink BT Reset", false, 0, SinkBluetoothResetPullErrorHasOccured))
  {
    bool NVMValue = m_Preferences.getBool("Sink BT Reset", true);
    if(NVMValue != DatalinkValue)
    {
      Serial << "Sink BT Reset Value Changed\n";
      m_Preferences.putBool("Sink BT Reset", DatalinkValue);
      SinkBluetoothReset_TX();
    }
  }
}
void Manager::SinkBluetoothReset_TX()
{
  bool NVMValue = m_Preferences.getBool("Sink BT Reset", true);
  static bool SinkBluetoothResetPushErrorHasOccured = false;
  m_SPIDataLinkSlave.PushValueToTXQueue(&NVMValue, "Sink BT Reset", 0, SinkBluetoothResetPushErrorHasOccured);
}

void Manager::SinkAutoReConnect_RX()
{
  bool DatalinkValue;
  static bool SinkAutoReConnectPullErrorHasOccured = false;
  if(true == m_SPIDataLinkSlave.GetValueFromRXQueue(&DatalinkValue, "Sink ReConnect", false, 0, SinkAutoReConnectPullErrorHasOccured))
  {
    bool NVMValue = m_Preferences.getBool("Sink ReConnect", true);
    if(NVMValue != DatalinkValue)
    {
      Serial << "Sink ReConnect Value Changed\n";
      m_Preferences.putBool("Sink ReConnect", DatalinkValue);
      SinkAutoReConnect_TX();
    }
  }
}

void Manager::SinkAutoReConnect_TX()
{
  bool NVMValue = m_Preferences.getBool("Sink ReConnect", true);
  static bool SinkAutoReConnectPushErrorHasOccured = false;
  m_SPIDataLinkSlave.PushValueToTXQueue(&NVMValue, "Sink ReConnect", 0, SinkAutoReConnectPushErrorHasOccured);
}
