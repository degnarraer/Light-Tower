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
#define BUILD_BLUETOOTH

#ifdef BUILD_BLUETOOTH

#include "Bluetooth_Device.h"

BluetoothConnectionStateCaller::BluetoothConnectionStateCaller( BluetoothA2DPCommon *BT )
															  : mp_BT(BT)
{
	if(mp_BT)
	{
		mp_BT->set_on_connection_state_changed(Static_BT_Connection_State_Change_Callback, this);
	}
}

void BluetoothConnectionStateCaller::Static_BT_Connection_State_Change_Callback(esp_a2d_connection_state_t state, void *parameter)
{
	((BluetoothConnectionStateCaller*)parameter)->BT_Connection_State_Change_Callback(state);
}

void BluetoothConnectionStateCaller::BT_Connection_State_Change_Callback(esp_a2d_connection_state_t state)
{
	if(mp_BT && mp_ConnectionStateCallee)
	{
		mp_ConnectionStateCallee->BluetoothConnectionStateChanged(state);
	}
}

void BluetoothConnectionStateCaller::RegisterForConnectionStateChangedCallBack(BluetoothConnectionStateCallee *Callee)
{
	mp_ConnectionStateCallee = Callee;
}

bool BluetoothConnectionStateCaller::IsConnected()
{
	return mp_BT->is_connected();
}

void Bluetooth_Source::Setup()
{
	ESP_LOGI("Bluetooth_Device", "%s: Setup", GetTitle().c_str());
}

void Bluetooth_Source::InstallDevice()
{
	ESP_LOGI("Bluetooth Device", "%s: Installing Bluetooth Device.", GetTitle().c_str());
	m_BTSource.set_reset_ble(m_ResetBLE);
	m_BTSource.set_auto_reconnect(m_AutoReConnect);
	m_BTSource.set_ssp_enabled(false);
	m_BTSource.set_local_name("LED Tower of Power");
	m_BTSource.set_task_core(1);
	m_BTSource.set_task_priority(THREAD_PRIORITY_HIGH);
	ESP_LOGI("Bluetooth_Device", "%s: Device Installed", GetTitle().c_str());
	if( xTaskCreatePinnedToCore( StaticCompatibleDeviceTrackerTaskLoop,   "CompatibleDeviceTrackerTask",  10000,  this,   THREAD_PRIORITY_MEDIUM,   &m_CompatibleDeviceTrackerTask, 1) != pdTRUE )
	{
		ESP_LOGE("InstallDevice", "ERROR! Unable to create task.");
	}
}

void Bluetooth_Source::SetMusicDataCallback(music_data_cb_t callback)
{
	m_MusicDataCallback = callback;
}

void Bluetooth_Source::StartDevice()
{
	ESP_LOGI("Bluetooth_Device", "Starting Bluetooth");
	InstallDevice();
	ESP_LOGI("Bluetooth_Device", "Bluetooth Started");
}

void Bluetooth_Source::StopDevice()
{
	if(true == m_Is_Running)
	{
		ESP_LOGI("Bluetooth_Device", "Stopping Bluetooth");
		m_BTSource.end(false);
		m_Is_Running = false;
		ESP_LOGI("Bluetooth_Device", "Bluetooth Stopped");
	}
}

void Bluetooth_Source::Connect( const char *SourceName, const char *SourceAddress )
{
	m_Name = String(SourceName);
	m_Address = String(SourceAddress);
	ESP_LOGI("Bluetooth_Device", "Starting Bluetooth with: \n\tName: \"%s\" \n\tAddress: \"%s\"", m_Name.c_str(), m_Address.c_str());
	m_BTSource.start_raw(m_MusicDataCallback);
	m_Is_Running = true;
}
void Bluetooth_Source::Disconnect()
{
	m_BTSource.disconnect();
	m_Is_Running = false;
}
void Bluetooth_Source::SetNameToConnect( const std::string& sourceName, const std::string& sourceAddress )
{
	ESP_LOGI( "Bluetooth_Source::ConnectToThisName", "Set Name to Connect: \"%s\" Address: \"%s\""
			, sourceName.c_str()
			, sourceAddress.c_str() );
	m_Name = String(sourceName.c_str());
	m_Address = String(sourceAddress.c_str());
}

