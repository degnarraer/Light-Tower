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
#include <freertos/portmacro.h>
#include "HTTP_Method.h"
#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include "AsyncTCP.h"
#include <Arduino_JSON.h>
#include <DataTypes.h>
#include <Helpers.h>
#include "Streaming.h"


class SettingsWebServerManager;

class WebSocketDataHandlerSender
{
  public:
    virtual void CheckForNewDataLinkValueAndSendToWebSocket(std::vector<KVP> &KeyValuePairs) = 0;
};

class WebSocketDataHandlerReceiver
{
  public:
    virtual bool ProcessWebSocketValueAndSendToDatalink(String WidgetId, String Value) = 0;
};

template<typename T>
class WebSocketDataHandler: public QueueController
                          , public CommonUtils
                          , public WebSocketDataHandlerReceiver
                          , public WebSocketDataHandlerSender
{
  public:
    WebSocketDataHandler(){}
    WebSocketDataHandler(const WebSocketDataHandler &t)
    {
      m_DataItem = t.m_DataItem;
      m_WidgetId = t.m_WidgetId;
      m_Value = t.m_Value;
      m_ReadUntilEmpty = t.m_ReadUntilEmpty;
      m_TicksToWait = t.m_TicksToWait;
      m_KeyValuePairs = t.m_KeyValuePairs;
    }
    WebSocketDataHandler( DataItem_t *DataItem
                        , String *WidgetId
                        , const size_t NumberOfWidgets
                        , bool ReadUntilEmpty
                        , TickType_t TicksToWait  )
                        : m_DataItem(DataItem) 
                        , m_WidgetId(WidgetId)
                        , m_NumberOfWidgets(NumberOfWidgets)
                        , m_ReadUntilEmpty(ReadUntilEmpty)
                        , m_TicksToWait(TicksToWait)
    {
    }
    virtual ~WebSocketDataHandler()
    {
    }
    
    void SetValue(T Value)
    {
      m_Value = Value;
    }
    
    T GetValue()
    {
      return m_Value;
    }
    
  protected:
    DataItem_t *m_DataItem = NULL;
    String *m_WidgetId = NULL;
    size_t m_NumberOfWidgets = 0;
    T m_Value;
    bool m_ReadUntilEmpty;
    TickType_t m_TicksToWait;
    std::vector<KVP> *m_KeyValuePairs;
    UBaseType_t m_TaskPriority;
    uint8_t m_CoreId;
    bool m_PushError = false;
    bool m_PullError = false;
    
    virtual void CheckForNewDataLinkValueAndSendToWebSocket(std::vector<KVP> &KeyValuePairs)
    {
      if(null != m_DataItem)
      {
        if(true == GetValueFromQueue(&m_Value, m_DataItem->QueueHandle_TX, m_DataItem->Name.c_str(), m_ReadUntilEmpty, m_TicksToWait, m_PullError))
        {
          //Serial << "Received Value: " << String(m_Value).c_str() << " to Send to Clients for Data Item: "<< m_DataItem->Name.c_str() << "\n";
          for (size_t i = 0; i < m_NumberOfWidgets; i++)
          {
            KeyValuePairs.push_back({ m_WidgetId[i].c_str(), String(m_Value).c_str() });
          }
        }
      }
    }
    
    virtual bool ProcessWebSocketValueAndSendToDatalink(String WidgetId, String Value)
    {
      bool Found = false;
      String InputId = WidgetId;
      if(null != m_DataItem)
      {
        for (size_t i = 0; i < m_NumberOfWidgets; i++)
        {
          if( m_WidgetId[i].equals(InputId) )
          {
            if( true == ConvertStringToDataBufferFromDataType(&m_Value, Value, m_DataItem->DataType))
            {
              Found = true;
              Serial << "Web Socket Data Received: " << String(m_Value).c_str() << " to Send to Clients for Data Item: "<< m_DataItem->Name.c_str() << "\n";
              PushValueToQueue(&m_Value, m_DataItem->QueueHandle_RX, m_DataItem->Name.c_str(), 0, m_PushError);
            }
            return Found;
          }
        }
      }
      return Found;
    }
};

