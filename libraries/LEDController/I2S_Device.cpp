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
                       , int SerialDataOutPin
                       , int MutePin )
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
                       , m_MutePin(MutePin)
{
}
I2S_Device::~I2S_Device()
{
}

void I2S_Device::Setup()
{
    m_BytesPerSample = m_BitsPerSample/8;
    m_ChannelBytesToRead = m_BytesPerSample * m_BufferSize;
    m_TotalBytesToRead  = m_ChannelBytesToRead * 2;
    m_ChannelSampleCount = m_ChannelBytesToRead / m_BytesPerSample;
	m_SampleCount = m_TotalBytesToRead / m_BytesPerSample;
    SetMuteState(m_MuteState);
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
		delete m_SoundBufferData;
		delete m_RightChannel_SoundBufferData;
		delete m_LeftChannel_SoundBufferData;
		vQueueDelete(m_i2s_Data_Buffer_Queue);
		vQueueDelete(m_i2s_Right_Data_Buffer_queue);
		vQueueDelete(m_i2s_Left_Data_Buffer_queue);
		m_Is_Running = false;
	}
}

void I2S_Device::SetSoundBufferData(int32_t *SoundBufferData)
{
  for(int i = 0; i < m_BufferSize * m_i2s_channel; ++i)
  {
    m_SoundBufferData[i] = SoundBufferData[i] * 0.1; //ADD VOLUME HERE
  }
  WriteSamples(m_SoundBufferData);
  if(true == DATA_TX_DEBUG) Serial <<  GetTitle() << "Sound Buffer Data Ready\n";
}

void I2S_Device::SetMuteState(Mute_State_t MuteState)
{
  pinMode(m_MutePin, OUTPUT);
  switch(MuteState)
  {
    case Mute_State_Un_Muted:
      digitalWrite(m_MutePin, LOW);
      m_MuteState = Mute_State_Un_Muted;
      break;
    case Mute_State_Muted:
      digitalWrite(m_MutePin, HIGH);
      m_MuteState = Mute_State_Muted;
      break;
    default:
      break;
  }
}

int I2S_Device::ReadSamples()
{
  *m_SoundBufferData = {0};
  *m_RightChannel_SoundBufferData = {0};
  *m_LeftChannel_SoundBufferData = {0};
  // read from i2s
  size_t bytes_read = 0;
  size_t samples_read = 0;
  size_t channel_samples_read = 0;
  i2s_read(m_I2S_PORT, m_SoundBufferData, m_TotalBytesToRead, &bytes_read, portMAX_DELAY);
  samples_read = bytes_read / m_BytesPerSample;
  channel_samples_read = bytes_read / (m_BytesPerSample * 2);
  if(bytes_read != m_TotalBytesToRead)Serial << "i2s Driver: " << GetTitle() << " Error Reading All Bytes\n";
  if(NULL != m_Callee) m_Callee->DataBufferModifyRX(GetTitle(), m_SoundBufferData, samples_read);
  if(xQueueSend(m_i2s_Data_Buffer_Queue, m_SoundBufferData, portMAX_DELAY) != pdTRUE){ Serial << "i2s Driver: " << GetTitle() << " Error Setting Data Queue\n"; }
  if(I2S_CHANNEL_STEREO == m_i2s_channel)
  {
    for(int i = 0; i < channel_samples_read; ++i)
    {
      m_RightChannel_SoundBufferData[i] = m_SoundBufferData[i];
      m_LeftChannel_SoundBufferData[i] = m_SoundBufferData[i+1];
    }
	if(NULL != m_Callee) m_Callee->RightChannelDataBufferModifyRX(GetTitle(), m_RightChannel_SoundBufferData, channel_samples_read);
	if(NULL != m_Callee) m_Callee->LeftChannelDataBufferModifyRX(GetTitle(), m_LeftChannel_SoundBufferData, channel_samples_read);
    if(xQueueSend(m_i2s_Right_Data_Buffer_queue, m_RightChannel_SoundBufferData, portMAX_DELAY) != pdTRUE){ Serial << "i2s Driver: " << GetTitle() << " Error Setting Right Channel Queue\n"; }
    if(xQueueSend(m_i2s_Left_Data_Buffer_queue, m_LeftChannel_SoundBufferData, portMAX_DELAY) != pdTRUE){ Serial << "i2s Driver: " << GetTitle() << " Error Setting Left Channel Queue\n"; }
  }
  return samples_read;
}

