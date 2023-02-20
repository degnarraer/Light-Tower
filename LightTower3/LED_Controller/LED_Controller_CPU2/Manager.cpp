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
                , SPIDataLinkToCPU1 &SPIDataLinkToCPU1
                , SPIDataLinkToCPU3 &SPIDataLinkToCPU3
                , Bluetooth_Source &BT_Out
                , I2S_Device &I2S_In
                , ContinuousAudioBuffer<AUDIO_BUFFER_SIZE> &AudioBuffer )
                : NamedItem(Title)
                , m_SoundProcessor(SoundProcessor)
                , m_SPIDataLinkToCPU1(SPIDataLinkToCPU1)
                , m_SPIDataLinkToCPU3(SPIDataLinkToCPU3)
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
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT, ESP_PWR_LVL_P9); //Set Bluetooth Power to Max
  m_SoundProcessor.SetupSoundProcessor();
  m_AudioBuffer.Initialize();
  m_I2S_In.StartDevice();
  m_BT_Out.StartDevice( m_Preferences.getString("Source SSID", "JBL Flip 6").c_str()
                      , m_Preferences.getBool("Source BT Reset", false)
                      , m_Preferences.getBool("Source ReConnect", false)
                      , m_Preferences.getBool("SSP Enabled", false) );
  LoadFromNVM();
}

void Manager::InitializeNVM(bool Reset)
{
  if(true == Reset || false == m_Preferences.getBool("NVM Initialized", false))
  {
    if(true == Reset) m_Preferences.clear();   
    m_Preferences.putString("Source SSID", "JBL Flip 6");
    m_Preferences.putBool("Source BT Reset", true);
    m_Preferences.putBool("Source ReConnect", true);
    m_Preferences.putBool("SSP Enabled", false);
    m_Preferences.putBool("NVM Initialized", true);
    m_Preferences.putBool("NVM Reset", false);
  }
}

void Manager::SaveToNVM()
{
  m_Preferences.putFloat("Amplitude Gain", m_AmplitudeGain);
  m_Preferences.putFloat("FFT Gain", m_FFTGain);
  m_Preferences.putBool("Source BT Reset", m_SourceBTReset);
  m_Preferences.putBool("Source ReConnect", m_SourceReConnect);
  m_Preferences.putBool("SSP Enabled", false);
}

void Manager::LoadFromNVM()
{
  m_AmplitudeGain = m_Preferences.getFloat("Amplitude Gain", 1.0);
  m_SoundProcessor.SetGain(m_AmplitudeGain);
  m_FFTGain = m_Preferences.getFloat("FFT Gain", 1.0);
  m_SoundProcessor.SetFFTGain(m_FFTGain);
  m_SourceBTReset = m_Preferences.getBool("Source BT Reset", m_SourceBTReset);
  m_Preferences.putBool("Source ReConnect", m_SourceReConnect);
}
void Manager::ProcessEventQueue20mS()
{
  m_I2S_In.ProcessEventQueue();
  MoveDataBetweenCPU1AndCPU3();
  AmplitudeGain_RX();
  FFTGain_RX();
  SourceBluetoothReset_RX();
  SourceAutoReConnect_RX();
  SourceSSID_RX();
}

void Manager::ProcessEventQueue1000mS()
{
  AmplitudeGain_TX();
  FFTGain_TX();
  SourceBluetoothReset_TX();
  SourceAutoReConnect_TX();
  SourceSSID_TX();
}

void Manager::ProcessEventQueue300000mS()
{
  ESP_LOGI("Manager", "Saving Settigns to NVM");
  SaveToNVM();
}

