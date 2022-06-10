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
							   , const char *SinkName
							   , i2s_port_t i2S_PORT
							   , i2s_mode_t Mode
							   , int SampleRate
							   , i2s_bits_per_sample_t i2s_BitsPerSample
							   , i2s_channel_fmt_t i2s_Channel_Fmt
							   , i2s_comm_format_t i2s_CommFormat
							   , i2s_channel_t i2s_channel
							   , bool Use_APLL
							   , size_t BufferCount
							   , size_t BufferSize
							   , int SerialClockPin
							   , int WordSelectPin
							   , int SerialDataInPin
							   , int SerialDataOutPin ): NamedItem(Title)
													   , m_BTSink(BTSink)
													   , mp_SinkName(SinkName)
													   , m_I2S_PORT(i2S_PORT)
													   , m_i2s_Mode(Mode)
													   , m_SampleRate(SampleRate)
													   , m_BitsPerSample(i2s_BitsPerSample)
													   , m_Channel_Fmt(i2s_Channel_Fmt)
													   , m_CommFormat(i2s_CommFormat)
													   , m_i2s_channel(i2s_channel)
													   , m_Use_APLL(Use_APLL)
													   , m_BufferCount(BufferCount)
													   , m_BufferSize(BufferSize)
													   , m_SerialClockPin(SerialClockPin)
													   , m_WordSelectPin(WordSelectPin)
													   , m_SerialDataInPin(SerialDataInPin)
													   , m_SerialDataOutPin(SerialDataOutPin)
{
    //ESP_LOGV("Function Debug", "%s, ", __func__); 
}
Bluetooth_Sink::~Bluetooth_Sink()
{
    //ESP_LOGV("Function Debug", "%s, ", __func__); 
	FreeMemory();
}

void Bluetooth_Sink::Setup()
{
    //ESP_LOGV("Function Debug", "%s, ", __func__); 
	ESP_LOGD("Bluetooth_Device", "%s: Setup", GetTitle());
	m_BytesPerSample = m_BitsPerSample/8;
	m_TotalBytesToRead = m_BytesPerSample * m_BufferSize;
	m_ChannelBytesToRead = m_TotalBytesToRead / 2;
	m_SampleCount = m_TotalBytesToRead / m_BytesPerSample;
	m_ChannelSampleCount = m_ChannelBytesToRead / m_BytesPerSample;
	AllocateMemory();
}
void Bluetooth_Sink::ResgisterForDataBufferRXCallback(Bluetooth_Sink_Callback* callee){ m_Callee = callee; }

//Callbacks for Bluetooth 
void Bluetooth_Sink::data_received_callback() 
{
    //ESP_LOGV("Function Debug", "%s, ", __func__); 
}

void Bluetooth_Sink::read_data_stream(const uint8_t *data, uint32_t length)
{  
    //ESP_LOGV("Function Debug", "%s, ", __func__); 
	for(int i = 0; i < length; ++i)
	{
		mp_Data[m_OurByteCount] = data[i];
		++m_OurByteCount;
		if(m_OurByteCount >= m_TotalBytesToRead)
		{
			m_OurByteCount = 0;
			if(NULL != m_Callee) 
			{
				m_Callee->DataBufferModifyRX(GetTitle(), mp_Data, m_TotalBytesToRead, m_SampleCount);
			}
			if(I2S_CHANNEL_STEREO == m_i2s_channel)
			{
				for(int j = 0; j < m_ChannelSampleCount; ++j)
				{
					int DataBufferIndex = m_BytesPerSample * j;
					for(int k = 0; k < m_BytesPerSample; ++k)
					{
						mp_RightData[DataBufferIndex + j] = mp_Data[2*DataBufferIndex + j];
						mp_LeftData[DataBufferIndex + j] = mp_Data[2*DataBufferIndex + m_BytesPerSample + j];
					}
				}
				if(NULL != m_Callee) 
				{
					m_Callee->RightChannelDataBufferModifyRX(GetTitle(), mp_RightData, m_ChannelBytesToRead, m_ChannelSampleCount);
					m_Callee->LeftChannelDataBufferModifyRX(GetTitle(), mp_LeftData, m_ChannelBytesToRead, m_ChannelSampleCount);
				}
			}				
		}
	}
}

void Bluetooth_Sink::AllocateMemory()
{
    //ESP_LOGV("Function Debug", "%s, ", __func__); 
	if(false == m_MemoryIsAllocated)
	{
		ESP_LOGD("Bluetooth_Device", "%s: Allocating Memory", GetTitle().c_str());
		mp_Data = (uint8_t*)ps_malloc(m_TotalBytesToRead);
		mp_RightData = (uint8_t*)ps_malloc(m_ChannelBytesToRead);
		mp_LeftData = (uint8_t*)ps_malloc(m_ChannelBytesToRead);
		m_MemoryIsAllocated = true;
		ESP_LOGD("Bluetooth_Device", "%s: Memory Allocated", GetTitle().c_str());
	}
}
void Bluetooth_Sink::FreeMemory()
{
    //ESP_LOGV("Function Debug", "%s, ", __func__); 
	if(true == m_MemoryIsAllocated)
	{
		ESP_LOGD("Bluetooth_Device", "%s: Freeing Memory", GetTitle().c_str());
		free(mp_Data);
		free(mp_RightData);
		free(mp_LeftData);
		m_MemoryIsAllocated = false;
		ESP_LOGD("Bluetooth_Device", "%s: Memory Freed", GetTitle().c_str());
	}
}
void Bluetooth_Sink::InstallDevice()
{
    //ESP_LOGV("Function Debug", "%s, ", __func__);   
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
	ESP_LOGD("Bluetooth_Device", "%s: Device Installed", GetTitle().c_str());
}
void Bluetooth_Sink::StartDevice()
{
    //ESP_LOGV("Function Debug", "%s, ", __func__);
	if(false == m_Is_Running)
	{
		ESP_LOGD("Bluetooth_Device", "Starting Bluetooth");
		AllocateMemory();
		InstallDevice();
		m_BTSink.start(mp_SinkName);
		m_Is_Running = true;
		ESP_LOGD("Bluetooth_Device", "Bluetooth Started");
	}
}
void Bluetooth_Sink::StopDevice()
{
    //ESP_LOGV("Function Debug", "%s, ", __func__);
	if(true == m_Is_Running)
	{
		ESP_LOGD("Bluetooth_Device", "Stopping Bluetooth");
		m_BTSink.stop();
		FreeMemory();
		m_Is_Running = false;
		ESP_LOGD("Bluetooth_Device", "Bluetooth Stopped");
	}
}