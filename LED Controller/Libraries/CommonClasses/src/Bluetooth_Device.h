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
#define BT_COMPATIBLE_DEVICE_TIMEOUT 30000

#include <vector>
#include <Arduino.h>
#include <mutex>
#include <memory>
#include "BluetoothA2DPSink.h"
#include "BluetoothA2DPSource.h"
#include "BluetoothA2DPCommon.h"
#include "Helpers.h"

class BluetoothConnectionStateCallee
{
	public:
		BluetoothConnectionStateCallee(){};
		virtual void BluetoothConnectionStateChanged(const esp_a2d_connection_state_t ConnectionState) = 0;
};

class BluetoothConnectionStateCaller
{
	public:
		BluetoothConnectionStateCaller( BluetoothA2DPCommon *BT );
		virtual ~BluetoothConnectionStateCaller()
		{
			vTaskDelete(m_Handle);
		}
		void RegisterForConnectionStateChangedCallBack(BluetoothConnectionStateCallee *Callee);
		bool IsConnected();
	
	protected:
		virtual bool GetConnectionStatus() = 0;
		BluetoothConnectionStateCallee *mp_ConnectionStateCallee = NULL;
	
	private:
		TaskHandle_t m_Handle;
		esp_a2d_connection_state_t m_ConnectionState;
		BluetoothA2DPCommon *mp_BT;
		static void StaticCheckBluetoothConnection(void *parameter);
		void UpdateConnectionStatus();
};

class BluetoothActiveDeviceUpdatee
{
	public:
		BluetoothActiveDeviceUpdatee(){};
		virtual void BluetoothActiveDeviceListUpdated(const std::vector<ActiveCompatibleDevice_t> &Devices) = 0;
};

class BluetoothActiveDeviceUpdater
{
	public:
		BluetoothActiveDeviceUpdater()
		{
		}
		virtual ~BluetoothActiveDeviceUpdater()
		{
		}
		void RegisterForActiveDeviceUpdate(BluetoothActiveDeviceUpdatee *Callee)
		{
			m_BluetoothActiveDeviceUpdatee = Callee;
		}
	protected:
		BluetoothActiveDeviceUpdatee *m_BluetoothActiveDeviceUpdatee = NULL;
};

class Bluetooth_Source: public NamedItem
					  , public BluetoothConnectionStateCaller
					  , public BluetoothActiveDeviceUpdater
					  , public CommonUtils
					  , public QueueController
{	
	public:
		Bluetooth_Source( String Title
						, BluetoothA2DPSource& BTSource)
						: NamedItem(Title)
						, BluetoothConnectionStateCaller(&BTSource)
						, m_BTSource(BTSource)
		{
			std::lock_guard<std::mutex> lock(m_ActiveCompatibleDevicesMutex);
		}
		Bluetooth_Source(const Bluetooth_Source&) = delete;
		virtual ~Bluetooth_Source()
		{
			vTaskDelete(m_CompatibleDeviceTrackerTask);
		}
		void Setup();
		void InstallDevice();
		void StartDevice();
		void StopDevice();
		void Connect( const char *SourceName, const char *SourceAddress );
		void Disconnect();
		void SetNameToConnect( const std::string& SourceName, const std::string& SourceAddress );
		void SetMusicDataCallback(music_data_cb_t callback);
		
		//Callback from BT Source for compatible devices to connect to
		bool ConnectToThisName(const std::string& name, esp_bd_addr_t address, int32_t rssi);
		void Set_NVS_Init(bool ResetNVS)
		{ 
			m_BTSource.set_nvs_init(ResetNVS);
		}
		void Set_Reset_BLE(bool ResetBLE)
		{
			m_ResetBLE = ResetBLE;
		}
		void Set_Auto_Reconnect(bool AutoReConnect)
		{
			m_AutoReConnect = AutoReConnect;
		}
		void Set_SSP_Enabled(bool SSPEnabled)
		{
			m_BTSource.set_ssp_enabled(SSPEnabled);
		}
		/// converts a esp_bd_addr_t to a string - the string is 18 characters long!
		const char* GetAddressString(esp_bd_addr_t bda)
		{
			static char bda_str[18];
			sprintf(bda_str, "%02x:%02x:%02x:%02x:%02x:%02x", bda[0], bda[1], bda[2], bda[3], bda[4], bda[5]);
			return (const char*)bda_str;
		}
	protected:
		bool GetConnectionStatus(){ return m_BTSource.is_connected(); }
	private:
	
		BluetoothA2DPSource& m_BTSource;
		music_data_cb_t m_MusicDataCallback = NULL;
		std::string m_Name;
		std::string m_Address;
		bool m_ResetBLE = true;
		bool m_AutoReConnect = false;
		std::mutex m_ActiveCompatibleDevicesMutex;
		std::vector<ActiveCompatibleDevice_t> m_ActiveCompatibleDevices;
		TaskHandle_t m_CompatibleDeviceTrackerTask;
		bool m_Is_Running = false;
		
		bool compatible_device_found(const std::string& name, esp_bd_addr_t address, int32_t rssi);
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
					, public BluetoothConnectionStateCaller
{
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
				  , BluetoothConnectionStateCaller(&BTSink)
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
				  , m_SerialClockPin(SerialClockPin)
				  , m_WordSelectPin(WordSelectPin)
				  , m_SerialDataInPin(SerialDataInPin)
				  , m_SerialDataOutPin(SerialDataOutPin){};		
	virtual ~Bluetooth_Sink(){};
	void Setup();
	void StartDevice();
	void StopDevice();
	void Connect(String SinkName, bool reconnect);
	void Disconnect();
	void Set_Auto_Reconnect(bool reconnect, int count=AUTOCONNECT_TRY_NUM )
	{
		m_AutoReConnect = reconnect;
    }

	//Callbacks from BluetoothSink  
	void data_received_callback();
	void read_data_stream(const uint8_t *data, uint32_t length);

	//Callback Registrtion to this class
	void ResgisterForRxCallback(Bluetooth_Sink_Callback *callee);
   
	protected:
		bool GetConnectionStatus(){ return m_BTSink.is_connected(); } 
	private:
		Bluetooth_Sink_Callback* m_Callee = NULL;
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
		const int m_SerialClockPin;
		const int m_WordSelectPin;
		const int m_SerialDataInPin;
		const int m_SerialDataOutPin;
		
		bool m_Is_Running = false;
		bool m_AutoReConnect = false;
		String m_SinkName;
		A2DPDefaultVolumeControl m_VolumeControl;
		void InstallDevice();
};