class WebSocketSSIDDataHandler: public WebSocketDataHandler<String>
{
  public:
    WebSocketSSIDDataHandler(){}
    WebSocketSSIDDataHandler( DataItem_t *DataItem
                              , String *WidgetIds
                              , const size_t NumberOfWidgets
                              , bool ReadUntilEmpty
                              , TickType_t TicksToWait )
                              : WebSocketDataHandler<String>(DataItem, WidgetIds, NumberOfWidgets, ReadUntilEmpty, TicksToWait)
    {
    }
    
    virtual ~WebSocketSSIDDataHandler()
    {
    }
    
  protected:
    void CheckForNewDataLinkValueAndSendToWebSocket(std::vector<KVP> &KeyValuePairs) override
    {
      if(null != m_DataItem)
      {
        char Buffer[m_DataItem->TotalByteCount];
        if(true == GetValueFromQueue(&Buffer, m_DataItem->QueueHandle_TX, m_DataItem->Name.c_str(), m_ReadUntilEmpty, m_TicksToWait, m_PullError))
        {
          m_Value = String(Buffer);
          //Serial << "Received Value: " << String(m_Value).c_str() << " to Send to Clients for Data Item: " << m_DataItem->Name.c_str() << "\n";
          for (size_t i = 0; i < m_NumberOfWidgets; i++)
          {
            KeyValuePairs.push_back({ m_WidgetId[i].c_str(), m_Value.c_str() });
          }
        }
      }
    }
    
    bool ProcessWebSocketValueAndSendToDatalink(String WidgetId, String Value) override
    {
      bool Found = false;
      String InputId = WidgetId;
      if(null != m_DataItem)
      {
        m_Value = Value;
        Wifi_Info_t WifiInfo = Wifi_Info_t(m_Value);
        for (size_t i = 0; i < m_NumberOfWidgets; i++)
        {
         // Serial << m_WidgetId[i] << " | " << WidgetId << " | " << Value << "\n";
          m_WidgetId[i].trim();
          InputId.trim();
          if( m_WidgetId[i].equals(InputId) )
          {
            Found = true;
            Serial << "Send Value: " << m_Value.c_str() << " to Send to Clients for Data Item: "<< m_DataItem->Name.c_str() << "\n";
            PushValueToQueue(&WifiInfo, m_DataItem->QueueHandle_RX, m_DataItem->Name.c_str(), 0, m_PushError);
          }
        }
      }
      return Found;
    }
};

class SettingsWebServerManager: public QueueManager
{  
  public:
    SettingsWebServerManager( String Title
                            , AsyncWebSocket &WebSocket )
                            : QueueManager(Title + "Queue Manager", GetDataItemConfigCount())
                            , m_WebSocket(WebSocket)
    {
      
    }
    
    virtual ~SettingsWebServerManager()
    {
    
    }
    
