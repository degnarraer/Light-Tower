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
#include <LinkedList.h>
#include "HTTP_Method.h"
#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include "AsyncTCP.h"
#include <Arduino_JSON.h>
#include <DataTypes.h>
#include <Helpers.h>
#include "Streaming.h"

class SettingsWebServerManager;

struct ReceiverHandler {
  String Widget = "";
  void (*CallBack)(SettingsWebServerManager*, const String&);

  bool operator==(const ReceiverHandler& rhs) const {
      bool a = Widget.equals(rhs.Widget);
      return a;
  }
};

struct SenderHandler{
  String Widget = "";
  void (*CallBack)(SettingsWebServerManager*, const String&);

  bool operator==(const SenderHandler& rhs) const {
      bool a = Widget.equals(rhs.Widget);
      return a;
  }
};

class WebSocketDataHandler
{
  public:
  void RegisterForWebSocketReceiveData(ReceiverHandler aReceiveHandler)
  {
    if(-1 == ReceiverFoundIndex(aReceiveHandler))
    {
      Serial << "Added Web Socket Receive Handler: " << aReceiveHandler.Widget << "\n";
      m_WidgetDataReceivers.add(aReceiveHandler);
    }
  }
  void DeRegisterForWebSocketReceiveData(ReceiverHandler aReceiveHandler)
  {
    int32_t FoundIndex = ReceiverFoundIndex(aReceiveHandler);
    if(0 < FoundIndex)
    {
      Serial << "Removed Web Socket Receive Handler: : " << aReceiveHandler.Widget << "\n";
      m_WidgetDataReceivers.remove(FoundIndex);
    }
  }
  virtual void RegisterForWebSocketSendData(SenderHandler aSenderHandler)
  {
    int32_t FoundIndex = SenderFoundIndex(aSenderHandler);
    if(-1 == FoundIndex)
    {
      Serial << "Added Web Socket Sender Handler: " << aSenderHandler.Widget << "\n";
      m_DataSenders.add(aSenderHandler);
    }
  }
  virtual void DeRegisterForWebSocketSendData(SenderHandler aSenderHandler)
  {
    int32_t FoundIndex = SenderFoundIndex(aSenderHandler);
    if(0 < FoundIndex)
    {
      Serial << "Removed Web Socket Sender Handler: " << aSenderHandler.Widget << "\n";
      m_DataSenders.remove(FoundIndex);
    }
  }
  String Name = "";

  protected:
  LinkedList<ReceiverHandler> m_WidgetDataReceivers = LinkedList<ReceiverHandler>();
  LinkedList<SenderHandler> m_DataSenders = LinkedList<SenderHandler>();

  private:
  int32_t ReceiverFoundIndex(ReceiverHandler aReceiverHandler)
  {
    int32_t FoundIndex = -1;
    for(int i = 0; i < m_WidgetDataReceivers.size(); ++i)
    {
      if(m_WidgetDataReceivers[i] == aReceiverHandler)
      {
        FoundIndex = i;
        exit;
      }
    }
    return FoundIndex;
  }
  int32_t SenderFoundIndex(SenderHandler aSenderHandler)
  {
    int32_t FoundIndex = -1;
    for(int i = 0; i < m_DataSenders.size(); ++i)
    {
      if(m_DataSenders[i] == aSenderHandler)
      {
        FoundIndex = i;
        exit;
      }
    }
    return FoundIndex;
  }
};

