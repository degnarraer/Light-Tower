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

#include "Streaming.h"
#include "SerialDataLinkConfig.h"
#include "Tunes.h"
#include "Manager.h"
#include "SettingsWebServer.h"
#include "SPIFFS.h"
#include "HardwareSerial.h"
#include "DataItem.h"

#define TASK_LOOP_COUNT_DEBUG false
#define TASK_STACK_SIZE_DEBUG false
unsigned long LoopCountTimer = 0;
TaskHandle_t Manager_Task;
uint32_t Manager_TaskLoopCount = 0;

TaskHandle_t SPI_RX_Task;
uint32_t SPI_RX_TaskLoopCount = 0;

TaskHandle_t WebServer_Task;
uint32_t WebServer_TaskLoopCount = 0;

TaskHandle_t TaskMonitor_Task;
uint32_t TaskMonitor_TaskLoopCount = 0;

// Create AsyncWebServer object on port 80
AsyncWebServer MyWebServer(80);

// Create WebSocket
AsyncWebSocket MyWebSocket("/ws");

// Create SPI Datalink to Get Data from CPU2
SPIDataLinkSlave m_SPIDataLinkSlave = SPIDataLinkSlave();

// Create Settings Web Server that uses the Socket 
SettingsWebServerManager m_SettingsWebServerManager( "My Settings Web Server Manager", MyWebSocket );

// Create Manager to Move Data Around
Manager m_Manager = Manager( "Manager", m_SPIDataLinkSlave, m_SettingsWebServerManager );

SerialPortMessageManager CPU1SerialPortMessageManager = SerialPortMessageManager("CPU1", Serial1);
SerialPortMessageManager CPU2SerialPortMessageManager = SerialPortMessageManager("CPU2", Serial2);
DataSerializer m_DataSerializer;
DataItem <int16_t>intItem1 = DataItem<int16_t>("Name1", 0, TXType_PERIODIC, 1000, m_DataSerializer, CPU1SerialPortMessageManager);
DataItem <int16_t>intItem2 = DataItem<int16_t>("Name2", 0, TXType_PERIODIC, 1000, m_DataSerializer, CPU1SerialPortMessageManager);
DataItem <int16_t>intItem3 = DataItem<int16_t>("Name3", 0, TXType_PERIODIC, 1000, m_DataSerializer, CPU1SerialPortMessageManager);
DataItem <int16_t>intItem4 = DataItem<int16_t>("Name4", 0, TXType_PERIODIC, 1000, m_DataSerializer, CPU1SerialPortMessageManager);
DataItem <int16_t>intItem5 = DataItem<int16_t>("Name5", 0, TXType_PERIODIC, 1000, m_DataSerializer, CPU1SerialPortMessageManager);
DataItem <int16_t>intItem6 = DataItem<int16_t>("Name6", 0, TXType_ON_UPDATE, 1000, m_DataSerializer, CPU1SerialPortMessageManager);
DataItem <int16_t>intItem7 = DataItem<int16_t>("Name7", 0, TXType_ON_UPDATE, 1000, m_DataSerializer, CPU1SerialPortMessageManager);
DataItem <int16_t>intItem8 = DataItem<int16_t>("Name8", 0, TXType_ON_UPDATE, 1000, m_DataSerializer, CPU1SerialPortMessageManager);
DataItem <int16_t>intItem9 = DataItem<int16_t>("Name9", 0, TXType_ON_UPDATE, 1000, m_DataSerializer, CPU1SerialPortMessageManager);
DataItem <int16_t>intItem10 = DataItem<int16_t>("Name10", 0, TXType_ON_UPDATE, 1000, m_DataSerializer, CPU1SerialPortMessageManager);
DataItem <int16_t>intItem11 = DataItem<int16_t>("Name11", 0, TXType_ON_UPDATE, 1000, m_DataSerializer, CPU1SerialPortMessageManager);
DataItem <int16_t>intItem12 = DataItem<int16_t>("Name12", 0, TXType_ON_UPDATE, 1000, m_DataSerializer, CPU1SerialPortMessageManager);

