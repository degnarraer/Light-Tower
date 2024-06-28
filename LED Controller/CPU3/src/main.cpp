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
#define SERIAL_RX_BUFFER_SIZE 2048

Preferences m_Preferences;
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
                                                   , m_Preferences
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
  ClearSerialBuffers(Serial);
  ClearSerialBuffers(Serial1);
  ClearSerialBuffers(Serial2);
  delay(500);
  Serial.begin(500000, SERIAL_8O2);
  Serial1.begin(500000, SERIAL_8O2, CPU1_RX, CPU1_TX);
  Serial2.begin(500000, SERIAL_8O2, CPU2_RX, CPU2_TX);
  Serial1.setRxBufferSize(SERIAL_RX_BUFFER_SIZE);
  Serial2.setRxBufferSize(SERIAL_RX_BUFFER_SIZE);
}


void InitLocalVariables()
{
  m_CPU1SerialPortMessageManager.SetupSerialPortMessageManager();
  m_CPU2SerialPortMessageManager.SetupSerialPortMessageManager();
  m_SettingsWebServerManager.SetupSettingsWebServerManager();
  InitWebSocket();
  m_SettingsWebServerManager.SetupWifi();
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
}

void loop()
{
}
