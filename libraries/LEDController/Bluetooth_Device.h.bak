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

#ifndef Bluetooth_Device_H
#define Bluetooth_Device_H 

#include <Arduino.h>
#include <DataTypes.h>
#include <Helpers.h>
#include "Streaming.h"
#include <BluetoothA2DPSink.h>

class Bluetooth_Sink_Callback
{
	public:
		Bluetooth_Sink_Callback(){}
		virtual ~Bluetooth_Sink_Callback(){}
	
		//Callbacks called by this class
		virtual void RightChannelDataBufferModifyRX(String DeviceTitle, uint8_t* DataBuffer, size_t Count) = 0;
		virtual void LeftChannelDataBufferModifyRX(String DeviceTitle, uint8_t* DataBuffer, size_t Count) = 0;
};

class Bluetooth_Sink: public NamedItem
					, public CommonUtils
                    , public QueueManager
{
  public:
    Bluetooth_Sink( String Title
				  , BluetoothA2DPSink& BTSink
				  , i2s_port_t i2S_PORT
				  , i2s_mode_t Mode
				  , int SampleRate
				  , i2s_bits_per_sample_t BitsPerSample
				  , i2s_channel_fmt_t i2s_Channel_Fmt
				  , i2s_comm_format_t CommFormat
				  , i2s_channel_t i2s_channel
				  , size_t BufferCount
				  , size_t BufferSize
				  , size_t OutputQueueSampleCount
				  , size_t QueueCount
				  , int SerialClockPin
				  , int WordSelectPin
				  , int SerialDataInPin
				  , int SerialDataOutPin );			
    virtual ~Bluetooth_Sink();
	void Setup();
	
	//Callbacks from BluetoothSink  
	void data_received_callback();
	void read_data_stream(const uint8_t *data, uint32_t length);
	
	//Callback Registrtion to this class
	void ResgisterForDataBufferRXCallback(Bluetooth_Sink_Callback* callee);
	void StartDevice();
	void StopDevice();
	
    size_t GetBytesToRead() {return m_TotalBytesToRead; }
    size_t GetChannelBytesToRead() {return m_ChannelBytesToRead; }
	size_t GetChannelSampleCount() { return m_ChannelSampleCount; }
	int GetSampleRate() { return m_SampleRate; }
    
    //QueueManager Interface
    DataItemConfig_t* GetDataItemConfig() { return m_ItemConfig; }
    size_t GetDataItemConfigCount() { return m_ConfigCount; }
    
  private:
    size_t m_ConfigCount = 0;
    DataItemConfig_t* m_ItemConfig = NULL;

	Bluetooth_Sink_Callback* m_Callee = NULL;
	SimpleExponentialVolumeControl m_VolumeControl;
	BluetoothA2DPSink& m_BTSink;
	i2s_port_t m_I2S_PORT;
    size_t m_SampleCount;
    size_t m_OutputQueueSampleCount;
	size_t m_QueueCount;
    size_t m_ChannelSampleCount;
    size_t m_BytesPerSample;
    size_t m_TotalBytesToRead;
    size_t m_ChannelBytesToRead;
	int32_t m_OurByteCount = 0;
	int32_t m_TheirByteCount = 0;
	int32_t m_DataBufferIndex = 0;
    const int m_SampleRate;
    const i2s_mode_t m_i2s_Mode;
    const i2s_bits_per_sample_t m_BitsPerSample;
    const i2s_comm_format_t m_CommFormat;
    const i2s_channel_fmt_t m_Channel_Fmt;
    const i2s_channel_t m_i2s_channel;
    const int m_BufferCount;
    const int m_BufferSize;
    const int m_SerialClockPin;
    const int m_WordSelectPin;
    const int m_SerialDataInPin;
    const int m_SerialDataOutPin;
	bool m_Is_Running = false;
	
	bool m_MemoryIsAllocated = false;
	void InstallDevice();
	void AllocateMemory();
	void FreeMemory();

};



#endif