// Static Callback for Web Socket
void OnEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
  m_SettingsWebServerManager.OnEvent(server, client, type, arg, data, len);
}

// Web Socket init to register web socket callback and connect it to the web server
void InitWebSocket()
{
  MyWebSocket.onEvent(OnEvent);
  MyWebServer.addHandler(&MyWebSocket);
}

// Init the web server to use the local SPIFFS memory and serve up index.html file.
void InitWebServer()
{
  // Web Server Root URL
  MyWebServer.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send(SPIFFS, "/index.html", "text/html");
  });
  MyWebServer.serveStatic("/", SPIFFS, "/").setCacheControl("max-age = 300");
}

// Start the web server
void StartWebServer()
{
  // Start server
  MyWebServer.begin();
}

// Initialize SPIFFS
void InitFileSystem()
{
  if (SPIFFS.begin())
  {
    Serial.println("SPIFFS mounted successfully");
    File root = SPIFFS.open("/");
    File file = root.openNextFile();
    while(file)
    {
        Serial.print("FILE: ");
        Serial.println(file.name());
        file = root.openNextFile();
    }
  }
  else
  {
    Serial.println("An error has occurred while mounting SPIFFS");
  }
}

// Init Tasks to run using FreeRTOS
void InitTasks()
{
  //xTaskCreatePinnedToCore( WebServer_TaskLoop,  "WebServer_Task",   10000,  NULL,  configMAX_PRIORITIES - 1,    &WebServer_Task,    0 );
  //xTaskCreatePinnedToCore( Manager_TaskLoop,    "Manager_Task",     10000,  NULL,  configMAX_PRIORITIES - 1,    &Manager_Task,      0 );
  //xTaskCreatePinnedToCore( SPI_RX_TaskLoop,     "SPI_RX_Task",      10000,  NULL,  configMAX_PRIORITIES - 1,    &SPI_RX_Task,       0 );
  //xTaskCreatePinnedToCore( TaskMonitorTaskLoop, "TaskMonitor_Task", 5000,   NULL,  configMAX_PRIORITIES - 4,    &TaskMonitor_Task,  0 );
}

void InitLocalVariables()
{
  m_Manager.Setup();
  m_SPIDataLinkSlave.SetupSPIDataLink();
  m_SettingsWebServerManager.SetupSettingsWebServerManager();
}

void setup(){
  Serial.begin(500000);
  Serial1.begin(500000, SERIAL_8N1, CPU1_RX, CPU1_TX);
  Serial2.begin(500000, SERIAL_8N1, CPU2_RX, CPU2_TX);
  CPU1SerialPortMessageManager.SetupSerialPortMessageManager();
  CPU2SerialPortMessageManager.SetupSerialPortMessageManager();
  intItem1.EnableDatalinkCommunication(true);
  delay(100);
  intItem2.EnableDatalinkCommunication(true);
  delay(100);
  intItem3.EnableDatalinkCommunication(true);
  delay(100);
  intItem4.EnableDatalinkCommunication(true);
  delay(100);
  intItem5.EnableDatalinkCommunication(true);
  delay(100);
  intItem6.EnableDatalinkCommunication(true);
  delay(100);
  intItem7.EnableDatalinkCommunication(true);
  delay(100);
  intItem8.EnableDatalinkCommunication(true);
  delay(100);
  intItem9.EnableDatalinkCommunication(true);
  delay(100);
  intItem10.EnableDatalinkCommunication(true);
  delay(100);
  intItem11.EnableDatalinkCommunication(true);
  delay(100);
  intItem12.EnableDatalinkCommunication(true);
  delay(100);
  //InitLocalVariables();
  //InitFileSystem();
  //InitWebServer();
  //InitWebSocket();
  InitTasks();
  //StartWebServer();
  //PrintMemory();
}

void loop()
{
}
#pragma GCC diagnostic pop

