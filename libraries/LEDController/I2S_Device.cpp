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
                       , int BufferCount
                       , int BufferSize
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
	FreeMemory();
}

void I2S_Device::Setup()
{
    m_BytesPerSample = m_BitsPerSample/8;
    m_ChannelSampleCount = m_BufferSize;
	m_SampleCount = m_ChannelSampleCount * 2;
    m_TotalBytesToRead = m_BytesPerSample * m_SampleCount;
    m_ChannelBytesToRead  = m_BytesPerSample * m_ChannelSampleCount;
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
		i2s_stop(m_I2S_PORT);
		FreeMemory();
		m_Is_Running = false;
	}
}
		
int32_t I2S_Device::GetDataBufferValue(uint8_t* DataBuffer, size_t index)
{
	switch(m_BytesPerSample)
	{
	  case 1:
		return ((int8_t*)DataBuffer)[index];
	  break;
	  case 2:
		return ((int16_t*)DataBuffer)[index];
	  break;
	  case 3:
	  case 4:
		return ((int32_t*)DataBuffer)[index];
	  break;
	}
}

void I2S_Device::SetDataBufferValue(uint8_t* DataBuffer, size_t index, int32_t value)
{
	switch(m_BytesPerSample)
	{
	  case 1:
		((int8_t*)DataBuffer)[index] = (int8_t)value;
	  break;
	  case 2:
		((int16_t*)DataBuffer)[index] = (int16_t)value;
	  break;
	  case 3:
	  case 4:
		((int32_t*)DataBuffer)[index] = (int32_t)value;
	  break;
	}
}

void I2S_Device::SetSoundBufferData(uint8_t *SoundBufferData, size_t ByteCount)
{
  WriteSamples(SoundBufferData, ByteCount);
  if(true == DATA_TX_DEBUG) Serial <<  GetTitle() << "Sound Buffer Data Ready\n";
}

int I2S_Device::ReadSamples()
{
  uint8_t* SoundBufferData = (uint8_t*)malloc(m_TotalBytesToRead);
  uint8_t* RightChannel_SoundBufferData = (uint8_t*)malloc(m_ChannelBytesToRead);
  uint8_t* LeftChannel_SoundBufferData = (uint8_t*)malloc(m_ChannelBytesToRead);
  // read from i2s
  size_t bytes_read = 0;
  size_t channel_bytes_read = 0;
  i2s_read(m_I2S_PORT, SoundBufferData, m_TotalBytesToRead, &bytes_read, portMAX_DELAY );
  channel_bytes_read = bytes_read / 2;
  
  if(bytes_read != m_TotalBytesToRead)Serial << GetTitle() << ": Error Reading All Bytes. Read: " << bytes_read << " out of " << m_TotalBytesToRead << "\n";
  if(NULL != m_Callee) m_Callee->DataBufferModifyRX(GetTitle(), *SoundBufferData, bytes_read);
  
  static bool MicDataBufferFull = false;
  if(uxQueueSpacesAvailable(m_i2s_Data_Buffer_Queue) > 0)
  {
	if(true == MicDataBufferFull){ MicDataBufferFull = false; Serial << "WARNING! " << GetTitle() << ": Data Buffer Queue Send Resumed\n"; }
    if(xQueueSend(m_i2s_Data_Buffer_Queue, SoundBufferData, 0) != pdTRUE){ Serial << GetTitle() << ": Error Setting Data Buffer Queue\n"; }
  }
  else
  { 
	if(false == MicDataBufferFull){ MicDataBufferFull = true; Serial << "WARNING! " << GetTitle() << ": Data Buffer Queue Full\n"; }
  }
  
  if(I2S_CHANNEL_STEREO == m_i2s_channel)
  {
	  
	int channel_samples_read = channel_bytes_read / m_BytesPerSample;
    for(int i = 0; i < channel_samples_read; ++i)
    {
	  int DataBufferIndex = m_BytesPerSample * i;
	  for(int j = 0; j < m_BytesPerSample; ++j)
	  {
		RightChannel_SoundBufferData[DataBufferIndex + j] = SoundBufferData[DataBufferIndex + j];
		LeftChannel_SoundBufferData[DataBufferIndex + j] = SoundBufferData[DataBufferIndex + m_BytesPerSample + j];
	  }
    }
	if(NULL != m_Callee) m_Callee->RightChannelDataBufferModifyRX(GetTitle(), *RightChannel_SoundBufferData, channel_bytes_read);
	if(NULL != m_Callee) m_Callee->LeftChannelDataBufferModifyRX(GetTitle(), *LeftChannel_SoundBufferData, channel_bytes_read);
    
	
	static bool MicRightDataBufferFull = false;
	if(uxQueueSpacesAvailable(m_i2s_Right_Data_Buffer_queue) > 0)
	{
		if(true == MicRightDataBufferFull){ MicRightDataBufferFull = false; Serial << "WARNING! " << GetTitle() << ": Data Buffer Queue Send Resumed\n"; }
		if(xQueueSend(m_i2s_Right_Data_Buffer_queue, RightChannel_SoundBufferData, 0) != pdTRUE){ Serial << GetTitle() << ": Error Setting Right Data Buffer Queue\n"; }
    }
	else
	{ 
		if(false == MicRightDataBufferFull){ MicRightDataBufferFull = true; Serial << "WARNING! " << GetTitle() << ": Right Data Buffer Queue Full\n"; }
	}
	
	
	static bool MicLeftDataBufferFull = false;
	if(uxQueueSpacesAvailable(m_i2s_Left_Data_Buffer_queue) > 0)
	{
		if(true == MicLeftDataBufferFull){ MicLeftDataBufferFull = false; Serial << "WARNING! " << GetTitle() << ": Left Data Buffer Queue Send Resumed\n"; }
		if(xQueueSend(m_i2s_Left_Data_Buffer_queue, LeftChannel_SoundBufferData, 0) != pdTRUE){ Serial << GetTitle() << ": Error Setting Left Data Buffer Queue\n"; }
    }
	else
	{ 
		if(false == MicLeftDataBufferFull){ MicLeftDataBufferFull = true; Serial << "WARNING! " << GetTitle() << ": Left Data Buffer Queue Full\n"; }
	}
  }
  delete SoundBufferData;
  delete RightChannel_SoundBufferData;
  delete LeftChannel_SoundBufferData;
  return bytes_read;
}

