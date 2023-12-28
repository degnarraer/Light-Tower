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
#ifndef WEB_SOCKET_DATA_HANDLER_H
#define WEB_SOCKET_DATA_HANDLER_H

#include "Arduino.h"
#include <freertos/portmacro.h>
#include <Streaming.h>
#include <Helpers.h>
#include <DataTypes.h>
#include "Tunes.h"
#include "DataItem.h"
#include "ESPAsyncWebServer.h"
#include "AsyncTCP.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-std=c++11"
#include <Arduino_JSON.h>
#pragma GCC diagnostic pop

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

class WebSocketDataProcessor
{
  public:
    WebSocketDataProcessor( AsyncWebSocket &WebSocket )
                          : m_WebSocket(WebSocket)
    {
      xTaskCreatePinnedToCore( StaticWebSocketDataProcessor_Task,  "WebServer_Task",   10000,  this,  configMAX_PRIORITIES - 1,    &m_WebSocketTaskHandle,    0 );
    }
    virtual ~WebSocketDataProcessor()
    {
      if(m_WebSocketTaskHandle) vTaskDelete(m_WebSocketTaskHandle);
    }
    void RegisterAsWebSocketDataReceiver(String Name, WebSocketDataHandlerReceiver *aReceiver);
    void DeRegisterAsWebSocketDataReceiver(String Name, WebSocketDataHandlerReceiver *aReceiver);
    void RegisterAsWebSocketDataSender(String Name, WebSocketDataHandlerSender *aSender);
    void DeRegisterAsWebSocketDataSender(String Name, WebSocketDataHandlerSender *aSender);
    bool ProcessWebSocketValueAndSendToDatalink(String WidgetId, String Value);
    
    static void StaticWebSocketDataProcessor_Task(void * parameter)
    {
      WebSocketDataProcessor *Processor = (WebSocketDataProcessor*)parameter;
      Processor->WebSocketDataProcessor_Task();
    }
  private:
    AsyncWebSocket &m_WebSocket;
    TaskHandle_t m_WebSocketTaskHandle;
    std::vector<WebSocketDataHandlerReceiver*> m_MyReceivers = std::vector<WebSocketDataHandlerReceiver*>();
    std::vector<WebSocketDataHandlerSender*> m_MySenders = std::vector<WebSocketDataHandlerSender*>();
    void WebSocketDataProcessor_Task();
    String Encode_Widget_Values_To_JSON(std::vector<KVP> &KeyValuePairs);
    void NotifyClients(String TextString);
};


