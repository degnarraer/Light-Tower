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
  m_Preferences.begin("My Settings", false);
  InitializeNVM(m_Preferences.getBool("NVM Reset", false));
  LoadFromNVM();
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT, ESP_PWR_LVL_P9); //Set Bluetooth Power to Max
  m_SoundProcessor.SetupSoundProcessor();
  m_AudioBuffer.Initialize();
  //m_I2S_In.StartDevice();
  //m_BT_Out.RegisterForConnectionStatusChangedCallBack(this);
  //m_BT_Out.RegisterForActiveDeviceUpdate(this);
  //m_BT_Out.StartDevice( m_SourceSSID.c_str(), m_SourceADDRESS.c_str() );
  m_CPU1SerialPortMessageManager.SetupSerialPortMessageManager();
  m_CPU3SerialPortMessageManager.SetupSerialPortMessageManager();
}

void Manager::InitializeNVM(bool Reset)
{
  if(true == Reset || false == m_Preferences.getBool("NVM Initialized", false))
  {
    if(true == Reset) m_Preferences.clear();
    m_Preferences.putString("Source SSID", "");
    m_Preferences.putString("Source ADDRESS", "");
    m_Preferences.putFloat("Amplitude Gain", 1.0);
    m_Preferences.putFloat("FFT Gain", 1.0);
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
  
  m_AmplitudeGain = m_Preferences.getFloat("Amplitude Gain", 1.0);
  m_SoundProcessor.SetGain(m_AmplitudeGain);
  
  m_FFTGain = m_Preferences.getFloat("FFT Gain", 1.0);
  m_SoundProcessor.SetFFTGain(m_FFTGain);
  
  m_SourceBTReset = m_Preferences.getBool("Source BT Reset", true);
  m_BT_Out.Set_Reset_BLE(m_SourceBTReset);
  
  m_NVSInit = m_Preferences.getBool("BT NVS Init", false);
  m_BT_Out.Set_NVS_Init(m_NVSInit);
  
  m_SourceBTReConnect = m_Preferences.getBool("Src ReConnect", true);
  m_BT_Out.Set_Auto_Reconnect(m_SourceBTReConnect);
  
  m_SSP_Enabled = m_Preferences.getBool("SSP Enabled", false);
  m_BT_Out.Set_SSP_Enabled(m_SSP_Enabled);
}
void Manager::ProcessEventQueue20mS()
{
  m_I2S_In.ProcessEventQueue();
}

void Manager::ProcessEventQueue1000mS()
{
  AmplitudeGain_TX();
  FFTGain_TX();
  BluetoothConnectionStatus_TX();
  SourceBluetoothReset_TX();
  SourceAutoReConnect_TX();
  SourceSSID_TX();
}

void Manager::ProcessEventQueue300000mS()
{
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
  /*
  if(m_BluetoothConnectionStatus != ConnectionStatus)
  {
    m_BluetoothConnectionStatus = ConnectionStatus;
    Serial << "Bluetooth Status Changed: " << m_BluetoothConnectionStatus << "\n";
    BluetoothConnectionStatus_TX();
  }
  */
}

//BluetoothActiveDeviceUpdatee Callback 
void Manager::BluetoothActiveDeviceListUpdated(const std::vector<ActiveCompatibleDevice_t> &Devices)
{
  /*
  for(int i = 0; i < Devices.size(); ++i)
  {  
    SSID_Info_With_LastUpdateTime_t SSID_Info = SSID_Info_With_LastUpdateTime_t(Devices[i].SSID.c_str(), Devices[i].ADDRESS.c_str(), millis()-Devices[i].LastUpdateTime, Devices[i].RSSI);
    Serial << SSID_Info.SSID << " | " << SSID_Info.TimeSinceUdpate << " | " << SSID_Info.RSSI << "\n";
    static bool FoundSpeakerSSIDSValuePushError = false;
    PushValueToQueue( &SSID_Info
                    , m_SPIDataLinkToCPU3.GetQueueHandleTXForDataItem("Found Speaker SSIDS")
                    , "Found Speaker SSIDS"
                    , 0
                    , FoundSpeakerSSIDSValuePushError );
  }
  */
}

void Manager::BluetoothConnectionStatus_TX()
{
  /*
  static bool SourceIsConnectedValuePushError = false;
  PushValueToQueue( &m_BluetoothConnectionStatus
                 , m_SPIDataLinkToCPU3.GetQueueHandleTXForDataItem("Source Connection Status")
                  , "Source Connection Status"
                  , 0
                  , SourceIsConnectedValuePushError );
                  */
}

void Manager::AmplitudeGain_RX()
{
  /*
  float DatalinkValue;
  static bool AmplitudeGainPullErrorHasOccured = false;
  if(true == m_SPIDataLinkToCPU3.GetValueFromRXQueue(&DatalinkValue, "Amplitude Gain", false, 0, AmplitudeGainPullErrorHasOccured))
  {
    if(false == AreEqual(DatalinkValue, m_SoundProcessor.GetGain()))
    {
      m_AmplitudeGain = DatalinkValue;
      Serial << "Amplitude Gain Value Value Changed: " << m_AmplitudeGain << "\n";
      m_SoundProcessor.SetGain(m_AmplitudeGain);
      m_Aplitude_Gain_NVM_Save_Ticker.once(10, Static_Amplitude_Gain_Save, this);
      AmplitudeGain_TX();
    }
  }
  */
}

void Manager::AmplitudeGain_TX()
{
  /*
  m_AmplitudeGain = m_SoundProcessor.GetGain();
  static bool AmplitudeGainPushErrorHasOccured = false;
  m_SPIDataLinkToCPU3.PushValueToTXQueue(&m_AmplitudeGain, "Amplitude Gain", 0, AmplitudeGainPushErrorHasOccured);
  */
}

void Manager::FFTGain_RX()
{
  /*
  float DatalinkValue;
  static bool FFTGainPullErrorHasOccured = false;
  if(true == m_SPIDataLinkToCPU3.GetValueFromRXQueue(&DatalinkValue, "FFT Gain", false, 0, FFTGainPullErrorHasOccured))
  {
    if(false == AreEqual(DatalinkValue, m_SoundProcessor.GetFFTGain()))
    {
      m_FFTGain = DatalinkValue;
      Serial << "FFT Gain Value Value Changed: " << m_FFTGain << "\n";
      m_SoundProcessor.SetFFTGain(m_FFTGain);
      m_FFT_Gain_NVM_Save_Ticker.once(10, Static_FFT_Gain_Save, this);
      FFTGain_TX();
    }
  }
  */
}

void Manager::FFTGain_TX()
{
  /*
  m_FFTGain = m_SoundProcessor.GetFFTGain();
  static bool FFTGainPushErrorHasOccured = false;
  m_SPIDataLinkToCPU3.PushValueToTXQueue(&m_FFTGain, "FFT Gain", 0, FFTGainPushErrorHasOccured);
  */
}

void Manager::SourceBluetoothReset_RX()
{
  /*
  bool DatalinkValue;
  static bool ResetBluetoothPullErrorHasOccured = false;
  if(true == m_SPIDataLinkToCPU3.GetValueFromRXQueue(&DatalinkValue, "Source BT Reset", false, 0, ResetBluetoothPullErrorHasOccured))
  {
    if(m_SourceBTReset != DatalinkValue)
    {
      Serial << "Source BT Reset Value Changed\n";
      m_SourceBTReset = DatalinkValue;
      m_BT_Out.Set_Reset_BLE(m_SourceBTReset);
      m_Preferences.putBool("Source BT Reset", m_SourceBTReset);
      SourceBluetoothReset_TX();
    }
  }
  */
}

void Manager::SourceBluetoothReset_TX()
{
  /*
  static bool ResetBluetoothPushErrorHasOccured = false;
  m_SPIDataLinkToCPU3.PushValueToTXQueue(&m_SourceBTReset, "Source BT Reset", 0, ResetBluetoothPushErrorHasOccured);
  */
}

void Manager::SourceAutoReConnect_RX()
{
  /*
  bool DatalinkValue;
  static bool AutoReConnectPullErrorHasOccured = false;
  if(true == m_SPIDataLinkToCPU3.GetValueFromRXQueue(&DatalinkValue, "Source ReConnect", false, 0, AutoReConnectPullErrorHasOccured))
  {
    if(m_SourceBTReConnect != DatalinkValue)
    {
      Serial << "Source ReConnect Value Changed\n";
      m_SourceBTReConnect = DatalinkValue;
      m_BT_Out.Set_Auto_Reconnect(m_SourceBTReConnect);
      m_Preferences.putBool("Src ReConnect", m_SourceBTReConnect);
      SourceAutoReConnect_TX();
    }
  }
  */
}

void Manager::SourceAutoReConnect_TX()
{
  /*
  static bool AutoReConnectPushErrorHasOccured = false;
  m_SPIDataLinkToCPU3.PushValueToTXQueue(&m_SourceBTReConnect, "Source ReConnect", 0, AutoReConnectPushErrorHasOccured);
  */
}

void Manager::SourceSSID_RX()
{
  /*
  SSID_Info_t DatalinkValue;
  static bool SourceSSIDPullErrorHasOccured = false;
  if(true == m_SPIDataLinkToCPU3.GetValueFromRXQueue(&DatalinkValue, "Sink SSID", false, 0, SourceSSIDPullErrorHasOccured))
  {
    Serial << "RX Datalink SSID Value: " << String(DatalinkValue.SSID) << "\n";
    Serial << "RX NVMValue Value: " << m_SourceSSID.c_str() << "\n";
    Serial << "RX Datalink SSID Value: " << String(DatalinkValue.ADDRESS) << "\n";
    Serial << "RX NVMValue Value: " << m_SourceADDRESS << "\n";
    if(!m_SourceSSID.equals(String(DatalinkValue.SSID)) || !m_SourceADDRESS.equals(String(DatalinkValue.ADDRESS)) )
    {
      Serial << "Source SSID Value Changed\n";
      m_SourceSSID = String(DatalinkValue.SSID);
      m_SourceADDRESS = String(DatalinkValue.ADDRESS);
      m_Preferences.putString("Source SSID", m_SourceSSID);
      m_Preferences.putString("Source ADDRESS", m_SourceADDRESS);
      m_BT_Out.SetSSIDToConnect( m_SourceSSID.c_str(), m_SourceADDRESS.c_str() );
      SourceSSID_TX();
    }
  }
  */
}

void Manager::SourceSSID_TX()
{
  /*
  m_SourceSSID = m_Preferences.getString("Source SSID", "").c_str();
  SSID_Info_t WifiInfo = SSID_Info_t(m_SourceSSID);
  static bool SourceSSIDPushErrorHasOccured = false;
  m_SPIDataLinkToCPU3.PushValueToTXQueue(&WifiInfo, "Source SSID", 0, SourceSSIDPushErrorHasOccured );
  */
}
