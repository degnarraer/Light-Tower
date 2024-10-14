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
	ESP_LOGI("Setup", "%s: Setup", GetTitle().c_str());
	m_DeviceProcessorQueueHandle = xQueueCreate(DEVICE_QUEUE_SIZE, sizeof(BT_Device_Info));
	if(m_DeviceProcessorQueueHandle != NULL)
	{
		ESP_LOGI("Setup", "Created compatible device Processor Queue.");
	}
	else
	{
		ESP_LOGE("Setup", "ERROR! Unable to create compatible device Processor Queue.");
	}
	if( xTaskCreatePinnedToCore( StaticCompatibleDeviceTrackerTaskLoop, "CompatibleDeviceTrackerTask", 10000, this, THREAD_PRIORITY_MEDIUM, &m_CompatibleDeviceTrackerTaskHandle, 1 ) == pdTRUE )
	{
		ESP_LOGI("Setup", "Created compatible device Tracker task.");
	}
	else
	{
		ESP_LOGE("Setup", "ERROR! Unable to create compatible device Tracker task.");
	}
	if(xTaskCreatePinnedToCore( StaticDeviceProcessingTask, "DeviceProcessingTask", 5000, this, THREAD_PRIORITY_MEDIUM, &m_DeviceProcessorTaskHandle, 1 ) == pdTRUE)
	{
		ESP_LOGI("Setup", "Created compatible device Processor task.");
	}
	else
	{
		ESP_LOGE("Setup", "ERROR! Unable to create compatible device Processor task.");
	}
}

void Bluetooth_Source::InstallDevice()
{
	ESP_LOGI("InstallDevice", "%s: Installing Bluetooth device.", GetTitle().c_str());
	if(!m_Is_Installed)
	{
		m_BTSource.set_reset_ble(m_ResetBLE);
		m_BTSource.set_auto_reconnect(m_AutoReConnect);
		m_BTSource.set_ssp_enabled(false);
		m_BTSource.set_local_name("LED Tower of Power");
		m_BTSource.set_task_core(1);
		m_BTSource.set_task_priority(THREAD_PRIORITY_HIGH);
		m_Is_Installed = true;
		ESP_LOGI("InstallDevice", "%s: Device installed", GetTitle().c_str());
	}
	else
	{
		ESP_LOGW("InstallDevice", "%s: Device already installed", GetTitle().c_str());
	}
}

void Bluetooth_Source::SetMusicDataCallback(music_data_cb_t callback)
{
	ESP_LOGI("SetMusicDataCallback", "Set music data callback");
	m_MusicDataCallback = callback;
}

void Bluetooth_Source::StartDevice()
{
	ESP_LOGI("Bluetooth_Device", "Starting Bluetooth");
	if(!m_Is_Installed)
	{
		InstallDevice();
	}
	m_Is_Started = true;
	ESP_LOGI("Bluetooth_Device", "Bluetooth started");
}

void Bluetooth_Source::StopDevice()
{
	if(true == m_Is_Connected)
	{
		ESP_LOGI("Bluetooth_Device", "Stopping Bluetooth");
		Disconnect();
		m_BTSource.end();
		m_Is_Started = false;
		m_Is_Installed = false;
		ESP_LOGI("Bluetooth_Device", "Bluetooth stopped");
	}
}

void Bluetooth_Source::Connect( const char *SourceName, const char *SourceAddress )
{
	StartDevice();
	m_Name = String(SourceName);
	m_Address = String(SourceAddress);
	ESP_LOGI("Bluetooth_Device", "Starting Bluetooth with: \n\tName: \"%s\" \n\tAddress: \"%s\"", m_Name.c_str(), m_Address.c_str());
	m_BTSource.start_raw(m_MusicDataCallback);
	m_Is_Connected = true;
}
void Bluetooth_Source::Disconnect()
{
	if(m_Is_Connected)
	{
		m_BTSource.disconnect();
		m_Is_Connected = false;
		ESP_LOGI("Bluetooth_Device", "Bluetooth source disconnected");
	}
}
void Bluetooth_Source::SetNameToConnect( const std::string& sourceName, const std::string& sourceAddress )
{
	ESP_LOGI( "ConnectToThisName", "Set name to connect: \"%s\" Address: \"%s\""
			, sourceName.c_str()
			, sourceAddress.c_str() );
	m_Name = String(sourceName.c_str());
	m_Address = String(sourceAddress.c_str());
}

//Callback from BT Source for compatible devices to connect to
bool Bluetooth_Source::ConnectToThisName(const std::string& name, esp_bd_addr_t address, int32_t rssi)
{
	const char* addressString = GetAddressString(address);
    ESP_LOGI("ConnectToThisName", "Connect to this device: Name: \"%s\" Address: \"%s\" RSSI: \"%i\" Target Name: \"%s\" Target Address: \"%s\"", name.c_str(), addressString, rssi, m_Name.c_str(), m_Address.c_str());
    if(m_Name.c_str() == name && m_Address == addressString)
	{
		return true;
	}
    else if(m_DeviceProcessorQueueHandle)
	{
		BT_Device_Info newDevice = BT_Device_Info(name.c_str(), addressString, rssi);
		if (xQueueSend(m_DeviceProcessorQueueHandle, &newDevice, (TickType_t)0) == pdPASS)
		{
			ESP_LOGD("ConnectToThisName", "Device info sent to queue");
		}
		else
		{
			ESP_LOGE("ConnectToThisName", "Failed to send device info to queue");
		}
	}
	else
	{
		ESP_LOGE("ConnectToThisName", "Queue Not Ready!");
	}
	return false;
}

