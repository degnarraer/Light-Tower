/*
    Light Tower by Rob Shockency
    Copyright (C) 2021 Rob Shockency degnarraer@yahoo.com

    This program is free software: you can redistribute it and/or modifyVision8_LTT
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

#include "Manager.h"
#include "Tunes.h"
#include "esp_log.h"
#include "DataItem/DataItems.h"
#define SERIAL_RX_BUFFER_SIZE 2048

Preferences m_Preferences;
PreferencesWrapper m_PreferencesWrapper = PreferencesWrapper("Settings", &m_Preferences);

I2S_Device m_I2S_In = I2S_Device( "I2S_In"
                                , I2S_NUM_1
                                , i2s_mode_t(I2S_MODE_SLAVE | I2S_MODE_RX)
                                , I2S_SAMPLE_RATE
                                , I2S_BITS_PER_SAMPLE_16BIT
                                , I2S_BITS_PER_SAMPLE_16BIT
                                , I2S_CHANNEL_FMT_RIGHT_LEFT
                                , i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB)
                                , I2S_CHANNEL_STEREO
                                , true
                                , false
                                , I2S_BUFFER_COUNT
                                , I2S_SAMPLE_COUNT
                                , I2S1_SCLCK_PIN
                                , I2S1_WD_PIN
                                , I2S1_SDIN_PIN
                                , I2S1_SDOUT_PIN );

BluetoothA2DPSource a2dp_source;
Bluetooth_Source m_BT_Out( "Bluetooth Source"
                         , BLUETOOTH_TASK_PRIORITY
                         , a2dp_source );
                              
FFT_Computer m_FFT_Computer = FFT_Computer(FFT_SIZE, HOP_SIZE, I2S_SAMPLE_RATE, DataWidth_16, FFT_COMPUTE_TASK_PRIORITY);
ContinuousAudioBuffer<AMPLITUDE_AUDIO_BUFFER_SIZE> m_Amplitude_AudioBuffer;

DataSerializer m_DataSerializer;
SerialPortMessageManager m_CPU1SerialPortMessageManager = SerialPortMessageManager("CPU1 Datalink Task", CPU1_RX, CPU1_TX, 500000, UART_NUM_1, &m_DataSerializer, DATALINK_TASK_PRIORITY);
SerialPortMessageManager m_CPU3SerialPortMessageManager = SerialPortMessageManager("CPU3 Datalink Task", CPU3_RX, CPU3_TX, 500000, UART_NUM_2, &m_DataSerializer, DATALINK_TASK_PRIORITY);


Sound_Processor m_SoundProcessor ( "Sound Processor"
                                 , m_FFT_Computer
                                 , m_Amplitude_AudioBuffer
                                 , m_CPU1SerialPortMessageManager
                                 , m_CPU3SerialPortMessageManager
                                 , m_PreferencesWrapper );                                            

Manager m_Manager( "Manager"
                 , m_SoundProcessor
                 , m_CPU1SerialPortMessageManager
                 , m_CPU3SerialPortMessageManager
                 , m_BT_Out
                 , m_I2S_In
                 , m_FFT_Computer
                 , m_Amplitude_AudioBuffer
                 , m_PreferencesWrapper);

void OutputSystemStatus()
{
  ESP_LOGW("SystemStatus", "Xtal Clock Frequency: %i MHz", getXtalFrequencyMhz());
  ESP_LOGW("SystemStatus", "CPU Clock Frequency: %i MHz", getCpuFrequencyMhz());
  ESP_LOGW("SystemStatus", "Apb Clock Frequency: %i Hz", getApbFrequency());
  ESP_LOGW("SystemStatus", "Total heap: %d", ESP.getHeapSize());
  ESP_LOGW("SystemStatus", "Free heap: %d", ESP.getFreeHeap());
  ESP_LOGW("SystemStatus", "Total PSRAM: %d", ESP.getPsramSize());
  ESP_LOGW("SystemStatus", "Free PSRAM: %d", ESP.getFreePsram());
}

void TestPSRam()
{
    const size_t theSize = 100;
    size_t freeSizeBefore = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    void* buffer1 = heap_caps_malloc(theSize, MALLOC_CAP_SPIRAM);
    assert(buffer1);
    size_t freeSizeAfter = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    size_t allocatedSize = freeSizeBefore - freeSizeAfter;
    ESP_LOGW("TestPSRam", "Requested: %zu bytes, Allocated (including overhead): %zu bytes", theSize, allocatedSize);
    assert(allocatedSize >= theSize);
    free(buffer1);
    buffer1 = nullptr;
    size_t freeSizeAfterFree = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    ESP_LOGW("TestPSRam", "Free size before: %zu, after: %zu, after free: %zu", freeSizeBefore, freeSizeAfter, freeSizeAfterFree);
    assert(freeSizeAfterFree == freeSizeBefore);
    
    freeSizeBefore = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    void* buffer2 = ps_malloc(theSize);
    assert(buffer2);
    freeSizeAfter = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    allocatedSize = freeSizeBefore - freeSizeAfter;
    ESP_LOGW("TestPSRam", "ps_malloc Requested: %zu bytes, Allocated (including overhead): %zu bytes", theSize, allocatedSize);
    assert(allocatedSize >= theSize);
    free(buffer2);
    buffer2 = nullptr;
    freeSizeAfterFree = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    ESP_LOGW("TestPSRam", "Free size before: %zu, after: %zu, after free: %zu", freeSizeBefore, freeSizeAfter, freeSizeAfterFree);
    assert(freeSizeAfterFree == freeSizeBefore);
}

void setup() 
{
  //PC Serial Communication
  Serial.begin(115200, SERIAL_8N1);
  Serial.flush();

  TestPSRam();
  m_Amplitude_AudioBuffer.Setup();
  m_PreferencesWrapper.Setup();
  m_CPU1SerialPortMessageManager.Setup();
  m_CPU3SerialPortMessageManager.Setup();
  m_I2S_In.Setup();
  m_BT_Out.Setup();
  m_Manager.Setup();
  m_SoundProcessor.Setup();
  OutputSystemStatus();
}

void loop()
{
  static unsigned long lastPrintTime = 0;
  unsigned long currentTime = millis();
  if (currentTime - lastPrintTime >= 1000) 
  {
    size_t heapSpace = ESP.getFreeHeap();
    size_t psramSpace = ESP.getFreePsram();
    Serial.printf("Heap Space Left: %u bytes, PSRAM Space Left: %u bytes\n", heapSpace, psramSpace);

    lastPrintTime = currentTime;
  }
}
