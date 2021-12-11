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
							   , int BufferCount
							   , int BufferSize
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
													   , m_SerialClockPin(SerialClockPin)
													   , m_WordSelectPin(WordSelectPin)
													   , m_SerialDataInPin(SerialDataInPin)
													   , m_SerialDataOutPin(SerialDataOutPin)
{
}
Bluetooth_Sink::~Bluetooth_Sink()
{
}

void Bluetooth_Sink::Setup()
{
	m_BytesPerSample = m_BitsPerSample/8;
	m_ChannelBytesToRead = m_BytesPerSample * m_BufferSize;
	m_TotalBytesToRead  = m_ChannelBytesToRead * 2;
	m_ChannelSampleCount = m_ChannelBytesToRead / m_BytesPerSample;
	m_SampleCount = m_TotalBytesToRead / m_BytesPerSample;
}
void Bluetooth_Sink::ResgisterForDataBufferRXCallback(Bluetooth_Sink_Callback* callee){ m_Callee = callee; }

//Callbacks for Bluetooth 
void Bluetooth_Sink::data_received_callback() 
{
}

void Bluetooth_Sink::read_data_stream(const uint8_t *data, uint32_t length)
{
	/*
  *m_SoundBufferData = {0};
  *m_LeftChannel_SoundBufferData = {0};
  *m_RightChannel_SoundBufferData = {0};
  
  for (int i=0; i<length/4; i+=2)
  {
	m_SoundBufferData[i] = m_RightChannel_SoundBufferData[i] = (int32_t)data[i];
	m_SoundBufferData[i+1] = m_LeftChannel_SoundBufferData[i] = (int32_t)data[i+1];
  }
  
  if(NULL != m_Callee) m_Callee->RightChannelDataBufferModifyRX(GetTitle(), m_RightChannel_SoundBufferData, length/4);
  if(NULL != m_Callee) m_Callee->LeftChannelDataBufferModifyRX(GetTitle(), m_LeftChannel_SoundBufferData, length/4);
  if(xQueueSend(m_Data_Buffer_Queue, m_SoundBufferData, portMAX_DELAY) != pdTRUE){ Serial << "Bluetooth Sink: Error Setting Data Queue\n"; }
  if(xQueueSend(m_Right_Data_Buffer_queue, m_RightChannel_SoundBufferData, portMAX_DELAY) != pdTRUE){ Serial << "Bluetooth Sink: Error Setting Right Channel Queue\n"; }
  if(xQueueSend(m_Left_Data_Buffer_queue, m_LeftChannel_SoundBufferData, portMAX_DELAY) != pdTRUE){ Serial << "Bluetooth Sink: Error Setting Left Channel Queue\n"; }
*/
}

void Bluetooth_Sink::InstallDevice()
{
	Serial << "Configuring Bluetooth\n";
	Serial << GetTitle() << ": Allocating Memory.\n";    
	m_SoundBufferData = (int32_t*)malloc(m_TotalBytesToRead);
	m_RightChannel_SoundBufferData = (int32_t*)malloc(m_ChannelBytesToRead);
	m_LeftChannel_SoundBufferData = (int32_t*)malloc(m_ChannelBytesToRead);

	CreateQueue(m_Data_Buffer_Queue, m_TotalBytesToRead, 10, true);
	CreateQueue(m_Right_Data_Buffer_queue, m_ChannelBytesToRead, 10, true);
	CreateQueue(m_Left_Data_Buffer_queue, m_ChannelBytesToRead, 10, true);

	static i2s_config_t i2s_config = {
	  .mode = m_i2s_Mode,
	  .sample_rate = m_SampleRate, // updated automatically by A2DP
	  .bits_per_sample = m_BitsPerSample,
	  .channel_format = m_Channel_Fmt,
	  .communication_format = m_CommFormat,
	  .intr_alloc_flags = 0, // default interrupt priority
	  .dma_buf_count = m_BufferCount,
	  .dma_buf_len = m_BufferSize,
	  .use_apll = true,
	  .tx_desc_auto_clear = true // avoiding noise in case of data unavailability
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
}
void Bluetooth_Sink::StartDevice()
{
	if(false == m_Is_Running)
	{
		InstallDevice();
		m_BTSink.start("Massive Cock");
		m_Is_Running = true;
	}
}
void Bluetooth_Sink::StopDevice()
{
	if(true == m_Is_Running)
	{
		m_BTSink.stop();
		delete m_SoundBufferData;
		delete m_RightChannel_SoundBufferData;
		delete m_LeftChannel_SoundBufferData;
		vQueueDelete(m_Data_Buffer_Queue);
		vQueueDelete(m_Right_Data_Buffer_queue);
		vQueueDelete(m_Left_Data_Buffer_queue);
		m_Is_Running = false;
	}
}
void Bluetooth_Sink::ProcessEventQueue()
{
}