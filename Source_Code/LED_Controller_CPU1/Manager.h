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

#ifndef I2S_EventHander_H
#define I2S_EventHander_H
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

enum InputType_t
{
  InputType_Microphone,
  InputType_Bluetooth
};

enum Mute_State_t
{
  Mute_State_Un_Muted = 0,
  Mute_State_Muted,
};


class Manager: public NamedItem
             , public I2S_Device_Callback
             , public Bluetooth_Sink_Callback
             , public SoundMeasureCalleeInterface
             , public BluetoothConnectionStatusCallee
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
    void ProcessEventQueue20mS();
    void ProcessEventQueue1000mS();
    void ProcessEventQueue300000mS();
    
    void SetInputType(InputType_t Type);
    //Bluetooth_Callback
    void BTDataReceived(uint8_t *data, uint32_t length);
    
    //I2S_Device_Callback
    void I2SDataReceived(String DeviceTitle, uint8_t *data, uint32_t length);

    //SoundMeasureCalleeInterface Callback
    void SoundStateChange(SoundState_t SoundState);

    //BluetoothConnectionStatusCallee Callback
    void BluetoothConnectionStatusChanged(ConnectionStatus_t ConnectionStatus);
    
  private:
    Preferences m_Preferences;
    DataSerializer m_DataSerializer;  
    SerialPortMessageManager m_CPU1SerialPortMessageManager = SerialPortMessageManager("CPU1", Serial1, m_DataSerializer);
    SerialPortMessageManager m_CPU3SerialPortMessageManager = SerialPortMessageManager("CPU3", Serial2, m_DataSerializer);

    DataItem<bool, 1> m_BluetoothSinkEnable = DataItem<bool, 1>( "BT_Sink_En", false, RxTxType_Rx_Echo_Value, UpdateStoreType_On_Rx, 5000, &m_Preferences, m_CPU3SerialPortMessageManager);
    DataItem<bool, 1> m_BluetoothSinkAutoReConnect = DataItem<bool, 1>( "BT_Sink_AR" , false , RxTxType_Rx_Echo_Value , UpdateStoreType_On_Rx , 5000 , &m_Preferences , m_CPU3SerialPortMessageManager);
    DataItem<ConnectionStatus_t, 1> m_BluetoothConnectionStatus = DataItem<ConnectionStatus_t, 1>( "Sink_Conn_Stat" , ConnectionStatus_t::Disconnected , RxTxType_Tx_On_Change_With_Heartbeat , UpdateStoreType_On_Tx , 5000 , NULL , m_CPU3SerialPortMessageManager);
    //DataItem<char, 30> m_BluetoothSinkSSID = DataItem<char, 30>( "BT_Sink_SSID" , "LED Tower of Power" , RxTxType_Rx_Echo_Value , UpdateStoreType_On_Rx , 5000 , &m_Preferences , m_CPU3SerialPortMessageManager);
    
    StatisticalEngine &m_StatisticalEngine;
    InputType_t m_InputType;
    Mute_State_t m_MuteState = Mute_State_Un_Muted;

    //Bluetooth Data
    Bluetooth_Sink &m_BT_In;
    
    //I2S Sound Data RX
    I2S_Device &m_Mic_In; 
    I2S_Device &m_I2S_Out;

    
    void InitializePreferences();
    void MoveDataToStatisticalEngine();
};

#endif
