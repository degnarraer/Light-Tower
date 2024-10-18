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
	m_BTSource.set_reset_ble(m_ResetBLE);
	m_BTSource.set_auto_reconnect(m_AutoReConnect);
	m_BTSource.set_ssp_enabled(false);
	m_BTSource.set_local_name("LED Tower of Power.");
	m_BTSource.set_task_core(1);
	m_BTSource.set_task_priority(THREAD_PRIORITY_HIGH);
	ESP_LOGI("InstallDevice", "%s: Device installed.", GetTitle().c_str());
}

void Bluetooth_Source::StartDevice()
{
	InstallDevice();
  	m_BTSource.set_ssid_callback(StaticConnectToThisName);
  	m_BTSource.set_discovery_mode_callback(Static_Discovery_Mode_Changed);
	m_BTSource.set_on_connection_state_changed(StaticBluetoothConnectionStateChanged);
	m_BTSource.set_connectable(true);
}

void Bluetooth_Source::StopDevice()
{
	m_BTSource.set_connectable(false);
	m_BTSource.end();
}

void Bluetooth_Source::StartDiscovery()
{
	m_BTSource.start_Discovery();
}
void Bluetooth_Source::StopDiscovery()
{
	m_BTSource.stop_Discovery();
}

void Bluetooth_Source::Connect()
{
	m_BTSource.start_raw(StaticSetBTTxData);
}

void Bluetooth_Source::Disconnect()
{
	m_BTSource.stop_Discovery();
	m_BTSource.disconnect();
}

void Bluetooth_Source::SetNameToConnect( const std::string& sourceName, const std::string& sourceAddress )
{
	ESP_LOGI( "ConnectToThisName", "Set name to connect: \"%s\" Address: \"%s\""
			, sourceName.c_str()
			, sourceAddress.c_str() );
	m_Name = String(sourceName.c_str());
	m_Address = String(sourceAddress.c_str());
}

void Bluetooth_Source::ResgisterForCallbacks(Bluetooth_Source_Callbacks *callee) 
{ 
	m_Callee = callee;
}

//Bluetooth_Callbacks
int32_t Bluetooth_Source::StaticMusicDataCallback(uint8_t *data, int32_t len)
{
	if (bT_source_instance) {
		return bT_source_instance->MusicDataCallback(data, len);
	}
	return 0;
}
int32_t Bluetooth_Source::MusicDataCallback(uint8_t *data, int32_t len)
{
	ESP_LOGI("BTDataReceived", "BT Data: %i bytes received.", len);
	if(NULL != m_Callee)
	{
		ESP_LOGI("Bluetooth Device", "Read data stream");
		return m_Callee->MusicDataCallback(data, len);
	}
	return 0;

}

void Bluetooth_Sink::StaticBTDataReceived() 
{
    if (bT_sink_instance) {
        bT_sink_instance->BTDataReceived();
    }
}

