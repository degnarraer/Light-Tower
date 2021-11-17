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
                       , DataManager &DataManager
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
                       : Task(Title, DataManager)
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
                       , m_SerialDataOutPin(SerialDataOutPin){}

void I2S_Device::StartDevice()
{
  InstallDevice();
  i2s_start(m_I2S_PORT);
}

void I2S_Device::StopDevice()
{
  i2s_stop(m_I2S_PORT);
}

//Task Interface
void I2S_Device::Setup()
{
  m_Buffer = (int32_t*)malloc(BYTES_TO_READ * 2);
  m_LeftChannel_Buffer = (int32_t*)malloc(BYTES_TO_READ);
  m_RightChannel_Buffer = (int32_t*)malloc(BYTES_TO_READ);
}
bool I2S_Device::CanRunMyTask()
{ 
  return (m_i2s_event_queue != NULL);
}
void I2S_Device::RunMyTask()
{
  ProcessEventQueue();
}

int I2S_Device::ReadSamples(int32_t *samples)
{
    // read from i2s
    size_t bytes_read = 0;
    size_t bytes_to_read = BYTES_TO_READ * 2;
    i2s_read(m_I2S_PORT, samples, bytes_to_read, &bytes_read, portMAX_DELAY);
    int samples_to_read = bytes_to_read / (SAMPLE_SIZE * 2);
    int samples_read = bytes_read / (SAMPLE_SIZE * 2);
    if(samples_read != samples_to_read) Serial.println("Error Reading All Samples");
    for(int i = 0; i < samples_to_read; ++i)
    {
      m_RightChannel_Buffer[i] = samples[i];
      m_LeftChannel_Buffer[i] = samples[i+1];
      /*
      Serial.print(-100000000);
      Serial.print(",");
      Serial.print(100000000);
      Serial.print(",");
      Serial.print(m_RightChannel_Buffer[i]);
      Serial.print(",");
      Serial.print(m_RightChannel_Buffer[i]);
      Serial.print(",");
      Serial.print(m_RightChannel_Buffer[i]);
      Serial.println();
      */
    }
    return samples_read;
}

int I2S_Device::WriteSamples(int32_t *samples)
{
    // write to i2s
    size_t bytes_written = 0;
    size_t bytes_to_write = (m_BitsPerSample/8) * 2 * m_BufferSize;
    i2s_write(m_I2S_PORT, samples, bytes_to_write, &bytes_written, portMAX_DELAY);
    
    int samples_to_write = bytes_to_write / (m_BitsPerSample/8);
    int samples_written = bytes_written / (m_BitsPerSample/8);
    if(samples_written != samples_to_write) Serial.println("Error Writting All Samples");
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
  err = i2s_driver_install(m_I2S_PORT, &i2s_config, m_BufferCount*2, &m_i2s_event_queue);
  if (err != ESP_OK) {
    Serial.printf("i2s Driver: Failed installing driver: %d\n", err);
    while (true);
  }
  if (m_i2s_event_queue == NULL)
  {
    Serial.println("i2s Driver: Failed to setup event queue.");
  }
  err = i2s_set_clk(m_I2S_PORT, m_SampleRate, m_BitsPerSample, m_i2s_channel);
  if (err != ESP_OK) {
    Serial.printf("i2s Driver: Failed setting clock: %d\n", err);
    while (true);
  }
  err = i2s_set_pin(m_I2S_PORT, &pin_config);
  if (err != ESP_OK) {
    Serial.printf("i2s Driver: Failed setting pin: %d\n", err);
    while (true);
  }
  Serial.println("i2s Driver: Driver Installed."); 
}

void I2S_Device::ProcessEventQueue()
{
  i2s_event_t i2sEvent = {};
  uint8_t i2sMsgCount = uxQueueMessagesWaiting(m_i2s_event_queue);
      
  Serial << "Queue: " << i2sMsgCount << "\n";

  // Iterate over all events in the i2s event queue
  for (uint8_t i = 0; i < i2sMsgCount; i++)
  {
      // Take next event from queue
      if ( xQueueReceive(m_i2s_event_queue, (void*) &i2sEvent, portMAX_DELAY) == pdTRUE )
      {
          switch (i2sEvent.type)
          {
              case I2S_EVENT_DMA_ERROR:
                  Serial.println("I2S_EVENT_DMA_ERROR");
                  break;
              case I2S_EVENT_TX_DONE:
                  //Serial.println("TX COMPLETED");
                  SendNotificationToCallees(SpeakerNotification);
                  break;
              case I2S_EVENT_RX_DONE:
                  {
                    //Serial.println("RX STARTED");
                    ReadSamples(m_Buffer);
                    SendNotificationToCallees(MicrophoneNotification);
                  }
                  break;
              case I2S_EVENT_MAX:
                  Serial.println("I2S_EVENT_MAX");
                  break;
          }
      }
  }
}
