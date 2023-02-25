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
#include <LinkedList.h>
#include "HTTP_Method.h"
#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include "AsyncTCP.h"
#include <Arduino_JSON.h>
#include <DataTypes.h>
#include <Helpers.h>
#include "Streaming.h"
#include <typeinfo>


class SettingsWebServerManager;

template<typename T>
class WebSocketDataHandler: public QueueController
{
  public:
    WebSocketDataHandler( const String DataItemName
                        , const String WidgetId
                        , T &Value
                        , QueueHandle_t QueueHandle
                        , size_t QueueTotalByteCount
                        , bool ReadUntilEmpty
                        , TickType_t TicksToWait
                        , LinkedList<KVP> &KeyValuePairs
                        , UBaseType_t TaskPriority
                        , uint8_t CoreId  )
                        : m_DataItemName(DataItemName) 
                        , m_WidgetId(WidgetId)
                        , m_Value(Value)
                        , m_QueueHandle(QueueHandle)
                        , m_QueueTotalByteCount(QueueTotalByteCount)
                        , m_ReadUntilEmpty(ReadUntilEmpty)
                        , m_TicksToWait(TicksToWait)
                        , m_KeyValuePairs(KeyValuePairs)
    {
      size_t TotalSize = sizeof(m_DataItemName) + sizeof(m_WidgetId) + sizeof(m_Value) + sizeof(m_ReadUntilEmpty) + sizeof(m_TicksToWait) + sizeof(m_KeyValuePairs); 
      xTaskCreatePinnedToCore( Static_WebSocketDataHandler_CheckForValue, "Web Socket Data Handler", TotalSize, this, TaskPriority, &m_Task_Handle, CoreId );
    }
    virtual ~WebSocketDataHandler(){}
    
  protected:
    const String m_DataItemName;
    const String m_WidgetId;
    T &m_Value;
    QueueHandle_t m_QueueHandle;
    size_t m_QueueTotalByteCount;
    bool m_ReadUntilEmpty;
    TickType_t m_TicksToWait;
    LinkedList<KVP> &m_KeyValuePairs;
    TaskHandle_t m_Task_Handle;
    bool m_PushError;
    
    void ProcessEventQueue()
    {
    }
    static void Static_WebSocketDataHandler_CheckForValue(void * parameter)
    {
      ((WebSocketDataHandler*)parameter)->CheckForValue();
    }
    virtual void CheckForValue()
    {
      if(true == GetValueFromQueue(&m_Value, m_QueueHandle, m_DataItemName, m_ReadUntilEmpty, m_TicksToWait, m_PushError))
      {
        Serial << "Received Value: " << String(m_Value).c_str() << " to Send to Clients for Data Item: "<< m_DataItemName.c_str() << "\n";
        m_KeyValuePairs.add({ m_WidgetId.c_str(), String(m_Value).c_str() });
      }
    }
};

