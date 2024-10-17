
#pragma once
#include "esp_log.h"
#define SERIAL_RX_BUFFER_SIZE 2048
                                  
StatisticalEngine m_StatisticalEngine = StatisticalEngine();
StatisticalEngineModelInterface m_StatisticalEngineModelInterface = StatisticalEngineModelInterface(m_StatisticalEngine);
VisualizationPlayer m_VisualizationPlayer = VisualizationPlayer(m_StatisticalEngineModelInterface);
                                                      
CalculateFPS m_CalculateFPS("Main Loop", 1000);
TaskScheduler m_Scheduler;

Manager m_Manager( "Manager"
                 , m_StatisticalEngine );


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
  psramInit();
  void* buffer1 = (void*)heap_caps_malloc(100, MALLOC_CAP_SPIRAM);
  if(buffer1)
  {
    ESP_LOGI("TestPSRam", "Heaps Cap Malloc memory allocated");
    free(buffer1);
    buffer1 = nullptr;
  }
  else
  {
    ESP_LOGE("TestPSRam", "Heaps Cap Malloc memory NOT allocated!");
  }

  void* buffer2 = ps_malloc(100);
  if(buffer2)
  {
    ESP_LOGI("TestPSRam", "PSRAM Allocated");
    free(buffer2);
    buffer2 = nullptr;
  }
  else
  {
    ESP_LOGE("TestPSRam", "PSRAM Not Allocated!");
  }
}

void SetComponentDebugLevels()
{ 
  //Global Setting
  esp_log_level_set("*", ESP_LOG_ERROR);

  //Component Setting
  esp_log_level_set("Startup", ESP_LOG_INFO);
}
