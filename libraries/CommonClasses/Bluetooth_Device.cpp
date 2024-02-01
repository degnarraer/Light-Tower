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
	m_BTSource.set_nvs_init(m_ResetNVS);
	m_BTSource.set_reset_ble(m_ResetBLE);
	m_BTSource.set_auto_reconnect(m_AutoReConnect);
	m_BTSource.set_ssp_enabled(m_SSPEnabled);
	xTaskCreatePinnedToCore( StaticCompatibleDeviceTrackerTaskLoop,   "CompatibleDeviceTrackerTask",  5000,  this,   THREAD_PRIORITY_MEDIUM,   &m_CompatibleDeviceTrackerTask, 1);
	m_BTSource.set_local_name("");
	m_BTSource.set_task_core(1);
	m_BTSource.set_task_priority(THREAD_PRIORITY_HIGH);
	ESP_LOGI("Bluetooth_Device", "%s: Device Installed", GetTitle().c_str());
}

void Bluetooth_Source::SetMusicDataCallback(music_data_cb_t callback)
{
	m_MusicDataCallback = callback;
}

void Bluetooth_Source::StartDevice( const char *SourceName
								  , const char *SourceAddress
								  , bool AutoReconnect
								  , bool ResetBLE
								  , bool ResetNVS )
{
	m_Name = SourceName;
	m_Address = SourceAddress;
	m_AutoReConnect = AutoReconnect;
	m_ResetBLE = ResetBLE;
	m_ResetNVS = ResetNVS;
	ESP_LOGI("Bluetooth_Device", "Starting Bluetooth");
	InstallDevice();
	m_BTSource.start_raw(m_MusicDataCallback);
	m_Is_Running = true;
	SetSearching();
	ESP_LOGI("Bluetooth_Device", "Bluetooth Started with: \n\tNAME: %s \n\tReset BLE: %i \n\tAuto Reconnect: %i \n\tSSP Enabled: %i", m_Name.c_str(), m_ResetBLE, m_AutoReConnect, m_SSPEnabled);
}

void Bluetooth_Source::SetNameToConnect( const char *SourceName, const char *SourceAddress )
{
	ESP_LOGI( "Bluetooth_Source::ConnectToThisName", "Set Name to Connect: \"%s\" Address: \"%s\""
			, SourceName
			, SourceAddress );
	m_Name = SourceName;
	m_Address = SourceAddress;
}

//Callback from BT Source for compatible devices to connect to
bool Bluetooth_Source::ConnectToThisName(const char*name, esp_bd_addr_t address, int32_t rssi)
{
	ESP_LOGD( "Bluetooth_Source::ConnectToThisName", "Connect to this name: \"%s\" Address: \"%s\""
			, String(name).c_str()
			, GetAddressString(address));
	bool result = compatible_device_found(name, address, rssi);
	if(result) SetPairing();
	return result;
}
		
bool Bluetooth_Source::compatible_device_found(const char* name, esp_bd_addr_t address, int32_t rssi)
{
	ESP_LOGD("Bluetooth_Device", "Compatible Device \"%s\" Address:\"%s\"", name, GetAddressString(address) );
	bool Found = false;
	String nameString(name);
	String addressString(GetAddressString(address));
	for(int i = 0; i < m_ActiveCompatibleDevices.size(); ++i)
	{
		m_ActiveCompatibleDevicesMutex.lock();
		if( 0 == strcmp(m_ActiveCompatibleDevices[i].address, addressString.c_str()))
		{
			Found = true;
			strcpy(m_ActiveCompatibleDevices[i].name, nameString.c_str());
			m_ActiveCompatibleDevices[i].rssi = rssi;
			m_ActiveCompatibleDevices[i].lastUpdateTime = millis();
			ESP_LOGD("Bluetooth_Device", "Compatible Device \"%s\" Updated", m_ActiveCompatibleDevices[i].name );
		}
		m_ActiveCompatibleDevicesMutex.unlock();
	}
	if(false == Found)
	{
		ActiveCompatibleDevice_t NewDevice;
		strcpy(NewDevice.name, nameString.c_str());
		strcpy(NewDevice.address, addressString.c_str());
		NewDevice.rssi = rssi;
		NewDevice.lastUpdateTime = millis();
		m_ActiveCompatibleDevicesMutex.lock();
		m_ActiveCompatibleDevices.push_back(NewDevice);
		m_ActiveCompatibleDevicesMutex.unlock();
		ESP_LOGI("Bluetooth_Device", "New Compatible Device Found: %s", NewDevice.name );
	}
	return Found;
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
			if(CurrentTime - m_ActiveCompatibleDevices[i].lastUpdateTime >= BT_COMPATIBLE_DEVICE_TIMEOUT)
			{
				m_ActiveCompatibleDevicesMutex.lock();
				m_ActiveCompatibleDevices.erase(m_ActiveCompatibleDevices.begin()+i);
				m_ActiveCompatibleDevicesMutex.unlock();
				break;
			}
		}
		for(int i = 0; i < m_ActiveCompatibleDevices.size(); ++i)
		{
			ESP_LOGD("Bluetooth_Device", "Scanned Device Name: %s \tRSSI: %i", m_ActiveCompatibleDevices[i].name, m_ActiveCompatibleDevices[i].rssi);
		}
		if(NULL != m_BluetoothActiveDeviceUpdatee)
		{
			m_ActiveCompatibleDevicesMutex.lock();
			m_BluetoothActiveDeviceUpdatee->BluetoothActiveDeviceListUpdated(m_ActiveCompatibleDevices);
			m_ActiveCompatibleDevicesMutex.unlock();
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
	m_BTSink.set_auto_reconnect(m_AutoReConnect);
	m_BTSink.set_bits_per_sample(m_BitsPerSample);
	m_BTSink.set_task_core(1);
	m_BTSink.set_task_priority(THREAD_PRIORITY_HIGH);
	m_BTSink.set_volume_control(&m_VolumeControl);
	m_BTSink.set_volume(100);
	ESP_LOGI("Bluetooth_Device", "%s: Device Installed", GetTitle().c_str());
}
void Bluetooth_Sink::StartDevice(String SinkName, bool reconnect)
{	
	m_SinkName = SinkName;
	m_AutoReConnect = reconnect;
	if( true == m_Is_Running && false == m_SinkName.equals(SinkName) )
	{
		ESP_LOGI("Bluetooth_Device", "ReStarting Bluetooth Sink with NAME: %s", m_SinkName.c_str());
		m_Is_Running = true;
		StopDevice();
		m_BTSink.start(m_SinkName.c_str());
		SetWaiting();
		ESP_LOGI("Bluetooth_Device", "Bluetooth Sink Started With NAME: %s Auto Reconnect: %i", m_SinkName.c_str(), m_AutoReConnect);
	}
	else if(false == m_Is_Running)
	{
		m_Is_Running = true;
		InstallDevice();
		ESP_LOGI("Bluetooth_Device", "Starting Bluetooth Sink with NAME: %s", m_SinkName.c_str());
		m_BTSink.start(m_SinkName.c_str());
		SetWaiting();
		ESP_LOGI("Bluetooth_Device", "Bluetooth Sink Started With NAME: %s Auto Reconnect: %i", m_SinkName.c_str(), m_AutoReConnect);
	}
}
void Bluetooth_Sink::StopDevice()
{
	if(true == m_Is_Running)
	{
		ESP_LOGI("Bluetooth_Device", "Stopping Bluetooth");
		m_BTSink.end(true);
		m_Is_Running = false;
		ESP_LOGI("Bluetooth_Device", "Bluetooth Stopped");
	}
}