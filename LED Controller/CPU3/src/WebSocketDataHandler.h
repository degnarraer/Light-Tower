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
#pragma once

#include <freertos/portmacro.h>
#include "Streaming.h"
#include "Helpers.h"
#include "DataTypes.h"
#include "Tunes.h"
#include "DataItem/DataItems.h"
#include "ESPAsyncWebServer.h"
#include "AsyncTCP.h"
#include "Arduino_JSON.h"
#include <mutex>
#include <cstring>

#define MESSAGE_LENGTH 500

class SettingsWebServerManager;

class WebSocketDataHandlerSender
{
  public:
    virtual void HandleWebSocketDataRequest() = 0;
};


class WebSocketDataHandlerReceiver
{
  public:
    virtual void HandleWebSocketRxNotification(const String& stringValue) = 0;
    virtual String GetSignal() = 0;
};

class WebSocketDataProcessor
{
  public:
    WebSocketDataProcessor( AsyncWebServer &webServer
                          , AsyncWebSocket &webSocket )
                          : m_WebServer(webServer)
                          , m_WebSocket(webSocket)
    {
      xTaskCreatePinnedToCore( StaticWebSocketDataProcessor_WebSocket_TxTask,  "WebServer_Task",   10000,  this,  THREAD_PRIORITY_MEDIUM,    &m_WebSocketTaskHandle,    0 );
    }
    virtual ~WebSocketDataProcessor()
    {
      std::lock_guard<std::recursive_mutex> lock(m_Tx_KeyValues_Mutex);
      if(m_WebSocketTaskHandle) vTaskDelete(m_WebSocketTaskHandle);
    }
    void RegisterForWebSocketRxNotification(const String& name, WebSocketDataHandlerReceiver *aReceiver);
    void DeRegisterForWebSocketRxNotification(const String& name, WebSocketDataHandlerReceiver *aReceiver);
    void RegisterForWebSocketTxNotification(const String& name, WebSocketDataHandlerSender *aSender);
    void DeRegisterForWebSocketTxNotification(const String& name, WebSocketDataHandlerSender *aSender);
    bool ProcessSignalValueAndSendToDatalink(const String& signalId, const String& value);
    void UpdateAllDataToClient(uint8_t clientId);
    static void StaticWebSocketDataProcessor_WebSocket_TxTask(void * parameter);
    void TxDataToWebSocket(String key, String value)
    {
      std::lock_guard<std::recursive_mutex> lock(m_Tx_KeyValues_Mutex);
      KVP keyValuePair = {key, value};
      m_Tx_KeyValues.push_back(keyValuePair);
    }
  private:
    AsyncWebServer &m_WebServer;
    AsyncWebSocket &m_WebSocket;
    TaskHandle_t m_WebSocketTaskHandle;
    std::vector<WebSocketDataHandlerReceiver*> m_MyRxNotifyees = std::vector<WebSocketDataHandlerReceiver*>();
    std::vector<WebSocketDataHandlerSender*> m_MyTxNotifyees = std::vector<WebSocketDataHandlerSender*>();
    std::vector<KVP> m_Tx_KeyValues = std::vector<KVP>();
    std::recursive_mutex m_Tx_KeyValues_Mutex;
    void WebSocketDataProcessor_WebSocket_TxTask();
    void Encode_Signal_Values_To_JSON(std::vector<KVP> &signalValue, String &result);
    void NotifyClient(uint8_t clientID, const String& textString);
    void NotifyClients(const String& textString);

    template<typename T>
    std::vector<T>* AllocateVectorOnHeap(size_t count)
    {
        T* data = static_cast<T*>(malloc(sizeof(T) * count));
        if (data)
        {
            return new std::vector<T>(data, data + count);
        }
        return nullptr;
    }
};

