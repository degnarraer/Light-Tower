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

#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include "SPIFFS.h"
#include "Streaming.h"
#include "SerialDataLinkConfig.h"
#include <Arduino_JSON.h>
#include "Tunes.h"

TaskHandle_t SPI_RX_Task;
uint32_t SPI_RX_TaskLoopCount = 0;


SPIDataLinkSlave m_SPIDataLinkSlave = SPIDataLinkSlave( "SPI Datalink"
                                                      , SPI1_PIN_SCK
                                                      , SPI1_PIN_MISO
                                                      , SPI1_PIN_MOSI
                                                      , SPI1_PIN_SS
                                                      , 2 );

 
// Replace with your network credentials
const char* ssid = "LED Tower of Power";
const char* password = "LEDs Rock";
 
// Set LED GPIO
const int ledPin = 2;
// Stores LED state
String ledState;

String message = "";
String sliderValue1 = "0";
String sliderValue2 = "0";
String sliderValue3 = "0";

//Json Variable to Hold Slider Values
JSONVar sliderValues;

int dutyCycle1;
int dutyCycle2;
int dutyCycle3;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");



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

// Initialize WiFi Client
void InitWiFiClient()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
}

void InitWiFiAP()
{
    // Setup ESP32 as Access Point
  IPAddress Ip(192, 168, 0, 1);
  IPAddress NMask(255, 255, 255, 0);
  
  WiFi.softAPConfig(Ip, Ip, NMask);
  WiFi.softAP(ssid, password);
  Serial.println(WiFi.softAPIP());  //Show ESP32 IP on serial
}

//Get Slider Values
String GetSliderValues()
{
  sliderValues["sliderValue1"] = String(sliderValue1);
  sliderValues["sliderValue2"] = String(sliderValue2);
  sliderValues["sliderValue3"] = String(sliderValue3);

  String jsonString = JSON.stringify(sliderValues);
  return jsonString;
}

void NotifyClients(String sliderValues)
{
  ws.textAll(sliderValues);
}

void HandleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    message = (char*)data;
    if (message.indexOf("1s") >= 0) {
      sliderValue1 = message.substring(2);
      dutyCycle1 = map(sliderValue1.toInt(), 0, 100, 0, 255);
      Serial.println(dutyCycle1);
      Serial.print(GetSliderValues());
      NotifyClients(GetSliderValues());
    }
    if (message.indexOf("2s") >= 0) {
      sliderValue2 = message.substring(2);
      dutyCycle2 = map(sliderValue2.toInt(), 0, 100, 0, 255);
      Serial.println(dutyCycle2);
      Serial.print(GetSliderValues());
      NotifyClients(GetSliderValues());
    }    
    if (message.indexOf("3s") >= 0) {
      sliderValue3 = message.substring(2);
      dutyCycle3 = map(sliderValue3.toInt(), 0, 100, 0, 255);
      Serial.println(dutyCycle3);
      Serial.print(GetSliderValues());
      NotifyClients(GetSliderValues());
    }
    if (strcmp((char*)data, "getValues") == 0) {
      NotifyClients(GetSliderValues());
    }
  }
}

void OnEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      HandleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

void InitWebSocket()
{
  ws.onEvent(OnEvent);
  server.addHandler(&ws);
}

void InitWebServer()
{
 // Web Server Root URL
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", "text/html");
  });
  
  server.serveStatic("/", SPIFFS, "/");

  // Start server
  server.begin();
}

void InitTasks()
{
  xTaskCreatePinnedToCore( SPI_RX_TaskLoop, "SPI_RX_Task",  3000,  NULL,  0,  &SPI_RX_Task, 0 );
}

void InitLocalVariables()
{
  m_SPIDataLinkSlave.SetupSPIDataLink(); 
}

void setup(){
  // Serial port for debugging purposes
  Serial.begin(500000);
  pinMode(ledPin, OUTPUT);

  InitLocalVariables();
  InitFileSystem();
  InitWiFiAP();
  InitWebSocket();
  InitWebServer();
  InitTasks();
}

void loop()
{
   
}

void SPI_RX_TaskLoop(void * parameter)
{
  while(true)
  {
    yield();
    ++SPI_RX_TaskLoopCount;
    m_SPIDataLinkSlave.ProcessDataRXEventQueue();
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }  
}
