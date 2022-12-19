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

#include "Manager.h"
#include "Tunes.h"
#include "VisualizationPlayer.h"
#include "Models.h"
#include "Tunes.h"
#include "esp_log.h"
#include "LED_Controller_Helpers.h"

unsigned long LoopCountTimer = 0;
TaskHandle_t DataMoverTask;
uint32_t DataMoveTaskLoopCount = 0;

TaskHandle_t VisualizationTask;
uint32_t VisualizationTaskLoopCount = 0;

TaskHandle_t TaskMonitorTask;
uint32_t TaskMonitorTaskLoopCount = 0;

TaskHandle_t SPI_RX_Task;
uint32_t SPI_RX_TaskLoopCount = 0;

void InitTasks()
{
  xTaskCreatePinnedToCore( DataMoverTaskLoop,     "DataMoverTask",        2000,  NULL,   configMAX_PRIORITIES - 1,  &DataMoverTask,       0 );
  xTaskCreatePinnedToCore( TaskMonitorTaskLoop,   "TaskMonitorTaskTask",  2000,  NULL,   configMAX_PRIORITIES - 1,  &TaskMonitorTask,     0 );
  xTaskCreatePinnedToCore( SPI_RX_TaskLoop,       "SPI_RX_Task",          3000,  NULL,   0,                         &SPI_RX_Task,         0 );
  xTaskCreatePinnedToCore( VisualizationTaskLoop, "VisualizationTask",    4000,  NULL,   configMAX_PRIORITIES - 10, &VisualizationTask,   1 ); //This has to be core 1 for some reason else bluetooth interfeeres with LEDs and makes them flicker
}

void setup()
{
  InitSerialCommunication();
  PrintStartupData();
  InitLocalVariables();
  InitTasks();
  PrintFreeHeap();
}

void loop()
{
}

void VisualizationTaskLoop(void * parameter)
{
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xFrequency = 20; //delay for mS
  while(true)
  {
    ++VisualizationTaskLoopCount;
    m_Scheduler.RunScheduler();
    vTaskDelayUntil( &xLastWakeTime, xFrequency );
  }
}

void SPI_RX_TaskLoop(void * parameter)
{
  while(true)
  {
    yield();
    ++SPI_RX_TaskLoopCount;
    m_SPIDataLinkSlave.ProcessDataRXEventQueue();
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }  
}

void TaskMonitorTaskLoop(void * parameter)
{
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xFrequency = 5000; //delay for mS
  while(true)
  {
    unsigned long CurrentTime = millis();
    ++TaskMonitorTaskLoopCount;

    if(true == TASK_LOOP_COUNT_DEBUG)
    {
      unsigned long DeltaTimeSeconds = (CurrentTime - LoopCountTimer) / 1000;
      ESP_LOGE("LED_Controller1", "DataMoveTaskLoopCount: %f", (float)DataMoveTaskLoopCount/(float)DeltaTimeSeconds);
      ESP_LOGE("LED_Controller1", "VisualizationTaskLoopCount: %f", (float)VisualizationTaskLoopCount/(float)DeltaTimeSeconds);
      ESP_LOGE("LED_Controller1", "TaskMonitorTaskLoopCount: %f", (float)TaskMonitorTaskLoopCount/(float)DeltaTimeSeconds);
      ESP_LOGE("LED_Controller1", "SPI_RX_TaskLoopCount: %f", (float)SPI_RX_TaskLoopCount/(float)DeltaTimeSeconds);
      DataMoveTaskLoopCount = 0;
      VisualizationTaskLoopCount = 0;
      TaskMonitorTaskLoopCount = 0;
      SPI_RX_TaskLoopCount = 0;
    }

    size_t StackSizeThreshold = 100;
    if( uxTaskGetStackHighWaterMark(DataMoverTask) < StackSizeThreshold )ESP_LOGW("LED_Controller1", "WARNING! DataMoverTask: Stack Size Low");
    if( uxTaskGetStackHighWaterMark(VisualizationTask) < StackSizeThreshold )ESP_LOGW("LED_Controller1", "WARNING! VisualizationTask: Stack Size Low");
    
    if(true == TASK_STACK_SIZE_DEBUG)
    {
      ESP_LOGE("LED_Controller1", "TaskMonitorTaskTask Free Heap: %i", uxTaskGetStackHighWaterMark(TaskMonitorTask));
      ESP_LOGE("LED_Controller1", "DataMoverTaskTask Free Heap: %i", uxTaskGetStackHighWaterMark(DataMoverTask));
      ESP_LOGE("LED_Controller1", "SPI_RX_Task Free Heap: %i", uxTaskGetStackHighWaterMark(SPI_RX_Task));
      ESP_LOGE("LED_Controller1", "VisualizationTask Free Heap: %i", uxTaskGetStackHighWaterMark(VisualizationTask));
    }
    LoopCountTimer = CurrentTime;
    vTaskDelayUntil( &xLastWakeTime, xFrequency );
  }
}

void DataMoverTaskLoop(void * parameter)
{
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xFrequency = 20; //delay for mS
  while(true)
  {
    ++DataMoveTaskLoopCount;
    m_Manager.ProcessEventQueue();
    vTaskDelayUntil( &xLastWakeTime, xFrequency );
  }
}
