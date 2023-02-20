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
TaskHandle_t Manager_20mS_Task;
uint32_t Manager_20mS_TaskLoopCount = 0;

TaskHandle_t Manager_1000mS_Task;
uint32_t Manager_1000mS_TaskLoopCount = 0;

TaskHandle_t VisualizationTask;
uint32_t VisualizationTaskLoopCount = 0;

TaskHandle_t TaskMonitorTask;
uint32_t TaskMonitorTaskLoopCount = 0;

TaskHandle_t SPI_Task;
uint32_t SPI_TaskLoopCount = 0;


void InitTasks()
{
  xTaskCreatePinnedToCore( Manager_20mS_TaskLoop,     "Manager_20mS_Task",    2000,  NULL,   configMAX_PRIORITIES - 1,  &Manager_20mS_Task,     0 );
  xTaskCreatePinnedToCore( Manager_1000mS_TaskLoop,   "Manager_1000mS_rTask", 2000,  NULL,   configMAX_PRIORITIES - 2,  &Manager_1000mS_Task,   0 );
  xTaskCreatePinnedToCore( SPI_TaskLoop,              "SPI_Task",             3000,  NULL,   configMAX_PRIORITIES - 2,  &SPI_Task,              0 );
  xTaskCreatePinnedToCore( TaskMonitorTaskLoop,       "TaskMonitorTaskTask",  2000,  NULL,   tskIDLE_PRIORITY,          &TaskMonitorTask,       0 );
  xTaskCreatePinnedToCore( VisualizationTaskLoop,     "VisualizationTask",    4000,  NULL,   configMAX_PRIORITIES - 1,  &VisualizationTask,     1 ); //This has to be core 1 for some reason else bluetooth interfeeres with LEDs and makes them flicker
}

void setup()
{
  InitSerialCommunication();
  PrintStartupData();
  InitLocalVariables();
  InitTasks();
  PrintFreeHeap();
  m_SPIDataLinkSlave.SetSpewToConsole(false, false);
}

void loop()
{
}

void VisualizationTaskLoop(void * parameter)
{
  //20 mS task rate
  const TickType_t xFrequency = 20;
  TickType_t xLastWakeTime = xTaskGetTickCount();
  while(true)
  {
    vTaskDelayUntil( &xLastWakeTime, xFrequency );
    ++VisualizationTaskLoopCount;
    m_Scheduler.RunScheduler();
  }
}

void SPI_TaskLoop(void * parameter)
{
  //20 mS task rate
  const TickType_t xFrequency = 20;
  TickType_t xLastWakeTime = xTaskGetTickCount();
  while(true)
  {
    vTaskDelayUntil( &xLastWakeTime, xFrequency );
    ++SPI_TaskLoopCount;
    m_SPIDataLinkSlave.ProcessEventQueue();
  }  
}

void TaskMonitorTaskLoop(void * parameter)
{
  //5000 mS task rate
  const TickType_t xFrequency = 5000;
  TickType_t xLastWakeTime = xTaskGetTickCount();
  while(true)
  {
    vTaskDelayUntil( &xLastWakeTime, xFrequency );
    unsigned long CurrentTime = millis();
    ++TaskMonitorTaskLoopCount;

    if(true == TASK_LOOP_COUNT_DEBUG)
    {
      unsigned long DeltaTimeSeconds = (CurrentTime - LoopCountTimer) / 1000;
      ESP_LOGE("LED_Controller1", "Manager_20mS_TaskLoopCount: %f", (float)Manager_20mS_TaskLoopCount/(float)DeltaTimeSeconds);
      ESP_LOGE("LED_Controller1", "Manager_1000mS_TaskLoopCount: %f", (float)Manager_1000mS_TaskLoopCount/(float)DeltaTimeSeconds);
      ESP_LOGE("LED_Controller1", "VisualizationTaskLoopCount: %f", (float)VisualizationTaskLoopCount/(float)DeltaTimeSeconds);
      ESP_LOGE("LED_Controller1", "TaskMonitorTaskLoopCount: %f", (float)TaskMonitorTaskLoopCount/(float)DeltaTimeSeconds);
      ESP_LOGE("LED_Controller1", "SPI_TaskLoopCount: %f", (float)SPI_TaskLoopCount/(float)DeltaTimeSeconds);
      Manager_20mS_TaskLoopCount = 0;
      VisualizationTaskLoopCount = 0;
      TaskMonitorTaskLoopCount = 0;
      SPI_TaskLoopCount = 0;
    }

    size_t StackSizeThreshold = 100;
    if( uxTaskGetStackHighWaterMark(Manager_20mS_Task) < StackSizeThreshold )ESP_LOGW("LED_Controller1", "WARNING! Manager_20mS_Task: Stack Size Low");
    if( uxTaskGetStackHighWaterMark(Manager_1000mS_Task) < StackSizeThreshold )ESP_LOGW("LED_Controller1", "WARNING! Manager_1000mS_Task: Stack Size Low");
    if( uxTaskGetStackHighWaterMark(VisualizationTask) < StackSizeThreshold )ESP_LOGW("LED_Controller1", "WARNING! VisualizationTask: Stack Size Low");
    if( uxTaskGetStackHighWaterMark(VisualizationTask) < StackSizeThreshold )ESP_LOGW("LED_Controller1", "WARNING! TaskMonitorTask: Stack Size Low");
    if( uxTaskGetStackHighWaterMark(VisualizationTask) < StackSizeThreshold )ESP_LOGW("LED_Controller1", "WARNING! SPI_Task: Stack Size Low");
    
    if(true == TASK_STACK_SIZE_DEBUG)
    {
      ESP_LOGE("LED_Controller1", "TaskMonitorTask Free Heap: %i", uxTaskGetStackHighWaterMark(TaskMonitorTask));
      ESP_LOGE("LED_Controller1", "Manager_20mS_Task Free Heap: %i", uxTaskGetStackHighWaterMark(Manager_20mS_Task));
      ESP_LOGE("LED_Controller1", "Manager_1000mS_Task Free Heap: %i", uxTaskGetStackHighWaterMark(Manager_1000mS_Task));
      ESP_LOGE("LED_Controller1", "SPI_Task Free Heap: %i", uxTaskGetStackHighWaterMark(SPI_Task));
      ESP_LOGE("LED_Controller1", "VisualizationTask Free Heap: %i", uxTaskGetStackHighWaterMark(VisualizationTask));
    }
    LoopCountTimer = CurrentTime;
  }
}

void Manager_20mS_TaskLoop(void * parameter)
{
  //20 mS task rate
  const TickType_t xFrequency = 20;
  TickType_t xLastWakeTime = xTaskGetTickCount();
  while(true)
  {
    vTaskDelayUntil( &xLastWakeTime, xFrequency );
    ++Manager_20mS_TaskLoopCount;
    m_Manager.ProcessEventQueue20mS();
  }
}

void Manager_1000mS_TaskLoop(void * parameter)
{
  //1000 mS task rate
  const TickType_t xFrequency = 1000;
  TickType_t xLastWakeTime = xTaskGetTickCount();
  while(true)
  {
    vTaskDelayUntil( &xLastWakeTime, xFrequency );
    ++Manager_1000mS_TaskLoopCount;
    m_Manager.ProcessEventQueue1000mS();
  }
}