//Callback from BT Source for compatible devices to connect to
bool Bluetooth_Source::ConnectToThisName(const std::string& name, esp_bd_addr_t address, int32_t rssi)
{
	ESP_LOGI( "Bluetooth_Source::ConnectToThisName", "Connect to this name: \"%s\" Address: \"%s\""
			, name.c_str()
			, GetAddressString(address));
	bool result = compatible_device_found(name, address, rssi);
	return result;
}
		
bool Bluetooth_Source::compatible_device_found(const std::string& name, esp_bd_addr_t address, int32_t rssi)
{
    bool found = false;
    std::string addressString(GetAddressString(address));
    ESP_LOGI("Bluetooth_Device", "Compatible Device Found. Name: \"%s\" Address: \"%s\"", name.c_str(), addressString.c_str());

    std::lock_guard<std::recursive_mutex> lock(m_ActiveCompatibleDevicesMutex);
    for (auto& device : m_ActiveCompatibleDevices)
    {
        if (strcmp(device.address, addressString.c_str()) == 0)
        {
            ESP_LOGI("Bluetooth_Device", "Compatible Device \"%s\" Updated", name.c_str());
            found = true;
            strncpy(device.name, name.c_str(), sizeof(device.name) - 1);
            device.name[sizeof(device.name) - 1] = '\0';
            device.rssi = rssi;
            device.lastUpdateTime = millis();
            break;
        }
    }
    if (!found)
    {
        ESP_LOGI("Bluetooth_Device", "New Compatible Device Found: %s", name.c_str());
        ActiveCompatibleDevice_t NewDevice;
        strncpy(NewDevice.name, name.c_str(), sizeof(NewDevice.name) - 1);
        NewDevice.name[sizeof(NewDevice.name) - 1] = '\0';
        strncpy(NewDevice.address, addressString.c_str(), sizeof(NewDevice.address) - 1);
        NewDevice.address[sizeof(NewDevice.address) - 1] = '\0';
        NewDevice.rssi = rssi;
        NewDevice.lastUpdateTime = millis();
        m_ActiveCompatibleDevices.push_back(NewDevice);
    }
    return found;
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
		std::lock_guard<std::recursive_mutex> lock(m_ActiveCompatibleDevicesMutex);
		unsigned long CurrentTime = millis();
		for(int i = 0; i < m_ActiveCompatibleDevices.size(); ++i)
		{
			if(CurrentTime - m_ActiveCompatibleDevices[i].lastUpdateTime >= BT_COMPATIBLE_DEVICE_TIMEOUT)
			{
				m_ActiveCompatibleDevices.erase(m_ActiveCompatibleDevices.begin()+i);
				break;
			}
		}
		for(int i = 0; i < m_ActiveCompatibleDevices.size(); ++i)
		{
			ESP_LOGD("Bluetooth_Device", "Scanned Device Name: %s \tRSSI: %i", m_ActiveCompatibleDevices[i].name, m_ActiveCompatibleDevices[i].rssi);
		}
		if(NULL != m_BluetoothActiveDeviceUpdatee)
		{
			m_BluetoothActiveDeviceUpdatee->BluetoothActiveDeviceListUpdated(m_ActiveCompatibleDevices);
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
void Bluetooth_Sink::StartDevice()
{
	if(false == m_Is_Running)
	{
		m_Is_Running = true;
		InstallDevice();
	}
}
void Bluetooth_Sink::StopDevice()
{
	Disconnect();
	if(true == m_Is_Running)
	{
		ESP_LOGI("Bluetooth_Device", "Stopping Bluetooth");
		m_BTSink.end(false);
		m_Is_Running = false;
		ESP_LOGI("Bluetooth_Device", "Bluetooth Stopped");
	}
}
void Bluetooth_Sink::Connect(String sinkName, bool reconnect)
{
	m_SinkName = sinkName;
	m_AutoReConnect = reconnect;
	m_BTSink.start(m_SinkName.c_str());
	ESP_LOGI("Bluetooth_Device", "Bluetooth Sink Started With NAME: \"%s\" Auto Reconnect: %i", m_SinkName.c_str(), m_AutoReConnect);
}
void Bluetooth_Sink::Disconnect()
{
	m_BTSink.disconnect();
	ESP_LOGI("Bluetooth_Device", "Bluetooth Sink Disconnected");
}

#endif