template<typename T, size_t COUNT>
class WebSocketDataHandler: public WebSocketDataHandlerReceiver
                          , public WebSocketDataHandlerSender
                          , public Rx_Value_Callee_Interface<T>
                          , public DataTypeFunctions
{
  public:
    WebSocketDataHandler()
    {
    }
    WebSocketDataHandler( const WebSocketDataHandler &t )
                        : m_WebSocketDataProcessor(t.m_WebSocketDataProcessor)
                        , m_DataItem(t.m_DataItem)
                        , m_Debug(t.m_Debug)
                        , m_Name(t.m_Name)
                        , m_Signal(t.m_Signal)
    {
      m_WebSocketDataProcessor.RegisterForWebSocketRxNotification(m_Name, this);
      m_WebSocketDataProcessor.RegisterForWebSocketTxNotification(m_Name, this);
      m_DataItem.RegisterForNewRxValueNotification(this);
    }
    WebSocketDataHandler( WebSocketDataProcessor &webSocketDataProcessor
                        , LocalDataItem<T, COUNT> &dataItem
                        , const bool &debug )
                        : m_WebSocketDataProcessor(webSocketDataProcessor)
                        , m_DataItem(dataItem)
                        , m_Debug(debug)
                        , m_Name(dataItem.GetName() + " Web Socket")
                        , m_Signal(dataItem.GetName())
    {
      m_WebSocketDataProcessor.RegisterForWebSocketRxNotification(m_Name, this);
      m_WebSocketDataProcessor.RegisterForWebSocketTxNotification(m_Name, this);
      m_DataItem.RegisterForNewRxValueNotification(this);
    }
    
    virtual ~WebSocketDataHandler()
    {
      m_WebSocketDataProcessor.DeRegisterForWebSocketRxNotification(m_Name, this);
      m_WebSocketDataProcessor.DeRegisterForWebSocketTxNotification(m_Name, this);
      m_DataItem.DeRegisterForNewRxValueNotification(this);
    }

    bool NewRxValueReceived(const Rx_Value_Caller_Interface<T>* sender, const T* values, size_t changeCount) override
    {
      ESP_LOGD( "NewRxValueReceived", "New DataItem Rx Value");
      bool success = false;
      if(sender == &m_DataItem)
      {
          m_WebSocketDataProcessor.TxDataToWebSocket(m_Signal, m_DataItem.GetValueAsString());
          success = true;
          ESP_LOGI( "NewRxValueReceived", "\"%s\": New Verified DataItem Rx Value: Set to Web Socket", m_Signal.c_str());
      }
      else
      {
        ESP_LOGW( "NewRxValueReceived", "WARNING! Rx value from unknown sender");
      }
      return success;
    }
    
    String GetSignal()
    {
      return m_Signal;
    }

    virtual String GetName() const
    {
      return m_Name;
    }

    
    virtual void HandleWebSocketDataRequest() override
    {
      m_WebSocketDataProcessor.TxDataToWebSocket(m_Signal, m_DataItem.GetValueAsString());
    }
    
    virtual void HandleWebSocketRxNotification(const String& stringValue) override
    {
      ESP_LOGI( "Web Socket Rx"
              , "\"%s\" WebSocket Rx Signal: \"%s\" Value: \"%s\""
              , m_Name.c_str()
              , m_Signal.c_str()
              , stringValue.c_str());
      m_DataItem.SetValueFromString(stringValue);
    }

    void TxDataToWebSocket(String key, String value)
    {
      m_WebSocketDataProcessor.TxDataToWebSocket(key, value);
    }
  protected:
    WebSocketDataProcessor &m_WebSocketDataProcessor;
    LocalDataItem<T, COUNT> &m_DataItem;
    const bool &m_Debug;
    const String m_Name;
    const String m_Signal;
};

class WebSocket_String_DataHandler: public WebSocketDataHandler<char, DATAITEM_STRING_LENGTH>
{
  public:
    WebSocket_String_DataHandler( WebSocketDataProcessor &WebSocketDataProcessor
                                , LocalDataItem<char, DATAITEM_STRING_LENGTH> &DataItem
                                , const bool Debug )
                                : WebSocketDataHandler<char, DATAITEM_STRING_LENGTH>( WebSocketDataProcessor
                                                                                    , DataItem
                                                                                    , Debug)
    {
    }
    
    virtual ~WebSocket_String_DataHandler()
    {
    }
};
