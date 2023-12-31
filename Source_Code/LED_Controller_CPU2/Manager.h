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
    TaskHandle_t m_Manager_20mS_Task;
    TaskHandle_t m_Manager_1000mS_Task;
    TaskHandle_t m_Manager_300000mS_Task;

    SerialPortMessageManager &m_CPU1SerialPortMessageManager;
    SerialPortMessageManager &m_CPU3SerialPortMessageManager;

    /*
    DataItem<SSID_Info_With_LastUpdateTime_t, 1> m_SSIDWLUT = DataItem<SSID_Info_With_LastUpdateTime_t, 1>( "Available SSID"
                                                                                                           , SSID_Info_With_LastUpdateTime_t("\0", "\0", 0, 0)
                                                                                                           , RxTxType_Tx_On_Change
                                                                                                           , 0
                                                                                                           , m_CPU3SerialPortMessageManager);
   */
    DataItem<ConnectionStatus_t, 1> m_ConnectionStatus = DataItem<ConnectionStatus_t, 1>( "Connection Status"
                                                                                         , Disconnected
                                                                                         , RxTxType_Tx_On_Change_With_Heartbeat
                                                                                         , UpdateStoreType_On_Tx
                                                                                         , 1000
                                                                                         , NULL
                                                                                         , m_CPU3SerialPortMessageManager);
    
    Sound_Processor &m_SoundProcessor;
    ContinuousAudioBuffer<AUDIO_BUFFER_SIZE> &m_AudioBuffer;
    Frame_t m_AmplitudeFrameBuffer[AMPLITUDE_BUFFER_FRAME_COUNT];
    Frame_t m_FFTFrameBuffer[FFT_SIZE];

    Ticker m_Aplitude_Gain_NVM_Save_Ticker;
    Ticker m_FFT_Gain_NVM_Save_Ticker;
    
    //I2S Sound Data
    I2S_Device &m_I2S_In;
    
    //Bluetooth Data
    Bluetooth_Source &m_BT_Out;
    const float Epsilon = 0.01;
    bool AreEqual(float A, float B)
    {
      return abs(A - B) < Epsilon;
    }
    
    void UpdateNotificationRegistrationStatus();
    
    Preferences m_Preferences;
    void InitializeNVM(bool Reset);
    void LoadFromNVM();

    ConnectionStatus_t m_BluetoothConnectionStatus = ConnectionStatus_t::Disconnected;
    void BluetoothConnectionStatus_TX();

    bool m_SSP_Enabled = false;
    bool m_NVSInit = true;
    
    float m_AmplitudeGain;
    void AmplitudeGain_RX();
    void AmplitudeGain_TX();

    void FFTGain_RX();
    void FFTGain_TX();

    bool m_SourceBTReset = true;
    void SourceBluetoothReset_RX();
    void SourceBluetoothReset_TX();
    
    
    bool m_SourceBTReConnect = false;
    void SourceAutoReConnect_RX();
    void SourceAutoReConnect_TX();

    String m_SourceSSID;
    String m_SourceADDRESS;
    void SourceSSID_RX();
    void SourceSSID_TX();
    
};

#endif
