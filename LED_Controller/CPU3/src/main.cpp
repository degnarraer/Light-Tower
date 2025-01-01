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
SerialPortMessageManager m_CPU1SerialPortMessageManager = SerialPortMessageManager("CPU1 Datalink Task", &Serial1, &m_DataSerializer, tskNO_AFFINITY, THREAD_PRIORITY_HIGH);
SerialPortMessageManager m_CPU2SerialPortMessageManager = SerialPortMessageManager("CPU2 Datalink Task", &Serial2, &m_DataSerializer, tskNO_AFFINITY, THREAD_PRIORITY_HIGH);

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
  Serial.flush();
  Serial1.setRxBufferSize(SERIAL_RX_BUFFER_SIZE);
  Serial1.begin(500000, SERIAL_8O2, CPU1_RX, CPU1_TX);
  Serial1.flush();
  Serial2.setRxBufferSize(SERIAL_RX_BUFFER_SIZE);
  Serial2.begin(500000, SERIAL_8O2, CPU2_RX, CPU2_TX);
  Serial2.flush();
  ESP_LOGI("SetupSerialPorts", "Serial Ports Setup");
}


void TestPSRam()
{
    const size_t theSize = 100;
    size_t freeSizeBefore = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    void* buffer1 = heap_caps_malloc(theSize, MALLOC_CAP_SPIRAM);
    assert(buffer1);
    size_t freeSizeAfter = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    size_t allocatedSize = freeSizeBefore - freeSizeAfter;
    ESP_LOGI("TestPSRam", "Requested: %zu bytes, Allocated (including overhead): %zu bytes", theSize, allocatedSize);
    assert(allocatedSize >= theSize);
    free(buffer1);
    buffer1 = nullptr;
    size_t freeSizeAfterFree = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    ESP_LOGI("TestPSRam", "Free size before: %zu, after: %zu, after free: %zu", freeSizeBefore, freeSizeAfter, freeSizeAfterFree);
    assert(freeSizeAfterFree == freeSizeBefore);
    
    freeSizeBefore = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    void* buffer2 = ps_malloc(theSize);
    assert(buffer2);
    freeSizeAfter = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    allocatedSize = freeSizeBefore - freeSizeAfter;
    ESP_LOGI("TestPSRam", "ps_malloc Requested: %zu bytes, Allocated (including overhead): %zu bytes", theSize, allocatedSize);
    assert(allocatedSize >= theSize);
    free(buffer2);
    buffer2 = nullptr;
    freeSizeAfterFree = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    ESP_LOGI("TestPSRam", "Free size before: %zu, after: %zu, after free: %zu", freeSizeBefore, freeSizeAfter, freeSizeAfterFree);
    assert(freeSizeAfterFree == freeSizeBefore);
}

void InitLocalVariables()
{
  m_PreferencesWrapper.Setup();
  m_CPU1SerialPortMessageManager.Setup();
  m_CPU2SerialPortMessageManager.Setup();
  m_SettingsWebServerManager.Setup();
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
  static unsigned long lastPrintTime = 0;
  unsigned long currentTime = millis();

  MyWebServer.handleClient();
  MyWebSocket.loop();

  if (currentTime - lastPrintTime >= 1000) {
    size_t heapSpace = ESP.getFreeHeap();
    size_t psramSpace = ESP.getFreePsram();
    Serial.printf("Heap Space Left: %u bytes, PSRAM Space Left: %u bytes\n", heapSpace, psramSpace);

    lastPrintTime = currentTime;
  }
}
