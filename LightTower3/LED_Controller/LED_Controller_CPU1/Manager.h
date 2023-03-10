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
#include "Serial_Datalink_Config.h"
#include "AudioBuffer.h"
#include <Preferences.h>

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
           , SPIDataLinkSlave &SPIDataLinkSlave
           , Bluetooth_Sink &BT_In
           , I2S_Device &Mic_In
           , I2S_Device &I2S_Out );
    virtual ~Manager();
    void Setup();
    void ProcessEventQueue20mS();
    void ProcessEventQueue1000mS();
    void ProcessEventQueue300000mS();
    void Process_I2S_EventQueue();
    
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
    StatisticalEngine &m_StatisticalEngine;
    SPIDataLinkSlave &m_SPIDataLinkSlave;
    InputType_t m_InputType;
    Mute_State_t m_MuteState = Mute_State_Un_Muted;

    //Bluetooth Data
    Bluetooth_Sink &m_BT_In;
    
    //I2S Sound Data RX
    I2S_Device &m_Mic_In; 
    I2S_Device &m_I2S_Out;

    
    Preferences m_Preferences;
    void InitializeNVM(bool Reset);
    void SaveToNVM();
    void LoadFromNVM();

    void MoveDataToStatisticalEngine();
    
    ConnectionStatus_t m_BluetoothConnectionStatus = ConnectionStatus_t::Disconnected;
    void BluetoothConnectionStatus_TX();

    
    SoundState_t m_SoundState = SoundState_t::LastingSilenceDetected;
    void SoundState_RX(SoundState_t SoundState);
    void SoundState_TX();

    String m_SinkSSID;
    void SinkSSID_TX();
    void SinkSSID_RX();
    
    bool m_SinkReset;
    void SinkBluetoothReset_RX();
    void SinkBluetoothReset_TX();

    bool m_SinkReConnect;
    void SinkAutoReConnect_RX();
    void SinkReConnect_TX();
    
    bool m_SinkEnable;
    void SinkEnable_RX();
    void SinkEnable_TX();
    
};

#endif
