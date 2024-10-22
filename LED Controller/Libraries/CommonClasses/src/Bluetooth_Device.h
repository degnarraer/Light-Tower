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

#pragma once
#define BT_COMPATIBLE_DEVICE_TIMEOUT 10000
#define DEVICE_QUEUE_SIZE 10

#include <vector>
#include <Arduino.h>
#include <mutex>
#include <queue>
#include <memory>
#include "BluetoothA2DPSink.h"
#include "BluetoothA2DPSource.h"
#include "BluetoothA2DPCommon.h"
#include "Helpers.h"


class Bluetooth_Source_Callbacks
{
	public:
		Bluetooth_Source_Callbacks(){}
		virtual ~Bluetooth_Source_Callbacks(){}
	
		//Callbacks called by this class
		virtual void BT_Data_Received() = 0;
		virtual void BT_Read_Data_Stream(const uint8_t *data, uint32_t length) = 0;
		virtual int32_t SetBTTxData(uint8_t *Data, int32_t channel_len) = 0;
		virtual void Discovery_Mode_Changed(esp_bt_gap_discovery_state_t discoveryMode) = 0;
		virtual int32_t MusicDataCallback(uint8_t *data, int32_t len) = 0;
		virtual void BluetoothConnectionStateChanged(const esp_a2d_connection_state_t connectionState, void* object) = 0;
		virtual void BluetoothActiveDeviceListUpdated(const std::vector<ActiveBluetoothDevice_t> Devices) = 0;
};

class Bluetooth_Source: public NamedItem
					  , public CommonUtils
					  , public QueueController
{	
	public:

		enum class DeviceState
		{
			Installed,
			Uninstalled,
			Running,
			Stopped
		};
		
		Bluetooth_Source( String Title
						, BluetoothA2DPSource& BTSource)
						: NamedItem(Title)
						, m_BTSource(BTSource)
		{
			bT_source_instance = this;
		}
		Bluetooth_Source(const Bluetooth_Source&) = delete;
		virtual ~Bluetooth_Source();
		static Bluetooth_Source* bT_source_instance;
		void Setup();
		void StartDevice();
		void StopDevice();
		void StartDiscovery();
		void StopDiscovery();
		void Connect();
		void Disconnect();
		void SetNameToConnect( const std::string& SourceName, const std::string& SourceAddress );
		
		//Callback Registrtion to this class
		void ResgisterForCallbacks(Bluetooth_Source_Callbacks *callee);

		//Bluetooth_Callbacks
		static int32_t StaticMusicDataCallback(uint8_t *data, int32_t len);
		int32_t MusicDataCallback(uint8_t *data, int32_t len);
		static void StaticBTDataReceived();
		void BTDataReceived();
		static void StaticBTReadDataStream(const uint8_t* data, uint32_t length);
		void BTReadDataStream(const uint8_t *data, uint32_t length);
		static int32_t StaticSetBTTxData(uint8_t *data, int32_t length);
		int32_t SetBTTxData(uint8_t *data, int32_t length);
		static void StaticBluetoothConnectionStateChanged(const esp_a2d_connection_state_t connectionState, void* object);
		void BluetoothConnectionStateChanged(const esp_a2d_connection_state_t connectionState, void* object);
		static void Static_Discovery_Mode_Changed(esp_bt_gap_discovery_state_t discoveryMode);
		void Discovery_Mode_Changed(esp_bt_gap_discovery_state_t discoveryMode);
		static bool StaticConnectToThisName(const char* name, esp_bd_addr_t address, int rssi);
		bool ConnectToThisName(std::string name, esp_bd_addr_t address, int rssi);
		void Set_NVS_Init(bool resetNVS);
		void Set_Reset_BLE(bool resetBLE);
		void Set_Auto_Reconnect(bool autoReConnect);
		void Set_SSP_Enabled(bool sSPEnabled);

		/// converts a esp_bd_addr_t to a string - the string is 18 characters long!
		const char* GetAddressString(esp_bd_addr_t bda);

	private:
	
		BluetoothA2DPSource& m_BTSource;
		Bluetooth_Source_Callbacks* m_Callee = NULL;
		music_data_cb_t m_MusicDataCallback = NULL;
		String m_Name;
		String m_Address;
		std::recursive_mutex m_ActiveCompatibleDevicesMutex;
		std::vector<ActiveBluetoothDevice_t> m_ActiveCompatibleDevices;
		QueueHandle_t m_DeviceProcessorQueueHandle= nullptr;
		TaskHandle_t m_CompatibleDeviceTrackerTaskHandle= nullptr;
		TaskHandle_t m_DeviceProcessorTaskHandle= nullptr;

		
    	DeviceState m_DeviceState = DeviceState::Uninstalled;
		void InstallDevice();
		void Compatible_Device_Found(BT_Device_Info newDevice);
		static void StaticCompatibleDeviceTrackerTaskLoop(void * Parameters);
		void CompatibleDeviceTrackerTaskLoop();
		static void StaticDeviceProcessingTask(void * Parameters);
		void DeviceProcessingTask();
};