template<typename T>
class WebSocketDataHandler: public WebSocketDataHandlerReceiver
                          , public WebSocketDataHandlerSender
                          , public NewRxTxValueCalleeInterface<T>
                          , public DataTypeFunctions
{
  public:
    WebSocketDataHandler()
    {
    }
    WebSocketDataHandler( const WebSocketDataHandler &t )
                        : m_Name(t.m_Name)
                        , m_WidgetIds(t.m_WidgetIds)
                        , m_WebSocketDataProcessor(t.m_WebSocketDataProcessor)
                        , m_IsReceiver(t.m_IsReceiver)
                        , m_IsSender(t.m_IsSender)
                        , m_NewRxTxValue(t.m_NewRxTxValue)
                        , m_Value(t.m_Value)
                        , m_Debug(t.m_Debug)
    {
      m_NewRxTxValue.RegisterForNewValueNotification(this);
    }
    WebSocketDataHandler( const String &Name
                        , const std::initializer_list<const char*>& WidgetIds
                        , WebSocketDataProcessor &WebSocketDataProcessor
                        , const bool &IsReceiver
                        , const bool &IsSender
                        , NewRxTxValueCallerInterface<T> &NewRxTxValue
                        , const bool &Debug )
                        : m_Name(Name)
                        , m_WidgetIds(WidgetIds.begin(), WidgetIds.end())
                        , m_WebSocketDataProcessor(WebSocketDataProcessor)
                        , m_IsReceiver(IsReceiver)
                        , m_IsSender(IsSender)
                        , m_NewRxTxValue(NewRxTxValue)
                        , m_Debug(Debug)
    {
      m_NewRxTxValue.RegisterForNewValueNotification(this);
      if(m_IsReceiver) m_WebSocketDataProcessor.RegisterAsWebSocketDataReceiver(m_Name, this);
      if(m_IsSender) m_WebSocketDataProcessor.RegisterAsWebSocketDataSender(m_Name, this);
    }
    
    virtual ~WebSocketDataHandler()
    {
      m_NewRxTxValue.DeRegisterForNewValueNotification(this);
      if(m_IsReceiver) m_WebSocketDataProcessor.DeRegisterAsWebSocketDataReceiver(m_Name, this);
      if(m_IsSender) m_WebSocketDataProcessor.DeRegisterAsWebSocketDataSender(m_Name, this);
    }
    
    void NewRxValueReceived(T* object)
    {
      T Value = *static_cast<T*>(object);
      m_DataLinkValue = Value;
    }
    void SetNewTxValue(T* Object)
    {
      ESP_LOGE( "WebSocketDataHandler: SetNewTxValue", "THIS IS NOT HANDLED YET");
    }
    
    T GetValue()
    {
      return m_Value;
    }
    
    String GetName()
    {
      return m_Name;
    }
  protected:
    const String &m_Name;
    WebSocketDataProcessor &m_WebSocketDataProcessor;
    const bool &m_IsReceiver;
    const bool &m_IsSender;
    std::vector<String> m_WidgetIds;
    T m_Value;
    T m_WebSocketValue;
    T m_DataLinkValue;
    NewRxTxValueCallerInterface<T> &m_NewRxTxValue;
    const bool &m_Debug;
    
    virtual void CheckForNewDataLinkValueAndSendToWebSocket(std::vector<KVP> &KeyValuePairs)
    {
      if(m_Value != m_DataLinkValue)
      {
        for (size_t i = 0; i < m_WidgetIds.size(); i++)
        {
          if(true == m_Debug) Serial << "Sending " << String(m_Value) << " to Web Socket\n";
          KeyValuePairs.push_back({ m_WidgetIds[i], String(m_Value) });
        }
        m_Value = m_DataLinkValue;
      }
    }
    
    virtual bool ProcessWebSocketValueAndSendToDatalink(String WidgetId, String StringValue)
    {
      bool Found = false;
      if (SetDataItemValueFromValueString(&m_WebSocketValue, StringValue, GetDataTypeFromType<T>()))
      {
        for (size_t i = 0; i < m_WidgetIds.size(); i++)
        {
          if( m_WidgetIds[i].equals(WidgetId) )
          {
            ESP_LOGD( "WebSocketDataHandler: ProcessWebSocketValueAndSendToDatalink"
                    , "Widget ID[%i]: %s  WidgetId: %s"
                    , i , m_WidgetIds[i].c_str(), WidgetId.c_str() );
            Found = true;
          }
        }
        if(m_Value != m_WebSocketValue && Found)
        {
          m_Value = m_WebSocketValue;
          m_NewRxTxValue.SetNewTxValue(&m_Value);
        }
      }
      return Found;
    }
};

class WebSocketSSIDArrayDataHandler: public WebSocketDataHandler<String>
{
  public:
    WebSocketSSIDArrayDataHandler( const String &Name 
                                 , const std::initializer_list<const char*>& WidgetIds
                                 , WebSocketDataProcessor &WebSocketDataProcessor
                                 , const bool &IsReceiver
                                 , const bool &IsSender
                                 , NewRxTxValueCallerInterface<String> &NewRxTxValue
                                 , const bool Debug )
                                 : WebSocketDataHandler<String>( Name
                                                               , WidgetIds
                                                               , WebSocketDataProcessor
                                                               , IsReceiver
                                                               , IsSender
                                                               , NewRxTxValue
                                                               , Debug)
    {
    }
    
    virtual ~WebSocketSSIDArrayDataHandler()
    {
    }
  protected:
  
