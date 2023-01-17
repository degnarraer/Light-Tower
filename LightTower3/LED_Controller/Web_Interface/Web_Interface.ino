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

// Replace with your network credentials
const char* ssid = "LED Tower of Power";
const char* password = "LEDs Rock";

String message = "";
String Amplitude_Gain_Slider = "1.0";
String FFT_Gain_Slider = "1.0";
String Red_Value_Slider = "0";
String Blue_Value_Slider = "0";
String Green_Value_Slider = "0";

int Amplitude_Gain_Slider_DutyCycle;
int FFT_Gain_Slider_DutyCycle;
int Red_Value_Slider_DutyCycle;
int Blue_Value_Slider_DutyCycle;
int Green_Value_Slider_DutyCycle;



// Create AsyncWebServer object on port 80
AsyncWebServer MyWebServer(80);
AsyncWebSocket MyWebSocket("/ws");

SPIDataLinkSlave m_SPIDataLinkSlave = SPIDataLinkSlave();

Manager m_Manager = Manager( "Manager"
                           , m_SPIDataLinkSlave );
 

void OnEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
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
  MyWebSocket.onEvent(OnEvent);
  MyWebServer.addHandler(&MyWebSocket);
}

void InitWebServer()
{
  // Web Server Root URL
  MyWebServer.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send(SPIFFS, "/index.html", "text/html");
  });
  MyWebServer.serveStatic("/", SPIFFS, "/");
}

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

void InitTasks()
{
  xTaskCreatePinnedToCore( SPI_RX_TaskLoop, "SPI_RX_Task",  3000,  NULL,  0,  &SPI_RX_Task, 0 );
  xTaskCreatePinnedToCore( Manager_TaskLoop, "Manager_Task",  3000,  NULL,  0,  &Manager_Task, 0 );
}

void InitLocalVariables()
{
  m_SPIDataLinkSlave.SetupSPIDataLink();
  m_SPIDataLinkSlave.SetSpewToConsole(true);
  //m_SimpleSettingsWebServer.SetupSimpleSettingsWebServer();
}

void setup(){
  Serial.begin(500000);
  InitFileSystem();
  InitLocalVariables();
  InitWiFiAP();
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

struct JSON_Data_Value
{
  String Name;
  String Value;
};

//Get Slider Values
String Encode_JSON_Data_Values_To_JSON(struct JSON_Data_Value *DataValues, size_t Count)
{
  JSONVar JSONVars;
  for(int i = 0; i < Count; ++i)
  {
      JSONVar SettingValues;
      SettingValues["Name"] = DataValues[i].Name;
      SettingValues["Value"] = DataValues[i].Value;
      JSONVars["DataValue" + String(i)] = SettingValues;
  }
  return JSON.stringify(JSONVars);
}

void NotifyClients(String TextString)
{
  MyWebSocket.textAll(TextString);
}

void HandleWebSocketMessage(void *arg, uint8_t *data, size_t len)
{
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
  {
    data[len] = 0;
    String message = String((char*)data);
    Serial.println(message);
    if (true == message.equals("Get All Values"))
    {
      Serial.println("Sending All Value");
      struct JSON_Data_Value Values[5] = { 
                                           { "Amplitude_Gain_Slider", Amplitude_Gain_Slider },
                                           { "FFT_Gain_Slider", FFT_Gain_Slider },
                                           { "Red_Value_Slider", Red_Value_Slider },
                                           { "Green_Value_Slider", Green_Value_Slider },
                                           { "Blue_Value_Slider", Blue_Value_Slider },
                                         };
      NotifyClients(Encode_JSON_Data_Values_To_JSON(Values, sizeof(Values)/sizeof(Values[0])));
    }
    else
    {
      JSONVar MyObject = JSON.parse(message);
      if (JSON.typeof(MyObject) == "undefined") {
        Serial.println("Parsing input failed!");
        return;
      }
      if (MyObject.hasOwnProperty("Name"))
      {
          if(String((const char*) MyObject["Name"]).equals("Amplitude_Gain_Slider"))
          {
            Amplitude_Gain_Slider = String((const char*)(MyObject["Value"]));
            Serial.println("Amplitude_Gain_Slider Value: " + Amplitude_Gain_Slider);
            struct JSON_Data_Value Values[1] = {
                                                 { "Amplitude_Gain_Slider", Amplitude_Gain_Slider }
                                               };
            NotifyClients(Encode_JSON_Data_Values_To_JSON(Values, sizeof(Values)/sizeof(Values[0])));
          }
          else if(String((const char*) MyObject["Name"]).equals("FFT_Gain_Slider"))
          {
            FFT_Gain_Slider = String((const char*)(MyObject["Value"]));
            Serial.println("FFT_Gain_Slider Value: " + FFT_Gain_Slider);
            struct JSON_Data_Value Values[1] = { 
                                                 { "FFT_Gain_Slider", FFT_Gain_Slider }
                                               };
            NotifyClients(Encode_JSON_Data_Values_To_JSON(Values, sizeof(Values)/sizeof(Values[0])));
          }
          else if(String((const char*) MyObject["Name"]).equals("Red_Value_Slider"))
          {
            Red_Value_Slider = String((const char*)(MyObject["Value"]));
            Serial.println("Red_Value_Slider Value: " + Red_Value_Slider);
            struct JSON_Data_Value Values[1] = { 
                                                 { "Red_Value_Slider", Red_Value_Slider }
                                               };
            NotifyClients(Encode_JSON_Data_Values_To_JSON(Values, sizeof(Values)/sizeof(Values[0])));
          }
          else if(String((const char*) MyObject["Name"]).equals("Green_Value_Slider"))
          {
            Green_Value_Slider = String((const char*)(MyObject["Value"]));
            Serial.println("Green_Value_Slider Value: " + Green_Value_Slider);
            struct JSON_Data_Value Values[1] = { 
                                                 { "Green_Value_Slider", Green_Value_Slider }
                                               };
            NotifyClients(Encode_JSON_Data_Values_To_JSON(Values, sizeof(Values)/sizeof(Values[0])));
          }
          else if(String((const char*) MyObject["Name"]).equals("Blue_Value_Slider"))
          {
            Blue_Value_Slider = String((const char*)(MyObject["Value"]));
            Serial.println("Blue_Value_Slider Value: " + MyObject["Value"]);
            struct JSON_Data_Value Values[1] = { 
                                                 { "Blue_Value_Slider", Blue_Value_Slider }
                                               };
            NotifyClients(Encode_JSON_Data_Values_To_JSON(Values, sizeof(Values)/sizeof(Values[0])));
          }
          else
          {
            Serial.println("Failed to Parse");
          }
      }
    }
  }
}
