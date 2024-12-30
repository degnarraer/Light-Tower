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

#ifdef BUILD_BLUETOOTH

#include "Bluetooth_Device.h"

Bluetooth_Source* Bluetooth_Source::bT_source_instance = nullptr;
void Bluetooth_Source::Setup()
{
	ESP_LOGI("Setup", "\"%s\": Setup", GetTitle().c_str());
	m_DeviceProcessorQueueHandle = xQueueCreate(DEVICE_QUEUE_SIZE, sizeof(BT_Device_Info));
	m_ActiveCompatibleDevicesSemaphore = xSemaphoreCreateMutex();
	if (m_ActiveCompatibleDevicesSemaphore == nullptr)
	{
		ESP_LOGE("WebSocketDataProcessor", "ERROR! Failed to create semaphore.");
	}
	if(m_DeviceProcessorQueueHandle != NULL)
	{
		ESP_LOGI("Setup", "Created compatible device Processor Queue.");
	}
	else
	{
		ESP_LOGE("Setup", "ERROR! Unable to create compatible device Processor Queue.");
	}
	if( xTaskCreate( StaticCompatibleDeviceTrackerTaskLoop, "CompatibleDeviceTrackerTask", 10000, this, THREAD_PRIORITY_LOW, &m_CompatibleDeviceTrackerTaskHandle ) == pdTRUE )
	{
		ESP_LOGI("Setup", "Created compatible device Tracker task.");
	}
	else
	{
		ESP_LOGE("Setup", "ERROR! Unable to create compatible device Tracker task.");
	}
	if(xTaskCreate( StaticDeviceProcessingTask, "DeviceProcessingTask", 5000, this, THREAD_PRIORITY_LOW, &m_DeviceProcessorTaskHandle ) == pdTRUE)
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
	ESP_LOGI("InstallDevice", "\"%s\": Installing Bluetooth device. Device currently: \"%s\".", GetTitle().c_str(), GetDeviceStateString().c_str());
	if(m_DeviceState == DeviceState_t::Uninstalled)
	{
		m_BTSource.set_ssp_enabled(false);
		m_BTSource.set_local_name("LED Tower of Power.");
		m_BTSource.set_ssid_callback(StaticConnectToThisName);
		m_BTSource.set_discovery_mode_callback(Static_Discovery_Mode_Changed);
		m_BTSource.set_on_connection_state_changed(StaticBluetoothConnectionStateChanged);
		m_BTSource.set_task_core(m_Core);
		m_BTSource.set_task_priority(m_Priority);
		m_BTSource.set_event_stack_size(5000);
		m_DeviceState = DeviceState_t::Installed;
		ESP_LOGI("InstallDevice", "\"%s\": Bluetooth Device installed. Device currently: \"%s\".", GetTitle().c_str(), GetDeviceStateString().c_str());
	}
	else
	{
		ESP_LOGW("InstallDevice", "WARNING! \"%s\": Bluetooth Device already installed", GetTitle().c_str());
	}
}

void Bluetooth_Source::StartDevice()
{
	ESP_LOGI("StartDevice", "\"%s\": Starting Bluetooth Device. Device currently: \"%s\".", GetTitle().c_str(), GetDeviceStateString().c_str());
	if(m_DeviceState != DeviceState_t::Running)
	{
		InstallDevice();
    	m_DeviceState = DeviceState_t::Running;
		ESP_LOGI("StartDevice", "\"%s\": Bluetooth Device Started. Device currently: \"%s\".", GetTitle().c_str(), GetDeviceStateString().c_str());
	}
	else
	{
		ESP_LOGW("StartDevice", "WARNING! \"%s\": Bluetooth Device already running. Device currently: \"%s\".", GetTitle().c_str(), GetDeviceStateString().c_str());
	}
}

void Bluetooth_Source::StopDevice()
{
	ESP_LOGI("StopDevice", "\"%s\": Stopping Bluetooth Device. Device currently: \"%s\".", GetTitle().c_str(), GetDeviceStateString().c_str());
	if(m_DeviceState == DeviceState_t::Running)
	{
		m_BTSource.end();
    	m_DeviceState = DeviceState_t::Stopped;
		ESP_LOGI("StopDevice", "\"%s\": Bluetooth Device Stopped. Device currently: \"%s\".", GetTitle().c_str(), GetDeviceStateString().c_str());
	}
	else
	{
		ESP_LOGW("StopDevice", "WARNING! \"%s\": Bluetooth Device not running. Device currently: \"%s\".", GetTitle().c_str(), GetDeviceStateString().c_str());
	}
}