class Bluetooth_Sink_Callbacks
{
	public:
		Bluetooth_Sink_Callbacks(){}
		virtual ~Bluetooth_Sink_Callbacks(){}
	
		virtual void BT_Data_Received() = 0;
		virtual void BT_Read_Data_Stream(const uint8_t *data, uint32_t length) = 0;
		virtual void BluetoothConnectionStateChanged(const esp_a2d_connection_state_t connectionState, void* object) = 0;
};

class Bluetooth_Sink: public NamedItem
					, public CommonUtils
				    , public QueueController
{
	enum class DeviceState
	{
		Installed,
		Uninstalled,
		Running,
		Stopped
	};
		
  public:
    Bluetooth_Sink( String Title
				  , BluetoothA2DPSink& BTSink
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
				  , m_I2S_PORT(i2S_PORT)
				  , m_i2s_Mode(Mode)
				  , m_SampleRate(SampleRate)
				  , m_BitsPerSample(BitsPerSample)
				  , m_CommFormat(i2s_CommFormat)
				  , m_Channel_Fmt(i2s_Channel_Fmt)
				  , m_i2s_channel(i2s_channel)
				  , m_Use_APLL(Use_APLL)
				  , m_BufferCount(BufferCount)
				  , m_BufferSize(BufferSize)
				  , m_I2SClockPin(SerialClockPin)
				  , m_I2SWordSelectPin(WordSelectPin)
				  , m_I2SDataInPin(SerialDataInPin)
				  , m_I2SDataOutPin(SerialDataOutPin)
	{
		bT_sink_instance = this;
	};		
	virtual ~Bluetooth_Sink(){};
	static Bluetooth_Sink* bT_sink_instance;
	void Setup();
	void StartDevice();
	void StopDevice();
	void Connect(String SinkName, bool reconnect);
	void Disconnect();
	void Set_Auto_Reconnect(bool reconnect, int count=AUTOCONNECT_TRY_NUM);
	//Callback Registrtion to this class
	void ResgisterForCallbacks(Bluetooth_Sink_Callbacks *callee);
	void Set_Stream_Reader(void (*callBack)(const uint8_t *, uint32_t), bool is_i2s );

    //Bluetooth_Callbacks
    static void StaticBTDataReceived();
    void BTDataReceived();
    static void StaticBTReadDataStream(const uint8_t* data, uint32_t length);
    void BTReadDataStream(const uint8_t *data, uint32_t length);
	static void StaticBluetoothConnectionStateChanged(const esp_a2d_connection_state_t connectionState, void* object)
	{
		if (bT_sink_instance) {
			bT_sink_instance->BluetoothConnectionStateChanged(connectionState, object);
		}
	}
	void BluetoothConnectionStateChanged(const esp_a2d_connection_state_t connectionState, void* object)
	{
		ESP_LOGI("BluetoothConnectionStateChanged", "Connection State: %i", connectionState);
		if(NULL != m_Callee)
		{
			m_Callee->BluetoothConnectionStateChanged(connectionState, object);
		}
	}

	private:
		Bluetooth_Sink_Callbacks* m_Callee = NULL;
		BluetoothA2DPSink& m_BTSink;
		i2s_port_t m_I2S_PORT;
		const i2s_mode_t m_i2s_Mode;
		const int m_SampleRate;
		const i2s_bits_per_sample_t m_BitsPerSample;
		const i2s_comm_format_t m_CommFormat;
		const i2s_channel_fmt_t m_Channel_Fmt;
		const i2s_channel_t m_i2s_channel;
		const bool m_Use_APLL;
		const int m_BufferCount;
		const int m_BufferSize;
		const int m_I2SClockPin;
		const int m_I2SWordSelectPin;
		const int m_I2SDataInPin;
		const int m_I2SDataOutPin;
		String m_SinkName;
		A2DPDefaultVolumeControl m_VolumeControl;
		void (*m_ScreamCallBack)(const uint8_t *, uint32_t);
		
    	DeviceState m_DeviceState = DeviceState::Uninstalled;
		void InstallDevice();
		void UninstallDevice();
};