void Bluetooth_Source::StaticDeviceProcessingTask(void * Parameters)
{
	Bluetooth_Source* BT_Source = (Bluetooth_Source*)Parameters;
	BT_Source->DeviceProcessingTask();
}

void Bluetooth_Source::DeviceProcessingTask()
{
  	while(true)
  	{
		BT_Device_Info receivedDevice;
        if (xQueueReceive(m_DeviceProcessorQueueHandle, &receivedDevice, portMAX_DELAY) == pdPASS)
        {
            Compatible_Device_Found(receivedDevice);
        }
    }
}

void Bluetooth_Source::Compatible_Device_Found(BT_Device_Info newDevice)
{
    ESP_LOGD("Bluetooth_Device", "compatible device found. Name: \"%s\" Address: \"%s\"", newDevice.name, newDevice.address);
	std::lock_guard<std::recursive_mutex> lock(m_ActiveCompatibleDevicesMutex);
	bool found = false;
	for (auto& device : m_ActiveCompatibleDevices)
	{
		if (device == newDevice)
		{
			found = true;
			device = newDevice;
			device.lastUpdateTime = millis();
			ESP_LOGD("Bluetooth_Device", "compatible device \"%s\" Updated", newDevice.name);
			break;
		}
	}
	if (!found)
	{
		ESP_LOGI("Bluetooth_Device", "New compatible device Found: %s", newDevice.name);
		m_ActiveCompatibleDevices.push_back(newDevice);
	}
}

void Bluetooth_Source::StaticCompatibleDeviceTrackerTaskLoop(void * Parameters)
{
	Bluetooth_Source* BT_Source = (Bluetooth_Source*)Parameters;
	BT_Source->CompatibleDeviceTrackerTaskLoop();
}

void Bluetooth_Source::CompatibleDeviceTrackerTaskLoop()
{
	const TickType_t xFrequency = 1000;
  	TickType_t xLastWakeTime = xTaskGetTickCount();
  	while(true)
  	{
		vTaskDelayUntil( &xLastWakeTime, xFrequency );
		std::lock_guard<std::recursive_mutex> lock(m_ActiveCompatibleDevicesMutex);
		unsigned long CurrentTime = millis();
		auto newEnd = std::remove_if(m_ActiveCompatibleDevices.begin(), m_ActiveCompatibleDevices.end(),
			[CurrentTime](const ActiveBluetoothDevice_t& device) {
				return CurrentTime - device.lastUpdateTime >= BT_COMPATIBLE_DEVICE_TIMEOUT;
			});
		m_ActiveCompatibleDevices.erase(newEnd, m_ActiveCompatibleDevices.end());
		if (m_BluetoothActiveDeviceUpdatee)
		{
			m_BluetoothActiveDeviceUpdatee->BluetoothActiveDeviceListUpdated(m_ActiveCompatibleDevices);
		}
	}
}

		
		
void Bluetooth_Sink::Setup()
{
	ESP_LOGI("Bluetooth_Device", "Bluetooth Sink: \"%s\": Setup", GetTitle().c_str());
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
		ESP_LOGI("Bluetooth Device", "Read data stream");
		m_Callee->BTDataReceived((uint8_t*)data, length);
	}
}

void Bluetooth_Sink::InstallDevice()
{
	ESP_LOGI("Bluetooth Device", "%s: Installing Bluetooth device.", GetTitle().c_str());
	if(!m_Is_Started)
	{
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
		m_Is_Installed = true;
		ESP_LOGI("Bluetooth_Device", "%s: Bluetooth Device installed", GetTitle().c_str());
	}
	else
	{
		ESP_LOGW("Bluetooth Device", "%s: Bluetooth device already installed.", GetTitle().c_str());
	}
}
void Bluetooth_Sink::StartDevice()
{
	ESP_LOGI("StartDevice", "Starting Bluetooth");
	if(!m_Is_Started)
	{
		InstallDevice();
		m_Is_Started = true;
		ESP_LOGI("StartDevice", "Bluetooth started");
	}
	else
	{
		ESP_LOGW("StopDevice", "Bluetooth already started.");
	}
}
void Bluetooth_Sink::StopDevice()
{
	ESP_LOGI("StopDevice", "Stopping Bluetooth");
	if(m_Is_Started)
	{
		Disconnect();
		m_BTSink.end();
		m_Is_Started = false;
		m_Is_Installed = false;
		ESP_LOGI("StopDevice", "Bluetooth stopped");
	}
	else
	{
		ESP_LOGW("StopDevice", "Bluetooth already stopped.");
	}
}
void Bluetooth_Sink::Connect(String sinkName, bool reconnect)
{
	if(!m_Is_Connected)
	{
		StartDevice();
		m_SinkName = sinkName;
		m_BTSink.start(m_SinkName.c_str());
		m_Is_Connected = true;
		ESP_LOGI("Bluetooth_Device", "Bluetooth sink started with Name: \"%s\" Auto Reconnect: %i", m_SinkName.c_str(), m_AutoReConnect);
	}
}
void Bluetooth_Sink::Disconnect()
{
	if(m_Is_Connected)
	{
		m_BTSink.disconnect();
		m_Is_Connected = false;
		ESP_LOGI("Bluetooth_Device", "Bluetooth sink disconnected");
	}
}

#endif