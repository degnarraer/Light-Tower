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
             , public Bluetooth_Sink_Callbacks
             , public SoundMeasureCalleeInterface
             , public CommonUtils
             , public QueueController
             , public SetupCallerInterface
{
  public:
    Manager( std::string Title
           , StatisticalEngine &statisticalEngine
           , Bluetooth_Sink &bluetooth_Sink
           , I2S_Device &microphone
           , I2S_Device &i2S_Out );

    // Delete copy constructor and copy assignment operator
    Manager(const Manager&) = delete;
    Manager& operator=(const Manager&) = delete;
    
    virtual ~Manager();
    void Setup();
    
    void SetInputSource(SoundInputSource_t Type);
    
    //Bluetooth Callbacks
		void BT_Data_Received()
    {
    }

		void BT_Read_Data_Stream(const uint8_t *data, uint32_t length)
    {
      //m_I2S_Out.WriteSoundBufferData((uint8_t *)data, length);
    }

    //SoundMeasureCalleeInterface Callback
    void SoundStateChange(SoundState_t SoundState);

    //BluetoothConnectionStateCallee Callback
    void BluetoothConnectionStateChanged(const esp_a2d_connection_state_t connectionState, void* object);
    
  private:
    Preferences m_Preferences;
    PreferencesWrapper m_PreferencesWrapper = PreferencesWrapper("Settings", &m_Preferences);
    DataSerializer m_DataSerializer;
    Bluetooth_Sink &m_Bluetooth_Sink;
    I2S_Device &m_Microphone;
    I2S_Device &m_I2S_Out;
    TaskHandle_t m_TaskHandle = nullptr;

    static void Static_Microphone_Request_Task(void * parameter)
    {
      Manager* manager = static_cast<Manager*>(parameter);
      manager->Microphone_Request_Task();
    }

    void Microphone_Request_Task()
    {
      while(true)
      {
        uint8_t buffer[512] = {0};
        m_I2S_Out.WriteSoundBufferData(buffer, m_Microphone.ReadSoundBufferData(buffer, 512));
      }
    }

    void SetupSerialPortManager();
    SerialPortMessageManager m_CPU2SerialPortMessageManager = SerialPortMessageManager("CPU2", &Serial1, &m_DataSerializer, 1);
    SerialPortMessageManager m_CPU3SerialPortMessageManager = SerialPortMessageManager("CPU3", &Serial2, &m_DataSerializer, 1);
    
    void SetupTasks();
    

    void CreateMicrophoneTask()
    {
      if( xTaskCreatePinnedToCore( Static_Microphone_Request_Task, "Microphone Request", 5000, this, THREAD_PRIORITY_HIGH,  &m_TaskHandle, 0 ) == pdTRUE )
      {
        ESP_LOGI("StartDevice", "%s: Microphone task started.", GetTitle().c_str());
      }
      else
      {
        ESP_LOGE("StartDevice", "ERROR! Unable to create Microphone task!");
      }
    }

    void DestroyMicrophoneTask()
    {
      ESP_LOGI("DestroyTask", "Destroying Microphone task.");
      if(m_TaskHandle != nullptr)
      {
        vTaskDelete(m_TaskHandle);
        m_TaskHandle = nullptr;
      }
      else
      {
        ESP_LOGW("DestroyTask", "WARNING! Unable to destroy Microphone task!");
      }
    }


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
    static void SoundInputSource_ValueChanged(const std::string &Name, void* object, void* arg)
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
    CallbackArguments m_SinkAutoReConnect_CallbackArgs = { &m_Bluetooth_Sink, &m_SinkName };
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

    static void SinkAutoReConnect_ValueChanged(const std::string &Name, void* object, void* arg)
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
    CallbackArguments m_SinkConnect_CallbackArgs = { &m_Bluetooth_Sink
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
    static void SinkConnect_ValueChanged(const std::string &Name, void* object, void* arg)
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
    CallbackArguments m_SinkDisconnect_CallbackArgs = {&m_Bluetooth_Sink};
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
    static void SinkDisconnect_ValueChanged(const std::string &Name, void* object, void* arg)
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
                                                                                                     
    CallbackArguments m_R_Max_Band_CallbackArgs = {this};
    NamedCallback_t m_R_Max_Band_Callback = {"R Max Band Callback", &R_Max_Band_ValueChanged, &m_R_Max_Band_CallbackArgs};
    MaxBandSoundData_t m_R_Max_Band_InitialValue = MaxBandSoundData_t();
    DataItem<MaxBandSoundData_t, 1> m_R_Max_Band = DataItem<MaxBandSoundData_t, 1>( "R_Max_Band"
                                                                                  , m_R_Max_Band_InitialValue
                                                                                  , RxTxType_Rx_Only
                                                                                  , 0
                                                                                  , &m_CPU2SerialPortMessageManager
                                                                                  , &m_R_Max_Band_Callback
                                                                                  , this );
    static void R_Max_Band_ValueChanged(const std::string &Name, void* object, void* arg)
    {
      //ESP_LOGI("R_Max_Band_ValueChanged", "R_Max_Band_ValueChanged.");
    }
                                                                                                     
    CallbackArguments m_R_Bands_CallbackArgs = {this};
    NamedCallback_t m_R_Bands_Callback = {"R Bands Callback", &R_Bands_ValueChanged, &m_R_Bands_CallbackArgs};
    float m_R_Bands_InitialValue = 0.0;
    DataItem<float, 32> m_R_Bands = DataItem<float, 32>( "R_Bands"
                                                       , m_R_Bands_InitialValue
                                                       , RxTxType_Rx_Only
                                                       , 0
                                                       , &m_CPU2SerialPortMessageManager
                                                       , &m_R_Bands_Callback
                                                       , this );
    static void R_Bands_ValueChanged(const std::string &Name, void* object, void* arg)
    {
      float *bands;
      bands = static_cast<float*>(object);
      String message;
      for(int i = 0; i < 32; ++i)
      {
        if(i != 0) message += "|";
        message += String(bands[i]);
      }
      ESP_LOGI("R_Max_Band_ValueChanged", "R_Band_ValueChanged: %s", message.c_str());
    }

    CallbackArguments m_L_Max_Band_CallbackArgs = {this};
    NamedCallback_t m_L_Max_Band_Callback = {"L Max Bands Callback", &L_Max_Band_ValueChanged, &m_L_Max_Band_CallbackArgs};
    MaxBandSoundData_t m_L_Max_Band_InitialValue = MaxBandSoundData_t();
    DataItem<MaxBandSoundData_t, 1> m_L_Max_Band = DataItem<MaxBandSoundData_t, 1>( "L_Max_Band"
                                                                                  , m_L_Max_Band_InitialValue
                                                                                  , RxTxType_Rx_Only
                                                                                  , 0
                                                                                  , &m_CPU2SerialPortMessageManager
                                                                                  , &m_L_Max_Band_Callback
                                                                                  , this );
    static void L_Max_Band_ValueChanged(const std::string &Name, void* object, void* arg)
    {
      //ESP_LOGI("L_Max_Band_ValueChanged", "L_Max_Band_ValueChanged.");
    }
                                                                                 
    CallbackArguments m_L_Bands_CallbackArgs = {this};
    NamedCallback_t m_L_Bands_Callback = {"L Bands Callback", &L_Bands_ValueChanged, &m_L_Bands_CallbackArgs};
    float m_L_Bands_InitialValue = 0.0;
    DataItem<float, 32> m_L_Bands = DataItem<float, 32>( "L_Bands"
                                                       , m_L_Bands_InitialValue
                                                       , RxTxType_Rx_Only
                                                       , 0
                                                       , &m_CPU2SerialPortMessageManager
                                                       , &m_L_Bands_Callback
                                                       , this );
    static void L_Bands_ValueChanged(const std::string &Name, void* object, void* arg)
    {
      float *bands;
      bands = static_cast<float*>(object);
      String message;
      for(int i = 0; i < 32; ++i)
      {
        if(i != 0) message += "|";
        message += String(bands[i]);
      }
      ESP_LOGI("L_Band_ValueChanged", "L_Band_ValueChanged: %s", message.c_str());
    }

    CallbackArguments m_PSF_CallbackArgs = {this};
    NamedCallback_t m_PSF_Callback = {"L Bands Callback", &m_PSF_ValueChanged, &m_PSF_CallbackArgs};
    ProcessedSoundFrame_t m_PSF_InitialValue = ProcessedSoundFrame_t();
    DataItem<ProcessedSoundFrame_t, 1> m_PSF = DataItem<ProcessedSoundFrame_t, 1>( "PSF"
                                                                               , m_PSF_InitialValue
                                                                               , RxTxType_Rx_Only
                                                                               , 0
                                                                               , &m_CPU2SerialPortMessageManager
                                                                               , &m_PSF_Callback
                                                                               , this );
    static void m_PSF_ValueChanged(const std::string &Name, void* object, void* arg)
    {
      if(arg && object)
      {
        CallbackArguments* arguments = static_cast<CallbackArguments*>(arg);
        assert(arguments->arg1 && "Null Pointer!");
        Manager *manager = static_cast<Manager*>(arguments->arg1);
        ProcessedSoundFrame_t PSF = *static_cast<ProcessedSoundFrame_t*>(object);
        //ESP_LOGI("m_PSF_ValueChanged", "R Channel Power: %f L Channel Power: %f", PSF.Channel1.NormalizedPower, PSF.Channel2.NormalizedPower);
      }
    }

};
