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
                       , int I2SClockPin
                       , int I2SWordSelectPin
                       , int I2SDataInPin
                       , int I2SDataOutPin )
                       : NamedItem(Title)
					   , m_I2S_PORT(i2S_PORT)
                       , m_SampleRate(SampleRate)
                       , m_i2s_Mode(Mode)
                       , m_BitsPerSample(i2s_BitsPerSample)
                       , m_CommFormat(i2s_CommFormat)
                       , m_Channel_Fmt(i2s_Channel_Fmt)
                       , m_i2s_channel(i2s_channel)
					   , m_Use_APLL(Use_APLL)
                       , m_BufferCount(BufferCount)
                       , m_BufferSize(BufferSize)
                       , m_I2SClockPin(I2SClockPin)
                       , m_I2SWordSelectPin(I2SWordSelectPin)
                       , m_I2SDataInPin(I2SDataInPin)
                       , m_I2SDataOutPin(I2SDataOutPin)
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
}
void I2S_Device::StartDevice()
{
  ESP_LOGI("StartDevice", "%s: Starting I2S device.", GetTitle().c_str());
  if(m_DeviceState != DeviceState::Running)
  {
    InstallDevice();
    if(ESP_OK == i2s_start(m_I2S_PORT))
    {
      CreateTask();
      m_DeviceState = DeviceState::Running;
      ESP_LOGI("StartDevice", "%s: I2S device started.", GetTitle().c_str());
    }
  }
  else
  {
      ESP_LOGW("StartDevice", "WARNING! %s: I2S device already started.", GetTitle().c_str());
  }
}

void I2S_Device::StopDevice()
{
  ESP_LOGI("StopDevice", "%s: Stopping I2S device.", GetTitle().c_str());
  if(m_DeviceState == DeviceState::Running && IsInitialized())
  {
    ESP_LOGI("StopDevice", "%s: Stopping I2S driver.", GetTitle().c_str());
    if(ESP_OK == i2s_stop(m_I2S_PORT))
    {
      ESP_LOGI("StopDevice", "%s: I2S driver stopped.", GetTitle().c_str());
      DestroyTask();
      ESP_LOGI("StopDevice", "%s: I2S device stopped.", GetTitle().c_str());
    }
    else
    {
      ESP_LOGW("StopDevice", "WARNING! %s: Unable to stop I2S driver.", GetTitle().c_str());
    }
  }
  else
  {
    ESP_LOGW("StopDevice", "%s: WARNING! I2S device not running.", GetTitle().c_str());
  }
  UninstallDevice();
}

size_t I2S_Device::WriteSoundBufferData(uint8_t *SoundBufferData, size_t ByteCount)
{
	return WriteSamples(SoundBufferData, ByteCount);
}

size_t I2S_Device::ReadSoundBufferData(uint8_t *SoundBufferData, size_t ByteCount)
{
	size_t bytes_read = 0;
  if(m_I2S_PORT != I2S_NUM_0 && m_I2S_PORT != I2S_NUM_1)
  {
      ESP_LOGE("I2S Device", "%s: ERROR! Invalid I2S port: %d", GetTitle().c_str(), m_I2S_PORT);
      return bytes_read;
  }
	if(i2s_read(m_I2S_PORT, SoundBufferData, ByteCount, &bytes_read, portMAX_DELAY ) != ESP_OK)
	{
		ESP_LOGE("i2S Device", "%s: ERROR! Unable to read samples.", GetTitle().c_str());
		return bytes_read;
	}
	return bytes_read;
}

size_t I2S_Device::ReadSamples()
{
	size_t bytes_read = 0;
	uint8_t DataBuffer[m_BufferSize];
  i2s_read(m_I2S_PORT, DataBuffer, m_TotalBytesToRead, &bytes_read, portMAX_DELAY);
  if(bytes_read == 0) return 0;
  ESP_LOGI("i2S Device", "%s: Read %i bytes of %i bytes.", GetTitle().c_str(), bytes_read, m_TotalBytesToRead);
  if(m_Callee)
	{
		m_Callee->I2SDataReceived(GetTitle().c_str(), DataBuffer, bytes_read);
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
		ESP_LOGE("i2S Device", "%s: ERROR! Unable to write all bytes.", GetTitle().c_str()); 
	}
	ESP_LOGI("i2S Device", "%s: Write %i bytes of %i bytes.", GetTitle().c_str(), bytes_written, ByteCount);
	return bytes_written;
}

void I2S_Device::UninstallDevice()
{
  ESP_LOGI("Uninstall Device", "%s: Uninstalling I2S device.", GetTitle().c_str());
  if(m_DeviceState == DeviceState::Stopped)
  {
    if(ESP_OK == i2s_driver_uninstall(m_I2S_PORT))
    {
      ESP_LOGI("Uninstall Device", "%s: I2S device uninstalled.", GetTitle().c_str());
      m_DeviceState = DeviceState::Uninstalled;
    }
    else
    {
      ESP_LOGE("Uninstall Device", "%s: Uninstall I2S device failed.", GetTitle().c_str());
    }
  }
  else
  {
    ESP_LOGE("Uninstall Device", "%s: I2S device not stopped.", GetTitle().c_str());
  }
}

