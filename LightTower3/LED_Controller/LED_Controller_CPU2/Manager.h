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

#include <I2S_Device.h>
#include <DataTypes.h>
#include <Helpers.h>
#include <SPI_Datalink.h>
#include "Sound_Processor.h"
#include "Serial_Datalink_Config.h"
#include <BluetoothA2DPSource.h>
#include "Bluetooth_Device.h"
#include "circle_buf.h"

class Manager: public NamedItem
             , public I2S_Device_Callback
             , public CommonUtils
{
  public:
    Manager( String Title
           , Sound_Processor &SoundProcessor
           , SerialDataLink &SerialDataLink
           , Bluetooth_Source &BT_Out
           , I2S_Device &I2S_In
           , I2S_Device &I2S_Out );
    virtual ~Manager();
    void AllocateMemory();
    void FreeMemory();
    void Setup();
    void Loop();
    void ProcessEventQueue();
    void WriteDataToBluetooth();

    //Bluetooth Get Data Callback
    int32_t SetBTTxData(uint8_t *Data, int32_t channel_len);
    
    //I2S_Device_Callback
    void I2SDataReceived(String DeviceTitle, const uint8_t *data, uint32_t length);
  
  private:
    Sound_Processor &m_SoundProcessor;
    SerialDataLink &m_SerialDataLink;

    //I2C Datalinkstatic 
    const static int32_t m_CircularBufferSize = 4 * I2S_SAMPLE_COUNT * I2S_BUFFER_COUNT;
    bfs::CircleBuf<Frame_t, m_CircularBufferSize> m_FrameBuffer;

    //I2S Sound Data RX
    I2S_Device &m_I2S_In;
    I2S_Device &m_I2S_Out;
    
    //Bluetooth Data
    Bluetooth_Source &m_BT_Out;

    Frame_t m_LinearFrameBuffer[I2S_SAMPLE_COUNT];
    int32_t m_RightDataBuffer[I2S_SAMPLE_COUNT];
    int32_t m_LeftDataBuffer[I2S_SAMPLE_COUNT];
    void UpdateNotificationRegistrationStatus();
};

#endif
