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
#ifndef MANAGER_H
#define MANAGER_H

#include <DataTypes.h>
#include <Helpers.h>
#include <I2S_Device.h>
#include <BluetoothA2DPSource.h>
#include "Bluetooth_Device.h"
#include "Sound_Processor.h"
#include "AudioBuffer.h"
#include <Preferences.h>
#include <Ticker.h>
#include "DataItem/DataItems.h"

class Manager: public NamedItem
             , public Bluetooth_Source_Callbacks
             , public I2S_Device_Callback
             , public CommonUtils
             , public QueueController
             , public SetupCallerInterface
{
  public:
    Manager( String Title
           , Sound_Processor &SoundProcessor
           , SerialPortMessageManager &CPU1SerialPortMessageManager
           , SerialPortMessageManager &CPU3SerialPortMessageManager
           , Bluetooth_Source &BT_Out
           , I2S_Device &I2S_Out
           , ContinuousAudioBuffer<AUDIO_BUFFER_SIZE> &AudioBuffer
           , IPreferences& preferencesInterface );
    
    // Delete copy constructor and copy assignment operator
    Manager(const Manager&) = delete;
    Manager& operator=(const Manager&) = delete;
    
    virtual ~Manager();
    void Setup();
    void StartBluetooth();
    void StopBluetooth();
    static void Static_TaskLoop_20mS(void * parameter);
    void TaskLoop_20mS();
    static void Static_TaskLoop_1000mS(void * parameter);
    void TaskLoop_1000mS();
   
    //Bluetooth Callbacks
    void BT_Data_Received()
    {

    }
		void BT_Read_Data_Stream(const uint8_t *data, uint32_t length)
    {
      
    }
		
    int32_t SetBTTxData(uint8_t *Data, int32_t channel_len);
    
    //Bluetooth Discovery Mode Changed Callback
    void Discovery_Mode_Changed(esp_bt_gap_discovery_state_t discoveryMode);
    
    int32_t MusicDataCallback(uint8_t *data, int32_t len)
    {
      return 0;
    }
    void BluetoothConnectionStateChanged(const esp_a2d_connection_state_t ConnectionState, void* object);
    void BluetoothActiveDeviceListUpdated(const std::vector<ActiveBluetoothDevice_t> Devices);
    
    //I2S_Device_Callback
    void I2SDataReceived(String DeviceTitle, uint8_t *Data, uint32_t channel_len);

  private:
    IPreferences& m_PreferencesInterface;
    SerialPortMessageManager &m_CPU1SerialPortMessageManager;
    SerialPortMessageManager &m_CPU3SerialPortMessageManager;
    Sound_Processor &m_SoundProcessor;
    ContinuousAudioBuffer<AUDIO_BUFFER_SIZE> &m_AudioBuffer;
    Frame_t m_AmplitudeFrameBuffer[AMPLITUDE_BUFFER_FRAME_COUNT];
    Frame_t m_FFTFrameBuffer[FFT_SIZE];

    //I2S Sound Data
    I2S_Device &m_I2S_In;
    
    //Bluetooth Data
    Bluetooth_Source &m_BT_Out;

    String ConnectionStatusStrings[4]
    {
      "DISCONNECTED",
      "CONNECTING",
      "CONNECTED",
      "DISCONNECTING"
    };

    struct CallbackArguments 
    {
      void* arg1;
      void* arg2;
      void* arg3;
      void* arg4;

      CallbackArguments(void* a1 = nullptr, void* a2 = nullptr, void* a3 = nullptr, void* a4 = nullptr)
        : arg1(a1), arg2(a2), arg3(a3), arg4(a4) {}
    };

    const ValidStringValues_t validBoolValues = { "0", "1" };

    //Selected Device Device
    CallbackArguments m_Selected_Device_CallbackArgs = {&m_BT_Out};
    NamedCallback_t m_Selected_Device_Callback = {"Target Compatible Device Callback", &Selected_Device_ValueChanged, &m_Selected_Device_CallbackArgs};
    const BluetoothDevice_t m_Selected_Device_InitialValue = {"", ""};
    DataItem<BluetoothDevice_t, 1> m_Selected_Device = DataItem<BluetoothDevice_t, 1>( "Selected_Device", m_Selected_Device_InitialValue, RxTxType_Rx_Echo_Value, 0, &m_CPU3SerialPortMessageManager, &m_Selected_Device_Callback, this);
    static void Selected_Device_ValueChanged(const String &Name, void* object, void* arg)
    {
      ESP_LOGI("TargetCompatibleDeviceValueChanged", "Target Compatible Device Value Changed Value Changed");
      if(arg && object)
      {
        CallbackArguments* arguments = static_cast<CallbackArguments*>(arg);
        if(arguments->arg1)
        {
          BluetoothDevice_t* pTargetCompatibleDevice = static_cast<BluetoothDevice_t*>(object);
          Bluetooth_Source* pBT_Out = static_cast<Bluetooth_Source*>(arguments->arg1);
          if(pBT_Out && pTargetCompatibleDevice)
          {
            pBT_Out->SetNameToConnect(pTargetCompatibleDevice->name, pTargetCompatibleDevice->address);
          }
          else
          {
            ESP_LOGE("TargetCompatibleDevice_ValueChanged", "Error! NULL Pointers.");
          }
        }
        else
        {
          ESP_LOGE("TargetCompatibleDevice_ValueChanged", "Error! NULL Pointers.");
        }
      }
      else
      {
        ESP_LOGE("TargetCompatibleDevice_ValueChanged", "Error! NULL Pointers.");
      }
    }

    //Bluetooth Source Discovery Mode
    Bluetooth_Discovery_Mode_t m_Bluetooth_Discovery_Mode_t_initialValue = Bluetooth_Discovery_Mode_t::Discovery_Mode_Unknown;
    DataItem<Bluetooth_Discovery_Mode_t, 1> m_Bluetooth_Discovery_Mode_t = DataItem<Bluetooth_Discovery_Mode_t, 1>( "Src_Discov_Mode", m_Bluetooth_Discovery_Mode_t_initialValue, RxTxType_Tx_On_Change_With_Heartbeat, 5000, &m_CPU3SerialPortMessageManager, nullptr, this );
    //Bluetooth Source Connection Status
    ConnectionStatus_t m_ConnectionStatus_InitialValue = ConnectionStatus_t::Disconnected;
    DataItem<ConnectionStatus_t, 1> m_ConnectionStatus = DataItem<ConnectionStatus_t, 1>( "Src_Conn_State", m_ConnectionStatus_InitialValue, RxTxType_Tx_On_Change_With_Heartbeat, 5000, &m_CPU3SerialPortMessageManager, nullptr, this );
    
    /*
    //Output Source Start Scan
    CallbackArguments m_SourceStartScan_CallbackArgs = { &m_BT_Out };
    NamedCallback_t m_SourceStartScan_Callback = { "Source StartScan Callback", &SourceStartScan_ValueChanged, &m_SourceStartScan_CallbackArgs};
    const bool m_SourceStartScan_InitialValue = false;
    DataItem<bool, 1> m_SourceStartScan = DataItem<bool, 1>( "Src_Start_Scan", m_SourceStartScan_InitialValue, RxTxType_Rx_Only, 0, &m_CPU3SerialPortMessageManager, &m_SourceStartScan_Callback, this, &validBoolValues);
    static void SourceStartScan_ValueChanged(const String &Name, void* object, void* arg)
    {
      if(arg && object)
      {
        CallbackArguments* arguments = static_cast<CallbackArguments*>(arg);
        if(arguments->arg1)
        {
          Bluetooth_Source *pBT_Out = static_cast<Bluetooth_Source*>(arguments->arg1);
          bool startScan = *static_cast<bool*>(object);
          if(startScan)
          {
            ESP_LOGI("SourceStartScan_ValueChanged", "Start Scanning for Devices.");
            pBT_Out->StartDiscovery();
          }
        }
      }
    }

    //Output Source Stop Scan
    CallbackArguments m_SourceStopScan_CallbackArgs = { &m_BT_Out };
    NamedCallback_t m_SourceStopScan_Callback = { "Source StopScan Callback", &SourceStopScan_ValueChanged, &m_SourceStopScan_CallbackArgs};
    const bool m_SourceStopScan_InitialValue = false;
    DataItem<bool, 1> m_SourceStopScan = DataItem<bool, 1>( "Src_Stop_Scan", m_SourceStopScan_InitialValue, RxTxType_Rx_Only, 0, &m_CPU3SerialPortMessageManager, &m_SourceStopScan_Callback, this, &validBoolValues);
    static void SourceStopScan_ValueChanged(const String &Name, void* object, void* arg)
    {
      if(arg && object)
      {
        CallbackArguments* arguments = static_cast<CallbackArguments*>(arg);
        if(arguments->arg1)
        {
          Bluetooth_Source *pBT_Out = static_cast<Bluetooth_Source*>(arguments->arg1);
          bool startScan = *static_cast<bool*>(object);
          if(startScan)
          {
            ESP_LOGI("SourceStopScan_ValueChanged", "Stop Scanning for Devices.");
            pBT_Out->StopDiscovery();
          }
        }
      }
    }
    */

    //Output Source Connect
    CallbackArguments m_OuputSourceConnect_CallbackArgs = {&m_BT_Out};
    NamedCallback_t m_OuputSourceConnect_Callback = { "Output Source Connect Callback", &OuputSourceConnect_ValueChanged, &m_OuputSourceConnect_CallbackArgs};
    const bool m_OuputSourceConnect_InitialValue = false;
    DataItem<bool, 1> m_OuputSourceConnect = DataItem<bool, 1>( "Src_Connect", m_OuputSourceConnect_InitialValue, RxTxType_Rx_Only, 0, &m_CPU3SerialPortMessageManager, &m_OuputSourceConnect_Callback, this, &validBoolValues);
    static void OuputSourceConnect_ValueChanged(const String &Name, void* object, void* arg)
    {
      if(arg && object)
      {
        CallbackArguments* arguments = static_cast<CallbackArguments*>(arg);
        if(arguments->arg1)
        {
          Bluetooth_Source *pBT_Out = static_cast<Bluetooth_Source*>(arguments->arg1);
          bool connect = *static_cast<bool*>(object);
          if(connect)
          {
            ESP_LOGI("OuputSourceConnect_ValueChanged", "Connect to Target Device: Name: \"%s\" Address: \"%s\"", "", "");
            pBT_Out->Connect();
          }
        }
      }
    }
    
    //Output Source Disconnect
    CallbackArguments m_OuputSourceDisconnect_CallbackArgs = {&m_BT_Out};
    NamedCallback_t m_OuputSourceDisconnect_Callback = { "Output Source DIsconnect Callback", &OuputSourceDisconnect_ValueChanged, &m_OuputSourceDisconnect_CallbackArgs};
    const bool m_OuputSourceDisconnect_InitialValue = false;
    DataItem<bool, 1> m_OuputSourceDisconnect = DataItem<bool, 1>( "Src_Disconnect", m_OuputSourceDisconnect_InitialValue, RxTxType_Rx_Only, 0, &m_CPU3SerialPortMessageManager, &m_OuputSourceDisconnect_Callback, this, &validBoolValues );
    static void OuputSourceDisconnect_ValueChanged(const String &Name, void* object, void* arg)
    {
      if(arg && object)
      {
        CallbackArguments* arguments = static_cast<CallbackArguments*>(arg);
        assert((arguments->arg1) && "Null Pointer!");
        Bluetooth_Source *BT_Out = static_cast<Bluetooth_Source*>(arguments->arg1);
        bool disconnect = *static_cast<bool*>(object);
        if(BT_Out && disconnect)
        {
          ESP_LOGI("OuputSourceDisconnect_ValueChanged", "Blueutooth Disconnecting");
          BT_Out->Disconnect();
        }
      }
    }
    
    //Scanned Device
    BT_Device_Info_With_Time_Since_Update m_ScannedDevice_InitialValue = {"", "", 0, 0, };
    DataItem<BT_Device_Info_With_Time_Since_Update, 1> m_ScannedDevice = DataItem<BT_Device_Info_With_Time_Since_Update, 1>( "Scan_BT_Devices", m_ScannedDevice_InitialValue, RxTxType_Tx_On_Change, 0, &m_CPU3SerialPortMessageManager, NULL, this );
    
    //Bluetooth Source Enable
    CallbackArguments m_BluetoothSourceEnable_CallbackArgs = {&m_BT_Out};
    NamedCallback_t m_BluetoothSourceEnable_Callback = {"Bluetooth Source Enable Callback", &BluetoothSourceEnable_ValueChanged, &m_BluetoothSourceEnable_CallbackArgs};
    const bool m_BluetoothSourceEnable_InitialValue = true;
    DataItemWithPreferences<bool, 1> m_BluetoothSourceEnable = DataItemWithPreferences<bool, 1>( "BT_Source_En", m_BluetoothSourceEnable_InitialValue, RxTxType_Rx_Echo_Value, 0, &m_PreferencesInterface, &m_CPU3SerialPortMessageManager, &m_BluetoothSourceEnable_Callback, this, &validBoolValues );
    static void BluetoothSourceEnable_ValueChanged(const String &Name, void* object, void* arg)
    {
      if(arg && object)
      {
        CallbackArguments* arguments = static_cast<CallbackArguments*>(arg);
        ESP_LOGW("BluetoothSourceEnable_ValueChanged", "Source Enable Not Yet Implemented");
      }
    }

    //Bluetooth Source Auto ReConnect
    CallbackArguments m_BluetoothSourceAutoReConnect_CallbackArgs = {&m_BT_Out};
    NamedCallback_t m_BluetoothSourceAutoReConnect_Callback = {"Bluetooth Source ReConnect Callback", &BluetoothSourceAutoReConnect_ValueChanged, &m_BluetoothSourceAutoReConnect_CallbackArgs};
    const bool m_BluetoothSourceAutoReConnect_InitialValue = false;
    DataItemWithPreferences<bool, 1> m_BluetoothSourceAutoReConnect = DataItemWithPreferences<bool, 1>( "BT_Source_AR", m_BluetoothSourceAutoReConnect_InitialValue, RxTxType_Rx_Echo_Value, 0, &m_PreferencesInterface, &m_CPU3SerialPortMessageManager, &m_BluetoothSourceAutoReConnect_Callback, this, &validBoolValues);
    static void BluetoothSourceAutoReConnect_ValueChanged(const String &Name, void* object, void* arg)
    {
      if(arg && object)
      {
        CallbackArguments* arguments = static_cast<CallbackArguments*>(arg);
        if(arguments->arg1)
        {
          Bluetooth_Source& BT_Out = *static_cast<Bluetooth_Source*>(arguments->arg1);
          bool autoReconnect = *static_cast<bool*>(object);
          ESP_LOGI("BluetoothSourceAutoReConnect_ValueChanged", "Bluetooth Source Auto Reconnect Value Changed Value Changed: %i", autoReconnect);
          BT_Out.Set_Auto_Reconnect(autoReconnect);
        }
        else
        {
          ESP_LOGE("BluetoothSourceAutoReConnect_ValueChanged", "ERROR! Null Pointer!");
        }
      }
    }
    
    //Bluetooth Source Reset
    CallbackArguments m_BluetoothReset_CallbackArgs = {&m_BT_Out, &m_Selected_Device};
    NamedCallback_t m_BluetoothReset_Callback = {"Bluetooth Reset Callback", &BluetoothReset_ValueChanged, &m_BluetoothReset_CallbackArgs};
    const bool m_BluetoothReset_InitialValue = false;
    DataItemWithPreferences<bool, 1> m_BluetoothReset = DataItemWithPreferences<bool, 1>( "BT_Src_Reset", m_BluetoothReset_InitialValue, RxTxType_Rx_Echo_Value, 0, &m_PreferencesInterface, &m_CPU3SerialPortMessageManager, &m_BluetoothReset_Callback, this, &validBoolValues );
    static void BluetoothReset_ValueChanged(const String &Name, void* object, void* arg)
    {
      if(arg && object)
      {
        CallbackArguments* arguments = static_cast<CallbackArguments*>(arg);
        if(arguments->arg1 && arguments->arg2)
        {
          /*
          Bluetooth_Source *pBT_Out = static_cast<Bluetooth_Source*>(arguments->arg1);
          DataItem<BluetoothDevice_t, 1> *pTargetDevice = static_cast<DataItem<BluetoothDevice_t, 1>*>(arguments->arg2);
          const BluetoothDevice_t InitialValue = {"", ""};
          bool resetBLE = *static_cast<bool*>(object);
          if(resetBLE)
          {
            ESP_LOGW("BluetoothReset_ValueChanged", "Bluetooth Source Reset Not Yet Implemented");
            pTargetDevice->ResetToDefaultValue();
            pBT_Out->Disconnect();
            pBT_Out->Set_Reset_BLE(true);
            pBT_Out->Connect(pTargetDevice->GetValuePointer()->name, pTargetDevice->GetValuePointer()->address);
            pBT_Out->Set_Reset_BLE(false);
            
          }
          */
        }
      }
    }

    //Sound Output Source
    const ValidStringValues_t validSoundOutputSourceValues = { "OFF", "Bluetooth" };
    CallbackArguments m_SoundOutputSource_CallbackArgs = {this};
    NamedCallback_t m_SoundOutputSource_Callback = {"Sound Output Source Callback", &SoundOutputSource_ValueChanged, &m_SoundOutputSource_CallbackArgs};
    const SoundOutputSource_t m_SoundOutputSource_InitialValue = SoundOutputSource_t::Bluetooth;
    DataItemWithPreferences<SoundOutputSource_t, 1> m_SoundOutputSource = DataItemWithPreferences<SoundOutputSource_t, 1>( "Output_Source", m_SoundOutputSource_InitialValue, RxTxType_Rx_Echo_Value, 0, &m_PreferencesInterface, &m_CPU3SerialPortMessageManager, &m_SoundOutputSource_Callback, this, &validSoundOutputSourceValues );
    static void SoundOutputSource_ValueChanged(const String &Name, void* object, void* arg)
    {
      if(arg && object)
      {
        CallbackArguments* arguments = static_cast<CallbackArguments*>(arg);
        assert((arguments->arg1) && "Null Pointer!");
        Manager* manager = static_cast<Manager*>(arguments->arg1);
        SoundOutputSource_t& soundOutputSource = *static_cast<SoundOutputSource_t*>(object);
        switch(soundOutputSource)
        {
          case SoundOutputSource_t::OFF:
            ESP_LOGI("Manager::SoundOutputSource_ValueChanged", "Sound Output Source Value Changed: \"OFF\"");
            manager->StopBluetooth();
          break;
          case SoundOutputSource_t::Bluetooth:
            ESP_LOGI("Manager::SoundOutputSource_ValueChanged", "Sound Output Source Value Changed: \"Bluetooth\"");
            manager->StartBluetooth();
          break;
          default:
            manager->StopBluetooth();
          break;
        }
      }
    }

    const float Epsilon = 0.01;
    bool AreEqual(float A, float B)
    {
      return abs(A - B) < Epsilon;
    }
};

#endif
