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
                       , BaseType_t Core
                       , i2s_port_t i2S_PORT
                       , i2s_mode_t Mode
                       , int SampleRate
                       , i2s_bits_per_sample_t i2s_BitsPerSampleIn
                       , i2s_bits_per_sample_t i2s_BitsPerSampleOut
                       , i2s_channel_fmt_t i2s_Channel_Fmt
                       , i2s_comm_format_t i2s_CommFormat
                       , i2s_channel_t i2s_channel
					             , bool Use_APLL
                       , bool fixedClock
                       , size_t BufferCount
                       , size_t BufferSize
                       , int I2SClockPin
                       , int I2SWordSelectPin
                       , int I2SDataInPin
                       , int I2SDataOutPin )
                       : NamedItem(Title)
                       , m_Core(Core)
					             , m_I2S_PORT(i2S_PORT)
                       , m_SampleRate(SampleRate)
                       , m_i2s_Mode(Mode)
                       , m_BitsPerSampleIn(i2s_BitsPerSampleIn)
                       , m_BitsPerSampleOut(i2s_BitsPerSampleOut)
                       , m_CommFormat(i2s_CommFormat)
                       , m_Channel_Fmt(i2s_Channel_Fmt)
                       , m_i2s_channel(i2s_channel)
					             , m_Use_APLL(Use_APLL)
                       , m_FixedClock(fixedClock)
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
  m_BytesPerSample = m_BitsPerSampleIn/8;
  m_ChannelSampleCount = m_BufferSize;
	m_SampleCount = m_ChannelSampleCount * 2;
  m_ChannelBytesToRead  = m_BytesPerSample * m_ChannelSampleCount;
  m_TotalBytesToRead = m_ChannelBytesToRead * 2;
}
void I2S_Device::StartDevice()
{
  ESP_LOGI("StartDevice", "%s: Starting I2S device.", GetTitle().c_str());
  if(IsInitialized())
  {
    if(m_DeviceState != DeviceState_t::Running)
    {
      InstallDevice();
      if(ESP_OK == i2s_start(m_I2S_PORT))
      {
        CreateTask();
        m_DeviceState = DeviceState_t::Running;
        ESP_LOGI("StartDevice", "%s: I2S device started. Device currently: \"%s\".", GetTitle().c_str(), GetDeviceStateString().c_str());
      }
    }
    else
    {
        ESP_LOGW("StartDevice", "WARNING! %s: I2S device already started. Device currently: \"%s\".", GetTitle().c_str(), GetDeviceStateString().c_str());
    }
  }
  else
  {
    ESP_LOGW("StopDevice", "%s: WARNING! I2S device not initialized. Device currently: \"%s\".", GetTitle().c_str(), GetDeviceStateString().c_str());
  }
}

void I2S_Device::StopDevice()
{
  ESP_LOGI("StopDevice", "%s: Stopping I2S device. Device currently: \"%s\".", GetTitle().c_str(), GetDeviceStateString().c_str());
  if(IsInitialized())
  {
    if(m_DeviceState == DeviceState_t::Running)
    {
      ESP_LOGI("StopDevice", "%s: Stopping I2S driver. Device currently: \"%s\".", GetTitle().c_str(), GetDeviceStateString().c_str());
      if(ESP_OK == i2s_stop(m_I2S_PORT))
      {
        ESP_LOGI("StopDevice", "%s: I2S driver stopped. Device currently: \"%s\".", GetTitle().c_str(), GetDeviceStateString().c_str());
        DestroyTask();
        ESP_LOGI("StopDevice", "%s: I2S device stopped. Device currently: \"%s\".", GetTitle().c_str(), GetDeviceStateString().c_str());
      }
      else
      {
        ESP_LOGW("StopDevice", "WARNING! %s: Unable to stop I2S driver. Device currently: \"%s\".", GetTitle().c_str(), GetDeviceStateString().c_str());
      }
    }
    else
    {
      ESP_LOGW("StopDevice", "%s: WARNING! I2S device not running. Device currently: \"%s\".", GetTitle().c_str(), GetDeviceStateString().c_str());
    }
  }
  else
  {
    ESP_LOGW("StopDevice", "%s: WARNING! I2S device not initialized. Device currently: \"%s\".", GetTitle().c_str(), GetDeviceStateString().c_str());
  }
  UninstallDevice();
}

