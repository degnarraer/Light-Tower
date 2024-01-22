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
#define BT_COMPATIBLE_DEVICE_TIMEOUT 30000

#include <vector> 
#include <Arduino.h>
#include <Helpers.h>
#include <BluetoothA2DPSink.h>
#include <BluetoothA2DPSource.h>
#include <BluetoothA2DPCommon.h>
class BluetoothConnectionStatusCallee
{
	public:
		BluetoothConnectionStatusCallee(){};
		virtual void BluetoothConnectionStatusChanged(const ConnectionStatus_t ConnectionStatus) = 0;
};

class BluetoothConnectionStatusCaller
{
	public:
		BluetoothConnectionStatusCaller()
		{
			xTaskCreatePinnedToCore( StaticCheckBluetoothConnection,   "BluetoothConnectionStatusCaller", 5000,  this,   THREAD_PRIORITY_MEDIUM,  &m_Handle, 1 );
		}
		virtual ~BluetoothConnectionStatusCaller()
		{
			//vTaskDelete(m_Handle);
		}
		void RegisterForConnectionStatusChangedCallBack(BluetoothConnectionStatusCallee *Callee)
		{
			m_ConnectionStatusCallee = Callee;
		}
		
		bool IsConnected()
		{
			return (m_ConnectionStatus == ConnectionStatus_t::Paired);
		}
	
	protected:
		virtual bool GetConnectionStatus() = 0;
		BluetoothConnectionStatusCallee *m_ConnectionStatusCallee = NULL;
		void SetWaiting()
		{ 
			m_ConnectionStatus = ConnectionStatus_t::Waiting;
			m_ConnectionStatusCallee->BluetoothConnectionStatusChanged(m_ConnectionStatus);
		}
		void SetSearching()
		{ 
			m_ConnectionStatus = ConnectionStatus_t::Searching;
			m_ConnectionStatusCallee->BluetoothConnectionStatusChanged(m_ConnectionStatus);
		}
		void SetPairing()
		{
			m_ConnectionStatus = ConnectionStatus_t::Pairing;
			m_ConnectionStatusCallee->BluetoothConnectionStatusChanged(m_ConnectionStatus);
		}
	
	private:
		ConnectionStatus_t m_ConnectionStatus = ConnectionStatus_t::Disconnected;
		TaskHandle_t m_Handle;
		static void StaticCheckBluetoothConnection(void *parameter)
		{
		  const TickType_t xFrequency = 100;
		  TickType_t xLastWakeTime = xTaskGetTickCount();
		  while(true)
		  {
			vTaskDelayUntil( &xLastWakeTime, xFrequency );
			((BluetoothConnectionStatusCaller*)parameter)->UpdateConnectionStatus();
		  }
		}
		void UpdateConnectionStatus()
		{
			ConnectionStatus_t StartingStatus = m_ConnectionStatus;
			switch(m_ConnectionStatus)
			{
				case ConnectionStatus_t::Disconnected:
					if(true == GetConnectionStatus())
					{
						m_ConnectionStatus = ConnectionStatus_t::Paired;
					}
					else
					{
						m_ConnectionStatus = ConnectionStatus_t::Disconnected;
					}
				break;
				case ConnectionStatus_t::Searching:
					if(true == GetConnectionStatus())
					{
						m_ConnectionStatus = ConnectionStatus_t::Paired;
					}
					else
					{
						m_ConnectionStatus = ConnectionStatus_t::Searching;
					}
				break;
				case ConnectionStatus_t::Waiting:
					if(true == GetConnectionStatus())
					{
						m_ConnectionStatus = ConnectionStatus_t::Paired;
					}
					else
					{
						m_ConnectionStatus = ConnectionStatus_t::Waiting;
					}
				break;
				case ConnectionStatus_t::Pairing:
					if(true == GetConnectionStatus())
					{
						m_ConnectionStatus = ConnectionStatus_t::Paired;
					}
					else
					{
						m_ConnectionStatus = ConnectionStatus_t::Pairing;
					}
				break;
				case ConnectionStatus_t::Paired:
					if(true == GetConnectionStatus())
					{
						m_ConnectionStatus = ConnectionStatus_t::Paired;
					}
					else
					{
						m_ConnectionStatus = ConnectionStatus_t::Disconnected;
					}
				break;
			}
			if(StartingStatus != m_ConnectionStatus)
			{
				m_ConnectionStatusCallee->BluetoothConnectionStatusChanged(m_ConnectionStatus);
			}
		}
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
					  , public BluetoothConnectionStatusCaller
					  , public BluetoothActiveDeviceUpdater
					  , public CommonUtils
					  , public QueueController
{	
	public:
		Bluetooth_Source( String Title
						, BluetoothA2DPSource& BTSource)
						: NamedItem(Title)
						, m_BTSource(BTSource)
		{
		}
		virtual ~Bluetooth_Source()
		{
			vTaskDelete(m_CompatibleDeviceTrackerTask);
		}
		void Setup();
		void InstallDevice();
		void StartDevice( const char *SourceName
						, const char *SourceAddress
						, bool AutoReconnect
						, bool ResetBLE
						, bool ResetNVS );
		void SetNameToConnect( const char *SourceName, const char *SourceAddress );
		void SetMusicDataCallback(music_data_cb_t callback);
		
		//Callback from BT Source for compatible devices to connect to
		bool ConnectToThisName(const char*name, esp_bd_addr_t address, int32_t rssi);
		void Set_NVS_Init(bool ResetNVS)
		{ 
			m_ResetNVS = ResetNVS;
			m_BTSource.set_nvs_init(m_ResetNVS);
		}
		void Set_Reset_BLE(bool ResetBLE)
		{
			m_ResetBLE = ResetBLE;
			m_BTSource.set_reset_ble(m_ResetBLE);
		}
		void Set_Auto_Reconnect(bool AutoReConnect)
		{
			m_AutoReConnect = AutoReConnect;
			m_BTSource.set_auto_reconnect(m_AutoReConnect);
		}
		void Set_SSP_Enabled(bool SSPEnabled)
		{
			m_SSPEnabled = SSPEnabled;
			m_BTSource.set_ssp_enabled(m_SSPEnabled);
		}		
		
	protected:
		bool GetConnectionStatus(){ return m_BTSource.is_connected(); }
	private:
	
		BluetoothA2DPSource& m_BTSource;
		music_data_cb_t m_MusicDataCallback = NULL;
		String m_NAME;
		String m_ADDRESS;
		bool m_ResetNVS = false;
		bool m_ResetBLE = true;
		bool m_AutoReConnect = false;
		bool m_SSPEnabled = false;
		
		
		std::vector<ActiveCompatibleDevice_t> m_ActiveCompatibleDevices;
		TaskHandle_t m_CompatibleDeviceTrackerTask;
		bool m_Is_Running = false;
		
		bool compatible_device_found(const char* name, esp_bd_addr_t address, int32_t rssi);
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
					, public BluetoothConnectionStatusCaller
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
				  , m_BTSink(BTSink)
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
	void StartDevice(String SinkName, bool reconnect);
	void StopDevice();
	void Set_Auto_Reconnect(bool reconnect, int count=AUTOCONNECT_TRY_NUM )
	{
		m_AutoReConnect = reconnect;
        m_BTSink.set_auto_reconnect(m_AutoReConnect);
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
		bool m_AutoReConnect = false;
		String m_SinkName;
		void InstallDevice();
};



#endif
