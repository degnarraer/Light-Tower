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
#include "DataItem.h"

class Manager: public NamedItem
             , public I2S_Device_Callback
             , public Bluetooth_Sink_Callback
             , public SoundMeasureCalleeInterface
             , public BluetoothConnectionStateCallee
             , public CommonUtils
             , public QueueController
{
  public:
    Manager( String Title
           , StatisticalEngine &StatisticalEngine
           , Bluetooth_Sink &BT_In
           , I2S_Device &Mic_In
           , I2S_Device &I2S_Out );
    virtual ~Manager();
    void Setup();

    //Tasks
    static void Static_Manager_20mS_TaskLoop(void * parameter);
    void ProcessEventQueue20mS();
    static void Static_Manager_1000mS_TaskLoop(void * parameter);
    void ProcessEventQueue1000mS();
    static void Static_Manager_300000mS_TaskLoop(void * parameter);
    void ProcessEventQueue300000mS();
    
    void SetInputSource(SoundInputSource_t Type);
    //Bluetooth_Callback
    void BTDataReceived(uint8_t *data, uint32_t length);
    
    //I2S_Device_Callback
    void I2SDataReceived(String DeviceTitle, uint8_t *data, uint32_t length);

    //SoundMeasureCalleeInterface Callback
    void SoundStateChange(SoundState_t SoundState);

    //BluetoothConnectionStateCallee Callback
    void BluetoothConnectionStateChanged(const esp_a2d_connection_state_t connectionState);
    
  private:
    Preferences m_Preferences;
    DataSerializer m_DataSerializer;

    void SetupSerialPortManager();
    SerialPortMessageManager m_CPU1SerialPortMessageManager = SerialPortMessageManager("CPU1", Serial1, m_DataSerializer);
    SerialPortMessageManager m_CPU3SerialPortMessageManager = SerialPortMessageManager("CPU3", Serial2, m_DataSerializer);
    
    void SetupTasks();
    TaskHandle_t m_Manager_20mS_Task;
    TaskHandle_t m_Manager_1000mS_Task;
    TaskHandle_t m_Manager_300000mS_Task;

    void RegisterForDataItemCallBacks()
    {
      m_SoundInputSource_CallbackArgs = {this};
      m_SoundInputSource_Callback = { m_SoundInputSource.GetName().c_str(), &SoundInputSource_ValueChanged, & m_SoundInputSource_CallbackArgs};
      m_SoundInputSource.RegisterNamedCallback(&m_SoundInputSource_Callback);
    }
    
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

    //Output Source Connect
    const SoundInputSource_t m_SoundInputSource_InitialValue = SoundInputSource_t::SoundInputSource_Microphone;
    DataItemWithPreferences<SoundInputSource_t, 1> m_SoundInputSource = DataItemWithPreferences<SoundInputSource_t, 1>( "Input_Source", m_SoundInputSource_InitialValue, RxTxType_Rx_Echo_Value, UpdateStoreType_On_Tx, 0, &m_Preferences, m_CPU3SerialPortMessageManager);
    CallbackArguments m_SoundInputSource_CallbackArgs;
    NamedCallback_t m_SoundInputSource_Callback;
    static void SoundInputSource_ValueChanged(const String &Name, void* object, void* arg)
    {
      if(arg)
      {
        CallbackArguments* arguments = static_cast<CallbackArguments*>(arg);
        if(arguments->arg1 && object)
        {
          Manager *manager = static_cast<Manager*>(arguments->arg1);
          SoundInputSource_t *inputSource = static_cast<SoundInputSource_t*>(object);
          manager->SetInputSource(*inputSource);
        }
        else
        {
          ESP_LOGE("SoundInputSourceValueChanged", "Null Pointers");
        }
      }
    }
    
    //Bluetooth Sink Enable
    const bool m_BluetoothSinkEnable_InitialValue = false;
    DataItemWithPreferences<bool, 1> m_BluetoothSinkEnable = DataItemWithPreferences<bool, 1>( "BT_Sink_En", m_BluetoothSinkEnable_InitialValue, RxTxType_Rx_Echo_Value, UpdateStoreType_On_Rx, 0, &m_Preferences, m_CPU3SerialPortMessageManager);

    //Bluetooth Sink Auto Reconnect
    const bool m_BluetoothSinkAutoReConnect_InitialValue = false;
    DataItemWithPreferences<bool, 1> m_BluetoothSinkAutoReConnect = DataItemWithPreferences<bool, 1>( "BT_Sink_AR", m_BluetoothSinkAutoReConnect_InitialValue, RxTxType_Rx_Echo_Value, UpdateStoreType_On_Rx, 0, &m_Preferences, m_CPU3SerialPortMessageManager);
    
    //Bluetooth Sink Connection Status
    const ConnectionStatus_t m_SinkConnectionStatus_InitialValue = ConnectionStatus_t::Disconnected;
    DataItem<ConnectionStatus_t, 1> m_BluetoothSinkConnectionStatus = DataItem<ConnectionStatus_t, 1>( "Sink_Conn_State", m_SinkConnectionStatus_InitialValue, RxTxType_Tx_On_Change_With_Heartbeat, UpdateStoreType_On_Tx, 5000, m_CPU3SerialPortMessageManager);

    //Bluetooth Sink Name
    const String m_BluetoothSinkName_InitialValue = "LED Tower of Power";
    StringDataItemWithPreferences m_BluetoothSinkName = StringDataItemWithPreferences( "BT_Sink_Name", m_BluetoothSinkName_InitialValue.c_str(), RxTxType_Rx_Echo_Value, UpdateStoreType_On_Rx, 0, &m_Preferences, m_CPU3SerialPortMessageManager);

    
    void SetupStatisticalEngine();
    StatisticalEngine &m_StatisticalEngine;
    Mute_State_t m_MuteState = Mute_State_Un_Muted;

    //Bluetooth Data
    void SetupBlueTooth();
    Bluetooth_Sink &m_BT_In;
    
    //I2S Sound Data RX
    void SetupI2S();
    I2S_Device &m_Mic_In; 
    I2S_Device &m_I2S_Out;

    
    void InitializePreferences();
    void MoveDataToStatisticalEngine();
};