class WebSocketStringDataHandler: public WebSocketDataHandler<String>
{
  public:
    WebSocketStringDataHandler( const String DataItemName
                              , const String WidgetId
                              , String &Value
                              , QueueHandle_t QueueHandle
                              , size_t ByteCount
                              , bool ReadUntilEmpty
                              , TickType_t TicksToWait
                              , LinkedList<KVP> &KeyValuePairs
                              , UBaseType_t TaskPriority
                              , uint8_t CoreId  )
                              : WebSocketDataHandler<String>(DataItemName, WidgetId, Value, QueueHandle, ByteCount, ReadUntilEmpty, TicksToWait, KeyValuePairs, TaskPriority, CoreId)
    {
    }
    virtual ~WebSocketStringDataHandler(){}
    void CheckForValue() override
    {
      char Buffer[m_QueueTotalByteCount];
      if(true == GetValueFromQueue(&Buffer, m_QueueHandle, m_WidgetId.c_str(), m_ReadUntilEmpty, m_TicksToWait, m_PushError))
      {
        m_Value = String(Buffer);
        Serial << "Received Value: " << m_Value << " to Send to Clients for Data Item: " << m_DataItemName.c_str() << "\n";
        m_KeyValuePairs.add({ m_WidgetId.c_str(), m_Value });
      }
    }
  protected:
  
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
    }
    
    void ProcessEventQueue()
    {
      LinkedList<KVP> KeyValuePairs;

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
    
  private:
    AsyncWebSocket &m_WebSocket;
    const char* ssid = "LED Tower of Power";
    const char* password = "LEDs Rock";

    String message = "";


    LinkedList<KVP> m_DataQueue = LinkedList<KVP>();
    
    //Sound State Value and Widget Name Values
    SoundState_t Sound_State;
    WebSocketDataHandler<SoundState_t> Sound_State_DataHandler = WebSocketDataHandler<SoundState_t>( "Sound State"
                                                                                                   , "Sound_State"
                                                                                                   , Sound_State
                                                                                                   , GetQueueHandleRXForDataItem("Sound State")
                                                                                                   , GetQueueByteCountForDataItem("Sound State")
                                                                                                   , true
                                                                                                   , 0
                                                                                                   , m_DataQueue
                                                                                                   , configMAX_PRIORITIES - 1
                                                                                                   , 0 );
                                                                                                   
    //Amplitude Gain Value and Widget Name Values
    float Amplitude_Gain = 1.0;
    WebSocketDataHandler<float> Amplitude_Gain1_DataHandler = WebSocketDataHandler<float>( "Amplitude Gain"
                                                                                         , "Amplitude_Gain_Slider1"
                                                                                         , Amplitude_Gain
                                                                                         , GetQueueHandleRXForDataItem("Amplitude Gain")
                                                                                         , GetQueueByteCountForDataItem("Amplitude Gain")
                                                                                         , true
                                                                                         , 0
                                                                                         , m_DataQueue
                                                                                         , configMAX_PRIORITIES - 1
                                                                                         , 0 );
                                                                                         
    WebSocketDataHandler<float> Amplitude_Gain2_DataHandler = WebSocketDataHandler<float>( "Amplitude Gain"
                                                                                         , "Amplitude_Gain_Slider2"
                                                                                         , Amplitude_Gain
                                                                                         , GetQueueHandleRXForDataItem("Amplitude Gain")
                                                                                         , GetQueueByteCountForDataItem("Amplitude Gain")
                                                                                         , true
                                                                                         , 0
                                                                                         , m_DataQueue
                                                                                         , configMAX_PRIORITIES - 1
                                                                                         , 0 );
    //FFT Gain Value and Widget Name Values
    float FFT_Gain = 1.0;
    WebSocketDataHandler<float> FFT_Gain1_DataHandler = WebSocketDataHandler<float>( "FFT Gain"
                                                                                   , "FFT_Gain_Slider1"
                                                                                   , Amplitude_Gain
                                                                                   , GetQueueHandleRXForDataItem("FFT Gain")
                                                                                   , GetQueueByteCountForDataItem("FFT Gain")
                                                                                   , true
                                                                                   , 0
                                                                                   , m_DataQueue
                                                                                   , configMAX_PRIORITIES - 1
                                                                                   , 0 );
                                                                                   
    WebSocketDataHandler<float> FFT_Gain2_DataHandler = WebSocketDataHandler<float>( "FFT Gain"
                                                                                   , "FFT_Gain_Slider2"
                                                                                   , Amplitude_Gain
                                                                                   , GetQueueHandleRXForDataItem("FFT Gain")
                                                                                   , GetQueueByteCountForDataItem("FFT Gain")
                                                                                   , true
                                                                                   , 0
                                                                                   , m_DataQueue
                                                                                   , configMAX_PRIORITIES - 1
                                                                                   , 0 );
                                                                                   

    //Sink SSID Value and Widget Name Values
    String SinkSSID = "";
    WebSocketStringDataHandler SinkSSID_DataHandler = WebSocketStringDataHandler( "Sink SSID"
                                                                                , "Sink_SSID_Text_Box"
                                                                                , SinkSSID
                                                                                , GetQueueHandleRXForDataItem("Sink SSID")
                                                                                , GetQueueByteCountForDataItem("Sink SSID")
                                                                                , true
                                                                                , 0
                                                                                , m_DataQueue
                                                                                , configMAX_PRIORITIES - 1
                                                                                , 0 );
    
    //Source SSID Value and Widget Name Values
    String SourceSSID = "";
    WebSocketStringDataHandler SourceSSID_DataHandler = WebSocketStringDataHandler( "Source SSID"
                                                                                  , "Source_SSID_Text_Box"
                                                                                  , SourceSSID
                                                                                  , GetQueueHandleRXForDataItem("Source SSID")
                                                                                  , GetQueueByteCountForDataItem("Source SSID")
                                                                                  , true
                                                                                  , 0
                                                                                  , m_DataQueue
                                                                                  , configMAX_PRIORITIES - 1
                                                                                  , 0 );

    //Red Value and Widget Name Values
    int8_t Red_Value;
    WebSocketDataHandler<int8_t> Red_Value_DataHandler = WebSocketDataHandler<int8_t> ( "Red Value"
                                                                                      , "Red_Value_Slider"
                                                                                      , Red_Value
                                                                                      , GetQueueHandleRXForDataItem("Red Value")
                                                                                      , GetQueueByteCountForDataItem("Red Value")
                                                                                      , true
                                                                                      , 0
                                                                                      , m_DataQueue
                                                                                      , configMAX_PRIORITIES - 1
                                                                                      , 0 );
    //Blue Value and Widget Name Values
    int8_t Blue_Value;
    WebSocketDataHandler<int8_t> Blue_Value_DataHandler = WebSocketDataHandler<int8_t> ( "Blue Value"
                                                                                       , "Blue_Value_Slider"
                                                                                       , Blue_Value
                                                                                       , GetQueueHandleRXForDataItem("Blue Value")
                                                                                       , GetQueueByteCountForDataItem("Blue Value")
                                                                                       , true
                                                                                       , 0
                                                                                       , m_DataQueue
                                                                                       , configMAX_PRIORITIES - 1
                                                                                       , 0 );
    //Red Value and Widget Name Values
    int8_t Green_Value;
    WebSocketDataHandler<int8_t> Green_Value_DataHandler = WebSocketDataHandler<int8_t> ( "Green Value"
                                                                                        , "Green_Value_Slider"
                                                                                        , Green_Value
                                                                                        , GetQueueHandleRXForDataItem("Green Value")
                                                                                        , GetQueueByteCountForDataItem("Green Value")
                                                                                        , true
                                                                                        , 0
                                                                                        , m_DataQueue
                                                                                        , configMAX_PRIORITIES - 1
                                                                                        , 0 );

    /*
    static void StaticHandleSinkSSIDValueReceive(SettingsWebServerManager *WebServerManager, const String &Value)
    {
      WebServerManager->HandleSinkSSIDValueReceive(Value);
    }
    void HandleSinkSSIDValueReceive(const String &Value)
    {
      SinkSSID = Value;
      Wifi_Info_t WifiInfo = Wifi_Info_t(SinkSSID);
      static bool SinkSSIDValuePushErrorhasOccured = false;
      PushValueToRXQueue(&WifiInfo, "Sink SSID", 0, SinkSSIDValuePushErrorhasOccured);
    }
    static void StaticHandleSinkSSIDValueSend(SettingsWebServerManager *WebServerManager, String &DataItemName, String &WidgetId, String &Value, LinkedList<KVP> &KeyValuePairs)
    {
      WebServerManager->HandleSinkSSIDValueSend(DataItemName, WidgetId, Value, KeyValuePairs);
    }
    void HandleSinkSSIDValueSend(String &DataItemName, String &WidgetId, String &Value, LinkedList<KVP> &KeyValuePairs)
    {
      static bool SinkSSIDPullErrorHasOccured = false;
      char Buffer[GetQueueByteCountForDataItem("Sink SSID")];
      if(true == GetValueFromTXQueue(&Buffer, "Sink SSID", true, 0, SinkSSIDPullErrorHasOccured))
      {
        SinkSSID = String(Buffer);
        //Serial << "Received Value to Send to Clients: Sink SSID: "<< SinkSSID << "\n";
        KeyValuePairs.add({ "Sink_SSID_Text_Box", SinkSSID });
      }
    }
    
    static void StaticHandleSourceSSIDValueReceive(SettingsWebServerManager *WebServerManager, const String &Value)
    {
      WebServerManager->HandleSourceSSIDValueReceive(Value);
    }
    void HandleSourceSSIDValueReceive(const String &Value)
    {
      SourceSSID = Value;
      Wifi_Info_t WifiInfo = Wifi_Info_t(SourceSSID);
      static bool SourceSSIDValuePushErrorhasOccured = false;
      PushValueToRXQueue(&WifiInfo, "Source SSID", 0, SourceSSIDValuePushErrorhasOccured);
    }
    static void StaticHandleSourceSSIDValueSend(SettingsWebServerManager *WebServerManager, String &DataItemName, String &WidgetId, String &Value, LinkedList<KVP> &KeyValuePairs)
    {
      WebServerManager->HandleSourceSSIDValueSend(DataItemName, WidgetId, Value, KeyValuePairs);
    }
    void HandleSourceSSIDValueSend(String &DataItemName, String &WidgetId, String &Value, LinkedList<KVP> &KeyValuePairs)
    {
      static bool SourceSSIDPullErrorHasOccured = false;
      char Buffer[GetQueueByteCountForDataItem("Source SSID")];
      if(true == GetValueFromTXQueue(&Buffer, "Source SSID", true, 0, SourceSSIDPullErrorHasOccured))
      {
        SourceSSID = String(Buffer);
        //Serial << "Received Value to Send to Clients: Source SSID: "<< SourceSSID << "\n";
        KeyValuePairs.add({ "Source_SSID_Text_Box", SourceSSID });
      }
    }
    */

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
    String Encode_JSON_Data_Values_To_JSON(LinkedList<KVP> &KeyValuePairs)
    {
      JSONVar JSONVars;
      for(int i = 0; i < KeyValuePairs.size(); ++i)
      {
          if( true == isAsciiString(KeyValuePairs.get(i).Key.c_str()) && true == isAsciiString(KeyValuePairs.get(i).Value.c_str()) )
          {
            JSONVar SettingValues;
            SettingValues["Id"] = KeyValuePairs.get(i).Key.c_str();
            SettingValues["Value"] = KeyValuePairs.get(i).Value.c_str();
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
              LinkedList<KVP> KeyValuePairs;
              KeyValuePairs.add({ "Sound_State", String(Sound_State).c_str() });
              //KeyValuePairs.add({ "Amplitude_Gain_Slider1", String(Amplitude_Gain).c_str() });
              //KeyValuePairs.add({ "Amplitude_Gain_Slider2", String(Amplitude_Gain).c_str() });
              //KeyValuePairs.add({ "FFT_Gain_Slider1", String(FFT_Gain).c_str() });
              //KeyValuePairs.add({ "FFT_Gain_Slider2", String(FFT_Gain).c_str() });
              //KeyValuePairs.add({ "Red_Value_Slider", String(Red_Value).c_str() });
              //KeyValuePairs.add({ "Green_Value_Slider", String(Green_Value).c_str() });
              //KeyValuePairs.add({ "Blue_Value_Slider", String(Blue_Value).c_str() });
              //KeyValuePairs.add({ "Sink_SSID_Text_Box", SinkSSID });
              NotifyClients(Encode_JSON_Data_Values_To_JSON(KeyValuePairs));
          }
          else
          {
            Serial.println("Unsupported Message: " + Message);
          }
        }
        else if( true == MyDataObject.hasOwnProperty("WidgetValue") )
        {
          if( true == MyDataObject["WidgetValue"].hasOwnProperty("Widget") && 
              true == MyDataObject["WidgetValue"].hasOwnProperty("Value") )
          {
            const String WidgetId = String( (const char*)MyDataObject["WidgetValue"]["Widget"]);
            const String Value = String( (const char*)MyDataObject["WidgetValue"]["Value"]);
            bool WidgetFound = false;
            /*
            for(int i = 0; i < m_WidgetDataReceivers.size(); ++i)
            {
              if(m_WidgetDataReceivers.get(i).WidgetId.equals(WidgetId))
              {
                Serial.println("Widget: " + WidgetId + "  Value Received: " + Value);
                WidgetFound = true;
                m_WidgetDataReceivers[i].CallBack(this, Value);
              }
            }
            */
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
