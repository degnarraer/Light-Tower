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

#include "Bluetooth_Device.h"

void Bluetooth_Source::Setup()
{
	ESP_LOGI("Bluetooth_Device", "%s: Setup", GetTitle().c_str());
}

void Bluetooth_Source::InstallDevice()
{
	ESP_LOGI("Bluetooth Device", "%s: Installing Bluetooth Device.", GetTitle().c_str());
	m_BTSource.set_nvs_init(true);
	m_BTSource.set_reset_ble(true);
	m_BTSource.set_auto_reconnect(true);
	m_BTSource.set_ssp_enabled(false);
	xTaskCreate( StaticCompatibleDeviceTrackerTaskLoop,   "CompatibleDeviceTrackerTask",  2000,  this,   configMAX_PRIORITIES - 3,   &CompatibleDeviceTrackerTask);
	m_BTSource.set_local_name("LED Tower of Power");
	m_BTSource.set_task_core(0);
	m_BTSource.set_task_priority(configMAX_PRIORITIES-1);
	ESP_LOGI("Bluetooth_Device", "%s: Device Installed", GetTitle().c_str());
}

void Bluetooth_Source::SetMusicDataCallback(music_data_cb_t callback)
{
	m_MusicDataCallback = callback;
}

void Bluetooth_Source::StartDevice()
{
	if(false == m_Is_Running)
	{
		ESP_LOGI("Bluetooth_Device", "Starting Bluetooth");
		InstallDevice();
		m_BTSource.start_raw(m_MusicDataCallback);
		m_Is_Running = true;
		ESP_LOGI("Bluetooth_Device", "Bluetooth Started");
	}
}

void Bluetooth_Source::StopDevice()
{

}
bool Bluetooth_Source::IsConnected() {return m_BTSource.is_connected();}

//Callback from BT Source for compatible devices to connect to
bool Bluetooth_Source::ConnectToThisSSID(const char*ssid, esp_bd_addr_t address, int32_t rssi)
{
	compatible_device_found(ssid, rssi);
	return String(mp_SourceName).equals(String(ssid));
}
		
void Bluetooth_Source::compatible_device_found(const char* ssid, int32_t rssi)
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

void Bluetooth_Source::StaticCompatibleDeviceTrackerTaskLoop(void * Parameters)
{
	Bluetooth_Source* BT_Source = (Bluetooth_Source*)Parameters;
	BT_Source->CompatibleDeviceTrackerTaskLoop();
}

void Bluetooth_Source::CompatibleDeviceTrackerTaskLoop()
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
		for(int i = 0; i < m_ActiveCompatibleDevices.size(); ++i)
		{
			ESP_LOGI("Bluetooth_Device", "Scanned Device Name: %s \tRSSI: %i", m_ActiveCompatibleDevices[i].Name.c_str(), m_ActiveCompatibleDevices[i].Rssi);
		}
		vTaskDelay(500 / portTICK_PERIOD_MS);
	}
}
		
		
		
void Bluetooth_Sink::Setup()
{
	ESP_LOGI("Bluetooth_Device", "%s: Setup", GetTitle().c_str());
}
void Bluetooth_Sink::ResgisterForRxCallback(Bluetooth_Sink_Callback* callee){ m_Callee = callee; }

//Callbacks for Bluetooth 
void Bluetooth_Sink::data_received_callback() 
{
}

void Bluetooth_Sink::read_data_stream(const uint8_t *data, uint32_t length)
{  
	if(NULL != m_Callee)
	{
		m_Callee->BTDataReceived((uint8_t*)data, length);
	}
}

void Bluetooth_Sink::InstallDevice()
{
	ESP_LOGI("Bluetooth Device", "%s: Installing Bluetooth Device.", GetTitle().c_str());
	static i2s_config_t i2s_config = {
	  .mode = m_i2s_Mode,
	  .sample_rate = m_SampleRate, // updated automatically by A2DP
	  .bits_per_sample = m_BitsPerSample,
	  .channel_format = m_Channel_Fmt,
	  .communication_format = m_CommFormat,
	  .intr_alloc_flags = 1, // default interrupt priority
	  .dma_buf_count = m_BufferCount,
	  .dma_buf_len = m_BufferSize,
	  .use_apll = m_Use_APLL,
	  .tx_desc_auto_clear = true, // avoiding noise in case of data unavailability
	  .fixed_mclk = 1
	};
	i2s_pin_config_t my_pin_config = 
	{
		.bck_io_num = m_SerialClockPin,
		.ws_io_num = m_WordSelectPin,
		.data_out_num = m_SerialDataOutPin,
		.data_in_num = m_SerialDataInPin
	};
	m_BTSink.set_pin_config(my_pin_config);
	m_BTSink.set_i2s_config(i2s_config);
	m_BTSink.set_i2s_port(m_I2S_PORT);
	m_BTSink.set_bits_per_sample(m_BitsPerSample);
	m_BTSink.set_task_core(1);
	m_BTSink.set_task_priority(configMAX_PRIORITIES-1);
	m_BTSink.set_volume_control(&m_VolumeControl);
	m_BTSink.set_volume(100);
	ESP_LOGI("Bluetooth_Device", "%s: Device Installed", GetTitle().c_str());
}
void Bluetooth_Sink::StartDevice()
{
	if(false == m_Is_Running)
	{
		ESP_LOGI("Bluetooth_Device", "Starting Bluetooth");
		InstallDevice();
		m_BTSink.start(mp_SinkName);
		m_Is_Running = true;
		ESP_LOGI("Bluetooth_Device", "Bluetooth Started");
	}
}
void Bluetooth_Sink::StopDevice()
{
	if(true == m_Is_Running)
	{
		ESP_LOGI("Bluetooth_Device", "Stopping Bluetooth");
		m_BTSink.stop();
		m_Is_Running = false;
		ESP_LOGI("Bluetooth_Device", "Bluetooth Stopped");
	}
}
bool Bluetooth_Sink::IsConnected() {return m_BTSink.is_connected();}