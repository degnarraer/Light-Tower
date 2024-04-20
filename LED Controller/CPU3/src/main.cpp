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

#include "Tunes.h"
#include "SettingsWebServer.h"
#include "SPIFFS.h"

DataSerializer m_DataSerializer;  
SerialPortMessageManager m_CPU1SerialPortMessageManager = SerialPortMessageManager("CPU1", Serial1, m_DataSerializer);
SerialPortMessageManager m_CPU2SerialPortMessageManager = SerialPortMessageManager("CPU2", Serial2, m_DataSerializer);


// Create AsyncWebServer object on port 80
AsyncWebServer MyWebServer(80);

// Create WebSocket
AsyncWebSocket MyWebSocket("/ws");

// Create Settings Web Server that uses the Socket 
SettingsWebServerManager m_SettingsWebServerManager( "My Settings Web Server Manager"
                                                   , MyWebSocket
                                                   , m_CPU1SerialPortMessageManager
                                                   , m_CPU2SerialPortMessageManager );

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
    ESP_LOGI("Settings_Web_Server", "SPIFFS mounted successfully");
    File root = SPIFFS.open("/");
    File file = root.openNextFile();
    while(file)
    {
      ESP_LOGD("Settings_Web_Server", "FILE: %s", file.name());
      file = root.openNextFile();
    }
  }
  else
  {
    ESP_LOGE("Settings_Web_Server", "An error has occurred while mounting SPIFFS");
  }
}

void SetupSerialPorts()
{
  Serial.flush();
  Serial.begin(500000, SERIAL_8N1);
  Serial1.flush();
  Serial1.begin(500000, SERIAL_8O2, CPU1_RX, CPU1_TX);
  Serial1.setRxBufferSize(4096);
  Serial2.flush();
  Serial2.begin(500000, SERIAL_8O2, CPU2_RX, CPU2_TX);
  Serial2.setRxBufferSize(4096);
}

void InitLocalVariables()
{
  m_SettingsWebServerManager.SetupSettingsWebServerManager();
  m_CPU1SerialPortMessageManager.SetupSerialPortMessageManager();
  m_CPU2SerialPortMessageManager.SetupSerialPortMessageManager();
  m_SettingsWebServerManager.BeginWebServer();
}

void PrintMemory()
{
  ESP_LOGI("Settings_Web_Server", "Total heap: %d", ESP.getHeapSize());
  ESP_LOGI("Settings_Web_Server", "Free heap: %d", ESP.getFreeHeap());
  ESP_LOGI("Settings_Web_Server", "Total PSRAM: %d", ESP.getPsramSize());
  ESP_LOGI("Settings_Web_Server", "Free PSRAM: %d", ESP.getFreePsram());
}

void setup()
{
  SetupSerialPorts();
  InitLocalVariables();
  InitFileSystem();
  InitWebServer();
  InitWebSocket();
  StartWebServer();
  PrintMemory();
}

void loop()
{
}