int I2S_Device::WriteSamples(int32_t *samples)
{
  // write to i2s
  size_t bytes_written = 0;
  size_t bytes_to_write = (m_BitsPerSample/8) * m_i2s_channel * m_BufferSize;
  i2s_write(m_I2S_PORT, samples, bytes_to_write, &bytes_written, portMAX_DELAY);
  
  int samples_to_write = bytes_to_write / (m_BytesPerSample);
  int samples_written = bytes_written / (m_BytesPerSample);
  if(samples_written != samples_to_write){ if(false == QUEUE_DEBUG) Serial << "i2s Driver: " << GetTitle() << " Error Writting All Samples\n"; }
  return samples_written;
}

void I2S_Device::InstallDevice()
{
	Serial << "Configuring I2S Device\n";
	Serial << GetTitle() << ": Allocating Memory.\n";    
    m_SoundBufferData = (int32_t*)malloc(m_TotalBytesToRead);
    m_RightChannel_SoundBufferData = (int32_t*)malloc(m_ChannelBytesToRead);
    m_LeftChannel_SoundBufferData = (int32_t*)malloc(m_ChannelBytesToRead);

	CreateQueue(m_i2s_Data_Buffer_Queue, m_TotalBytesToRead, 10, true);
	CreateQueue(m_i2s_Right_Data_Buffer_queue, m_ChannelBytesToRead, 10, true);
	CreateQueue(m_i2s_Left_Data_Buffer_queue, m_ChannelBytesToRead, 10, true);
    
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
    .use_apll = true,
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
    if(false == DATA_RX_DEBUG)Serial << "i2s Driver: " << GetTitle() << " Failed installing driver: " << err << "\n";
    while (true);
  }
  if (m_i2s_event_queue == NULL)
  {
    if(false == DATA_RX_DEBUG)Serial << "i2s Driver: " << GetTitle() << " Failed to setup event queue.\n";
  }
  err = i2s_set_clk(m_I2S_PORT, m_SampleRate, m_BitsPerSample, m_i2s_channel);
  if (err != ESP_OK) {
    if(false == DATA_RX_DEBUG)Serial << "i2s Driver: " << GetTitle() << " Failed setting clock: " << err << "\n";
    while (true);
  }
  err = i2s_set_pin(m_I2S_PORT, &pin_config);
  if (err != ESP_OK) {
    if(false == DATA_RX_DEBUG)Serial << "i2s Driver: " << GetTitle() << " Failed setting pin: " << err << "\n";
    while (true);
  }
  if(false == DATA_RX_DEBUG)Serial << "i2s Driver: " << GetTitle() << " Driver Installed.\n";
}

void I2S_Device::ProcessEventQueue()
{
  if(NULL != m_i2s_event_queue)
  {
    i2s_event_t i2sEvent = {};
    uint8_t i2sMsgCount = uxQueueMessagesWaiting(m_i2s_event_queue);    
    // Iterate over all events in the i2s event queue
    //for (uint8_t i = 0; i < i2sMsgCount; ++i)
    if(i2sMsgCount > 0)
	{
	  if(true == QUEUE_DEBUG)Serial << GetTitle() << " Queue Count: " << i2sMsgCount << "\n";
      // Take next event from queue
      if ( xQueueReceive(m_i2s_event_queue, (void*) &i2sEvent, portMAX_DELAY) == pdTRUE )
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
