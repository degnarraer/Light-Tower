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
PreferencesWrapper m_PreferencesWrapper = PreferencesWrapper(&m_Preferences);
DataSerializer m_DataSerializer;  
SerialPortMessageManager m_CPU1SerialPortMessageManager = SerialPortMessageManager("CPU1", &Serial1, &m_DataSerializer);
SerialPortMessageManager m_CPU2SerialPortMessageManager = SerialPortMessageManager("CPU2", &Serial2, &m_DataSerializer);

// Create AsyncWebServer object on port 80
AsyncWebServer MyWebServer(80);

// Create WebSocket
AsyncWebSocket MyWebSocket("/ws");

// Create Settings Web Server that uses the Socket 
SettingsWebServerManager m_SettingsWebServerManager( "My Settings Web Server Manager"
                                                   , MyWebSocket
                                                   , MyWebServer
                                                   , m_PreferencesWrapper
                                                   , m_CPU1SerialPortMessageManager
                                                   , m_CPU2SerialPortMessageManager );
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

void ClearSerialBuffers(HardwareSerial &serial)
{
    serial.flush();
    while (serial.available() > 0) {
        serial.read();
    }
}
void SetupSerialPorts()
{
  delay(500);
  Serial.begin(500000, SERIAL_8O2);
  ClearSerialBuffers(Serial);
  Serial1.setRxBufferSize(SERIAL_RX_BUFFER_SIZE);
  Serial1.begin(500000, SERIAL_8O2, CPU1_RX, CPU1_TX);
  ClearSerialBuffers(Serial1);
  Serial2.setRxBufferSize(SERIAL_RX_BUFFER_SIZE);
  Serial2.begin(500000, SERIAL_8O2, CPU2_RX, CPU2_TX);
  ClearSerialBuffers(Serial2);
}


void TestPSRam()
{
  uint32_t expectedAllocationSize = 4096000; //4MB of PSRam
  uint32_t allocationSize = ESP.getPsramSize() - ESP.getFreePsram();
  assert(0 == allocationSize && "psram allocation should be 0 at start");
  byte* psdRamBuffer = (byte*)ps_malloc(expectedAllocationSize);
  allocationSize = ESP.getPsramSize() - ESP.getFreePsram();
  assert(expectedAllocationSize == allocationSize && "Failed to allocated psram");
  free(psdRamBuffer);
  allocationSize = ESP.getPsramSize() - ESP.getFreePsram();
  assert(0 == allocationSize && "Failed to free allocated psram");
}

void InitLocalVariables()
{
  m_CPU1SerialPortMessageManager.SetupSerialPortMessageManager();
  m_CPU2SerialPortMessageManager.SetupSerialPortMessageManager();
  m_SettingsWebServerManager.SetupSettingsWebServerManager();
  InitWebSocket();
  m_SettingsWebServerManager.StartWiFi();
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
  PrintMemory();
  //TestPSRam();
}

void loop()
{
}
