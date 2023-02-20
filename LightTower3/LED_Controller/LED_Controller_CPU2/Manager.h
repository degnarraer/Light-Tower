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
#include "Serial_Datalink_Config.h"
#include "AudioBuffer.h"
#include <Preferences.h>

class Manager: public NamedItem
             , public I2S_Device_Callback
             , public CommonUtils
             , public QueueController
{
  public:
    Manager( String Title
           , Sound_Processor &SoundProcessor
           , SPIDataLinkToCPU1 &SPIDataLinkToCPU1
           , SPIDataLinkToCPU3 &SPIDataLinkToCPU3
           , Bluetooth_Source &BT_Out
           , I2S_Device &I2S_Out
           , ContinuousAudioBuffer<AUDIO_BUFFER_SIZE> &AudioBuffer);
    virtual ~Manager();
    void Setup();
    void ProcessEventQueue20mS();
    void ProcessEventQueue1000mS();
    void ProcessEventQueue300000mS();
    void SaveToNVM();
    void LoadFromNVM();

    //Bluetooth Set Data Callback
    int32_t SetBTTxData(uint8_t *Data, int32_t channel_len);
    
    //I2S_Device_Callback
    void I2SDataReceived(String DeviceTitle, uint8_t *Data, uint32_t channel_len);

  private:
    Sound_Processor &m_SoundProcessor;
    SPIDataLinkToCPU1 &m_SPIDataLinkToCPU1;
    SPIDataLinkToCPU3 &m_SPIDataLinkToCPU3;
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
    
    void UpdateNotificationRegistrationStatus();
    void MoveDataBetweenCPU1AndCPU3();
    
    Preferences m_Preferences;
    void InitializeNVM(bool Reset);

    float m_AmplitudeGain;
    void AmplitudeGain_RX();
    void AmplitudeGain_TX();

    float m_FFTGain;
    void FFTGain_RX();
    void FFTGain_TX();

    bool m_SourceBTReset;
    void SourceBluetoothReset_RX();
    void SourceBluetoothReset_TX();
    
    bool m_SourceReConnect;
    void SourceAutoReConnect_RX();
    void SourceAutoReConnect_TX();

    String m_SourceSSID;
    void SourceSSID_RX();
    void SourceSSID_TX();
    
};

#endif
