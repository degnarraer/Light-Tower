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
             , public I2S_Device_Callback
             , public BluetoothConnectionStateCallee
             , public BluetoothActiveDeviceUpdatee
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
   
    //Bluetooth Set Data Callback
    int32_t SetBTTxData(uint8_t *Data, int32_t channel_len);
    
    //I2S_Device_Callback
    void I2SDataReceived(String DeviceTitle, uint8_t *Data, uint32_t channel_len);

    //BluetoothConnectionStateCallee Callback 
    void BluetoothConnectionStateChanged(const esp_a2d_connection_state_t ConnectionState);
    
    //BluetoothActiveDeviceUpdatee Callback 
    void BluetoothActiveDeviceListUpdated(const std::vector<ActiveCompatibleDevice_t> Devices);

  private:
    IPreferences& m_PreferencesInterface;    
    TaskHandle_t m_Manager_20mS_Task;
    TaskHandle_t m_Manager_1000mS_Task;
    TaskHandle_t m_Manager_300000mS_Task;

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
    };
    struct Callback2Arguments 
    {
      void* arg1;
      void* arg2;
    };
    struct Callback3Arguments 
    {
      void* arg1;
      void* arg2;
      void* arg3;
    };

    const ValidStringValues_t validBoolValues = { "0", "1" };
        
    //Bluetooth Source Connection Status
    ConnectionStatus_t m_ConnectionStatus_InitialValue = ConnectionStatus_t::Disconnected;
    DataItem<ConnectionStatus_t, 1> m_ConnectionStatus = DataItem<ConnectionStatus_t, 1>( "Src_Conn_State"
                                                                                        , m_ConnectionStatus_InitialValue
                                                                                        , RxTxType_Tx_On_Change_With_Heartbeat
                                                                                        , 5000
                                                                                        , &m_CPU3SerialPortMessageManager
                                                                                        , nullptr
                                                                                        , this );
    
    //Output Source Connect
    Callback2Arguments m_OuputSourceConnect_CallbackArgs = {&m_BT_Out, &m_TargetCompatibleDevice};
    NamedCallback_t m_OuputSourceConnect_Callback = { "Output Source Connect Callback"
                                                    , &OutputSourceConnect_ValueChanged
                                                    , &m_OuputSourceConnect_CallbackArgs};
    const bool m_OuputSourceConnect_InitialValue = false;
    DataItem<bool, 1> m_OuputSourceConnect = DataItem<bool, 1>( "Src_Connect"
                                                              , m_OuputSourceConnect_InitialValue
                                                              , RxTxType_Rx_Only
                                                              , 0
                                                              , &m_CPU3SerialPortMessageManager
                                                              , &m_OuputSourceConnect_Callback
                                                              , this
                                                              , &validBoolValues);
    static void OutputSourceConnect_ValueChanged(const String &Name, void* object, void* arg)
    {
      ESP_LOGI("OutputSourceConnect_ValueChanged", "Ouput Source Connect Value Changed ");
      if(arg && object)
      {
        Callback2Arguments* arguments = static_cast<Callback2Arguments*>(arg);
        if(arguments->arg1 && arguments->arg2)
        {
          Bluetooth_Source *pBT_Out = static_cast<Bluetooth_Source*>(arguments->arg1);
          CompatibleDevice_t *pTargetDevice = static_cast<CompatibleDevice_t*>(arguments->arg2);
          bool connect = *static_cast<bool*>(object);
          if(pBT_Out && pTargetDevice)
          {
            if(connect)
            {
              ESP_LOGI("OutputSourceConnect_ValueChanged", "Connect to Target Device: Name: \"%s\" Address: \"%s\"", pTargetDevice->name, pTargetDevice->address);
              pBT_Out->Connect("","");
            }
          }
        }
      }
    }
    
    //Output Source Disconnect
    CallbackArguments m_OuputSourceDisconnect_CallbackArgs = {&m_BT_Out};
    NamedCallback_t m_OuputSourceDisconnect_Callback = { "Output Source DIsconnect Callback"
                                                       , &OuputSourceDisconnect_ValueChanged
                                                       , &m_OuputSourceDisconnect_CallbackArgs};
    const bool m_OuputSourceDisconnect_InitialValue = false;
    DataItem<bool, 1> m_OuputSourceDisconnect = DataItem<bool, 1>( "Src_Disconnect"
                                                                 , m_OuputSourceDisconnect_InitialValue
                                                                 , RxTxType_Rx_Only
                                                                 , 0
                                                                 , &m_CPU3SerialPortMessageManager
                                                                 , &m_OuputSourceDisconnect_Callback
                                                                 , this
                                                                 , &validBoolValues );
    static void OuputSourceDisconnect_ValueChanged(const String &Name, void* object, void* arg)
    {
      ESP_LOGI("OuputSourceDisconnect_ValueChanged", "Ouput Source Disconnect Value Changed ");
      if(arg && object)
      {
        CallbackArguments* arguments = static_cast<CallbackArguments*>(arg);
        assert((arguments->arg1) && "Null Pointer!");
        Bluetooth_Source *BT_Out = static_cast<Bluetooth_Source*>(arguments->arg1);
        bool disconnect = *static_cast<bool*>(object);
        if(BT_Out && disconnect)
        {
          BT_Out->Disconnect();
        }
      }
    }
    
    //Scanned Device
    ActiveCompatibleDevice_t m_ScannedDevice_InitialValue = {"", "", 0, 0, 0};
    DataItem<ActiveCompatibleDevice_t, 1> m_ScannedDevice = DataItem<ActiveCompatibleDevice_t, 1>( "Scan_BT_Device"
                                                                                                , m_ScannedDevice_InitialValue
                                                                                                , RxTxType_Tx_On_Change
                                                                                                , 0
                                                                                                , &m_CPU3SerialPortMessageManager
                                                                                                , NULL
                                                                                                , this );
    
    //Bluetooth Source Enable
    CallbackArguments m_BluetoothSourceEnable_CallbackArgs = {&m_BT_Out};
    NamedCallback_t m_BluetoothSourceEnable_Callback = {"Bluetooth Source Enable Callback", &BluetoothSourceEnable_ValueChanged, &m_BluetoothSourceEnable_CallbackArgs};
    const bool m_BluetoothSourceEnable_InitialValue = true;
    DataItemWithPreferences<bool, 1> m_BluetoothSourceEnable = DataItemWithPreferences<bool, 1>( "BT_Source_En"
                                                                                               , m_BluetoothSourceEnable_InitialValue
                                                                                               , RxTxType_Rx_Echo_Value
                                                                                               , 0
                                                                                               , &m_PreferencesInterface
                                                                                               , &m_CPU3SerialPortMessageManager
                                                                                               , &m_BluetoothSourceEnable_Callback
                                                                                               , this
                                                                                               , &validBoolValues );
    static void BluetoothSourceEnable_ValueChanged(const String &Name, void* object, void* arg)
    {
      if(arg && object)
      {
        CallbackArguments* arguments = static_cast<CallbackArguments*>(arg);
        ESP_LOGI("Manager::TargetCompatibleDeviceValueChanged", "Bluetooth Source Enable Value Changed Value Changed");
      }
    }

    //Bluetooth Source Auto ReConnect
    CallbackArguments m_BluetoothSourceAutoReConnect_CallbackArgs = {&m_BT_Out};
    NamedCallback_t m_BluetoothSourceAutoReConnect_Callback = {"Bluetooth Source ReConnect Callback", &BluetoothSourceAutoReConnect_ValueChanged, &m_BluetoothSourceAutoReConnect_CallbackArgs};
    const bool m_BluetoothSourceAutoReConnect_InitialValue = false;
    DataItemWithPreferences<bool, 1> m_BluetoothSourceAutoReConnect = DataItemWithPreferences<bool, 1>( "BT_Source_AR"
                                                                                                      , m_BluetoothSourceAutoReConnect_InitialValue
                                                                                                      , RxTxType_Rx_Echo_Value
                                                                                                      , 0
                                                                                                      , &m_PreferencesInterface
                                                                                                      , &m_CPU3SerialPortMessageManager
                                                                                                      , &m_BluetoothSourceAutoReConnect_Callback
                                                                                                      , this
                                                                                                      , &validBoolValues);
    static void BluetoothSourceAutoReConnect_ValueChanged(const String &Name, void* object, void* arg)
    {
      if(arg && object)
      {
        CallbackArguments* arguments = static_cast<CallbackArguments*>(arg);
        assert((arguments->arg1) && "Null Pointer!");
        Bluetooth_Source& BT_Out = *static_cast<Bluetooth_Source*>(arguments->arg1);
        bool autoReconnect = *static_cast<bool*>(object);
        ESP_LOGI("Manager::BluetoothSourceAutoReConnect_ValueChanged", "Bluetooth Source Auto Reconnect Value Changed Value Changed: %i", autoReconnect);
        BT_Out.Set_Auto_Reconnect(autoReconnect);
      }
    }
    
    //Bluetooth Source Reset
    CallbackArguments m_BluetoothReset_CallbackArgs = {&m_BT_Out};
    NamedCallback_t m_BluetoothReset_Callback = {"Bluetooth Reset Callback", &BluetoothReset_ValueChanged, &m_BluetoothReset_CallbackArgs};
    const bool m_BluetoothReset_InitialValue = true;
    DataItemWithPreferences<bool, 1> m_BluetoothReset = DataItemWithPreferences<bool, 1>( "BT_Src_Reset", m_BluetoothReset_InitialValue, RxTxType_Rx_Echo_Value, 0, &m_PreferencesInterface, &m_CPU3SerialPortMessageManager, &m_BluetoothReset_Callback, this, &validBoolValues);
    static void BluetoothReset_ValueChanged(const String &Name, void* object, void* arg)
    {
      if(arg && object)
      {
        CallbackArguments* arguments = static_cast<CallbackArguments*>(arg);
        assert((arguments->arg1) && "Null Pointer!");
        Bluetooth_Source& BT_Out = *static_cast<Bluetooth_Source*>(arguments->arg1);
        bool resetBLE = *static_cast<bool*>(object);
        ESP_LOGI("Manager::BluetoothSourceAutoReConnect_ValueChanged", "Bluetooth Source Reset Value Changed: %i", resetBLE);
        BT_Out.Set_Reset_BLE(resetBLE);
      }
    }
    
    //Bluetooth Reset NVS
    CallbackArguments m_BluetoothResetNVS_CallbackArgs = {&m_BT_Out};
    NamedCallback_t m_BluetoothResetNVS_Callback = {"Bluetooth Reset NVS Callback", &BluetoothResetNVS_ValueChanged, &m_BluetoothResetNVS_CallbackArgs};
    const bool m_BluetoothResetNVS_InitialValue = true;
    DataItem<bool, 1> m_BluetoothResetNVS = DataItem<bool, 1>( "BT_SRC_NVS_Rst"
                                                             , m_BluetoothResetNVS_InitialValue
                                                             , RxTxType_Rx_Echo_Value
                                                             , 0
                                                             , &m_CPU3SerialPortMessageManager
                                                             , &m_BluetoothResetNVS_Callback
                                                             , this
                                                             , &validBoolValues );
    static void BluetoothResetNVS_ValueChanged(const String &Name, void* object, void* arg)
    {
      if(arg && object)
      {
        CallbackArguments* arguments = static_cast<CallbackArguments*>(arg);
        assert((arguments->arg1) && "Null Pointer!");
        Bluetooth_Source& BT_Out = *static_cast<Bluetooth_Source*>(arguments->arg1);
        bool resetNVS = *static_cast<bool*>(object);
        ESP_LOGI("Manager::BluetoothSourceResetNVS_ValueChanged", "Bluetooth Source Reset NVS Value Changed: %i", resetNVS);
        BT_Out.Set_NVS_Init(resetNVS);
      }
    }
    
    //Target Compatible Device
    CallbackArguments m_TargetCompatibleDevice_CallbackArgs = {&m_BT_Out};
    NamedCallback_t m_TargetCompatibleDevice_Callback = {"Target Compatible Device Callback", &TargetCompatibleDevice_ValueChanged, &m_TargetCompatibleDevice_CallbackArgs};
    const CompatibleDevice_t m_TargetCompatibleDevice_InitialValue = {"", ""};
    DataItem<CompatibleDevice_t, 1> m_TargetCompatibleDevice = DataItem<CompatibleDevice_t, 1>( "Target_Device", m_TargetCompatibleDevice_InitialValue, RxTxType_Rx_Echo_Value, 0, &m_CPU3SerialPortMessageManager, &m_TargetCompatibleDevice_Callback, this);
    static void TargetCompatibleDevice_ValueChanged(const String &Name, void* object, void* arg)
    {
      ESP_LOGI("TargetCompatibleDeviceValueChanged", "Target Compatible Device Value Changed Value Changed");
      if(arg && object)
      {
        CallbackArguments* arguments = static_cast<CallbackArguments*>(arg);
        assert((arguments->arg1) && "Null Pointer!");
        CompatibleDevice_t* targetCompatibleDevice = static_cast<CompatibleDevice_t*>(object);
        Bluetooth_Source* BT_Out = static_cast<Bluetooth_Source*>(arguments->arg1);
        if(BT_Out && targetCompatibleDevice)
        {
          BT_Out->SetNameToConnect(targetCompatibleDevice->name, targetCompatibleDevice->address);
        }
      }
    }

    //Sound Output Source
    const ValidStringValues_t validSoundOutputSourceValues = { "OFF", "Bluetooth" };
    CallbackArguments m_SoundOutputSource_CallbackArgs = {this};
    NamedCallback_t m_SoundOutputSource_Callback = {"Sound Output Source Callback", &SoundOutputSource_ValueChanged, &m_SoundOutputSource_CallbackArgs};
    const SoundOutputSource_t m_SoundOutputSource_InitialValue = SoundOutputSource_t::Bluetooth;
    DataItemWithPreferences<SoundOutputSource_t, 1> m_SoundOutputSource = DataItemWithPreferences<SoundOutputSource_t, 1>( "Output_Source"
                                                                                                                         , m_SoundOutputSource_InitialValue
                                                                                                                         , RxTxType_Rx_Echo_Value
                                                                                                                         , 0
                                                                                                                         , &m_PreferencesInterface
                                                                                                                         , &m_CPU3SerialPortMessageManager
                                                                                                                         , &m_SoundOutputSource_Callback
                                                                                                                         , this
                                                                                                                         , &validSoundOutputSourceValues );
    static void SoundOutputSource_ValueChanged(const String &Name, void* object, void* arg)
    {
      if(arg && object)
      {
        CallbackArguments* arguments = static_cast<CallbackArguments*>(arg);
        assert((arguments->arg1) && "Null Pointer!");
        Manager* manager = static_cast<Manager*>(arguments->arg1);
        SoundOutputSource_t& soundOutputSource = *static_cast<SoundOutputSource_t*>(object);
        ESP_LOGI("Manager::SoundOutputSource_ValueChanged", "Sound Output Source Value Changed: %i", soundOutputSource);
        switch(soundOutputSource)
        {
          case SoundOutputSource_t::OFF:
            manager->StopBluetooth();
          break;
          case SoundOutputSource_t::Bluetooth:
            manager->StartBluetooth();
          break;
          default:
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
