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
                           , AsyncWebSocket WebSocket )
                           : NamedItem(Title)
                           , m_WebSocket(WebSocket)
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
    String EncodeJSON(String Name, String Value)
    {
      JSONVar SettingValues;
      SettingValues["Name"] = Name;
      SettingValues["Value"] = Value;
      String jsonString = JSON.stringify(SettingValues);
      Serial.println(jsonString);
      return jsonString;
    }
    
    void NotifyClients(String TextString)
    {
      m_WebSocket.textAll(TextString);
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
          NotifyClients(EncodeJSON("Amplitude_Gain_Slider", Amplitude_Gain_Slider));
          NotifyClients(EncodeJSON("FFT_Gain_Slider", FFT_Gain_Slider));
          NotifyClients(EncodeJSON("Red_Value_Slider", Red_Value_Slider));
          NotifyClients(EncodeJSON("Green_Value_Slider", Green_Value_Slider));
          NotifyClients(EncodeJSON("Blue_Value_Slider", Blue_Value_Slider));
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
              Serial.print("MyObject[\"Name\"] = ");
              Serial.println(MyObject["Name"]);
              if(String((const char*) MyObject["Name"]).equals("Amplitude_Gain_Slider"))
              {
                Amplitude_Gain_Slider = String((const char*)(MyObject["Value"]));
                Serial.println("Amplitude_Gain_Slider Value: " + Amplitude_Gain_Slider);
                NotifyClients(EncodeJSON("Amplitude_Gain_Slider", Amplitude_Gain_Slider));
              }
              else if(String((const char*) MyObject["Name"]).equals("FFT_Gain_Slider"))
              {
                FFT_Gain_Slider = String((const char*)(MyObject["Value"]));
                Serial.println("FFT_Gain_Slider Value: " + FFT_Gain_Slider);
                NotifyClients(EncodeJSON("FFT_Gain_Slider", FFT_Gain_Slider));
              }
              else if(String((const char*) MyObject["Name"]).equals("Red_Value_Slider"))
              {
                Red_Value_Slider = String((const char*)(MyObject["Value"]));
                Serial.println("Red_Value_Slider Value: " + Red_Value_Slider);
                NotifyClients(EncodeJSON("Red_Value_Slider", Red_Value_Slider));
              }
              else if(String((const char*) MyObject["Name"]).equals("Green_Value_Slider"))
              {
                Green_Value_Slider = String((const char*)(MyObject["Value"]));
                Serial.println("Green_Value_Slider Value: " + Green_Value_Slider);
                NotifyClients(EncodeJSON("Green_Value_Slider", Green_Value_Slider));
              }
              else if(String((const char*) MyObject["Name"]).equals("Blue_Value_Slider"))
              {
                Blue_Value_Slider = String((const char*)(MyObject["Value"]));
                Serial.println("Blue_Value_Slider Value: " + MyObject["Value"]);
                NotifyClients(EncodeJSON("Blue_Value_Slider", Blue_Value_Slider));
              }
              else
              {
                Serial.println("Failed to Parse");
              }
          }
        }
      }
    }
};


#endif
