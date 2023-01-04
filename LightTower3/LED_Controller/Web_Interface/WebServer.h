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
#ifndef WEBSERVER_H
#define WEBSERVER_H

#include "Arduino.h"
#include "HTTP_Method.h"
#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include "AsyncTCP.h"
#include <Arduino_JSON.h>
#include <DataTypes.h>
#include <Helpers.h>
#include "Streaming.h"

class SimpleSettingsWebServer: public NamedItem
{
  public:
    SimpleSettingsWebServer( String Title
                           , AsyncWebSocket aWebSocket )
                           : NamedItem(Title)
                           , m_WebSocket(aWebSocket)
    {
    
    }
    virtual ~SimpleSettingsWebServer()
    {
    
    }
    void SetupSimpleSettingsWebServer()
    {
      InitWiFiAP();
    }
    
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
    
  private:
    AsyncWebSocket &m_WebSocket; 

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
    JSONVar SettingValues;
    
    int dutyCycle1;
    int dutyCycle2;
    int dutyCycle3;
    
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
    String GetSettingValues()
    {
      SettingValues["sliderValue1"] = String(sliderValue1);
      SettingValues["sliderValue2"] = String(sliderValue2);
      SettingValues["sliderValue3"] = String(sliderValue3);
    
      String jsonString = JSON.stringify(SettingValues);
      return jsonString;
    }
    
    void NotifyClients(String SettingValues)
    {
      m_WebSocket.textAll(SettingValues);
    }
    
    void HandleWebSocketMessage(void *arg, uint8_t *data, size_t len)
    {
      AwsFrameInfo *info = (AwsFrameInfo*)arg;
      if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
      {
        data[len] = 0;
        message = (char*)data;
        if (message.indexOf("1s") >= 0) {
          sliderValue1 = message.substring(2);
          dutyCycle1 = map(sliderValue1.toInt(), 0, 100, 0, 255);
          Serial.println(dutyCycle1);
          Serial.print(GetSettingValues());
          NotifyClients(GetSettingValues());
        }
        if (message.indexOf("2s") >= 0)
        {
          sliderValue2 = message.substring(2);
          dutyCycle2 = map(sliderValue2.toInt(), 0, 100, 0, 255);
          Serial.println(dutyCycle2);
          Serial.print(GetSettingValues());
          NotifyClients(GetSettingValues());
        }    
        if (message.indexOf("3s") >= 0)
        {
          sliderValue3 = message.substring(2);
          dutyCycle3 = map(sliderValue3.toInt(), 0, 100, 0, 255);
          Serial.println(dutyCycle3);
          Serial.print(GetSettingValues());
          NotifyClients(GetSettingValues());
        }
        if (strcmp((char*)data, "getValues") == 0)
        {
          NotifyClients(GetSettingValues());
        }
      }
    }
};


#endif
