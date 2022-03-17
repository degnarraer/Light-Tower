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
#include "Sound_Processor.h"
#include "Serial_Datalink_Config.h"
#include "BluetoothA2DPSource_Override.h"

class Manager: public NamedItem
             , public I2S_Device_Callback
             , public CommonUtils
             , public QueueManager
{
  public:
    Manager( String Title
           , Sound_Processor &SoundProcessor
           , SerialDataLink &SerialDataLink
           , BluetoothA2DPSource &BT_Source
           , I2S_Device &I2S_In
           , I2S_Device &I2S_Out );
    virtual ~Manager();
    void Setup();
    void ProcessEventQueue();
    void WriteDataToBluetooth();
    int32_t get_data_channels(Frame *frame, int32_t channel_len);

    //QueueManager Interface
    DataItemConfig_t* GetDataItemConfig() { return m_ItemConfig; }
    size_t GetDataItemConfigCount() { return m_ConfigCount; }
    
    //I2S_Device_Callback
    void DataBufferModifyRX(String DeviceTitle, uint8_t* DataBuffer, size_t ByteCount, size_t SampleCount);
    void RightChannelDataBufferModifyRX(String DeviceTitle, uint8_t* DataBuffer, size_t ByteCount, size_t SampleCount);
    void LeftChannelDataBufferModifyRX(String DeviceTitle, uint8_t* DataBuffer, size_t ByteCount, size_t SampleCount);
    
  private:
    Sound_Processor &m_SoundProcessor;
    SerialDataLink &m_SerialDataLink;
    BluetoothA2DPSource &m_BT_Source;
    I2S_Device &m_I2S_In;
    I2S_Device &m_I2S_Out;

    
    //QueueManager Configuration
    static const size_t m_ConfigCount = 1;
    DataItemConfig_t m_ItemConfig[m_ConfigCount]
    {
      { "BT_IN", DataType_Frame_t, I2S_SAMPLE_COUNT,   Transciever_TX,   10 },
    };
    Frame_t m_DataFrame1[I2S_SAMPLE_COUNT/2];
    Frame_t m_DataFrame2[I2S_SAMPLE_COUNT/2];
};

#endif
