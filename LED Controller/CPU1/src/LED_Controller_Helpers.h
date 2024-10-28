
#pragma once
#include "esp_log.h"
#define SERIAL_RX_BUFFER_SIZE 2048

BluetoothA2DPSink m_BTSink;
Bluetooth_Sink m_BT_In = Bluetooth_Sink( "Bluetooth"
                                       , m_BTSink
                                       , I2S_NUM_0                          // I2S Interface
                                       , i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_TX)
                                       , I2S_SAMPLE_RATE
                                       , I2S_BITS_PER_SAMPLE_16BIT
                                       , I2S_CHANNEL_FMT_RIGHT_LEFT
                                       , i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB)
                                       , I2S_CHANNEL_STEREO
                                       , true                               // Use APLL
                                       , false
                                       , I2S_BUFFER_COUNT                   // Buffer Count
                                       , I2S_CHANNEL_SAMPLE_COUNT           // Buffer Size
                                       , I2S2_SCLCK_PIN                     // Serial Clock Pin
                                       , I2S2_WD_PIN                        // Word Selection Pin
                                       , I2S2_SDIN_PIN                      // Serial Data In Pin
                                       , I2S2_SDOUT_PIN );                  // Serial Data Out Pin

I2S_Device m_Mic_In = I2S_Device( "Microphone"
                                , I2S_NUM_0                          // I2S Interface
                                , i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX)
                                , I2S_SAMPLE_RATE
                                , I2S_BITS_PER_SAMPLE_32BIT
                                , I2S_BITS_PER_SAMPLE_16BIT
                                , I2S_CHANNEL_FMT_RIGHT_LEFT
                                , i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB)
                                , I2S_CHANNEL_STEREO
                                , true                               // Use APLL
                                , false                              // Fixed Clock
                                , I2S_BUFFER_COUNT                   // Buffer Count
                                , I2S_CHANNEL_SAMPLE_COUNT           // Buffer Size
                                , I2S1_SCLCK_PIN                     // Serial Clock Pin
                                , I2S1_WD_PIN                        // Word Selection Pin
                                , I2S1_SDIN_PIN                      // Serial Data In Pin
                                , I2S1_SDOUT_PIN );                  // Serial Data Out Pin

I2S_Device m_I2S_Out = I2S_Device( "I2S Out"
                                 , I2S_NUM_1                        // I2S Interface
                                 , i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_TX)
                                 , I2S_SAMPLE_RATE
                                 , I2S_BITS_PER_SAMPLE_16BIT
                                 , I2S_BITS_PER_SAMPLE_16BIT
                                 , I2S_CHANNEL_FMT_RIGHT_LEFT
                                 , i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB)
                                 , I2S_CHANNEL_STEREO
                                 , true                            // Use APLL
                                 , false                           // Fixed Clock
                                 , I2S_BUFFER_COUNT                // Buffer Count
                                 , I2S_CHANNEL_SAMPLE_COUNT        // Buffer Size
                                 , I2S2_SCLCK_PIN                  // Serial Clock Pin
                                 , I2S2_WD_PIN                     // Word Selection Pin
                                 , I2S2_SDIN_PIN                   // Serial Data In Pin
                                 , I2S2_SDOUT_PIN );               // Serial Data Out Pin

StatisticalEngine m_StatisticalEngine = StatisticalEngine();
StatisticalEngineModelInterface m_StatisticalEngineModelInterface = StatisticalEngineModelInterface(m_StatisticalEngine);
VisualizationPlayer m_VisualizationPlayer = VisualizationPlayer(m_StatisticalEngineModelInterface);
                                                      
CalculateFPS m_CalculateFPS("Main Loop", 1000);
TaskScheduler m_Scheduler;

Manager m_Manager( "Manager"
                 , m_StatisticalEngine
                 , m_BT_In
                 , m_Mic_In
                 , m_I2S_Out );


void InitLocalVariables()
{
  m_Manager.Setup();
  //m_Scheduler.AddTask(m_CalculateFPS);
  //m_Scheduler.AddTask(m_StatisticalEngineModelInterface);
  //m_Scheduler.AddTask(m_VisualizationPlayer);
}

void PrintStartupData()
{
  ESP_LOGI("Startup", "Serial Datalink Configured");
  ESP_LOGI("Startup", "Xtal Clock Frequency: %i MHz", getXtalFrequencyMhz());
  ESP_LOGI("Startup", "CPU Clock Frequency: %i MHz", getCpuFrequencyMhz());
  ESP_LOGI("Startup", "Apb Clock Frequency: %i Hz", getApbFrequency());
}
void PrintFreeHeap()
{
  ESP_LOGI("Startup", "Total heap: %d", ESP.getHeapSize());
  ESP_LOGI("Startup", "Free heap: %d", ESP.getFreeHeap());
  ESP_LOGI("Startup", "Total PSRAM: %d", ESP.getPsramSize());
  ESP_LOGI("Startup", "Free PSRAM: %d", ESP.getFreePsram());
}

void InitSerialCommunication()
{
  //PC Serial Communication
  Serial.begin(115200);
  Serial.flush();
  
  Serial1.setRxBufferSize(SERIAL_RX_BUFFER_SIZE);
  Serial1.begin(500000, SERIAL_8O2, CPU2_RX, CPU2_TX);
  Serial1.flush();
  
  Serial2.setRxBufferSize(SERIAL_RX_BUFFER_SIZE);
  Serial2.begin(500000, SERIAL_8O2, CPU3_RX, CPU3_TX);
  Serial2.flush();
}

void TestPSRam()
{
    const size_t theSize = 100;
    size_t freeSizeBefore = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    void* buffer1 = heap_caps_malloc(theSize, MALLOC_CAP_SPIRAM);
    assert(buffer1);
    size_t freeSizeAfter = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    size_t allocatedSize = freeSizeBefore - freeSizeAfter;
    ESP_LOGI("TestPSRam", "Requested: %zu bytes, Allocated (including overhead): %zu bytes", theSize, allocatedSize);
    assert(allocatedSize >= theSize);
    free(buffer1);
    buffer1 = nullptr;
    size_t freeSizeAfterFree = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    ESP_LOGI("TestPSRam", "Free size before: %zu, after: %zu, after free: %zu", freeSizeBefore, freeSizeAfter, freeSizeAfterFree);
    assert(freeSizeAfterFree == freeSizeBefore);
    
    freeSizeBefore = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    void* buffer2 = ps_malloc(theSize);
    assert(buffer2);
    freeSizeAfter = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    allocatedSize = freeSizeBefore - freeSizeAfter;
    ESP_LOGI("TestPSRam", "ps_malloc Requested: %zu bytes, Allocated (including overhead): %zu bytes", theSize, allocatedSize);
    assert(allocatedSize >= theSize);
    free(buffer2);
    buffer2 = nullptr;
    freeSizeAfterFree = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    ESP_LOGI("TestPSRam", "Free size before: %zu, after: %zu, after free: %zu", freeSizeBefore, freeSizeAfter, freeSizeAfterFree);
    assert(freeSizeAfterFree == freeSizeBefore);
}

void SetComponentDebugLevels()
{ 
  //Global Setting
  esp_log_level_set("*", ESP_LOG_ERROR);

  //Component Setting
  esp_log_level_set("Startup", ESP_LOG_INFO);
}
