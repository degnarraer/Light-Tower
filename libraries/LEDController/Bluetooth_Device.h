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
		void Setup()
		{
			m_BTSource.set_nvs_init(true);
			m_BTSource.set_reset_ble(true);
			m_BTSource.set_auto_reconnect(false);
			m_BTSource.set_ssp_enabled(false);
			xTaskCreate( StaticCompatibleDeviceTrackerTaskLoop,   "CompatibleDeviceTrackerTask",  2000,  this,   configMAX_PRIORITIES - 3,   &CompatibleDeviceTrackerTask);
			m_BTSource.set_local_name("LED Tower of Power");
			m_BTSource.set_task_priority(configMAX_PRIORITIES - 0);
		}
		void SetMusicDataCallback(music_data_cb_t callback)
		{
			m_MusicDataCallback = callback;
		}
		void Set_SSID_To_Connect_Check_Callback(ssid_to_connect_check_cb_t callback)
		{
			m_SSID_To_Connect_Check_Callback = callback;
		}
		void StartDevice()
		{
			m_BTSource.start_raw(m_SSID_To_Connect_Check_Callback, m_MusicDataCallback);
		}
		void StopDevice()
		{
		}
		bool IsConnected() {return m_BTSource.is_connected();}
		
		//Callback from BT Source for compatible devices to connect to
		bool ConnectToThisSSID(const char* ssid, int32_t rssi)
		{
			compatible_device_found(ssid, rssi);
			Serial << String(mp_SourceName).c_str() << " equals " << String(ssid).c_str() << "\n";
			return String(mp_SourceName).equals(String(ssid));
		}
	private:
	
		BluetoothA2DPSource& m_BTSource;
		music_data_cb_t m_MusicDataCallback = NULL;
		ssid_to_connect_check_cb_t m_SSID_To_Connect_Check_Callback = NULL;
		const char *mp_SourceName;
		std::vector<ActiveCompatibleDevices_t> m_ActiveCompatibleDevices;
		TaskHandle_t CompatibleDeviceTrackerTask;
		
		void compatible_device_found(const char* ssid, int32_t rssi)
		{
			bool Found = false;
			String SSID = String(ssid);
			for(int i = 0; i < m_ActiveCompatibleDevices.size(); ++i)
			{
				if(0 == m_ActiveCompatibleDevices[i].Name.compare(SSID.c_str()))
				{
					Found = true;
					m_ActiveCompatibleDevices[i].LastUpdateTime = millis();
					m_ActiveCompatibleDevices[i].Rssi = rssi;
					break;
				}
			}
			if(false == Found)
			{
				ActiveCompatibleDevices_t NewDevice;
				NewDevice.Name = SSID.c_str();
				NewDevice.Rssi = rssi;
				NewDevice.LastUpdateTime = millis();
				m_ActiveCompatibleDevices.push_back(NewDevice);
			}	
		}
		static void StaticCompatibleDeviceTrackerTaskLoop(void * Parameters)
		{
			Bluetooth_Source* BT_Source = (Bluetooth_Source*)Parameters;
			BT_Source->CompatibleDeviceTrackerTaskLoop();
		}
		void CompatibleDeviceTrackerTaskLoop()
		{
			while(true)
			{
				unsigned long CurrentTime = millis();
				for(int i = 0; i < m_ActiveCompatibleDevices.size(); ++i)
				{
					if(CurrentTime - m_ActiveCompatibleDevices[i].LastUpdateTime >= BT_COMPATIBLE_DEVICE_TIMEOUT)
					{
						m_ActiveCompatibleDevices.erase(m_ActiveCompatibleDevices.begin()+i);
						break;
					}
				}
				if(0 < m_ActiveCompatibleDevices.size())ESP_LOGE("Bluetooth_Device",  "**************DEVICES**************");
				for(int i = 0; i < m_ActiveCompatibleDevices.size(); ++i)
				{
					ESP_LOGE("Bluetooth_Device", "Device name: %s Device rssi: %i", m_ActiveCompatibleDevices[i].Name.c_str(), m_ActiveCompatibleDevices[i].Rssi);
				}
				vTaskDelay(500 / portTICK_PERIOD_MS);
			}
		}
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
	
	//Callbacks from BluetoothSink  
	void data_received_callback();
	void read_data_stream(const uint8_t *data, uint32_t length);
	
	//Callback Registrtion to this class
	void ResgisterForRxCallback(Bluetooth_Sink_Callback* callee);
    
  private:
	Bluetooth_Sink_Callback* m_Callee = NULL;
	SimpleExponentialVolumeControl m_VolumeControl;
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
