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


#pragma once
#include <DataTypes.h>
#include <Helpers.h>
#include <I2S_Device.h>
#include <BluetoothA2DPSink.h>
#include "Bluetooth_Device.h"
#include "Statistical_Engine.h"
#include "AudioBuffer.h"
#include <Preferences.h>
#include "HardwareSerial.h"
#include "DataItem/DataItems.h"

class Manager: public NamedItem
             , public I2S_Device_Callback
             , public Bluetooth_Sink_Callback
             , public SoundMeasureCalleeInterface
             , public BluetoothConnectionStateCallee
             , public CommonUtils
             , public QueueController
             , public SetupCallerInterface
{
  public:
    Manager( String Title
           , StatisticalEngine &StatisticalEngine );

    // Delete copy constructor and copy assignment operator
    Manager(const Manager&) = delete;
    Manager& operator=(const Manager&) = delete;
    
    virtual ~Manager();
    void Setup();

    //Tasks
    static void Static_Manager_10mS_TaskLoop(void * parameter);
    void ProcessEventQueue10mS();
    static void Static_Manager_1000mS_TaskLoop(void * parameter);
    void ProcessEventQueue1000mS();
    static void Static_Manager_300000mS_TaskLoop(void * parameter);
    void ProcessEventQueue300000mS();
    
    void SetInputSource(SoundInputSource_t Type);
    
    //Bluetooth Callbacks
		void BT_Data_Received()
    {
      
    }
		void BT_Read_Data_Stream(const uint8_t *data, uint32_t length)
    {

    }

    //I2S_Device_Callback
    void I2SDataReceived(String DeviceTitle, uint8_t *data, uint32_t length);

    //SoundMeasureCalleeInterface Callback
    void SoundStateChange(SoundState_t SoundState);

    //BluetoothConnectionStateCallee Callback
    void BluetoothConnectionStateChanged(const esp_a2d_connection_state_t connectionState);
    
  private:
    Preferences m_Preferences;
    PreferencesWrapper m_PreferencesWrapper = PreferencesWrapper("Settings", &m_Preferences);
    DataSerializer m_DataSerializer;

    void SetupSerialPortManager();
    SerialPortMessageManager m_CPU1SerialPortMessageManager = SerialPortMessageManager("CPU1", &Serial1, &m_DataSerializer);
    SerialPortMessageManager m_CPU3SerialPortMessageManager = SerialPortMessageManager("CPU3", &Serial2, &m_DataSerializer);
    
    void SetupTasks();
    TaskHandle_t m_Manager_10mS_TaskHandle;
    TaskHandle_t m_Manager_1000mS_TaskHandle;
    TaskHandle_t m_Manager_300000mS_TaskHandle;

    BluetoothA2DPSink m_BTSink;
    Bluetooth_Sink m_BT_In = Bluetooth_Sink( "Bluetooth"
                                          , m_BTSink
                                          , I2S_NUM_1                          // I2S Interface
                                          , i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_TX)
                                          , I2S_SAMPLE_RATE
                                          , I2S_BITS_PER_SAMPLE_16BIT
                                          , I2S_CHANNEL_FMT_RIGHT_LEFT
                                          , i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB)
                                          , I2S_CHANNEL_STEREO
                                          , true                               // Use APLL                                    
                                          , I2S_BUFFER_COUNT                   // Buffer Count
                                          , I2S_CHANNEL_SAMPLE_COUNT           // Buffer Size
                                          , I2S2_SCLCK_PIN                     // Serial Clock Pin
                                          , I2S2_WD_PIN                        // Word Selection Pin
                                          , I2S2_SDIN_PIN                      // Serial Data In Pin
                                          , I2S2_SDOUT_PIN );                  // Serial Data Out Pin

    I2S_Device m_Mic_In = I2S_Device( "Microphone"
                                    , I2S_NUM_0                          // I2S Interface
                                    , i2s_mode_t(I2S_MODE_SLAVE | I2S_MODE_RX)
                                    , I2S_SAMPLE_RATE
                                    , I2S_BITS_PER_SAMPLE_32BIT
                                    , I2S_CHANNEL_FMT_RIGHT_LEFT
                                    , i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB)
                                    , I2S_CHANNEL_STEREO
                                    , true                               // Use APLL
                                    , I2S_BUFFER_COUNT                   // Buffer Count
                                    , I2S_CHANNEL_SAMPLE_COUNT           // Buffer Size
                                    , I2S1_SCLCK_PIN                     // Serial Clock Pin
                                    , I2S1_WD_PIN                        // Word Selection Pin
                                    , I2S1_SDIN_PIN                      // Serial Data In Pin
                                    , I2S1_SDOUT_PIN );                  // Serial Data Out Pin

    I2S_Device m_I2S_Out = I2S_Device( "I2S Out"
                                    , I2S_NUM_1                        // I2S Interface
                                    , i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_TX)
                                    , I2S_SAMPLE_RATE
                                    , I2S_BITS_PER_SAMPLE_16BIT
                                    , I2S_CHANNEL_FMT_RIGHT_LEFT
                                    , i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB)
                                    , I2S_CHANNEL_STEREO
                                    , true                            // Use APLL
                                    , I2S_BUFFER_COUNT                // Buffer Count
                                    , I2S_CHANNEL_SAMPLE_COUNT        // Buffer Size
                                    , I2S2_SCLCK_PIN                  // Serial Clock Pin
                                    , I2S2_WD_PIN                     // Word Selection Pin
                                    , I2S2_SDIN_PIN                   // Serial Data In Pin
                                    , I2S2_SDOUT_PIN );               // Serial Data Out Pin
    
    String ConnectionStatusStrings[5]
    {
      "DISCONNECTED",
      "CONNECTING",
      "CONNECTED",
      "DISCONNECTING",
      "Unknown",
    };

    ValidStringValues_t validBoolValues = {"0", "1"};

    struct CallbackArguments 
    {
      void* arg1;
      void* arg2;
      void* arg3;
      void* arg4;
      
      CallbackArguments(void* a1 = nullptr, void* a2 = nullptr, void* a3 = nullptr, void* a4 = nullptr)
        : arg1(a1), arg2(a2), arg3(a3), arg4(a4) {}
    };

    void SetupStatisticalEngine();
    StatisticalEngine &m_StatisticalEngine;
    Mute_State_t m_MuteState = Mute_State_t::Mute_State_Un_Muted;

    //Bluetooth Data
    void SetupDevices();
    
    //I2S Sound Data RX
    void SetupI2S();

    void MoveDataToStatisticalEngine();

    //Input Source
    CallbackArguments m_SoundInputSource_CallbackArgs = {this};
    NamedCallback_t m_SoundInputSource_Callback = { "Sound Input Source Callback"
                                                  , &SoundInputSource_ValueChanged
                                                  , &m_SoundInputSource_CallbackArgs };
    const SoundInputSource_t m_SoundInputSource_InitialValue = SoundInputSource_t::Microphone;
    DataItemWithPreferences<SoundInputSource_t, 1> m_SoundInputSource = DataItemWithPreferences<SoundInputSource_t, 1>( "Input_Source"
                                                                                                                      , m_SoundInputSource_InitialValue
                                                                                                                      , RxTxType_Rx_Echo_Value
                                                                                                                      , 0
                                                                                                                      , &m_PreferencesWrapper
                                                                                                                      , &m_CPU3SerialPortMessageManager
                                                                                                                      , &m_SoundInputSource_Callback
                                                                                                                      , this
                                                                                                                      , NULL );
    static void SoundInputSource_ValueChanged(const String &Name, void* object, void* arg)
    {
      if(arg && object)
      {
        CallbackArguments* arguments = static_cast<CallbackArguments*>(arg);
        assert(arguments->arg1 && "Null Pointer!");
        Manager *manager = static_cast<Manager*>(arguments->arg1);
        SoundInputSource_t inputSource = *static_cast<SoundInputSource_t*>(object);
        manager->SetInputSource(inputSource);
      }
    }

    //Bluetooth Sink Name
    const std::string m_SinkName_InitialValue = "LED Tower of Power";
    StringDataItemWithPreferences m_SinkName = StringDataItemWithPreferences( "Sink_Name"
                                                                            , m_SinkName_InitialValue
                                                                            , RxTxType_Rx_Echo_Value
                                                                            , 0
                                                                            , &m_PreferencesWrapper
                                                                            , &m_CPU3SerialPortMessageManager
                                                                            , nullptr
                                                                            , this );

    //Bluetooth Sink Auto Reconnect
    CallbackArguments m_SinkAutoReConnect_CallbackArgs = { &m_BT_In, &m_SinkName };
    NamedCallback_t m_SinkAutoReConnect_Callback = { "Sink Connect Callback"
                                                   , &SinkAutoReConnect_ValueChanged
                                                   , &m_SinkAutoReConnect_CallbackArgs };
    const bool m_SinkAutoReConnect_InitialValue = false;
    DataItemWithPreferences<bool, 1> m_SinkAutoReConnect = DataItemWithPreferences<bool, 1>( "BT_Sink_AR"
                                                                                           , m_SinkAutoReConnect_InitialValue
                                                                                           , RxTxType_Rx_Echo_Value
                                                                                           , 0
                                                                                           , &m_PreferencesWrapper
                                                                                           , &m_CPU3SerialPortMessageManager
                                                                                           , NULL
                                                                                           , this
                                                                                           , &validBoolValues );

    static void SinkAutoReConnect_ValueChanged(const String &Name, void* object, void* arg)
    {
      if(arg && object)
      {
        CallbackArguments* pArguments = static_cast<CallbackArguments*>(arg);
        if(pArguments->arg1 && pArguments->arg2)
        {
          Bluetooth_Sink* pBT_In = static_cast<Bluetooth_Sink*>(pArguments->arg1);
          DataItemWithPreferences<bool, 1> *sinkAutoReConnect = static_cast<DataItemWithPreferences<bool, 1>*>(pArguments->arg2);
          ESP_LOGI("SinkAutoReConnect_ValueChanged", "Sink Auto ReConnect Value Changed: %i", sinkAutoReConnect->GetValue());
          pBT_In->Set_Auto_Reconnect(sinkAutoReConnect->GetValue());
        }
        else
        {
          ESP_LOGE("SinkAutoReConnect_ValueChanged", "ERROR! Null Pointers!");
        }
      }
    }

    //Sink Connect
    CallbackArguments m_SinkConnect_CallbackArgs = { &m_BT_In
                                                    , &m_SinkName
                                                    , &m_SinkAutoReConnect };
    NamedCallback_t m_SinkConnect_Callback = { "Sink Connect Callback"
                                             , &SinkConnect_ValueChanged
                                             , &m_SinkConnect_CallbackArgs };
    const bool m_SinkConnect_InitialValue = false;
    DataItem<bool, 1> m_SinkConnect = DataItem<bool, 1>( "Sink_Connect"
                                                       , m_SinkConnect_InitialValue
                                                       , RxTxType_Rx_Echo_Value
                                                       , 0
                                                       , &m_CPU3SerialPortMessageManager
                                                       , &m_SinkConnect_Callback
                                                       , this );
    static void SinkConnect_ValueChanged(const String &Name, void* object, void* arg)
    {
      if(arg && object)
      {
        CallbackArguments* pArguments = static_cast<CallbackArguments*>(arg);
        if(pArguments->arg1 && pArguments->arg2 && pArguments->arg3)
        {
          Bluetooth_Sink* pBT_In = static_cast<Bluetooth_Sink*>(pArguments->arg1);
          StringDataItemWithPreferences* pBluetoothSinkName = static_cast<StringDataItemWithPreferences*>(pArguments->arg2);
          DataItemWithPreferences<bool, 1>* pBluetoothSinkAutoReConnect = static_cast<DataItemWithPreferences<bool, 1>*>(pArguments->arg3);
          bool sinkConnect = *static_cast<bool*>(object);
          if(sinkConnect)
          {
            ESP_LOGI("SinkConnect_ValueChanged", "Sink Connecting");
            pBT_In->Connect(pBluetoothSinkName->GetValuePointer(), pBluetoothSinkAutoReConnect->GetValue());
          }
        }
        else
        {
            ESP_LOGI("SinkConnect_ValueChanged", "Sink Connecting");
        }
      }
    }

    //Sink Disconnect
    CallbackArguments m_SinkDisconnect_CallbackArgs = {&m_BT_In};
    NamedCallback_t m_SinkDisconnect_Callback = { "Sink Disconnect Callback"
                                                , &SinkDisconnect_ValueChanged
                                                , &m_SinkDisconnect_CallbackArgs };
    const bool m_SinkDisconnect_InitialValue = false;
    DataItem<bool, 1> m_SinkDisconnect = DataItem<bool, 1>( "Sink_Disconnect"
                                                          , m_SinkDisconnect_InitialValue
                                                          , RxTxType_Rx_Echo_Value
                                                          , 0
                                                          , &m_CPU3SerialPortMessageManager
                                                          , &m_SinkDisconnect_Callback
                                                          , this );
    static void SinkDisconnect_ValueChanged(const String &Name, void* object, void* arg)
    {
      if(arg && object)
      {
        CallbackArguments* pArguments = static_cast<CallbackArguments*>(arg);
        assert(pArguments->arg1 && "Null Pointer!");
        Bluetooth_Sink* pBT_In = static_cast<Bluetooth_Sink*>(pArguments->arg1);
        bool sinkDisconnect = *static_cast<bool*>(object);
        if(sinkDisconnect)
        {
          ESP_LOGI("SinkDisconnect_ValueChanged", "Sink Disconnecting");
          pBT_In->Disconnect();
        }
      }
    }

    //Bluetooth Sink Enable
    const bool m_BluetoothSinkEnable_InitialValue = false;
    DataItemWithPreferences<bool, 1> m_BluetoothSinkEnable = DataItemWithPreferences<bool, 1>( "BT_Sink_En"
                                                                                             , m_BluetoothSinkEnable_InitialValue
                                                                                             , RxTxType_Rx_Echo_Value
                                                                                             , 0
                                                                                             , &m_PreferencesWrapper
                                                                                             , &m_CPU3SerialPortMessageManager
                                                                                             , NULL
                                                                                             , this
                                                                                             , &validBoolValues );

    //Bluetooth Sink Connection Status
    const ConnectionStatus_t m_SinkConnectionStatus_InitialValue = ConnectionStatus_t::Disconnected;
    DataItem<ConnectionStatus_t, 1> m_BluetoothSinkConnectionStatus = DataItem<ConnectionStatus_t, 1>( "Sink_Conn_State"
                                                                                                     , m_SinkConnectionStatus_InitialValue
                                                                                                     , RxTxType_Tx_On_Change_With_Heartbeat
                                                                                                     , 5000
                                                                                                     , &m_CPU3SerialPortMessageManager
                                                                                                     , NULL
                                                                                                     , this );

};