void Bluetooth_Sink::BTDataReceived()
{
  	ESP_LOGI("BTDataReceived", "BT Data: received.");
	if(NULL != m_Callee)
	{
		ESP_LOGI("Bluetooth Device", "Read data stream");
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
}
void Bluetooth_Source::StaticBTReadDataStream(const uint8_t* data, uint32_t length) 
{
    if (bT_source_instance) {
        bT_source_instance->BTReadDataStream(data, length);
    }
}

void Bluetooth_Source::BTReadDataStream(const uint8_t *data, uint32_t length)
{
  ESP_LOGI("BTDataReceived", "BT Data: %i bytes received.", length);
	if(NULL != m_Callee)
	{
		ESP_LOGI("Bluetooth Device", "Read data stream");
		m_Callee->BT_Read_Data_Stream(data, length);
	}
}

void Bluetooth_Source::StaticBTDataReceived() 
{
    if (bT_source_instance) {
        bT_source_instance->BTDataReceived();
    }
}

void Bluetooth_Source::BTDataReceived()
{
  	ESP_LOGI("BTDataReceived", "BT Data: received.");
	if(NULL != m_Callee)
	{
		ESP_LOGI("Bluetooth Device", "Read data stream");
		m_Callee->BT_Data_Received();
	}
}

int32_t Bluetooth_Source::StaticSetBTTxData(uint8_t *data, int32_t length)
{
	if (bT_source_instance) {
		return bT_source_instance->SetBTTxData(data, length);
	}
	return 0;
}

int32_t Bluetooth_Source::SetBTTxData(uint8_t *data, int32_t length)
{
	ESP_LOGI("BTDataReceived", "BT Data: %i bytes received.", length);
	if(NULL != m_Callee)
	{
		ESP_LOGI("Bluetooth Device", "Read data stream");
		return m_Callee->SetBTTxData(data, length);
	}
	return 0;
}
	
void Bluetooth_Source::StaticBluetoothConnectionStateChanged(const esp_a2d_connection_state_t connectionState, void* object)
{
	if (bT_source_instance) {
		bT_source_instance->BluetoothConnectionStateChanged(connectionState, object);
	}
}
void Bluetooth_Source::BluetoothConnectionStateChanged(const esp_a2d_connection_state_t connectionState, void* object)
{
	ESP_LOGI("BluetoothConnectionStateChanged", "Connection State: %i", connectionState);
	if(NULL != m_Callee)
	{
		ESP_LOGI("Bluetooth Device", "Read data stream");
		m_Callee->BluetoothConnectionStateChanged(connectionState, object);
	}
}
void Bluetooth_Source::Static_Discovery_Mode_Changed(esp_bt_gap_discovery_state_t discoveryMode)
{
	if (bT_source_instance) {
		bT_source_instance->Discovery_Mode_Changed(discoveryMode);
	}
}
void Bluetooth_Source::Discovery_Mode_Changed(esp_bt_gap_discovery_state_t discoveryMode)
{
	ESP_LOGI("Discovery_Mode_Changed", "Discovery Mode: %i", discoveryMode);
	if(NULL != m_Callee)
	{
		ESP_LOGI("Bluetooth Device", "Read data stream");
		m_Callee->Discovery_Mode_Changed(discoveryMode);
	}
}

bool Bluetooth_Source::StaticConnectToThisName(const char* name, esp_bd_addr_t address, int rssi)
{
    if (bT_source_instance) {
        return bT_source_instance->ConnectToThisName(std::string(name), address, rssi);
    }
    return false;
}

bool Bluetooth_Source::ConnectToThisName(std::string name, esp_bd_addr_t address, int rssi)
{
    std::string addressString = GetAddressString(address);
    ESP_LOGI("ConnectToThisName", "Connect to this device: Name: \"%s\" Address: \"%s\" RSSI: \"%i\" Target Name: \"%s\" Target Address: \"%s\"", 
             name.c_str(), addressString.c_str(), rssi, m_Name.c_str(), m_Address.c_str());
    
    // Use string content comparison, not pointer comparison
    if (m_Name.c_str() == name && m_Address.c_str() == addressString) {
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

void Bluetooth_Source::Set_NVS_Init(bool ResetNVS)
{ 
	m_BTSource.set_nvs_init(ResetNVS);
}
void Bluetooth_Source::Set_Reset_BLE(bool ResetBLE)
{
	m_ResetBLE = ResetBLE;
}
void Bluetooth_Source::Set_Auto_Reconnect(bool AutoReConnect)
{
	m_AutoReConnect = AutoReConnect;
	m_BTSource.set_auto_reconnect(m_AutoReConnect);
}
void Bluetooth_Source::Set_SSP_Enabled(bool SSPEnabled)
{
	m_BTSource.set_ssp_enabled(SSPEnabled);
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
	if(BT_Source)
	{
		BT_Source->DeviceProcessingTask();
	}
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
	if(BT_Source)
	{
		BT_Source->CompatibleDeviceTrackerTaskLoop();
	}
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
		if (m_Callee)
		{
			m_Callee->BluetoothActiveDeviceListUpdated(m_ActiveCompatibleDevices);
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
  ESP_LOGI("BTDataReceived", "BT Data: %i bytes received.", length);
	if(NULL != m_Callee)
	{
		ESP_LOGI("Bluetooth Device", "Read data stream");
		m_Callee->BT_Read_Data_Stream(data, length);
	}
}


void Bluetooth_Sink::InstallDevice()
{
	ESP_LOGI("Bluetooth Device", "%s: Installing Bluetooth device.", GetTitle().c_str());
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
	m_BTSink.set_task_priority(THREAD_PRIORITY_HIGH);
	m_BTSink.set_volume_control(&m_VolumeControl);
	m_BTSink.set_volume(100);
	ESP_LOGI("Bluetooth_Device", "%s: Bluetooth Device installed", GetTitle().c_str());

}
void Bluetooth_Sink::StartDevice()
{
	InstallDevice();
	m_BTSink.set_stream_reader(StaticBTReadDataStream, true);
  	m_BTSink.set_on_data_received(StaticBTDataReceived);
	m_BTSink.set_on_connection_state_changed(StaticBluetoothConnectionStateChanged);
	m_BTSink.set_connectable(true);
}
void Bluetooth_Sink::StopDevice()
{
	m_BTSink.set_connectable(false);
	m_BTSink.end();
}
void Bluetooth_Sink::Connect(String sinkName, bool reconnect)
{
	m_SinkName = sinkName;
	m_BTSink.set_auto_reconnect(reconnect);
	m_BTSink.start(m_SinkName.c_str());
}
void Bluetooth_Sink::Disconnect()
{
	m_BTSink.disconnect();
}
void Bluetooth_Sink::Set_Auto_Reconnect(bool reconnect, int count)
{
	m_BTSink.set_auto_reconnect(reconnect, count);
}

#endif