String Bluetooth_Source::GetDeviceStateString()
{
  String result = "";
  switch(m_DeviceState)
  {
    case DeviceState_t::Installed:
      result = "Installed";
    break;
    case DeviceState_t::Uninstalled:
      result = "Uninstalled";
    break;
    case DeviceState_t::Running:
      result = "Running";
    break;
    case DeviceState_t::Stopped:
      result = "Stopped";
    break;
    default:
      result = "Unknown";
    break;
  }
  return result;
}

void Bluetooth_Source::Connect()
{
	m_BTSource.start_raw(StaticSetBTTxData);
}

void Bluetooth_Source::Disconnect()
{
	m_BTSource.disconnect();
}

void Bluetooth_Source::SetNameToConnect( const std::string& sourceName, const std::string& sourceAddress )
{
	ESP_LOGI( "ConnectToThisName", "Set name to connect: \"%s\" Address: \"%s\""
			, sourceName.c_str()
			, sourceAddress.c_str() );
	m_Name = sourceName;
	m_Address = sourceAddress;
}

void Bluetooth_Source::ResgisterForCallbacks(Bluetooth_Source_Callbacks *callee) 
{
	m_Callee = callee;
}

//Bluetooth_Callbacks
int32_t Bluetooth_Source::StaticMusicDataCallback(uint8_t *data, int32_t len)
{
	return bT_source_instance->MusicDataCallback(data, len);
}

int32_t Bluetooth_Source::MusicDataCallback(uint8_t *data, int32_t len)
{
	if(m_Callee)
	{
		return m_Callee->MusicDataCallback(data, len);
	}
	return 0;
}

void Bluetooth_Sink::StaticBTDataReceived() 
{
    bT_sink_instance->BTDataReceived();
}

void Bluetooth_Sink::BTDataReceived()
{
	if(m_Callee)
	{
		m_Callee->BT_Data_Received();
	}
}

Bluetooth_Source::~Bluetooth_Source()
{
	if(m_CompatibleDeviceTrackerTaskHandle)
	{
		vTaskDelete(m_CompatibleDeviceTrackerTaskHandle);
		m_CompatibleDeviceTrackerTaskHandle = nullptr;
	}
	if(m_DeviceProcessorTaskHandle)
	{
		vTaskDelete(m_DeviceProcessorTaskHandle);
		m_DeviceProcessorTaskHandle = nullptr;
	}
	if(m_DeviceProcessorQueueHandle) 
	{
		vQueueDelete(m_DeviceProcessorQueueHandle);
		m_DeviceProcessorQueueHandle = nullptr;
	}
	if(m_ActiveCompatibleDevicesSemaphore)
	{
		vSemaphoreDelete(m_ActiveCompatibleDevicesSemaphore);
		m_ActiveCompatibleDevicesSemaphore = nullptr;
	}
}
void Bluetooth_Source::StaticBTReadDataStream(const uint8_t* data, uint32_t length) 
{
    bT_source_instance->BTReadDataStream(data, length);
}

void Bluetooth_Source::BTReadDataStream(const uint8_t *data, uint32_t length)
{
	if(m_Callee)
	{
		m_Callee->BT_Read_Data_Stream(data, length);
	}
}

void Bluetooth_Source::StaticBTDataReceived() 
{
    bT_source_instance->BTDataReceived();
}

void Bluetooth_Source::BTDataReceived()
{
	if(m_Callee)
	{
		m_Callee->BT_Data_Received();
	}
}

int32_t Bluetooth_Source::StaticSetBTTxData(uint8_t *data, int32_t length)
{
	return bT_source_instance->SetBTTxData(data, length);
}

int32_t Bluetooth_Source::SetBTTxData(uint8_t *data, int32_t length)
{
	if(m_Callee)
	{
		return m_Callee->SetBTTxData(data, length);
	}
	return 0;
}
	