void Manager::MoveDataBetweenCPU1AndCPU3()
{
  struct Signal
  {
    String Name;
    bool A_To_B;
    bool B_To_A;
  };
  const uint8_t count = 5;
  Signal Signals[count] = { { "Sound State", true, false }
                          , { "Sink Connected", true, false } 
                          , { "Sink ReConnect", true, true } 
                          , { "Sink BT Reset", true, true } 
                          , { "Sink SSID", true, true }  };     
  for(int i = 0; i < count; ++i)
  {
    if(Signals[i].A_To_B)
    {
      MoveDataFromQueueToQueue( "MoveDataBetweenCPU1AndCPU3: " + Signals[i].Name
                              , m_SPIDataLinkToCPU1.GetQueueHandleRXForDataItem(Signals[i].Name.c_str())
                              , m_SPIDataLinkToCPU3.GetQueueHandleTXForDataItem(Signals[i].Name.c_str())
                              , m_SPIDataLinkToCPU1.GetTotalByteCountForDataItem(Signals[i].Name.c_str())
                              , 0
                              , false );
    }
    if(Signals[i].B_To_A)
    {
      MoveDataFromQueueToQueue( "MoveDataBetweenCPU3AndCPU1: " + Signals[i].Name
                              , m_SPIDataLinkToCPU3.GetQueueHandleRXForDataItem(Signals[i].Name.c_str())
                              , m_SPIDataLinkToCPU1.GetQueueHandleTXForDataItem(Signals[i].Name.c_str())
                              , m_SPIDataLinkToCPU3.GetTotalByteCountForDataItem(Signals[i].Name.c_str())
                              , 0
                              , false ); 
    }
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

void Manager::AmplitudeGain_RX()
{
  float DatalinkValue;
  static bool AmplitudeGainPullErrorHasOccured = false;
  if(true == m_SPIDataLinkToCPU3.GetValueFromRXQueue(&DatalinkValue, "Amplitude Gain", false, 0, AmplitudeGainPullErrorHasOccured))
  {
    if(false == AreEqual(DatalinkValue, m_SoundProcessor.GetGain()))
    {
      m_AmplitudeGain = DatalinkValue;
      Serial << "Amplitude Gain Value Value Changed: " << m_AmplitudeGain << "\n";
      m_SoundProcessor.SetGain(m_AmplitudeGain);
      AmplitudeGain_TX();
    }
  }
}
void Manager::AmplitudeGain_TX()
{
  m_AmplitudeGain = m_SoundProcessor.GetGain();
  static bool AmplitudeGainPushErrorHasOccured = false;
  m_SPIDataLinkToCPU3.PushValueToTXQueue(&m_AmplitudeGain, "Amplitude Gain", 0, AmplitudeGainPushErrorHasOccured);
}

void Manager::FFTGain_RX()
{
  float DatalinkValue;
  static bool FFTGainPullErrorHasOccured = false;
  if(true == m_SPIDataLinkToCPU3.GetValueFromRXQueue(&DatalinkValue, "FFT Gain", false, 0, FFTGainPullErrorHasOccured))
  {
    if(false == AreEqual(DatalinkValue, m_SoundProcessor.GetFFTGain()))
    {
      m_FFTGain = DatalinkValue;
      Serial << "FFT Gain Value Value Changed: " << m_FFTGain << "\n";
      m_SoundProcessor.SetFFTGain(m_FFTGain);
      FFTGain_TX();
    }
  }
}
void Manager::FFTGain_TX()
{
  m_FFTGain = m_SoundProcessor.GetFFTGain();
  static bool FFTGainPushErrorHasOccured = false;
  m_SPIDataLinkToCPU3.PushValueToTXQueue(&m_FFTGain, "FFT Gain", 0, FFTGainPushErrorHasOccured);
}

void Manager::SourceBluetoothReset_RX()
{
  bool NVMValue = m_Preferences.getBool("Source BT Reset", true);
  bool DatalinkValue;
  static bool ResetBluetoothPullErrorHasOccured = false;
  if(true == m_SPIDataLinkToCPU3.GetValueFromRXQueue(&DatalinkValue, "Source BT Reset", false, 0, ResetBluetoothPullErrorHasOccured))
  {
    if(NVMValue != DatalinkValue)
    {
      Serial << "Source BT Reset Value Changed\n";
      m_Preferences.putBool("Source BT Reset", DatalinkValue);
      SourceBluetoothReset_TX();
    }
  }
}
void Manager::SourceBluetoothReset_TX()
{
  bool NVMValue = m_Preferences.getBool("Source BT Reset", true);
  static bool ResetBluetoothPushErrorHasOccured = false;
  m_SPIDataLinkToCPU3.PushValueToTXQueue(&NVMValue, "Source BT Reset", 0, ResetBluetoothPushErrorHasOccured);
}

void Manager::SourceAutoReConnect_RX()
{
  bool NVMValue = m_Preferences.getBool("Source ReConnect", true);
  bool DatalinkValue;
  static bool AutoReConnectPullErrorHasOccured = false;
  if(true == m_SPIDataLinkToCPU3.GetValueFromRXQueue(&DatalinkValue, "Source ReConnect", false, 0, AutoReConnectPullErrorHasOccured))
  {
    if(NVMValue != DatalinkValue)
    {
      Serial << "Source ReConnect Value Changed\n";
      m_Preferences.putBool("Source ReConnect", DatalinkValue);
      SourceAutoReConnect_TX();
    }
  }
}

void Manager::SourceAutoReConnect_TX()
{
  bool NVMValue = m_Preferences.getBool("Source ReConnect", true);
  static bool AutoReConnectPushErrorHasOccured = false;
  m_SPIDataLinkToCPU3.PushValueToTXQueue(&NVMValue, "Source ReConnect", 0, AutoReConnectPushErrorHasOccured);
}

void Manager::SourceSSID_RX()
{
  String DatalinkValue;
  static bool MySSIDPullErrorHasOccured = false;
  if(true == m_SPIDataLinkToCPU3.GetValueFromRXQueue(&DatalinkValue, "Sink SSID", false, 0, MySSIDPullErrorHasOccured))
  {
    m_SourceSSID = m_Preferences.getString("Source SSID", "LED Tower of Power").c_str();
    Serial << "RX Datalink Value: " << DatalinkValue.c_str() << "\n";
    Serial << "RX NVMValue Value: " << m_SourceSSID.c_str() << "\n";
    if(!m_SourceSSID.equals(DatalinkValue))
    {
      Serial << "Source SSID Value Changed\n";
      m_SourceSSID = DatalinkValue;
      m_Preferences.putString("Source SSID", m_SourceSSID);
      m_BT_Out.StartDevice( m_Preferences.getString("Source SSID", "JBL Flip 6").c_str()
                          , m_Preferences.getBool("Source BT Reset", false)
                          , m_Preferences.getBool("Source ReConnect", false)
                          , m_Preferences.getBool("SSP Enabled", false) );
      SourceSSID_TX();
    }
  }
}

void Manager::SourceSSID_TX()
{
  m_SourceSSID = m_Preferences.getString("Source SSID", "").c_str();
  Wifi_Info_t WifiInfo = Wifi_Info_t(m_SourceSSID);
  static bool SourceSSIDPushErrorHasOccured = false;
  m_SPIDataLinkToCPU3.PushValueToTXQueue(&WifiInfo, "Source SSID", 0, SourceSSIDPushErrorHasOccured );
  
}
