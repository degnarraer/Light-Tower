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
  m_BT_In.RegisterForConnectionStatusChangedCallBack(this);
  m_Mic_In.Setup();
  m_I2S_Out.Setup();
  m_Mic_In.SetCallback(this);
  m_StatisticalEngine.RegisterForSoundStateChangeNotification(this);
  SetInputType(InputType_Bluetooth);
  //SetInputType(InputType_Microphone);
  LoadFromNVM();
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

    //Close Initialization
    m_Preferences.putBool("NVM Initialized", true);
    m_Preferences.putBool("NVM Reset", false);
  }
}

void Manager::SaveToNVM()
{
    m_Preferences.putString("Sink SSID", m_SinkSSID);
    m_Preferences.putBool("Sink BT Reset", m_SinkReset);
    m_Preferences.putBool("Sink ReConnect", m_SinkReConnect);
}

void Manager::LoadFromNVM()
{
  m_SinkSSID = m_Preferences.getString("Sink SSID", "LED Tower of Power");
  m_SinkReset = m_Preferences.getBool("Sink Reset", true);
  m_SinkReConnect = m_Preferences.getBool("Sink ReConnect", true);
}

void Manager::SoundStateChange(SoundState_t SoundState)
{
  SoundState_RX(SoundState);
}

void Manager::ProcessEventQueue20mS()
{
  Process_I2S_EventQueue();
  MoveDataToStatisticalEngine();
  SinkSSID_RX();
  SinkBluetoothReset_RX();
  SinkAutoReConnect_RX();
}

void Manager::ProcessEventQueue1000mS()
{
  BluetoothConnectionStatus_TX();
  SoundState_TX();
  SinkSSID_TX();
  SinkBluetoothReset_TX();
  SinkReConnect_TX();
}

void Manager::ProcessEventQueue300000mS()
{
  ESP_LOGI("Manager", "Saving Settigns to NVM");
  SaveToNVM();
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
void Manager::Process_I2S_EventQueue()
{    
  if(m_InputType == InputType_Microphone)
  {
    m_Mic_In.ProcessEventQueue();
  }
  m_I2S_Out.ProcessEventQueue();
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
                            , m_StatisticalEngine.GetQueueHandleRXForDataItem(Signals[i].c_str())
                            , m_SPIDataLinkSlave.GetTotalByteCountForDataItem(Signals[i].c_str())
                            , 0
                            , false );
  }
}

void Manager::BluetoothConnectionStatus_TX()
{
  static bool SinkIsConnectedValuePushError = false;
  PushValueToQueue( &m_BluetoothConnectionStatus
                  , m_SPIDataLinkSlave.GetQueueHandleTXForDataItem("Sink Connection Status")
                  , "Sink Connection Status"
                  , 0
                  , SinkIsConnectedValuePushError ); 
}

//BluetoothConnectionStateCallee Callback
void Manager::BluetoothConnectionStatusChanged(ConnectionStatus_t ConnectionStatus)
{
  m_BluetoothConnectionStatus = ConnectionStatus;
  BluetoothConnectionStatus_TX();
}

void Manager::SoundState_RX(SoundState_t SoundState)
{
  if(m_SoundState != SoundState)
  {
      m_SoundState = SoundState;
      SoundState_TX();
  }
  
}
void Manager::SoundState_TX()
{
  static bool SoundStateValuePushError = false;
  PushValueToQueue( &m_SoundState
                  , m_SPIDataLinkSlave.GetQueueHandleTXForDataItem("Sound State")
                  , "Sound State"
                  , 0
                  , SoundStateValuePushError );
}

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
  m_SPIDataLinkSlave.PushValueToTXQueue(&WifiInfo, "Sink SSID", 0, SinkSSIDPushErrorHasOccured );
}


void Manager::SinkBluetoothReset_RX()
{
  bool DatalinkValue;
  static bool SinkBluetoothResetPullErrorHasOccured = false;
  if(true == m_SPIDataLinkSlave.GetValueFromRXQueue(&DatalinkValue, "Sink BT Reset", false, 0, SinkBluetoothResetPullErrorHasOccured))
  {
    if(m_SinkReset != DatalinkValue)
    {
      Serial << "Sink Bluetooth Reset Value Changed\n";
      m_SinkReset = DatalinkValue;
      SinkBluetoothReset_TX();
    }
  }
}
void Manager::SinkBluetoothReset_TX()
{
  static bool SinkBluetoothResetPushErrorHasOccured = false;
  m_SPIDataLinkSlave.PushValueToTXQueue(&m_SinkReset, "Sink BT Reset", 0, SinkBluetoothResetPushErrorHasOccured);
}

void Manager::SinkAutoReConnect_RX()
{
  bool DatalinkValue;
  static bool SinkAutoReConnectPullErrorHasOccured = false;
  if(true == m_SPIDataLinkSlave.GetValueFromRXQueue(&DatalinkValue, "Sink ReConnect", false, 0, SinkAutoReConnectPullErrorHasOccured))
  {
    if(m_SinkReConnect != DatalinkValue)
    {
      Serial << "Sink ReConnect Value Changed\n";
      m_SinkReConnect = DatalinkValue;
      SinkReConnect_TX();
    }
  }
}

void Manager::SinkReConnect_TX()
{
  static bool SinkAutoReConnectPushErrorHasOccured = false;
  m_SPIDataLinkSlave.PushValueToTXQueue(&m_SinkReConnect, "Sink ReConnect", 0, SinkAutoReConnectPushErrorHasOccured);
}