void Bluetooth_Source::StaticBluetoothConnectionStateChanged(const esp_a2d_connection_state_t connectionState, void* object)
{
	bT_source_instance->BluetoothConnectionStateChanged(connectionState, object);
}
void Bluetooth_Source::BluetoothConnectionStateChanged(const esp_a2d_connection_state_t connectionState, void* object)
{
	if(m_Callee)
	{
		m_Callee->BluetoothConnectionStateChanged(connectionState, object);
	}
}
void Bluetooth_Source::Static_Discovery_Mode_Changed(esp_bt_gap_discovery_state_t discoveryMode)
{
	bT_source_instance->Discovery_Mode_Changed(discoveryMode);
}
void Bluetooth_Source::Discovery_Mode_Changed(esp_bt_gap_discovery_state_t discoveryMode)
{
	if(m_Callee)
	{
		m_Callee->Discovery_Mode_Changed(discoveryMode);
	}
}

bool Bluetooth_Source::StaticConnectToThisName(const char* name, esp_bd_addr_t address, int rssi)
{
    return bT_source_instance->ConnectToThisName(std::string(name), address, rssi); 
}

bool Bluetooth_Source::ConnectToThisName(std::string name, esp_bd_addr_t address, int rssi)
{
    std::string addressString = GetAddressString(address);
    ESP_LOGD("ConnectToThisName", "Connect to this device: Name: \"%s\" Address: \"%s\" RSSI: \"%i\" Target Name: \"%s\" Target Address: \"%s\"", 
             name, addressString, rssi, m_Name, m_Address);
    
    // Use string content comparison, not pointer comparison
    if (m_Name == name && m_Address == addressString) {
        return true;
    }
    else if (m_DeviceProcessorQueueHandle)
    {
        BT_Device_Info newDevice(name.c_str(), addressString.c_str(), rssi);
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

void Bluetooth_Source::Set_NVS_Init(bool resetNVS)
{
	m_BTSource.set_nvs_init(resetNVS);
}
void Bluetooth_Source::Set_Reset_BLE(bool resetBLE)
{
	m_BTSource.set_reset_ble(resetBLE);
}
void Bluetooth_Source::Set_Auto_Reconnect(bool autoReConnect)
{
	m_BTSource.set_auto_reconnect(autoReConnect);
}
void Bluetooth_Source::Set_SSP_Enabled(bool sSPEnabled)
{
	m_BTSource.set_ssp_enabled(sSPEnabled);
}

/// converts a esp_bd_addr_t to a string - the string is 18 characters long!
const char* Bluetooth_Source::GetAddressString(esp_bd_addr_t bda)
{
	static char bda_str[18];
	sprintf(bda_str, "%02x:%02x:%02x:%02x:%02x:%02x", bda[0], bda[1], bda[2], bda[3], bda[4], bda[5]);
	return (const char*)bda_str;
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
		while(xQueueReceive(m_DeviceProcessorQueueHandle, &receivedDevice, pdMS_TO_TICKS(0)) == pdTRUE)
		{
			Compatible_Device_Found(receivedDevice);
			vTaskDelay(pdMS_TO_TICKS(20));
		}
		vTaskDelay(pdMS_TO_TICKS(20));
    }
}

void Bluetooth_Source::Compatible_Device_Found(BT_Device_Info newDevice)
{
	if (xSemaphoreTakeRecursive(m_ActiveCompatibleDevicesSemaphore, pdMS_TO_TICKS(0)) == pdTRUE)
	{
		ESP_LOGD("Bluetooth_Device", "compatible device found. Name: \"%s\" Address: \"%s\"", newDevice.name, newDevice.address);
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
		xSemaphoreGiveRecursive(m_ActiveCompatibleDevicesSemaphore);
	}
	else
	{
		ESP_LOGW("Semaphore Take Failure", "WARNING! Failed to take Semaphore");
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

	while (true)
	{
		vTaskDelayUntil(&xLastWakeTime, xFrequency);
		unsigned long CurrentTime = millis();
		std::vector<ActiveBluetoothDevice_t> activeDevicesCopy;

		if (xSemaphoreTakeRecursive(m_ActiveCompatibleDevicesSemaphore, pdMS_TO_TICKS(5)) == pdTRUE)
		{
			// Use standard erase-remove idiom to filter devices
			auto newEnd = std::remove_if(
				m_ActiveCompatibleDevices.begin(),
				m_ActiveCompatibleDevices.end(),
				[CurrentTime](const ActiveBluetoothDevice_t& device) {
					return (CurrentTime - device.lastUpdateTime) >= BT_COMPATIBLE_DEVICE_TIMEOUT;
				});

			// Erase outdated devices
			m_ActiveCompatibleDevices.erase(newEnd, m_ActiveCompatibleDevices.end());

			// Make a copy of the updated device list
			activeDevicesCopy = m_ActiveCompatibleDevices;

			// Release the semaphore after modifications
			xSemaphoreGiveRecursive(m_ActiveCompatibleDevicesSemaphore);
		}
        else
        {
            ESP_LOGW("Semaphore Take Failure", "WARNING! Failed to take Semaphore");
        }

		// Notify the callee about updated active devices
		if (m_Callee)
		{
			m_Callee->BluetoothActiveDeviceListUpdated(activeDevicesCopy);
		}
	}
}


Bluetooth_Sink* Bluetooth_Sink::bT_sink_instance = nullptr;
void Bluetooth_Sink::Setup()
{
	ESP_LOGI("Bluetooth_Device", "Bluetooth Sink: \"%s\": Setting Up", GetTitle().c_str());
	esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT, ESP_PWR_LVL_P9);
	ESP_LOGI("Bluetooth_Device", "Bluetooth Sink: \"%s\": Setup", GetTitle().c_str());
}
void Bluetooth_Sink::ResgisterForCallbacks(Bluetooth_Sink_Callbacks* callee){ m_Callee = callee; }

//Callbacks for Bluetooth
void Bluetooth_Sink::StaticBTReadDataStream(const uint8_t* data, uint32_t length) 
{
    if (bT_sink_instance) {
        bT_sink_instance->BTReadDataStream(data, length);
    }
}

void Bluetooth_Sink::BTReadDataStream(const uint8_t *data, uint32_t length)
{
  	ESP_LOGV("BTDataReceived", "BT Data: %i bytes received.", length);
	if(m_Callee)
	{
		m_Callee->BT_Read_Data_Stream(data, length);
	}
}

void Bluetooth_Sink::InstallDevice()
{
	ESP_LOGI("InstallDevice", "\"%s\": Installing Bluetooth Device. Device currently: \"%s\".", GetTitle().c_str(), GetDeviceStateString().c_str());
	if(m_DeviceState == DeviceState_t::Uninstalled)
	{
		static i2s_config_t i2s_config = {
		.mode = m_i2s_Mode,
		.sample_rate = m_SampleRate,
		.bits_per_sample = m_BitsPerSample,
		.channel_format = m_Channel_Fmt,
		.communication_format = m_CommFormat,
		.intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
		.dma_buf_count = m_BufferCount,
		.dma_buf_len = m_BufferSize,
		.use_apll = m_Use_APLL,
		.tx_desc_auto_clear = true,
		.fixed_mclk = m_FixedClock
		};
		i2s_pin_config_t my_pin_config = 
		{
			.bck_io_num = m_I2SClockPin,
			.ws_io_num = m_I2SWordSelectPin,
			.data_out_num = m_I2SDataOutPin,
			.data_in_num = m_I2SDataInPin
		};
		m_BTSink.set_task_core(m_Core);
		m_BTSink.set_task_priority(m_Priority);
		m_BTSink.set_pin_config(my_pin_config);
		m_BTSink.set_i2s_config(i2s_config);
		m_BTSink.set_i2s_port(m_I2S_PORT);
		m_BTSink.set_bits_per_sample(m_BitsPerSample);
		m_BTSink.set_event_stack_size(5000);
		m_BTSink.set_volume_control(&m_VolumeControl);
		m_BTSink.set_volume(100);
		m_BTSink.set_stream_reader(StaticBTReadDataStream, true);
		m_BTSink.set_on_data_received(StaticBTDataReceived);
		m_BTSink.set_on_connection_state_changed(StaticBluetoothConnectionStateChanged);
		m_DeviceState = DeviceState_t::Installed;
		ESP_LOGI("InstallDevice", "\"%s\": Bluetooth Device installed. Device currently \"%s\".", GetTitle().c_str(), GetDeviceStateString().c_str());
	}
	else
	{
		ESP_LOGW("InstallDevice", "WARNING! \"%s\": Bluetooth Device already installed. Device currently \"%s\".", GetTitle().c_str(), GetDeviceStateString().c_str());
	}
}
void Bluetooth_Sink::UninstallDevice()
{
	ESP_LOGI("UninstallDevice", "\"%s\": Uninstalling Bluetooth Device. Device currently \"%s\".", GetTitle().c_str(), GetDeviceStateString().c_str());
	if(m_DeviceState == DeviceState_t::Stopped)
	{
		m_BTSink.end();
		m_DeviceState = DeviceState_t::Uninstalled;
		ESP_LOGI("UninstallDevice", "\"%s\": Bluetooth Device uninstalled. Device currently \"%s\".", GetTitle().c_str(), GetDeviceStateString().c_str());
	}
	else
	{
		ESP_LOGW("UninstallDevice", "WARNING! \"%s\": Bluetooth Device not stopped. Device currently \"%s\".", GetTitle().c_str(), GetDeviceStateString().c_str());
	}
}
void Bluetooth_Sink::StartDevice()
{
	ESP_LOGI("UninstallDevice", "\"%s\": Starting Bluetooth Device. Device currently \"%s\".", GetTitle().c_str(), GetDeviceStateString().c_str());
	if(m_DeviceState != DeviceState_t::Running)
	{
		InstallDevice();
		m_DeviceState = DeviceState_t::Running;
		ESP_LOGI("StartDevice", "\"%s\": Bluetooth Device Started. Device currently \"%s\".", GetTitle().c_str(), GetDeviceStateString().c_str());
	}
	else
	{
		ESP_LOGW("StartDevice", "WARNING! \"%s\": Bluetooth Device already running. Device currently \"%s\".", GetTitle().c_str(), GetDeviceStateString().c_str());
	}
}
void Bluetooth_Sink::StopDevice()
{
	ESP_LOGI("StopDevice", "\"%s\": Stopping Bluetooth Device. Device currently \"%s\".", GetTitle().c_str(), GetDeviceStateString().c_str());
	if(m_DeviceState == DeviceState_t::Running)
	{
		Disconnect();
		m_DeviceState = DeviceState_t::Stopped;
		ESP_LOGI("StopDevice", "\"%s\": Bluetooth Device stopped. Device currently \"%s\".", GetTitle().c_str(), GetDeviceStateString().c_str());
		UninstallDevice();	
	}
	else
	{
		ESP_LOGW("StopDevice", "WARNING! \"%s\": Bluetooth Device already stopped. Device currently \"%s\".", GetTitle().c_str(), GetDeviceStateString().c_str());
	}
}
void Bluetooth_Sink::Connect(String sinkName, bool reconnect)
{
	ESP_LOGI("Connect", "\"%s\": Connecting Bluetooth Device. Device currently \"%s\".", GetTitle().c_str(), GetDeviceStateString().c_str());
	if(m_DeviceState == DeviceState_t::Running)
	{
		m_SinkName = sinkName;
		m_BTSink.start(m_SinkName.c_str(), reconnect);
		ESP_LOGI("Connect", "\"%s\": Bluetooth Device Connecting. Device currently \"%s\".", GetTitle().c_str(), GetDeviceStateString().c_str());
	}
	else
	{
		ESP_LOGW("Connect", "WARNING! \"%s\": Bluetooth must be running. Device currently \"%s\".", GetTitle().c_str(), GetDeviceStateString().c_str());
	}
}
void Bluetooth_Sink::Disconnect()
{
	ESP_LOGI("Disconnect", "\"%s\": Disconnecting Bluetooth Device. Device currently \"%s\".", GetTitle().c_str(), GetDeviceStateString().c_str());
	if(m_DeviceState == DeviceState_t::Running)
	{
		m_BTSink.disconnect();
		ESP_LOGI("Disconnect", "\"%s\": Bluetooth Device Disconnected. Device currently \"%s\".", GetTitle().c_str(), GetDeviceStateString().c_str());
	}
	else
	{
		ESP_LOGW("Disconnect", "WARNING! \"%s\": Bluetooth must be running", GetTitle().c_str());
	}
}

String Bluetooth_Sink::GetDeviceStateString()
{
  String result = "";
  switch(m_DeviceState)
  {
    case DeviceState_t::Installed:
      result = "Installed";
    break;
    case DeviceState_t::Uninstalled:
      result = "Uninstalled";
    break;
    case DeviceState_t::Running:
      result = "Running";
    break;
    case DeviceState_t::Stopped:
      result = "Stopped";
    break;
    default:
      result = "Unknown";
    break;
  }
  return result;
}

void Bluetooth_Sink::Set_Auto_Reconnect(bool reconnect, int count)
{
	m_BTSink.set_auto_reconnect(reconnect, count);
}

#endif