DeviceState_t I2S_Device::GetDeviceState()
{
  return m_DeviceState;
}

bool I2S_Device::IsRunning()
{
  return m_DeviceState == DeviceState_t::Running;
}

String I2S_Device::GetDeviceStateString()
{
  String result = "";
  switch(m_DeviceState)
  {
    case DeviceState_t::Installed:
      result = "Installed";
    break;
    case DeviceState_t::Uninstalled:
      result = "Uninstalled";
    break;
    case DeviceState_t::Running:
      result = "Running";
    break;
    case DeviceState_t::Stopped:
      result = "Stopped";
    break;
    default:
      result = "Unknown";
    break;
  }
  return result;
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
	if(i2s_read(m_I2S_PORT, SoundBufferData, ByteCount, &bytes_read, TIME_TO_WAIT_FOR_SOUND ) != ESP_OK)
	{
		ESP_LOGE("i2S Device", "%s: ERROR! Unable to read samples.", GetTitle().c_str());
		return bytes_read;
	}
	return bytes_read;
}

size_t I2S_Device::ReadSamples()
{
    size_t bytes_read = 0;
    uint8_t *dataBuffer = (uint8_t*) ps_malloc(m_TotalBytesToRead);
    if (dataBuffer == nullptr) {
        ESP_LOGE("I2S Device", "%s: ERROR! Failed to allocate memory for DataBuffer.", GetTitle().c_str());
        return 0;
    }
    if (m_I2S_PORT != I2S_NUM_0 && m_I2S_PORT != I2S_NUM_1) {
        ESP_LOGE("I2S Device", "%s: ERROR! Invalid I2S Port.", GetTitle().c_str());
        free(dataBuffer);
        return 0;
    }
    esp_err_t result = i2s_read(m_I2S_PORT, dataBuffer, m_TotalBytesToRead, &bytes_read, TIME_TO_WAIT_FOR_SOUND );
    if (result != ESP_OK) {
        ESP_LOGE("I2S Device", "%s: ERROR! i2s_read failed with error: %s", GetTitle().c_str(), esp_err_to_name(result));
        free(dataBuffer);
        return 0;
    }
    if (bytes_read == 0) {
        free(dataBuffer);
        return 0;
    }
    ESP_LOGV("I2S Device", "%s: Read %i bytes of %i bytes.", GetTitle().c_str(), bytes_read, m_TotalBytesToRead);
    if (m_Callee)
    {
      std::vector<uint8_t> newBuffer = ConvertBitDepth(dataBuffer, bytes_read, m_BitsPerSampleIn, m_BitsPerSampleOut);
      m_Callee->I2SDataReceived(GetTitle().c_str(), newBuffer.data(), newBuffer.size());
    }
    free(dataBuffer);

    return bytes_read;
}

size_t I2S_Device::WriteSamples(uint8_t *samples, size_t byteCount)
{
    size_t bytes_written = 0;

    // Check if the I2S port is valid
    if (m_I2S_PORT != I2S_NUM_0 && m_I2S_PORT != I2S_NUM_1) {
        ESP_LOGE("I2S Device", "%s: ERROR! Invalid I2S Port.", GetTitle().c_str());
        return 0;
    }

    // Write data with a 1-second timeout
    esp_err_t result = i2s_write(m_I2S_PORT, samples, byteCount, &bytes_written, TIME_TO_WAIT_FOR_SOUND );
    if (result != ESP_OK) {
        ESP_LOGE("I2S Device", "%s: ERROR! i2s_write failed with code: %d", GetTitle().c_str(), result);
        return 0;
    }

    // Log the number of bytes written
    ESP_LOGV("I2S Device", "%s: Write %i bytes of %i bytes.", GetTitle().c_str(), bytes_written, byteCount);

    return bytes_written;
}

