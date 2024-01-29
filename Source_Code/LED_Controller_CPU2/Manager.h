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
#include "DataItem.h"

class Manager: public NamedItem
             , public I2S_Device_Callback
             , public BluetoothConnectionStatusCallee
             , public BluetoothActiveDeviceUpdatee
             , public CommonUtils
             , public QueueController
{
  public:
    Manager( String Title
           , Sound_Processor &SoundProcessor
           , SerialPortMessageManager &CPU1SerialPortMessageManager
           , SerialPortMessageManager &CPU3SerialPortMessageManager
           , Bluetooth_Source &BT_Out
           , I2S_Device &I2S_Out
           , ContinuousAudioBuffer<AUDIO_BUFFER_SIZE> &AudioBuffer);
    virtual ~Manager();
    void Setup();
    void ConnectToTargetDevice();
    static void Static_TaskLoop_20mS(void * parameter);
    void TaskLoop_20mS();
    static void Static_TaskLoop_1000mS(void * parameter);
    void TaskLoop_1000mS();
   
    //Bluetooth Set Data Callback
    int32_t SetBTTxData(uint8_t *Data, int32_t channel_len);
    
    //I2S_Device_Callback
    void I2SDataReceived(String DeviceTitle, uint8_t *Data, uint32_t channel_len);

    //BluetoothConnectionStatusCallee Callback 
    void BluetoothConnectionStatusChanged(const ConnectionStatus_t ConnectionStatus);
    
    //BluetoothActiveDeviceUpdatee Callback 
    void BluetoothActiveDeviceListUpdated(const std::vector<ActiveCompatibleDevice_t> &Devices);

  private:
    Preferences m_Preferences;
    void InitializePreferences();
    
    TaskHandle_t m_Manager_20mS_Task;
    TaskHandle_t m_Manager_1000mS_Task;
    TaskHandle_t m_Manager_300000mS_Task;

    SerialPortMessageManager &m_CPU1SerialPortMessageManager;
    SerialPortMessageManager &m_CPU3SerialPortMessageManager;

    void RegisterForDataItemCallBacks()
    {
      NamedCallback_t soundOutputSourceCallback = {m_SoundOutputSource.GetName().c_str(), &SoundOutputSourceValueChanged};
      m_SoundOutputSource.RegisterNamedCallback(&soundOutputSourceCallback);

      NamedCallback_t targetCompatibleDeviceCallback = {m_TargetCompatibleDevice.GetName().c_str(), &TargetCompatibleDeviceValueChanged};
      m_TargetCompatibleDevice.RegisterNamedCallback(&targetCompatibleDeviceCallback);
    }
    
    //Bluetooth Source Enable
    bool m_BluetoothSourceEnable_InitialValue = true;
    DataItemWithPreferences<bool, 1> m_BluetoothSourceEnable = DataItemWithPreferences<bool, 1>( "BT_Source_En", m_BluetoothSourceEnable_InitialValue, RxTxType_Rx_Echo_Value, UpdateStoreType_On_Rx, 0, &m_Preferences, m_CPU3SerialPortMessageManager);

    //Bluetooth Source Auto ReConnect
    bool m_BluetoothSourceAutoReConnect_InitialValue = false;
    DataItemWithPreferences<bool, 1> m_BluetoothSourceAutoReConnect = DataItemWithPreferences<bool, 1>( "BT_Source_AR", m_BluetoothSourceAutoReConnect_InitialValue, RxTxType_Rx_Echo_Value, UpdateStoreType_On_Rx, 0, &m_Preferences, m_CPU3SerialPortMessageManager);
 
    //Bluetooth Source Reset
    bool m_BluetoothReset_InitialValue = true;
    DataItemWithPreferences<bool, 1> m_BluetoothReset = DataItemWithPreferences<bool, 1>( "BT_Src_Reset", m_BluetoothReset_InitialValue, RxTxType_Rx_Echo_Value, UpdateStoreType_On_Rx, 0, &m_Preferences, m_CPU3SerialPortMessageManager);
 
    //Bluetooth Reset NVS
    bool m_BluetoothResetNVS_InitialValue = true;
    DataItemWithPreferences<bool, 1> m_BluetoothResetNVS = DataItemWithPreferences<bool, 1>( "BT_SRC_NVS_Rst", m_BluetoothResetNVS_InitialValue, RxTxType_Rx_Echo_Value, UpdateStoreType_On_Rx, 0, &m_Preferences, m_CPU3SerialPortMessageManager);
 
    //Bluetooth Source Connection Status
    ConnectionStatus_t m_ConnectionStatus_InitialValue = ConnectionStatus_t::Disconnected;
    DataItem<ConnectionStatus_t, 1> m_ConnectionStatus = DataItem<ConnectionStatus_t, 1>( "Src_Conn_Stat", m_ConnectionStatus_InitialValue, RxTxType_Tx_On_Change_With_Heartbeat, UpdateStoreType_On_Tx, 1000, m_CPU3SerialPortMessageManager);
  
    //Scanned Device
    BT_Device_Info_With_Time_Since_Update_t m_ScannedDevice_InitialValue = {"", "", 0, 0};
    DataItem<BT_Device_Info_With_Time_Since_Update_t, 1> m_ScannedDevice = DataItem<BT_Device_Info_With_Time_Since_Update_t, 1>( "Scan_BT_Device", m_ScannedDevice_InitialValue, RxTxType_Tx_On_Change, UpdateStoreType_On_Tx, 0, m_CPU3SerialPortMessageManager);

    //Target Compatible Device
    CompatibleDevice_t m_TargetCompatibleDevice_InitialValue = {"", ""};
    DataItem<CompatibleDevice_t, 1> m_TargetCompatibleDevice = DataItem<CompatibleDevice_t, 1>( "Target_Device", m_TargetCompatibleDevice_InitialValue, RxTxType_Rx_Echo_Value, UpdateStoreType_On_Rx, 0, m_CPU3SerialPortMessageManager);
    static void TargetCompatibleDeviceValueChanged(const String &Name, void* object)
    {
      ESP_LOGI("Manager::TargetCompatibleDeviceValueChanged", "Target Compatible Device Value Changed Value Changed");
    }

    //Sound Output Source
    const SoundOutputSource_t m_SoundOutputSource_InitialValue = SoundOutputSource_t::SoundOutputSource_Bluetooth;
    DataItemWithPreferences<SoundOutputSource_t, 1> m_SoundOutputSource = DataItemWithPreferences<SoundOutputSource_t, 1>( "Output_Source", m_SoundOutputSource_InitialValue, RxTxType_Rx_Echo_Value, UpdateStoreType_On_Rx, 0, &m_Preferences, m_CPU3SerialPortMessageManager);
    static void SoundOutputSourceValueChanged(const String &Name, void* object)
    {
      ESP_LOGI("Manager::SoundOutputSourceValueChanged", "Sound Output Source Value Changed");
    }
    
    Sound_Processor &m_SoundProcessor;
    ContinuousAudioBuffer<AUDIO_BUFFER_SIZE> &m_AudioBuffer;
    Frame_t m_AmplitudeFrameBuffer[AMPLITUDE_BUFFER_FRAME_COUNT];
    Frame_t m_FFTFrameBuffer[FFT_SIZE];

    //I2S Sound Data
    I2S_Device &m_I2S_In;
    
    //Bluetooth Data
    Bluetooth_Source &m_BT_Out;
    const float Epsilon = 0.01;
    bool AreEqual(float A, float B)
    {
      return abs(A - B) < Epsilon;
    }
};

#endif
