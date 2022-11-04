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

#include "I2S_Device.h"

I2S_Device::I2S_Device ( String Title
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
                       , int SerialDataOutPin )
                       : NamedItem(Title)
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
}
I2S_Device::~I2S_Device()
{
	UninstallDevice();
}

void I2S_Device::Setup()
{
    m_BytesPerSample = m_BitsPerSample/8;
    m_ChannelSampleCount = m_BufferSize;
	m_SampleCount = m_ChannelSampleCount * 2;
    m_ChannelBytesToRead  = m_BytesPerSample * m_ChannelSampleCount;
    m_TotalBytesToRead = m_ChannelBytesToRead * 2;
	DataType_t DataType;
	switch(m_BitsPerSample)
	{	
		case I2S_BITS_PER_SAMPLE_32BIT:
			DataType = DataType_Int32_t;
		break;
		case I2S_BITS_PER_SAMPLE_24BIT:
			DataType = DataType_Int32_t;
		break;
		case I2S_BITS_PER_SAMPLE_16BIT:
			DataType = DataType_Int16_t;
		break;
		case I2S_BITS_PER_SAMPLE_8BIT:
			DataType = DataType_Int8_t;
		break;
		default:
		break;
	}
}

void I2S_Device::StartDevice()
{
	if(false == m_Is_Running)
	{
		InstallDevice();
		i2s_start(m_I2S_PORT);
		m_Is_Running = true;
	}
}

void I2S_Device::StopDevice()
{
	if(true == m_Is_Running)
	{
		UninstallDevice();
		i2s_stop(m_I2S_PORT);
		m_Is_Running = false;
	}
}

size_t I2S_Device::WriteSoundBufferData(uint8_t *SoundBufferData, size_t ByteCount)
{
	return WriteSamples(SoundBufferData, ByteCount);
}

size_t I2S_Device::ReadSoundBufferData(uint8_t *SoundBufferData, size_t ByteCount)
{
	size_t bytes_read = 0;
	if(i2s_read(m_I2S_PORT, SoundBufferData, ByteCount, &bytes_read, portMAX_DELAY ) != ESP_OK)
	{
		ESP_LOGE("i2S Device", "%s: Error Reading Samples\n", GetTitle());
		return bytes_read;
	}
	return bytes_read;
}

size_t I2S_Device::ReadSamples()
{
	size_t bytes_read = 0;
	if(NULL != m_Callee)
	{
		uint8_t DataBuffer[m_BufferSize];
		i2s_read(m_I2S_PORT, DataBuffer, m_TotalBytesToRead, &bytes_read, portMAX_DELAY );
		if(bytes_read == 0) return 0;
		m_Callee->I2SDataReceived(GetTitle(), DataBuffer, bytes_read);
	}
	return bytes_read;
}

size_t I2S_Device::WriteSamples(uint8_t *samples, size_t ByteCount)
{
	// write to i2s
	size_t bytes_written = 0;
	i2s_write(m_I2S_PORT, samples, ByteCount, &bytes_written, portMAX_DELAY);
	if(bytes_written != ByteCount)
	{
		ESP_LOGE("i2S Device", "%s: Error Writting All Bytes\n", GetTitle()); 
	}
	return bytes_written;
}

void I2S_Device::UninstallDevice()
{
	i2s_driver_uninstall(m_I2S_PORT);
}
void I2S_Device::InstallDevice()
{
	ESP_LOGI("i2S Device", "%s: Configuring I2S Device.", GetTitle());
  esp_err_t err;
  // The I2S config as per the example
  const i2s_config_t i2s_config = {
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
    .fixed_mclk = 0
  };

  // The pin config as per the setup
  const i2s_pin_config_t pin_config = 
    {
      .bck_io_num = m_SerialClockPin,         // Serial Clock (SCK)
      .ws_io_num = m_WordSelectPin,           // Word Select (WS)
      .data_out_num = m_SerialDataOutPin,     // not used (only for speakers)
      .data_in_num = m_SerialDataInPin        // Serial Data (SD)
    };
  // Configuring the I2S driver and pins.
  // This function must be called before any I2S driver read/write operations.
  err = i2s_driver_install(m_I2S_PORT, &i2s_config, m_BufferCount, &m_i2s_event_queue);
  if (err != ESP_OK)
  {
	ESP_LOGE("i2S Device", "%s: Failed installing driver: %s", GetTitle().c_str(), err);
    ESP.restart();
  }
  if (NULL == m_i2s_event_queue)
  {
	ESP_LOGE("i2S Device", "%s: Failed to setup event queue!", GetTitle().c_str());
	ESP.restart();
  }
  err = i2s_set_clk(m_I2S_PORT, m_SampleRate, m_BitsPerSample, m_i2s_channel);
  if (err != ESP_OK)
  {
	ESP_LOGE("i2S Device", "%s: Failed setting clock: %s", GetTitle().c_str(), err);
	ESP.restart();
  }
  err = i2s_set_pin(m_I2S_PORT, &pin_config);
  if (err != ESP_OK)
  {
	ESP_LOGE("i2S Device", "%s: Failed setting pin: %s", GetTitle().c_str(), err);
    ESP.restart();
  }
  ESP_LOGI("i2S Device", "%s: Driver Installed.", GetTitle().c_str());
}

void I2S_Device::ProcessEventQueue()
{
    //ESP_LOGV("Function Debug", "%s, ", __func__);
	if(NULL != m_i2s_event_queue)
	{
		i2s_event_t i2sEvent = {};
		uint8_t i2sMsgCount = uxQueueMessagesWaiting(m_i2s_event_queue);   
		// Iterate over all events in the i2s event queue
		for( int i = 0; i < i2sMsgCount; ++i )
		{
			ESP_LOGV("i2S Device", "%s: Queue Count: %i", GetTitle(), i2sMsgCount);
			// Take next event from queue
			if ( xQueueReceive(m_i2s_event_queue, (void*) &i2sEvent, 0) == pdTRUE )
			{
				switch (i2sEvent.type)
				{
					case I2S_EVENT_DMA_ERROR:
						ESP_LOGE("i2S Device", "%s: I2S_EVENT_DMA_ERROR", GetTitle().c_str());
						break;
					case I2S_EVENT_TX_DONE:
						ESP_LOGV("i2S Device", "%s: TX Done", GetTitle().c_str());
						break;
					case I2S_EVENT_RX_DONE:
						{
						  ESP_LOGV("i2S Device", "%s: RX", GetTitle().c_str());
						  ReadSamples();
						}
						break;
					case I2S_EVENT_MAX:
						ESP_LOGE("i2S Device", "WARNING! I2S_EVENT_MAX");
						break;
				}	
			}
		}
	}
}