    void SetupSettingsWebServerManager()
    {
      SetupQueueManager();
      InitWiFiAP();
      Sound_State_DataHandler = WebSocketDataHandler<SoundState_t>( GetPointerToDataItemWithName("Sound State"), new String[1]{"Speaker_Image"}, 1, true, 0 );
      RegisterAsWebSocketDataSender(&Sound_State_DataHandler);
      
      Amplitude_Gain_DataHandler = WebSocketDataHandler<float>( GetPointerToDataItemWithName("Amplitude Gain"), new String[2]{"Amplitude_Gain_Slider1", "Amplitude_Gain_Slider2"}, 2, true, 0 );
      RegisterAsWebSocketDataReceiver(&Amplitude_Gain_DataHandler);
      RegisterAsWebSocketDataSender(&Amplitude_Gain_DataHandler);
      
      FFT_Gain_DataHandler = WebSocketDataHandler<float>( GetPointerToDataItemWithName("FFT Gain"), new String[2]{"FFT_Gain_Slider1", "FFT_Gain_Slider2"}, 2, true, 0 );
      RegisterAsWebSocketDataReceiver(&FFT_Gain_DataHandler);
      RegisterAsWebSocketDataSender(&FFT_Gain_DataHandler);
      
      SinkSSID_DataHandler = WebSocketSSIDDataHandler( GetPointerToDataItemWithName("Sink SSID"), new String[1]{"Sink_SSID_Text_Box"}, 1, true, 0 );
      RegisterAsWebSocketDataReceiver(&SinkSSID_DataHandler);
      RegisterAsWebSocketDataSender(&SinkSSID_DataHandler);
    
      SourceSSID_DataHandler = WebSocketSSIDDataHandler( GetPointerToDataItemWithName("Source SSID"), new String[1]{"Source_SSID_Text_Box"}, 1, true, 0 );
      RegisterAsWebSocketDataReceiver(&SourceSSID_DataHandler);
      RegisterAsWebSocketDataSender(&SourceSSID_DataHandler);

      Red_Value_DataHandler = WebSocketDataHandler<uint8_t>( GetPointerToDataItemWithName("Red Value"), new String[1]{"Red_Value_Slider"}, 1, true, 0 );
      RegisterAsWebSocketDataReceiver(&Red_Value_DataHandler);
      RegisterAsWebSocketDataSender(&Red_Value_DataHandler);

      Blue_Value_DataHandler = WebSocketDataHandler<uint8_t>( GetPointerToDataItemWithName("Blue Value"), new String[1]{"Blue_Value_Slider"}, 1, true, 0 );
      RegisterAsWebSocketDataReceiver(&Blue_Value_DataHandler);
      RegisterAsWebSocketDataSender(&Blue_Value_DataHandler);

      Green_Value_DataHandler = WebSocketDataHandler<uint8_t>( GetPointerToDataItemWithName("Green Value"), new String[1]{"Green_Value_Slider"}, 1, true, 0 );
      RegisterAsWebSocketDataReceiver(&Green_Value_DataHandler);
      RegisterAsWebSocketDataSender(&Green_Value_DataHandler);
    }
    
