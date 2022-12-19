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
#define BT_COMPATIBLE_DEVICE_TIMEOUT 5000

#include <vector> 
#include <Arduino.h>
#include <Helpers.h>
#include <BluetoothA2DPSink.h>
#include <BluetoothA2DPSource.h>

class Bluetooth_Source: public NamedItem
					  , public CommonUtils
					  , public QueueController
{
	struct ActiveCompatibleDevices_t
	{
		std::string Name;
		int32_t Rssi;
		unsigned long LastUpdateTime;
	};
	
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
		void Setup();
		void InstallDevice();
		void StartDevice();
		void StopDevice();
		bool IsConnected();
		void SetMusicDataCallback(music_data_cb_t callback);
		
		//Callback from BT Source for compatible devices to connect to
		bool ConnectToThisSSID(const char*ssid, esp_bd_addr_t address, int32_t rssi);
	private:
	
		BluetoothA2DPSource& m_BTSource;
		music_data_cb_t m_MusicDataCallback = NULL;
		const char *mp_SourceName;
		std::vector<ActiveCompatibleDevices_t> m_ActiveCompatibleDevices;
		TaskHandle_t CompatibleDeviceTrackerTask;
		bool m_Is_Running = false;
		
		void compatible_device_found(const char* ssid, int32_t rssi);
		static void StaticCompatibleDeviceTrackerTaskLoop(void * Parameters);
		void CompatibleDeviceTrackerTaskLoop();
};

class Bluetooth_Sink_Callback
{
	public:
		Bluetooth_Sink_Callback(){}
		virtual ~Bluetooth_Sink_Callback(){}
	
		//Callbacks called by this class
		virtual void BTDataReceived(uint8_t *data, uint32_t length) = 0;
};
class Bluetooth_Sink: public NamedItem
					, public CommonUtils
				    , public QueueController
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
				  , i2s_comm_format_t i2s_CommFormat
				  , i2s_channel_t i2s_channel
				  , bool Use_APLL
				  , size_t BufferCount
				  , size_t BufferSize
				  , int SerialClockPin
				  , int WordSelectPin
				  , int SerialDataInPin
				  , int SerialDataOutPin )
				  : NamedItem(Title)
				  , m_BTSink(BTSink)
				  , mp_SinkName(SinkName)
				  , m_I2S_PORT(i2S_PORT)
				  , m_i2s_Mode(Mode)
				  , m_SampleRate(SampleRate)
				  , m_BitsPerSample(BitsPerSample)
				  , m_Channel_Fmt(i2s_Channel_Fmt)
				  , m_CommFormat(i2s_CommFormat)
				  , m_i2s_channel(i2s_channel)
				  , m_Use_APLL(Use_APLL)
				  , m_BufferCount(BufferCount)
				  , m_BufferSize(BufferSize)
				  , m_SerialClockPin(SerialClockPin)
				  , m_WordSelectPin(WordSelectPin)
				  , m_SerialDataInPin(SerialDataInPin)
				  , m_SerialDataOutPin(SerialDataOutPin){};		
    virtual ~Bluetooth_Sink(){};
	void Setup();
	void StartDevice();
	void StopDevice();
	bool IsConnected();
	
	//Callbacks from BluetoothSink  
	void data_received_callback();
	void read_data_stream(const uint8_t *data, uint32_t length);
	
	//Callback Registrtion to this class
	void ResgisterForRxCallback(Bluetooth_Sink_Callback* callee);
    
  private:
	Bluetooth_Sink_Callback* m_Callee = NULL;
	A2DPDefaultVolumeControl m_VolumeControl;
	BluetoothA2DPSink& m_BTSink;
	i2s_port_t m_I2S_PORT;
    
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
	void InstallDevice();

};



#endif