void I2S_Device::InstallDevice()
{
  if(m_DeviceState == DeviceState::Uninstalled)
  {
    ESP_LOGI("i2S Device", "%s: Installing I2S device.", GetTitle().c_str());
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
      .bck_io_num = m_I2SClockPin,         // Serial Clock (SCK)
      .ws_io_num = m_I2SWordSelectPin,           // Word Select (WS)
      .data_out_num = m_I2SDataOutPin,     // not used (only for speakers)
      .data_in_num = m_I2SDataInPin        // Serial Data (SD)
    };
    // Configuring the I2S driver and pins.
    // This function must be called before any I2S driver read/write operations.
    err = i2s_driver_install(m_I2S_PORT, &i2s_config, m_BufferCount, &m_i2s_event_queueHandle);
    if (err != ESP_OK)
    {
      ESP_LOGE("i2S Device", "ERROR! %s: Failed installing driver: %s.", GetTitle().c_str(), err);
      ESP.restart();
    }
    if (NULL == m_i2s_event_queueHandle)
    {
      ESP_LOGE("i2S Device", "ERROR! %s: Failed to setup event queue.", GetTitle().c_str());
      ESP.restart();
    }
    err = i2s_set_clk(m_I2S_PORT, m_SampleRate, m_BitsPerSample, m_i2s_channel);
    if (err != ESP_OK)
    {
      ESP_LOGE("i2S Device", "ERROR! %s: Failed setting clock: %s.", GetTitle().c_str(), err);
      ESP.restart();
    }
    err = i2s_set_pin(m_I2S_PORT, &pin_config);
    if (err != ESP_OK)
    {
      ESP_LOGE("i2S Device", "ERROR! %s: Failed setting pin: %s.", GetTitle().c_str(), err);
      ESP.restart();
    }
    ESP_LOGI("i2S Device", "%s: Device Installed.", GetTitle().c_str());
    m_DeviceState = DeviceState::Installed;
  }
  else
  {
    ESP_LOGI("i2S Device", "%s: Device already Installed.", GetTitle().c_str());
  }
}

bool I2S_Device::IsInitialized() 
{
  return (m_I2S_PORT != I2S_NUM_MAX);
}

void I2S_Device::CreateTask()
{
  if( xTaskCreatePinnedToCore( Static_ProcessEventQueue, "I2S_Device_10mS_Task", 10000, this, THREAD_PRIORITY_HIGH,  &m_TaskHandle, 0 ) == pdTRUE )
  {
    ESP_LOGI("StartDevice", "%s: I2S device task started.", GetTitle().c_str());
  }
  else
  {
    ESP_LOGE("StartDevice", "ERROR! %s: Unable to create task!", GetTitle().c_str());
  }
}

void I2S_Device::DestroyTask()
{
  ESP_LOGI("DestroyTask", "%s: destroying I2S device task.", GetTitle().c_str());
  if(m_TaskHandle != NULL)
  {
    vTaskDelete(m_TaskHandle);
    m_TaskHandle = nullptr;
    m_DeviceState = DeviceState::Stopped;
    ESP_LOGI("DestroyTask", "%s: I2S device task destroyed.", GetTitle().c_str());
  }
  else
  {
    ESP_LOGW("DestroyTask", "WARNING! %s: Unable to destroy task!", GetTitle().c_str());
  }
}

void I2S_Device::Static_ProcessEventQueue(void * parameter)
{
  I2S_Device* device = static_cast<I2S_Device*>(parameter);
  device->ProcessEventQueue();
}

void I2S_Device::ProcessEventQueue()
{
  const TickType_t xFrequency = 20;
  TickType_t xLastWakeTime = xTaskGetTickCount();
  while(true)
  {
    vTaskDelayUntil( &xLastWakeTime, xFrequency );
    if(m_i2s_event_queueHandle)
    {
      i2s_event_t i2sEvent = {};
      uint8_t i2sMsgCount = uxQueueMessagesWaiting(m_i2s_event_queueHandle);
      for( int i = 0; i < i2sMsgCount; ++i )
      {
        if ( xQueueReceive(m_i2s_event_queueHandle, (void*) &i2sEvent, 0) == pdTRUE )
        {
          switch (i2sEvent.type)
          {
            case I2S_EVENT_DMA_ERROR:
              ESP_LOGE("ProcessEventQueue", "ERROR! %s: I2S_EVENT_DMA_ERROR.", GetTitle().c_str());
            break;
            case I2S_EVENT_TX_DONE:
              ESP_LOGV("ProcessEventQueue", "%s: TX Done", GetTitle().c_str());
            break;
            case I2S_EVENT_RX_DONE:
              {
                ESP_LOGV("ProcessEventQueue", "%s: RX", GetTitle().c_str());
                ReadSamples();
              }
            break;
            case I2S_EVENT_MAX:
              ESP_LOGW("ProcessEventQueue", "WARNING! I2S_EVENT_MAX");
            break;
          }	
        }
      }
    }
  }
}
