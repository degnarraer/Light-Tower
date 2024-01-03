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
    static void Static_TaskLoop_20mS(void * parameter);
    void TaskLoop_20mS();
    static void Static_TaskLoop_1000mS(void * parameter);
    void TaskLoop_1000mS();
   
    //Bluetooth Set Data Callback
    int32_t SetBTTxData(uint8_t *Data, int32_t channel_len);
    
    //I2S_Device_Callback
    void I2SDataReceived(String DeviceTitle, uint8_t *Data, uint32_t channel_len);

    //BluetoothConnectionStatusCallee Callback 
    void BluetoothConnectionStatusChanged(ConnectionStatus_t ConnectionStatus);
    
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
   
    DataItemWithPreferences<bool, 1> m_BluetoothSourceEnable = DataItemWithPreferences<bool, 1>( "BT_Source_En", false, RxTxType_Rx_Echo_Value, UpdateStoreType_On_Rx, 5000, &m_Preferences, m_CPU3SerialPortMessageManager);
    DataItemWithPreferences<bool, 1> m_BluetoothSourceAutoReConnect = DataItemWithPreferences<bool, 1>( "BT_Source_AR", false, RxTxType_Tx_On_Change_With_Heartbeat, UpdateStoreType_On_Rx, 5000, &m_Preferences, m_CPU3SerialPortMessageManager);
    DataItem<ConnectionStatus_t, 1> m_ConnectionStatus = DataItem<ConnectionStatus_t, 1>( "Src_Conn_Stat", ConnectionStatus_t::Disconnected, RxTxType_Tx_On_Change_With_Heartbeat, UpdateStoreType_On_Tx, 1000, m_CPU3SerialPortMessageManager);
    DataItem<BT_Info_With_LastUpdateTime_t, 1> m_ScannedName = DataItem<BT_Info_With_LastUpdateTime_t, 1>( "Scan_BT_Device", {"", "", 0, 0}, RxTxType_Tx_On_Change, UpdateStoreType_On_Tx, 0, m_CPU3SerialPortMessageManager);
    
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
