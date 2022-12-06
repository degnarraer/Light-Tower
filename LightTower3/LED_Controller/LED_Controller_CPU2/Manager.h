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

class Manager: public NamedItem
             , public I2S_Device_Callback
             , public CommonUtils
             , public QueueController
{
  public:
    Manager( String Title
           , Sound_Processor &SoundProcessor
           , SPIDataLinkMaster &SPIDataLinkMaster
           , Bluetooth_Source &BT_Out
           , I2S_Device &I2S_Out
           , ContinuousAudioBuffer<AUDIO_BUFFER_SIZE> &AudioBuffer);
    virtual ~Manager();
    void Setup();
    void ProcessEventQueue();

    //Bluetooth Set Data Callback
    int32_t SetBTTxData(uint8_t *Data, int32_t channel_len);
    
    //I2S_Device_Callback
    void I2SDataReceived(String DeviceTitle, uint8_t *Data, uint32_t channel_len);

  private:
    Sound_Processor &m_SoundProcessor;
    SPIDataLinkMaster &m_SPIDataLinkMaster;
    ContinuousAudioBuffer<AUDIO_BUFFER_SIZE> &m_AudioBuffer;
    Frame_t m_AmplitudeFrameBuffer[AMPLITUDE_BUFFER_FRAME_COUNT];
    Frame_t m_FFTFrameBuffer[FFT_BUFFER_FRAME_COUNT];
    
    //I2S Sound Data
    I2S_Device &m_I2S_In;
    
    //Bluetooth Data
    Bluetooth_Source &m_BT_Out;

    void UpdateNotificationRegistrationStatus();
};

#endif
