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

TaskHandle_t VisualizationTask;


void InitTasks()
{
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
