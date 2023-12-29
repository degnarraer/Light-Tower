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

TaskHandle_t Manager_20mS_Task;

TaskHandle_t Manager_1000mS_Task;

TaskHandle_t Manager_300000mS_Task;

TaskHandle_t VisualizationTask;

TaskHandle_t TaskMonitorTask;


void InitTasks()
{
  xTaskCreatePinnedToCore( Manager_20mS_TaskLoop,     "Manager_20mS_Task",      5000,  NULL,   configMAX_PRIORITIES - 1,  &Manager_20mS_Task,     0 );
  xTaskCreatePinnedToCore( Manager_1000mS_TaskLoop,   "Manager_1000mS_rTask",   5000,  NULL,   configMAX_PRIORITIES - 3,  &Manager_1000mS_Task,   0 );
  xTaskCreatePinnedToCore( Manager_300000mS_TaskLoop, "Manager_300000mS_Task",  5000,  NULL,   configMAX_PRIORITIES - 3,  &Manager_300000mS_Task, 0 );
  xTaskCreatePinnedToCore( VisualizationTaskLoop,     "VisualizationTask",      5000,  NULL,   configMAX_PRIORITIES - 1,  &VisualizationTask,     1 );
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
  const TickType_t xFrequency = 10;
  TickType_t xLastWakeTime = xTaskGetTickCount();
  while(true)
  {
    vTaskDelayUntil( &xLastWakeTime, xFrequency );
    m_Scheduler.RunScheduler();
  }
}

void Manager_20mS_TaskLoop(void * parameter)
{
  const TickType_t xFrequency = 10;
  TickType_t xLastWakeTime = xTaskGetTickCount();
  while(true)
  {
    vTaskDelayUntil( &xLastWakeTime, xFrequency );
    m_Manager.ProcessEventQueue20mS();
  }
}

void Manager_1000mS_TaskLoop(void * parameter)
{
  const TickType_t xFrequency = 1000;
  TickType_t xLastWakeTime = xTaskGetTickCount();
  while(true)
  {
    vTaskDelayUntil( &xLastWakeTime, xFrequency );
    m_Manager.ProcessEventQueue1000mS();
  }
}

void Manager_300000mS_TaskLoop(void * parameter)
{
  const TickType_t xFrequency = 300000;
  TickType_t xLastWakeTime = xTaskGetTickCount();
  while(true)
  {
    vTaskDelayUntil( &xLastWakeTime, xFrequency );
    m_Manager.ProcessEventQueue300000mS();
  }
}