int I2S_Device::WriteSamples(uint8_t *samples, size_t ByteCount)
{
  // write to i2s
  size_t bytes_written = 0;
  i2s_write(m_I2S_PORT, samples, ByteCount, &bytes_written, portMAX_DELAY);
  if(bytes_written != ByteCount){ if(false == QUEUE_DEBUG) Serial << GetTitle() << ": Error Writting All Samples\n"; }
  return bytes_written;
}

void I2S_Device::InstallDevice()
{
	Serial << "Configuring I2S Device\n";
	AllocateMemory();
    
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
    .use_apll = false,
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
  if (err != ESP_OK) {
    if(false == DATA_RX_DEBUG)Serial << GetTitle() << ": Failed installing driver: " << err << "\n";
    ESP.restart();
  }
  if (m_i2s_event_queue == NULL)
  {
    if(false == DATA_RX_DEBUG)Serial << GetTitle() << ": Failed to setup event queue.\n";
  }
  err = i2s_set_clk(m_I2S_PORT, m_SampleRate, m_BitsPerSample, m_i2s_channel);
  if (err != ESP_OK) {
    if(false == DATA_RX_DEBUG)Serial << GetTitle() << ": Failed setting clock: " << err << "\n";
    ESP.restart();
  }
  err = i2s_set_pin(m_I2S_PORT, &pin_config);
  if (err != ESP_OK) {
    if(false == DATA_RX_DEBUG)Serial << GetTitle() << ": Failed setting pin: " << err << "\n";
    ESP.restart();
  }
  if(false == DATA_RX_DEBUG)Serial << GetTitle() << ": Driver Installed.\n";
}

void I2S_Device::ProcessEventQueue()
{
  if(NULL != m_i2s_event_queue)
  {
    i2s_event_t i2sEvent = {};
    uint8_t i2sMsgCount = uxQueueMessagesWaiting(m_i2s_event_queue);    
    // Iterate over all events in the i2s event queue
    //for (uint8_t i = 0; i < i2sMsgCount; ++i)
    for( int i = 0; i < i2sMsgCount; ++i )
	{
	  if(true == QUEUE_DEBUG)Serial << GetTitle() << " Queue Count: " << i2sMsgCount << "\n";
      // Take next event from queue
      if ( xQueueReceive(m_i2s_event_queue, (void*) &i2sEvent, 0) == pdTRUE )
      {
        switch (i2sEvent.type)
        {
            case I2S_EVENT_DMA_ERROR:
                Serial.println("I2S_EVENT_DMA_ERROR");
                break;
            case I2S_EVENT_TX_DONE:
                if(true == QUEUE_INDEPTH_DEBUG)Serial << GetTitle() << " TX\n";
                break;
            case I2S_EVENT_RX_DONE:
                {
                  if(true == QUEUE_INDEPTH_DEBUG)Serial << GetTitle() << " RX\n";
                  ReadSamples();
                }
                break;
            case I2S_EVENT_MAX:
                Serial.println("I2S_EVENT_MAX");
                break;
        }
      }
    } 
  }
}
void I2S_Device::AllocateMemory()
{
	Serial << GetTitle() << ": Allocating Memory.\n";    

	CreateQueue(m_i2s_Data_Buffer_Queue, m_TotalBytesToRead, 10, true);
	CreateQueue(m_i2s_Right_Data_Buffer_queue, m_ChannelBytesToRead, 10, true);
	CreateQueue(m_i2s_Left_Data_Buffer_queue, m_ChannelBytesToRead, 10, true);
}
void I2S_Device::FreeMemory()
{
	if(m_i2s_Data_Buffer_Queue)
	{
		vQueueDelete(m_i2s_Data_Buffer_Queue);
		m_i2s_Data_Buffer_Queue = NULL;
	}
	if(m_i2s_Right_Data_Buffer_queue)
	{
		vQueueDelete(m_i2s_Right_Data_Buffer_queue);
		m_i2s_Right_Data_Buffer_queue = NULL;
	}
	if(m_i2s_Left_Data_Buffer_queue)
	{
		vQueueDelete(m_i2s_Left_Data_Buffer_queue);
		m_i2s_Left_Data_Buffer_queue = NULL;
	}
}
