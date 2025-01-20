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
#include "LED_Controller_Helpers.h"

TaskHandle_t VisualizationTask;

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

void InitTasks()
{
  xTaskCreate( VisualizationTaskLoop, "VisualizationTask", 5000, NULL, THREAD_PRIORITY_MEDIUM, &VisualizationTask );
}

void setup()
{
  InitSerialCommunication();
  TestPSRam();
  PrintFreeHeap();
  PrintStartupData();
  InitLocalVariables();
  InitTasks();
  PrintFreeHeap();
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

