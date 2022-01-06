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
							   , size_t BTCallbackSampleCount 
							   , size_t QueueCount
							   , int SerialClockPin
							   , int WordSelectPin
							   , int SerialDataInPin
							   , int SerialDataOutPin ): NamedItem(Title)
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
													   , m_BTCallbackSampleCount(BTCallbackSampleCount)
													   , m_QueueCount(QueueCount)
													   , m_SerialClockPin(SerialClockPin)
													   , m_WordSelectPin(WordSelectPin)
													   , m_SerialDataInPin(SerialDataInPin)
													   , m_SerialDataOutPin(SerialDataOutPin)
{
}
Bluetooth_Sink::~Bluetooth_Sink()
{
	FreeMemory();
}

void Bluetooth_Sink::Setup()
{
	Serial << GetTitle() << ": Setup\n";
	m_BytesPerSample = m_BitsPerSample/8;
	m_TotalBytesToRead = m_BytesPerSample * m_BTCallbackSampleCount;
	m_SampleCount = m_TotalBytesToRead * m_BytesPerSample;
	m_ChannelBytesToRead  = m_TotalBytesToRead / 2;
	m_ChannelSampleCount = m_ChannelBytesToRead * m_BytesPerSample;
	AllocateMemory();
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
	if(m_BTCallbackSampleCount != SampleCount)
	{
		if(false == BTSinkDataError){ BTSinkDataError = true; Serial << "WARNING! " << GetTitle() << " expects: " << m_BTCallbackSampleCount << " Samples, but instead received: " << SampleCount << "\n"; }
	}
	else
	{
		if(true == BTSinkDataError){ BTSinkDataError = false; Serial << "WARNING! " <<  GetTitle() << " Data Correct\n"; }
	}
	if( NULL != m_RightChannel_SoundBufferData 
	 && NULL != m_LeftChannel_SoundBufferData
	 && NULL != m_Right_Data_Buffer_Queue
	 && NULL != m_Left_Data_Buffer_Queue )
	{
		int channel_samples_read = length / m_BytesPerSample / 2;
		m_TheirByteCount = 0;
		for(int i = 0; i < channel_samples_read; ++i)
		{
			for(int j = 0; j < m_BytesPerSample; ++j)
			{
				m_RightChannel_SoundBufferData[m_OurByteCount] = data[m_TheirByteCount];
				m_LeftChannel_SoundBufferData[m_OurByteCount] = data[m_TheirByteCount + m_BytesPerSample];
				++m_OurByteCount;
				++m_TheirByteCount;
				if(m_OurByteCount >= m_ChannelBytesToRead)
				{
					m_OurByteCount = 0;
					SendData();
				}
			}
		}
	}
}

void Bluetooth_Sink::SendData()
{
	//static bool BTSinkQueueFull = false;
	static bool BTSinkRightQueueFull = false;
	static bool BTSinkLeftQueueFull = false;			
	if(NULL != m_Callee) m_Callee->RightChannelDataBufferModifyRX(GetTitle(), m_RightChannel_SoundBufferData, m_ChannelBytesToRead);
	if(NULL != m_Callee) m_Callee->LeftChannelDataBufferModifyRX(GetTitle(), m_LeftChannel_SoundBufferData, m_ChannelBytesToRead);
	
	if(NULL != m_Right_Data_Buffer_Queue && uxQueueSpacesAvailable(m_Right_Data_Buffer_Queue) > 0)
	{
		if(true == BTSinkRightQueueFull){ BTSinkRightQueueFull = false; Serial << "WARNING! " << GetTitle() << ": Right Data Buffer Queue Sent\n"; }
		if(xQueueSend(m_Right_Data_Buffer_Queue, m_RightChannel_SoundBufferData, portMAX_DELAY) != pdTRUE){ Serial << GetTitle() << ": Error Setting Data Queue\n"; }
	}
	else 
	{ 
		if(false == BTSinkRightQueueFull){ BTSinkRightQueueFull = true; Serial << "WARNING! " << GetTitle() << ": Right Data Buffer Queue Full\n"; }
	}
	
	if(NULL != m_Left_Data_Buffer_Queue && uxQueueSpacesAvailable(m_Left_Data_Buffer_Queue) > 0)
	{
		if(true == BTSinkLeftQueueFull){ BTSinkLeftQueueFull = false; Serial << "WARNING! " << GetTitle() << ": Left Data Buffer Queue Sent\n"; }
		if(xQueueSend(m_Left_Data_Buffer_Queue, m_LeftChannel_SoundBufferData, portMAX_DELAY) != pdTRUE){ Serial << GetTitle() << ": Error Setting Data Queue\n"; }
	}
	else
	{ 
		if(false == BTSinkLeftQueueFull){ BTSinkLeftQueueFull = true; Serial << "WARNING! " << GetTitle() << ": Left Data Buffer Queue Full\n"; }
	}
}
void Bluetooth_Sink::AllocateMemory()
{
	if(false == m_MemoryIsAllocated)
	{
		Serial << GetTitle() << ": Allocating Memory\n";
		m_RightChannel_SoundBufferData = (uint8_t*)malloc(m_ChannelBytesToRead);
		m_LeftChannel_SoundBufferData = (uint8_t*)malloc(m_ChannelBytesToRead);

		CreateQueue(m_Right_Data_Buffer_Queue, m_ChannelBytesToRead, m_QueueCount, true);
		CreateQueue(m_Left_Data_Buffer_Queue, m_ChannelBytesToRead, m_QueueCount, true);
		m_MemoryIsAllocated = true;
		Serial << GetTitle() << ": Memory Allocated\n";
	}
}
void Bluetooth_Sink::FreeMemory()
{
	if(true == m_MemoryIsAllocated)
	{
		delete m_RightChannel_SoundBufferData;
		delete m_LeftChannel_SoundBufferData;
		if(NULL != m_Right_Data_Buffer_Queue)
		{
			vQueueDelete(m_Right_Data_Buffer_Queue);
			m_Right_Data_Buffer_Queue = NULL;
		}
		if(NULL != m_Left_Data_Buffer_Queue)
		{
			vQueueDelete(m_Left_Data_Buffer_Queue);
			m_Left_Data_Buffer_Queue = NULL;
		}
		m_MemoryIsAllocated = false;
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
		m_BTSink.start("Sound");
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
void Bluetooth_Sink::ProcessEventQueue()
{
}