void PrintMemory()
{
  ESP_LOGE("Settings_Web_Server", "Total heap: %d", ESP.getHeapSize());
  ESP_LOGE("Settings_Web_Server", "Free heap: %d", ESP.getFreeHeap());
  ESP_LOGE("Settings_Web_Server", "Total PSRAM: %d", ESP.getPsramSize());
  ESP_LOGE("Settings_Web_Server", "Free PSRAM: %d", ESP.getFreePsram());
}

void SPI_RX_TaskLoop(void * parameter)
{
  const TickType_t xFrequency = 20;
  TickType_t xLastWakeTime = xTaskGetTickCount();
  while(true)
  {
    vTaskDelayUntil( &xLastWakeTime, xFrequency );
    ++SPI_RX_TaskLoopCount;
    m_SPIDataLinkSlave.ProcessEventQueue();
  }  
}

void Manager_TaskLoop(void * parameter)
{
  const TickType_t xFrequency = 20;
  TickType_t xLastWakeTime = xTaskGetTickCount();
  while(true)
  {
    vTaskDelayUntil( &xLastWakeTime, xFrequency );
    ++Manager_TaskLoopCount;
    m_Manager.ProcessEventQueue();
  }
}

void WebServer_TaskLoop(void * parameter)
{
  const TickType_t xFrequency = 20;
  TickType_t xLastWakeTime = xTaskGetTickCount();
  while(true)
  {
    vTaskDelayUntil( &xLastWakeTime, xFrequency );
    ++WebServer_TaskLoopCount;
    m_SettingsWebServerManager.ProcessEventQueue();
  }  
}


void TaskMonitorTaskLoop(void * parameter)
{
  const TickType_t xFrequency = 5000;
  TickType_t xLastWakeTime = xTaskGetTickCount();
  while(true)
  {
    vTaskDelayUntil( &xLastWakeTime, xFrequency );
    unsigned long CurrentTime = millis();
    ++TaskMonitor_TaskLoopCount;
    if(true == TASK_LOOP_COUNT_DEBUG)
    {
      unsigned long DeltaTimeSeconds = (CurrentTime - LoopCountTimer) / 1000;
      ESP_LOGE("Settings_Web_Server", "Manager_TaskLoopCount: %f", (float)Manager_TaskLoopCount/(float)DeltaTimeSeconds);
      ESP_LOGE("Settings_Web_Server", "SPI_RX_TaskLoopCount: %f", (float)SPI_RX_TaskLoopCount/(float)DeltaTimeSeconds);
      ESP_LOGE("Settings_Web_Server", "WebServer_TaskLoopCount: %f", (float)WebServer_TaskLoopCount/(float)DeltaTimeSeconds);
      ESP_LOGE("Settings_Web_Server", "TaskMonitor_TaskLoopCount: %f", (float)TaskMonitor_TaskLoopCount/(float)DeltaTimeSeconds);
      
      Manager_TaskLoopCount = 0;
      SPI_RX_TaskLoopCount = 0;
      WebServer_TaskLoopCount = 0;
      TaskMonitor_TaskLoopCount = 0;
    }

    size_t StackSizeThreshold = 100;
    if( uxTaskGetStackHighWaterMark(Manager_Task) < StackSizeThreshold )ESP_LOGW("LED_Controller2", "WARNING! Manager_Task: Stack Size Low");
    
    if(true == TASK_STACK_SIZE_DEBUG)
    {
      ESP_LOGE("Settings_Web_Server", "Manager_Task Free Heap: %i", uxTaskGetStackHighWaterMark(Manager_Task));
      ESP_LOGE("Settings_Web_Server", "SPI_RX_Task Free Heap: %i", uxTaskGetStackHighWaterMark(SPI_RX_Task));
      ESP_LOGE("Settings_Web_Server", "SPI_RX_Task Free Heap: %i", uxTaskGetStackHighWaterMark(WebServer_Task));
      ESP_LOGE("Settings_Web_Server", "TaskMonitor_Task Free Heap: %i", uxTaskGetStackHighWaterMark(TaskMonitor_Task));
    }
    LoopCountTimer = CurrentTime;
  }
}
