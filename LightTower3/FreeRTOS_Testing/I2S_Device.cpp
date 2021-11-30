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
                       : m_I2S_PORT(i2S_PORT)
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
  m_Title = Title;
}
I2S_Device::~I2S_Device()
{
}

void I2S_Device::Setup()
{
    m_BytesPerSample = m_BitsPerSample/8;
    m_BytesToRead  = m_BytesPerSample * m_BufferSize * m_i2s_channel;
    m_ChannelBytesToRead = m_BytesPerSample * m_BufferSize;
    m_SampleCount = m_BytesToRead / (m_BytesPerSample * m_i2s_channel);
    m_TotalBuffers = m_BufferSize * m_i2s_channel;

    Serial << m_Title << ": Allocating Memory.\n";    
    m_SoundBufferData = (int32_t*)malloc(m_BytesToRead);
    m_RightChannel_SoundBufferData = (int32_t*)malloc(m_ChannelBytesToRead);
    m_LeftChannel_SoundBufferData = (int32_t*)malloc(m_ChannelBytesToRead);
   
    Serial << m_Title << ": Creating Sound Buffer queue.\n";
    m_i2s_Data_Buffer_Queue = xQueueCreate( 10, m_BytesToRead );
    if(m_i2s_Data_Buffer_Queue == NULL){Serial.println("Error creating the queue");}
    
    Serial << m_Title << ": Creating Right Channel Sound Buffer queue.\n";
    m_i2s_Right_Data_Buffer_queue = xQueueCreate( 10, m_ChannelBytesToRead );
    if(m_i2s_Right_Data_Buffer_queue == NULL){Serial.println("Error creating the queue");}

    Serial << m_Title << ": Creating Left Channel Sound Buffer queue.\n";
    m_i2s_Left_Data_Buffer_queue = xQueueCreate( 10, m_ChannelBytesToRead );
    if(m_i2s_Left_Data_Buffer_queue == NULL){Serial.println("Error creating the queue");}
    
    SetMuteState(m_MuteState);
}

void I2S_Device::StartDevice()
{
  InstallDevice();
  i2s_start(m_I2S_PORT);
}

void I2S_Device::StopDevice()
{
  i2s_stop(m_I2S_PORT);
}

void I2S_Device::SetSoundBufferData(int32_t *SoundBufferData)
{
  for(int i = 0; i < m_BufferSize * m_i2s_channel; ++i)
  {
    m_SoundBufferData[i] = SoundBufferData[i] * 0.1; //ADD VOLUME HERE
  }
  WriteSamples(m_SoundBufferData);
  if(true == DATA_TX_DEBUG) Serial <<  m_Title << "Sound Buffer Data Ready\n";
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
  i2s_read(m_I2S_PORT, m_SoundBufferData, m_BytesToRead, &bytes_read, portMAX_DELAY);
  samples_read = m_BytesToRead / (m_BytesPerSample * m_i2s_channel);
  if(bytes_read != m_BytesToRead)Serial.println("Error Reading All Bytes");
  if(xQueueSend(m_i2s_Data_Buffer_Queue, m_SoundBufferData, portMAX_DELAY) != pdTRUE){Serial.println("Error Setting Queue");}
  if(I2S_CHANNEL_STEREO == m_i2s_channel)
  {
    for(int i = 0; i < samples_read; ++i)
    {
      m_RightChannel_SoundBufferData[i] = m_SoundBufferData[i];
      m_LeftChannel_SoundBufferData[i] = m_SoundBufferData[i+1];
    }
    if(xQueueSend(m_i2s_Right_Data_Buffer_queue, m_RightChannel_SoundBufferData, portMAX_DELAY) != pdTRUE){ Serial.println("Error Setting Queue"); }
    if(xQueueSend(m_i2s_Left_Data_Buffer_queue, m_LeftChannel_SoundBufferData, portMAX_DELAY) != pdTRUE){ Serial.println("Error Setting Queue"); }
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
  if(samples_written != samples_to_write){if(false == QUEUE_DEBUG)Serial.println("Error Writting All Samples");}
  return samples_written;
}

void I2S_Device::InstallDevice()
{
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
    if(false == DATA_RX_DEBUG)Serial << m_Title << " i2s Driver: Failed installing driver: " << err << "\n";
    while (true);
  }
  if (m_i2s_event_queue == NULL)
  {
    if(false == DATA_RX_DEBUG)Serial << m_Title << " i2s Driver: Failed to setup event queue.\n";
  }
  err = i2s_set_clk(m_I2S_PORT, m_SampleRate, m_BitsPerSample, m_i2s_channel);
  if (err != ESP_OK) {
    if(false == DATA_RX_DEBUG)Serial << m_Title << " i2s Driver: Failed setting clock: " << err << "\n";
    while (true);
  }
  err = i2s_set_pin(m_I2S_PORT, &pin_config);
  if (err != ESP_OK) {
    if(false == DATA_RX_DEBUG)Serial << m_Title << " i2s Driver: Failed setting pin: " << err << "\n";
    while (true);
  }
  if(false == DATA_RX_DEBUG)Serial << m_Title << " i2s Driver: Driver Installed.\n";
}

void I2S_Device::ProcessEventQueue()
{
  if(NULL != m_i2s_event_queue)
  {
    i2s_event_t i2sEvent = {};
    uint8_t i2sMsgCount = uxQueueMessagesWaiting(m_i2s_event_queue);
    if(true == QUEUE_DEBUG) Serial << m_Title << " Queue Count: " << i2sMsgCount << "\n";
    
    // Iterate over all events in the i2s event queue
    for (uint8_t i = 0; i < i2sMsgCount; ++i)
    {
      if(true == QUEUE_DEBUG)Serial << m_Title << " Queue: " << i+1 << " of " << i2sMsgCount << "\n";
      // Take next event from queue
      if ( xQueueReceive(m_i2s_event_queue, (void*) &i2sEvent, portMAX_DELAY) == pdTRUE )
      {
        switch (i2sEvent.type)
        {
            case I2S_EVENT_DMA_ERROR:
                Serial.println("I2S_EVENT_DMA_ERROR");
                break;
            case I2S_EVENT_TX_DONE:
                if(true == QUEUE_INDEPTH_DEBUG)Serial << m_Title << " TX\n";
                break;
            case I2S_EVENT_RX_DONE:
                {
                  if(true == QUEUE_INDEPTH_DEBUG)Serial << m_Title << " RX\n";
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
