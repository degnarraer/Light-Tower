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
#include "WebServer.h"
#include "SPIFFS.h"

TaskHandle_t SPI_RX_Task;
uint32_t SPI_RX_TaskLoopCount = 0;

TaskHandle_t Manager_Task;
uint32_t Manager_TaskLoopCount = 0;

// Create AsyncWebServer object on port 80
AsyncWebServer MyWebServer(80);

// Create WebSocket
AsyncWebSocket MyWebSocket("/ws");

// Create Settings Web Server that uses the Socket 
SettingsWebServerManager m_SettingsWebServerManager( "My Settings Web Server Manager", MyWebSocket );

// Create SPI Datalink to Get Data from CPU2
SPIDataLinkSlave m_SPIDataLinkSlave = SPIDataLinkSlave();

// Create Manager to Move Data Around
Manager m_Manager = Manager( "Manager", m_SPIDataLinkSlave );

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
  MyWebServer.serveStatic("/", SPIFFS, "/");
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
  }
  else
  {
    Serial.println("An error has occurred while mounting SPIFFS");
  }
}

// Init Tasks to run using FreeRTOS
void InitTasks()
{
  xTaskCreatePinnedToCore( SPI_RX_TaskLoop, "SPI_RX_Task",  3000,  NULL,  0,  &SPI_RX_Task, 0 );
  xTaskCreatePinnedToCore( Manager_TaskLoop, "Manager_Task",  3000,  NULL,  0,  &Manager_Task, 0 );
}

void InitLocalVariables()
{
  m_SPIDataLinkSlave.SetupSPIDataLink();
  m_SPIDataLinkSlave.SetSpewToConsole(true);
  m_SettingsWebServerManager.SetupSettingsWebServerManager();
}

void setup(){
  Serial.begin(500000);
  InitFileSystem();
  InitLocalVariables();
  InitWebSocket();
  InitWebServer();
  InitTasks();
  StartWebServer();
}

void loop()
{
   MyWebSocket.cleanupClients();
}

void SPI_RX_TaskLoop(void * parameter)
{
  while(true)
  {
    yield();
    ++SPI_RX_TaskLoopCount;
    m_SPIDataLinkSlave.ProcessEventQueue();
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }  
}

void Manager_TaskLoop(void * parameter)
{
  while(true)
  {
    yield();
    ++Manager_TaskLoopCount;
    m_Manager.ProcessEventQueue();
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }  
}
