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

Bluetooth_Sink::Bluetooth_Sink ( String Title
							   , BluetoothA2DPSink& BTSink
							   , i2s_port_t i2S_PORT
							   , i2s_mode_t Mode
							   , int SampleRate
							   , i2s_bits_per_sample_t i2s_BitsPerSample
							   , i2s_channel_fmt_t i2s_Channel_Fmt
							   , i2s_comm_format_t i2s_CommFormat
							   , i2s_channel_t i2s_channel
							   , size_t BufferCount
							   , size_t BufferSize
							   , size_t OutputQueueSampleCount 
							   , size_t QueueCount
							   , int SerialClockPin
							   , int WordSelectPin
							   , int SerialDataInPin
							   , int SerialDataOutPin ): NamedItem(Title)
													   , QueueManager(Title + "_QueueManager")
													   , m_BTSink(BTSink)
													   , m_I2S_PORT(i2S_PORT)
													   , m_i2s_Mode(Mode)
													   , m_SampleRate(SampleRate)
													   , m_BitsPerSample(i2s_BitsPerSample)
													   , m_Channel_Fmt(i2s_Channel_Fmt)
													   , m_CommFormat(i2s_CommFormat)
													   , m_i2s_channel(i2s_channel)
													   , m_BufferCount(BufferCount)
													   , m_BufferSize(BufferSize)
													   , m_OutputQueueSampleCount(OutputQueueSampleCount)
													   , m_QueueCount(QueueCount)
													   , m_SerialClockPin(SerialClockPin)
													   , m_WordSelectPin(WordSelectPin)
													   , m_SerialDataInPin(SerialDataInPin)
													   , m_SerialDataOutPin(SerialDataOutPin)
{

}
Bluetooth_Sink::~Bluetooth_Sink()
{
	if(NULL != m_ItemConfig) delete m_ItemConfig;
	FreeMemory();
}

void Bluetooth_Sink::Setup()
{
	Serial << GetTitle() << ": Setup\n";
	m_BytesPerSample = m_BitsPerSample/8;
	m_ChannelBytesToRead = m_BytesPerSample * m_OutputQueueSampleCount;
	m_SampleCount = m_TotalBytesToRead * m_BytesPerSample;
	m_TotalBytesToRead  = m_ChannelBytesToRead * 2;
	m_ChannelSampleCount = m_ChannelBytesToRead * m_BytesPerSample;
	m_ConfigCount = 2;
	m_ItemConfig = new DataItemConfig_t[m_ConfigCount]
	{
		{ "R_BT",	DataType_Int32_t,	m_OutputQueueSampleCount,    Transciever_RX,   m_QueueCount },
		{ "L_BT", 	DataType_Int32_t,	m_OutputQueueSampleCount,    Transciever_RX,   m_QueueCount }
	};
	AllocateMemory();
	SetupQueueManager(m_ConfigCount);
}
void Bluetooth_Sink::ResgisterForDataBufferRXCallback(Bluetooth_Sink_Callback* callee){ m_Callee = callee; }

//Callbacks for Bluetooth 
void Bluetooth_Sink::data_received_callback() 
{
}

void Bluetooth_Sink::read_data_stream(const uint8_t *data, uint32_t length)
{  
	static bool BTSinkDataError = false;
	int SampleCount = length/m_BytesPerSample;
	
	QueueHandle_t RightQueue = GetQueueHandleRXForDataItem("R_BT");
	QueueHandle_t LeftQueue = GetQueueHandleRXForDataItem("L_BT");
	if( NULL != RightQueue && NULL != LeftQueue )
	{
		int channel_samples_read = length / m_BytesPerSample / 2;
		m_TheirByteCount = 0;
		for(int i = 0; i < channel_samples_read; ++i)
		{
			for(int j = 0; j < m_BytesPerSample; ++j)
			{
				m_RightData[m_OurByteCount] = data[m_TheirByteCount];
				m_LeftData[m_OurByteCount] = data[m_TheirByteCount + m_BytesPerSample];
				++m_OurByteCount;
				++m_TheirByteCount;
				if(m_OurByteCount >= m_ChannelBytesToRead)
				{
					m_OurByteCount = 0;
					if(NULL != m_Callee) m_Callee->RightChannelDataBufferModifyRX(GetTitle(), m_RightData, m_ChannelBytesToRead);
					if(NULL != m_Callee) m_Callee->LeftChannelDataBufferModifyRX(GetTitle(), m_LeftData, m_ChannelBytesToRead);
					PushValueToQueue(m_RightData, RightQueue, false, false);
					PushValueToQueue(m_LeftData, LeftQueue, false, false);     
				}
			}
		}
	}
}

void Bluetooth_Sink::AllocateMemory()
{
	if(false == m_MemoryIsAllocated)
	{
		Serial << GetTitle() << ": Allocating Memory\n";
		m_RightData = (uint8_t*)malloc(m_ChannelBytesToRead);
		m_LeftData = (uint8_t*)malloc(m_ChannelBytesToRead);
		m_MemoryIsAllocated = true;
		Serial << GetTitle() << ": Memory Allocated\n";
	}
}
void Bluetooth_Sink::FreeMemory()
{
	if(true == m_MemoryIsAllocated)
	{
		Serial << GetTitle() << ": Freeing Memory\n";
		delete m_RightData;
		delete m_LeftData;
		m_MemoryIsAllocated = false;
		Serial << GetTitle() << ": Memory Freed\n";
	}
}
void Bluetooth_Sink::InstallDevice()
{
	Serial << GetTitle() << ": Installing Device\n";    
	static i2s_config_t i2s_config = {
	  .mode = m_i2s_Mode,
	  .sample_rate = m_SampleRate, // updated automatically by A2DP
	  .bits_per_sample = m_BitsPerSample,
	  .channel_format = m_Channel_Fmt,
	  .communication_format = m_CommFormat,
	  .intr_alloc_flags = 1, // default interrupt priority
	  .dma_buf_count = m_BufferCount,
	  .dma_buf_len = m_BufferSize,
	  .use_apll = false,
	  .tx_desc_auto_clear = true, // avoiding noise in case of data unavailability
	  .fixed_mclk = 0
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
	m_BTSink.set_task_priority(configMAX_PRIORITIES - 1);
	m_BTSink.set_volume_control(&m_VolumeControl);
	m_BTSink.set_volume(200);
	Serial << GetTitle() << ": Device Installed\n";
}
void Bluetooth_Sink::StartDevice()
{
	if(false == m_Is_Running)
	{
		Serial << GetTitle() << ": Starting\n";
		AllocateMemory();
		InstallDevice();
		m_BTSink.start("LED Tower Of Power");
		m_Is_Running = true;
		Serial << GetTitle() << ": Started\n";
	}
}
void Bluetooth_Sink::StopDevice()
{
	if(true == m_Is_Running)
	{
		Serial << GetTitle() << ": Stopping\n";
		m_BTSink.stop();
		FreeMemory();
		m_Is_Running = false;
		Serial << GetTitle() << ": Stopped\n";
	}
}