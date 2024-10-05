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
#if __cplusplus < 201402L
#error "C++14 or later is required!"
#endif

#include "Tunes.h"
#include "SettingsWebServer.h"

#define SERIAL_RX_BUFFER_SIZE 2048

Preferences m_Preferences;
PreferencesWrapper m_PreferencesWrapper = PreferencesWrapper("Settings", &m_Preferences);
DataSerializer m_DataSerializer;  
SerialPortMessageManager m_CPU1SerialPortMessageManager = SerialPortMessageManager("CPU1", &Serial1, &m_DataSerializer);
SerialPortMessageManager m_CPU2SerialPortMessageManager = SerialPortMessageManager("CPU2", &Serial2, &m_DataSerializer);

// Create AsyncWebServer object on port 80
WebServer MyWebServer(80);

// Create WebSocket
WebSocketsServer MyWebSocket(81);

// Create Settings Web Server that uses the Socket 
SettingsWebServerManager m_SettingsWebServerManager( "My Settings Web Server Manager"
                                                   , MyWebSocket
                                                   , MyWebServer
                                                   , m_PreferencesWrapper
                                                   , m_CPU1SerialPortMessageManager
                                                   , m_CPU2SerialPortMessageManager );

void SetupSerialPorts()
{
  Serial.begin(115200, SERIAL_8O2);
  delay(500);
  Serial.flush();
  Serial1.setRxBufferSize(SERIAL_RX_BUFFER_SIZE);
  Serial1.begin(500000, SERIAL_8O2, CPU1_RX, CPU1_TX);
  delay(100);
  Serial1.flush();
  Serial2.setRxBufferSize(SERIAL_RX_BUFFER_SIZE);
  Serial2.begin(500000, SERIAL_8O2, CPU2_RX, CPU2_TX);
  delay(100);
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

void InitLocalVariables()
{
  m_CPU1SerialPortMessageManager.SetupSerialPortMessageManager();
  m_CPU2SerialPortMessageManager.SetupSerialPortMessageManager();
  m_SettingsWebServerManager.SetupSettingsWebServerManager();
}

void PrintMemory(const char* message)
{
  ESP_LOGI("PrintMemory", "%s", message);
  ESP_LOGI("PrintMemory", "Total heap: %d", ESP.getHeapSize());
  ESP_LOGI("PrintMemory", "Free heap: %d", ESP.getFreeHeap());
  ESP_LOGI("PrintMemory", "Total PSRAM: %d", ESP.getPsramSize());
  ESP_LOGI("PrintMemory", "Free PSRAM: %d", ESP.getFreePsram());
}

void setup()
{
  SetupSerialPorts();
  PrintMemory("Before Initialization");
  TestPSRam();
  InitLocalVariables();
  PrintMemory("After Initialization");
}

void loop()
{
  MyWebServer.handleClient();
  MyWebSocket.loop();
}
