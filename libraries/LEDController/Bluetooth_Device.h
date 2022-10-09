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
#include <BluetoothA2DPSource.h>

class Bluetooth_Source: public NamedItem
					  , public CommonUtils
{
	public:
		Bluetooth_Source( String Title
						, BluetoothA2DPSource& BTSource
						, const char *SourceName )
						: NamedItem(Title)
						, m_BTSource(BTSource)
						, mp_SourceName(SourceName)
		{
		}
		virtual ~Bluetooth_Source(){}
		void Setup()
		{
			m_BTSource.set_nvs_init(true);
			m_BTSource.set_reset_ble(true);
			m_BTSource.set_auto_reconnect(false);
			m_BTSource.set_ssp_enabled(false);
			m_BTSource.set_local_name("LED Tower of Power");
			m_BTSource.set_task_priority(configMAX_PRIORITIES - 0);
		}
		void SetCallback(music_data_channels_cb_t callback)
		{
			m_callback = callback;
		}			
		void StartDevice()
		{
			m_BTSource.start(mp_SourceName, m_callback);
		}
		void StopDevice()
		{
		}
		bool IsConnected() {return m_BTSource.is_connected();}
	private:
		BluetoothA2DPSource& m_BTSource;
		music_data_channels_cb_t m_callback = NULL;
		const char *mp_SourceName;
};

class Bluetooth_Sink_Callback
{
	public:
		Bluetooth_Sink_Callback(){}
		virtual ~Bluetooth_Sink_Callback(){}
	
		//Callbacks called by this class
		virtual void DataBufferModifyRX(String DeviceTitle, uint8_t* DataBuffer, size_t ByteCount, size_t SampleCount) = 0;
		virtual void RightChannelDataBufferModifyRX(String DeviceTitle, uint8_t* DataBuffer, size_t ByteCount, size_t SampleCount) = 0;
		virtual void LeftChannelDataBufferModifyRX(String DeviceTitle, uint8_t* DataBuffer, size_t ByteCount, size_t SampleCount) = 0;
};
class Bluetooth_Sink: public NamedItem
					, public CommonUtils
{
  public:
    Bluetooth_Sink( String Title
				  , BluetoothA2DPSink& BTSink
				  , const char *SinkName
				  , i2s_port_t i2S_PORT
				  , i2s_mode_t Mode
				  , int SampleRate
				  , i2s_bits_per_sample_t BitsPerSample
				  , i2s_channel_fmt_t i2s_Channel_Fmt
				  , i2s_comm_format_t CommFormat
				  , i2s_channel_t i2s_channel
				  , bool Use_APLL
				  , size_t BufferCount
				  , size_t BufferSize
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
    
  private:
	uint8_t* mp_Data;
	uint8_t* mp_RightData;
	uint8_t* mp_LeftData;
    size_t m_ConfigCount = 0;

	Bluetooth_Sink_Callback* m_Callee = NULL;
	SimpleExponentialVolumeControl m_VolumeControl;
	BluetoothA2DPSink& m_BTSink;
	i2s_port_t m_I2S_PORT;
    size_t m_SampleCount;
    size_t m_ChannelSampleCount;
    size_t m_BytesPerSample;
    size_t m_TotalBytesToRead;
    size_t m_ChannelBytesToRead;
	int32_t m_OurByteCount = 0;
	int32_t m_DataBufferIndex = 0;
    const int m_SampleRate;
    const i2s_mode_t m_i2s_Mode;
    const i2s_bits_per_sample_t m_BitsPerSample;
    const i2s_comm_format_t m_CommFormat;
    const i2s_channel_fmt_t m_Channel_Fmt;
    const i2s_channel_t m_i2s_channel;
	const bool m_Use_APLL;
    const size_t m_BufferCount;
    const size_t m_BufferSize;
    const int m_SerialClockPin;
    const int m_WordSelectPin;
    const int m_SerialDataInPin;
    const int m_SerialDataOutPin;
	bool m_Is_Running = false;
	const char *mp_SinkName;
	bool m_MemoryIsAllocated = false;
	void InstallDevice();
	void AllocateMemory();
	void FreeMemory();

};



#endif