    void CheckForNewDataLinkValueAndSendToWebSocket(std::vector<KVP> &KeyValuePairs) override
    {
      /*
      SSID_Info_With_LastUpdateTime_t Received_SSID;
      unsigned long CurrentTime = millis();
      size_t TotalSSIDs = uxQueueMessagesWaiting(m_DataItem->QueueHandle_TX);
      for(int i = 0; i < TotalSSIDs; ++i)
      {
        if(true == GetValueFromQueue(&Received_SSID, m_DataItem->QueueHandle_TX, m_DataItem->Name, false, m_TicksToWait, m_PullError))
        {
          bool Found = false;
          for(int j = 0; j < m_ActiveSSIDs.size(); ++j)
          {
            if( true == m_ActiveSSIDs[j].SSID.equals(Received_SSID.SSID) )
            {
              Found = true;
              m_ActiveSSIDs[j].RSSI = Received_SSID.RSSI;
              m_ActiveSSIDs[j].LastUpdateTime = CurrentTime;
              if(ACTIVE_SSID_TIMEOUT <= Received_SSID.TimeSinceUdpate)
              {
                ESP_LOGI("WebSocketDataHandler", "SSID Timedout: %s", Received_SSID.SSID);
                m_ActiveSSIDs.erase(m_ActiveSSIDs.begin()+j);
              }
              break;
            }
          }
          if(false == Found && ACTIVE_SSID_TIMEOUT >= Received_SSID.TimeSinceUdpate )
          {
            ESP_LOGI("WebSocketDataHandler", "Found New SSID: %s", Received_SSID.SSID);
            ActiveCompatibleDevice_t NewDevice;
            NewDevice.SSID = Received_SSID.SSID;
            NewDevice.ADDRESS = Received_SSID.ADDRESS;
            NewDevice.RSSI = Received_SSID.RSSI;
            NewDevice.LastUpdateTime = CurrentTime;
            m_ActiveSSIDs.push_back(NewDevice);
          }
        }
      }
      
      std::vector<KVT> KeyValueTupleVector;
      for(int i = 0; i < m_ActiveSSIDs.size(); ++i)
      {
        KVT KeyValueTuple;
        KeyValueTuple.Key = m_ActiveSSIDs[i].SSID;
        KeyValueTuple.Value1 = m_ActiveSSIDs[i].ADDRESS;
        KeyValueTuple.Value2 = String(m_ActiveSSIDs[i].RSSI);
        KeyValueTupleVector.push_back(KeyValueTuple);
      }
      if(0 < KeyValueTupleVector.size())
      {
        for(size_t i = 0; i < m_NumberOfWidgets; i++)
        {
          KeyValuePairs.push_back({ mp_WidgetId[i], Encode_SSID_Values_To_JSON(KeyValueTupleVector) });
        }
      }
    */
    }
    
    bool ProcessWebSocketValueAndSendToDatalink(String WidgetId, String Value) override
    {
      return false;
    }
  private:
    //Datalink
    std::vector<ActiveCompatibleDevice_t> m_ActiveSSIDs;
    String Encode_SSID_Values_To_JSON(std::vector<KVT> &KeyValueTuple)
    {
      JSONVar JSONVars;
      for(int i = 0; i < KeyValueTuple.size(); ++i)
      { 
        JSONVar SSIDValues;
        SSIDValues["SSID"] = KeyValueTuple[i].Key;
        SSIDValues["ADDRESS"] = KeyValueTuple[i].Value1;
        SSIDValues["RSSI"] = KeyValueTuple[i].Value2;
        JSONVars["SSIDValue" + String(i)] = SSIDValues;
      }
      String Result = JSON.stringify(JSONVars);
      return Result;
    }
};

class WebSocketSSIDDataHandler: public WebSocketDataHandler<String>
{
  public:
    WebSocketSSIDDataHandler( const String Name
                            , const std::initializer_list<const char*>& WidgetIds
                            , WebSocketDataProcessor &WebSocketDataProcessor
                            , const bool &IsReceiver
                            , const bool &IsSender
                            , NewRxTxValueCallerInterface<String> &NewRxTxValue
                            , const bool Debug )
                            : WebSocketDataHandler<String>( Name
                                                          , WidgetIds
                                                          , WebSocketDataProcessor
                                                          , IsReceiver
                                                          , IsSender
                                                          , NewRxTxValue
                                                          , Debug)
    {
    }
    
    virtual ~WebSocketSSIDDataHandler()
    {
    }
    
  protected:
  
    void CheckForNewDataLinkValueAndSendToWebSocket(std::vector<KVP> &KeyValuePairs) override
    {
      ESP_LOGE( "WebSocketDataHandler: CheckForNewDataLinkValueAndSendToWebSocket", "THIS IS NOT HANDLED YET");
    }

    bool ProcessWebSocketValueAndSendToDatalink(String WidgetId, String Value) override
    {
      ESP_LOGE( "WebSocketDataHandler: ProcessWebSocketValueAndSendToDatalink", "THIS IS NOT HANDLED YET");
      return false;
    }
};
#endif