void I2S_Device::UninstallDevice()
{
  ESP_LOGI("Uninstall Device", "%s: Uninstalling I2S device. Device currently: \"%s\".", GetTitle().c_str(), GetDeviceStateString().c_str());
  if(m_DeviceState == DeviceState_t::Uninstalled)
  {
    ESP_LOGW("Uninstall Device", "%s: I2S device already uninstalled. Device currently: \"%s\".", GetTitle().c_str(), GetDeviceStateString().c_str());
  }
  else
  {
    if(m_DeviceState == DeviceState_t::Stopped)
    {
      if(ESP_OK == i2s_driver_uninstall(m_I2S_PORT))
      { 
        m_DeviceState = DeviceState_t::Uninstalled;
        ESP_LOGI("Uninstall Device", "%s: I2S device uninstalled. Device currently: \"%s\".", GetTitle().c_str(), GetDeviceStateString().c_str());
      }
      else
      {
        ESP_LOGE("Uninstall Device", "%s: Uninstall I2S device failed. Device currently: \"%s\".", GetTitle().c_str(), GetDeviceStateString().c_str());
      }
    }
    else
    {
      ESP_LOGE("Uninstall Device", "%s: I2S device not stopped. Device currently: \"%s\".", GetTitle().c_str(), GetDeviceStateString().c_str());
    }
  }
}

void I2S_Device::InstallDevice()
{
  if(m_DeviceState == DeviceState_t::Uninstalled)
  {
    ESP_LOGI("i2S Device", "%s: Installing I2S device. Device currently: \"%s\".", GetTitle().c_str(), GetDeviceStateString().c_str());
    esp_err_t err;
    // The I2S config as per the example
    const i2s_config_t i2s_config = {
      .mode = m_i2s_Mode,
      .sample_rate = m_SampleRate,
      .bits_per_sample = m_BitsPerSampleIn,
      .channel_format = m_Channel_Fmt,
      .communication_format = m_CommFormat,
      .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
      .dma_buf_count = m_BufferCount,
      .dma_buf_len = m_BufferSize,
      .use_apll = m_Use_APLL,
      .tx_desc_auto_clear = true,
      .fixed_mclk = m_FixedClock
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
    err = i2s_set_clk(m_I2S_PORT, m_SampleRate, m_BitsPerSampleIn, m_i2s_channel);
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
    m_DeviceState = DeviceState_t::Installed;
    ESP_LOGI("i2S Device", "%s: Device Installed. Device currently: \"%s\".", GetTitle().c_str(), GetDeviceStateString().c_str());
  }
  else
  {
    ESP_LOGI("i2S Device", "%s: Device already Installed. Device currently: \"%s\".", GetTitle().c_str(), GetDeviceStateString().c_str());
  }
}

bool I2S_Device::IsInitialized() 
{
  return (m_I2S_PORT < I2S_NUM_MAX);
}

void I2S_Device::CreateTask()
{
  if( xTaskCreatePinnedToCore( Static_ProcessEventQueue, "I2S_Device_Task", 10000, this, THREAD_PRIORITY_RT,  &m_TaskHandle, m_Core ) == pdTRUE )
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
    m_DeviceState = DeviceState_t::Stopped;
    ESP_LOGI("DestroyTask", "%s: I2S device task destroyed. Device currently: \"%s\".", GetTitle().c_str(), GetDeviceStateString().c_str());
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
  const TickType_t xFrequency = 50;
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
              ESP_LOGV("ProcessEventQueue", "%s: RX", GetTitle().c_str());
              ReadSamples();
            break;
            case I2S_EVENT_MAX:
              ESP_LOGW("ProcessEventQueue", "WARNING! I2S_EVENT_MAX");
            break;
          }	
        }
        else
        {
              ESP_LOGE("ProcessEventQueue", "ERROR! %s: Failed to receive event queue.", GetTitle().c_str());
        }
      }
    }
  }
}