class SettingsWebServerManager: public QueueManager
                              , public WebSocketDataHandler
{
  
  struct KeyValuePair
  {
    String Key;
    String Value;
  };
  typedef KeyValuePair KVP;
  
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
      
      ReceiverHandler aReceiveHandler;
      aReceiveHandler.Widget = "Amplitude_Gain_Slider1";
      aReceiveHandler.CallBack = StaticHandleAmplitudeGainReceive;
      RegisterForWebSocketReceiveData(aReceiveHandler);
      
      aReceiveHandler.Widget = "Amplitude_Gain_Slider2";
      aReceiveHandler.CallBack = StaticHandleAmplitudeGainReceive;
      RegisterForWebSocketReceiveData(aReceiveHandler);
      
      aReceiveHandler.Widget = "FFT_Gain_Slider1";
      aReceiveHandler.CallBack = StaticHandleFFTGainReceive;
      RegisterForWebSocketReceiveData(aReceiveHandler);
     
      aReceiveHandler.Widget = "FFT_Gain_Slider2";
      aReceiveHandler.CallBack = StaticHandleFFTGainReceive;
      RegisterForWebSocketReceiveData(aReceiveHandler);
      
      aReceiveHandler.Widget = "Red_Value_Slider";
      aReceiveHandler.CallBack = StaticHandleRedValueReceive;
      RegisterForWebSocketReceiveData(aReceiveHandler);
      
      aReceiveHandler.Widget = "Blue_Value_Slider";
      aReceiveHandler.CallBack = StaticHandleBlueValueReceive;
      RegisterForWebSocketReceiveData(aReceiveHandler);
      
      aReceiveHandler.Widget = "Green_Value_Slider";
      aReceiveHandler.CallBack = StaticHandleGreenValueReceive;
      RegisterForWebSocketReceiveData(aReceiveHandler);
    }
    
    void ProcessEventQueue()
    {
      LinkedList<KVP> KeyValuePairs;
      //SOUND STATE TX QUEUE
      static bool SoundStatePullErrorHasOccured = false;
      if(true == GetValueFromTXQueue(&Sound_State, "Sound State",true, 0, SoundStatePullErrorHasOccured))
      {
        //Serial << "Received Value to Send to Clients: Sound State: "<< Sound_State << "\n";
        KeyValuePairs.add({ "Speaker_Image", String(Sound_State).c_str() });
      }
      
      //Amplitude Gain TX QUEUE
      static bool AmplitudeGainPullErrorHasOccured = false;
      if(true == GetValueFromTXQueue(&Amplitude_Gain, "Amplitude Gain", true, 0, AmplitudeGainPullErrorHasOccured))
      {
        //Serial << "Received Value to Send to Clients: Amplitude Gain: "<< Amplitude_Gain << "\n";
        KeyValuePairs.add({ "Amplitude_Gain_Slider1", String(Amplitude_Gain).c_str() });
        KeyValuePairs.add({ "Amplitude_Gain_Slider2", String(Amplitude_Gain).c_str() });
      }
      
      //FFT Gain TX QUEUE
      static bool FFTGainPullErrorHasOccured = false;
      if(true == GetValueFromTXQueue(&FFT_Gain, "FFT Gain", true, 0, FFTGainPullErrorHasOccured))
      {
        //Serial << "Received Value to Send to Clients: FFT Gain: "<< FFT_Gain << "\n";
        KeyValuePairs.add({ "FFT_Gain_Slider1", String(FFT_Gain).c_str() });
        KeyValuePairs.add({ "FFT_Gain_Slider2", String(FFT_Gain).c_str() });
      }
      if(KeyValuePairs.size() > 0)
      {
        NotifyClients(Encode_JSON_Data_Values_To_JSON(KeyValuePairs));
      }
      
      //Sink SSID TX QUEUE
      static bool SinkSSIDPullErrorHasOccured = false;
      if(true == GetValueFromTXQueue(&SinkSSID, "Sink SSID", true, 0, SinkSSIDPullErrorHasOccured))
      {
        Serial << "Received Value to Send to Clients: Sink SSID: "<< SinkSSID << "\n";
        KeyValuePairs.add({ "Sink SSID", SinkSSID });
      }
      
      //Source SSID TX QUEUE
      static bool SourceSSIDPullErrorHasOccured = false;
      if(true == GetValueFromTXQueue(&SourceSSID, "Source SSID", true, 0, SourceSSIDPullErrorHasOccured))
      {
        Serial << "Received Value to Send to Clients: Source SSID: "<< SourceSSID << "\n";
        KeyValuePairs.add({ "Source SSID", SourceSSID });
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
    
  private:
    AsyncWebSocket &m_WebSocket;
    // Replace with your network credentials
    const char* ssid = "LED Tower of Power";
    const char* password = "LEDs Rock";

    String message = "";

    //Amplitude Gain Value and Widget Name Values
    float Amplitude_Gain = 1.0;
    static void StaticHandleAmplitudeGainReceive(SettingsWebServerManager *WebServerManager, const String &Value)
    {
      WebServerManager->HandleAmplitudeGainReceive(Value);
    }
    void HandleAmplitudeGainReceive(const String &Value)
    {
      Amplitude_Gain = Value.toFloat();
      static bool AmplitudeGainPushErrorhasOccured = false;
      PushValueToRXQueue(&Amplitude_Gain, "Amplitude Gain", 0, AmplitudeGainPushErrorhasOccured);
    }

    //FFT Gain Value and Widget Name Values
    float FFT_Gain = 1.0;
    static void StaticHandleFFTGainReceive(SettingsWebServerManager *WebServerManager, const String &Value)
    {
      WebServerManager->HandleFFTGainReceive(Value);
    }
    void HandleFFTGainReceive(const String &Value)
    {
      FFT_Gain = Value.toFloat();
      static bool FFTGainPushErrorhasOccured = false;
      PushValueToRXQueue(&Amplitude_Gain, "FFT Gain", 0, FFTGainPushErrorhasOccured);
    }

    //Sound State Value and Widget Name Values
    SoundState_t Sound_State;

    //Sink SSID Value and Widget Name Values
    String SinkSSID = "";
    
    //Source SSID Value and Widget Name Values
    String SourceSSID = "";

    //Red Value and Widget Name Values
    uint32_t Red_Value;
    static void StaticHandleRedValueReceive(SettingsWebServerManager *WebServerManager, const String &Value)
    {
      WebServerManager->HandleRedValueReceive(Value);
    }
    void HandleRedValueReceive(const String &Value)
    {
      Red_Value = Value.toInt();
      static bool RedValuePushErrorhasOccured = false;
      PushValueToRXQueue(&Red_Value, "Red_Value", 0, RedValuePushErrorhasOccured);
    }

    
    //Blue Value and Widget Name Values
    uint32_t Blue_Value;
    static void StaticHandleBlueValueReceive(SettingsWebServerManager *WebServerManager, const String &Value)
    {
      WebServerManager->HandleBlueValueReceive(Value);
    }
    void HandleBlueValueReceive(const String &Value)
    {
      Blue_Value = Value.toInt();
      static bool BlueValuePushErrorhasOccured = false;
      PushValueToRXQueue(&Blue_Value, "Blue_Value", 0, BlueValuePushErrorhasOccured);
    }
    
    //Green Value and Widget Name Values
    uint32_t Green_Value;
    static void StaticHandleGreenValueReceive(SettingsWebServerManager *WebServerManager, const String &Value)
    {
      WebServerManager->HandleGreenValueReceive(Value);
    }
    void HandleGreenValueReceive(const String &Value)
    {
      Green_Value = Value.toInt();
      static bool GreenValuePushErrorhasOccured = false;
      PushValueToRXQueue(&Green_Value, "Green_Value", 0, GreenValuePushErrorhasOccured);
    }

    //QueueManager Interface
    static const size_t m_WebServerConfigCount = 13;
    DataItemConfig_t m_ItemConfig[m_WebServerConfigCount]
    {
      { "Source Connected",     DataType_bool_t,        1,    Transciever_RX,   10  },
      { "Source ReConnect",     DataType_bool_t,        1,    Transciever_TXRX, 4   },
      { "Source BT Reset",      DataType_bool_t,        1,    Transciever_TXRX, 4   },
      { "Source SSID",          DataType_String_t,      1,    Transciever_TXRX, 4   },
      { "Sink Connected",       DataType_bool_t,        1,    Transciever_RX,   4   },
      { "Sink ReConnect",       DataType_bool_t,        1,    Transciever_TXRX, 4   },
      { "Sink BT Reset",        DataType_bool_t,        1,    Transciever_TXRX, 4   },
      { "Sink SSID",            DataType_String_t,      1,    Transciever_TXRX, 4   },
      { "Sound State",          DataType_SoundState_t,  1,    Transciever_TX,   10  },
      { "Amplitude Gain",       DataType_Float_t,       1,    Transciever_TXRX, 10  },
      { "FFT Gain",             DataType_Float_t,       1,    Transciever_TXRX, 10  },
      { "Found Speaker SSIDS",  DataType_String_t,      1,    Transciever_TXRX, 4   },
      { "Target Speaker SSID",  DataType_String_t,      1,    Transciever_TXRX, 4   },
    };
    DataItemConfig_t* GetDataItemConfig() { return m_ItemConfig; }
    size_t GetDataItemConfigCount() { return m_WebServerConfigCount; }
    
    //Get Slider Values
    String Encode_JSON_Data_Values_To_JSON(LinkedList<KVP> &KeyValuePairs)
    {
      JSONVar JSONVars;
      for(int i = 0; i < KeyValuePairs.size(); ++i)
      {
          JSONVar SettingValues;
          SettingValues["Name"] = KeyValuePairs.get(i).Key.c_str();
          SettingValues["Value"] = KeyValuePairs.get(i).Value.c_str();
          JSONVars["DataValue" + String(i)] = SettingValues;
      }
      String Result = JSON.stringify(JSONVars);
      return Result;
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
              KeyValuePairs.add({ "Amplitude_Gain_Slider1", String(Amplitude_Gain).c_str() });
              KeyValuePairs.add({ "Amplitude_Gain_Slider2", String(Amplitude_Gain).c_str() });
              KeyValuePairs.add({ "FFT_Gain_Slider1", String(FFT_Gain).c_str() });
              KeyValuePairs.add({ "FFT_Gain_Slider2", String(FFT_Gain).c_str() });
              KeyValuePairs.add({ "Red_Value_Slider", String(Red_Value).c_str() });
              KeyValuePairs.add({ "Green_Value_Slider", String(Green_Value).c_str() });
              KeyValuePairs.add({ "Blue_Value_Slider", String(Blue_Value).c_str() });
              KeyValuePairs.add({ "Sound_State", String(Sound_State).c_str() });
              KeyValuePairs.add({ "Sink_SSID", String(SinkSSID).c_str() });
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
            const String Widget = String( (const char*)MyDataObject["WidgetValue"]["Widget"]);
            const String Value = String( (const char*)MyDataObject["WidgetValue"]["Value"]);
            bool WidgetFound = false;
            for(int i = 0; i < m_WidgetDataReceivers.size(); ++i)
            {
              if(m_WidgetDataReceivers.get(i).Widget.equals(Widget))
              {
                Serial.println("Widget: " + Widget + "  Value Received: " + Value);
                WidgetFound = true;
                m_WidgetDataReceivers[i].CallBack(this, Value);
              }
            }
            if(!WidgetFound)
            {
              Serial.println("Unknown Known Widget: " + Widget);
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