    void ProcessEventQueue()
    {
      std::vector<KVP> KeyValuePairs = std::vector<KVP>();
      for(int i = 0; i < m_MySenders.size(); ++i)
      {
        m_MySenders[i]->CheckForNewDataLinkValueAndSendToWebSocket(KeyValuePairs);
      }
      if(KeyValuePairs.size() > 0)
      {
        NotifyClients(Encode_JSON_Data_Values_To_JSON(KeyValuePairs));
      }
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

    void RegisterAsWebSocketDataReceiver(WebSocketDataHandlerReceiver *aReceiver)
    {
      for(int i = 0; i < m_MyReceivers.size(); ++i)
      {
        if(m_MyReceivers[i] == aReceiver)
        {
          return;
        }
      }
      m_MyReceivers.push_back(aReceiver);
    }

    void DeRegisterAsWebSocketDataReceiver(WebSocketDataHandlerReceiver *aReceiver)
    {
      for(int i = 0; i < m_MyReceivers.size(); ++i)
      {
        if(m_MyReceivers[i] == aReceiver)
        {
          m_MyReceivers.erase(m_MyReceivers.begin() + i);
          return;
        }
      }
    }
    
    void RegisterAsWebSocketDataSender(WebSocketDataHandlerSender *aSender)
    {
      for(int i = 0; i < m_MySenders.size(); ++i)
      {
        if(m_MySenders[i] == aSender)
        {
          return;
        }
      }
      m_MySenders.push_back(aSender);
    }
    
    void DeRegisterAsWebSocketDataSender(WebSocketDataHandlerSender *aSender)
    {
      for(int i = 0; i < m_MySenders.size(); ++i)
      {
        if(m_MySenders[i] == aSender)
        {
          m_MySenders.erase(m_MySenders.begin() + i);
          return;
        }
      }
    }
    
  private:
    AsyncWebSocket &m_WebSocket;
    const char* ssid = "LED Tower of Power";
    const char* password = "LEDs Rock";
    String message = "";

    std::vector<WebSocketDataHandlerReceiver*> m_MyReceivers;
    std::vector<WebSocketDataHandlerSender*> m_MySenders;

    //Sound State Value and Widget Name Values
    SoundState_t Sound_State;
    WebSocketDataHandler<SoundState_t> Sound_State_DataHandler;

    //Amplitude Gain Value and Widget Name Values
    WebSocketDataHandler<float> Amplitude_Gain_DataHandler;
    
    //FFT Gain Value and Widget Name Values
    WebSocketDataHandler<float> FFT_Gain_DataHandler;

    //Sink SSID Value and Widget Name Values
    WebSocketSSIDDataHandler SinkSSID_DataHandler;
    
    //Source SSID Value and Widget Name Values
    WebSocketSSIDDataHandler SourceSSID_DataHandler;

    //Red Value and Widget Name Values
    WebSocketDataHandler<uint8_t> Red_Value_DataHandler;
    
    //Blue Value and Widget Name Values
    WebSocketDataHandler<uint8_t> Blue_Value_DataHandler;
    
    //Red Value and Widget Name Values
    WebSocketDataHandler<uint8_t> Green_Value_DataHandler;

    //QueueManager Interface
    static const size_t m_WebServerConfigCount = 13;
    DataItemConfig_t m_ItemConfig[m_WebServerConfigCount]
    {
      { "Sound State",          DataType_SoundState_t,  1,    Transciever_TX,   10  },
      { "Source Connected",     DataType_bool_t,        1,    Transciever_RX,   10  },
      { "Source ReConnect",     DataType_bool_t,        1,    Transciever_TXRX, 4   },
      { "Source BT Reset",      DataType_bool_t,        1,    Transciever_TXRX, 4   },
      { "Source SSID",          DataType_Wifi_Info_t,   1,    Transciever_TXRX, 4   },
      { "Sink Connected",       DataType_bool_t,        1,    Transciever_RX,   4   },
      { "Sink ReConnect",       DataType_bool_t,        1,    Transciever_TXRX, 4   },
      { "Sink BT Reset",        DataType_bool_t,        1,    Transciever_TXRX, 4   },
      { "Sink SSID",            DataType_Wifi_Info_t,   1,    Transciever_TXRX, 4   },
      { "Amplitude Gain",       DataType_Float_t,       1,    Transciever_TXRX, 10  },
      { "FFT Gain",             DataType_Float_t,       1,    Transciever_TXRX, 10  },
      { "Found Speaker SSIDS",  DataType_Wifi_Info_t,   10,   Transciever_TXRX, 4   },
      { "Target Speaker SSID",  DataType_Wifi_Info_t,   10,   Transciever_TXRX, 4   },
    };
    DataItemConfig_t* GetDataItemConfig() { return m_ItemConfig; }
    size_t GetDataItemConfigCount() { return m_WebServerConfigCount; }
    
    //Get Slider Values
    String Encode_JSON_Data_Values_To_JSON(std::vector<KVP> &KeyValuePairs)
    {
      JSONVar JSONVars;
      for(int i = 0; i < KeyValuePairs.size(); ++i)
      {
          if( true == isAsciiString(KeyValuePairs[i].Key.c_str()) && true == isAsciiString(KeyValuePairs[i].Value.c_str()) )
          {
            JSONVar SettingValues;
            SettingValues["Id"] = KeyValuePairs[i].Key.c_str();
            SettingValues["Value"] = KeyValuePairs[i].Value.c_str();
            JSONVars["DataValue" + String(i)] = SettingValues;
          }
      }
      String Result = JSON.stringify(JSONVars);
      return Result;
    }
    
    bool isAsciiString(const char* str)
    {
      for (size_t i = 0; str[i] != '\0'; i++) {
        if (str[i] < 0 || str[i] > 127) {
          // Character is not ASCII
          return false;
        }
      }
      // All characters are ASCII
      return true;
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
        String WebSocketData = String((char*)data);
        Serial << "WebSocket Data from Client: " << WebSocketData << "\n";
        JSONVar MyDataObject = JSON.parse(WebSocketData);
        if (JSON.typeof(MyDataObject) == "undefined")
        {
          Serial.println("Parsing Web Socket Data failed!");
          return;
        }
        if( true == MyDataObject.hasOwnProperty("Message") )
        {
          const String Message = String( (const char*)MyDataObject["Message"]);
          Serial.println("Message: " + Message);
          if (Message.equals("Get All Values"))
          {
              Serial.println("Sending All Values");
              std::vector<KVP> KeyValuePairs = std::vector<KVP>();
              KeyValuePairs.push_back({ "Sound_State", String(Sound_State).c_str() });
              KeyValuePairs.push_back({ "Amplitude_Gain_Slider1", String(Amplitude_Gain_DataHandler.GetValue()).c_str() });
              KeyValuePairs.push_back({ "Amplitude_Gain_Slider2", String(Amplitude_Gain_DataHandler.GetValue()).c_str() });
              KeyValuePairs.push_back({ "FFT_Gain_Slider1", String(FFT_Gain_DataHandler.GetValue()).c_str() });
              KeyValuePairs.push_back({ "FFT_Gain_Slider2", String(FFT_Gain_DataHandler.GetValue()).c_str() });
              KeyValuePairs.push_back({ "Red_Value_Slider", String(Red_Value_DataHandler.GetValue()).c_str() });
              KeyValuePairs.push_back({ "Green_Value_Slider", String(Green_Value_DataHandler.GetValue()).c_str() });
              KeyValuePairs.push_back({ "Blue_Value_Slider", String(Blue_Value_DataHandler.GetValue()).c_str() });
              KeyValuePairs.push_back({ "Sink_SSID_Text_Box", SinkSSID_DataHandler.GetValue() });
              KeyValuePairs.push_back({ "Source_SSID_Text_Box", SourceSSID_DataHandler.GetValue() });
              NotifyClients(Encode_JSON_Data_Values_To_JSON(KeyValuePairs));
          }
          else
          {
            Serial.println("Unsupported Message: " + Message);
          }
        }
        else if( true == MyDataObject.hasOwnProperty("WidgetValue") )
        {
          if( true == MyDataObject["WidgetValue"].hasOwnProperty("Id") && 
              true == MyDataObject["WidgetValue"].hasOwnProperty("Value") )
          {
            const String WidgetId = String( (const char*)MyDataObject["WidgetValue"]["Id"]);
            const String Value = String( (const char*)MyDataObject["WidgetValue"]["Value"]);
            bool WidgetFound = false;
            
            for(int i = 0; i < m_MyReceivers.size(); ++i)
            {
              if(true == m_MyReceivers[i]->ProcessWebSocketValueAndSendToDatalink(WidgetId.c_str(), Value.c_str()))
              {
                Serial.println("Widget Id: " + WidgetId + "  Value Received: " + Value);
                WidgetFound = true;
              }
            }
            if(!WidgetFound)
            {
              Serial.println("Unknown Known Widget: " + WidgetId);
            }
          }
          else
          {
            Serial.println("Misconfigured Widget Value Data: " + MyDataObject["DataValue"]);
          }
        }
        else
        {
          Serial.println("Unsupported Web Socket Data: " + WebSocketData);
        }
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
};


#endif
