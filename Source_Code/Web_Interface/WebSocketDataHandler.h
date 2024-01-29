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

#define MAX_VALUES_TO_SEND_AT_ONCE 50

class SettingsWebServerManager;
class WebSocketDataHandlerSender
{
  public:
    virtual void AppendCurrentValueToKVP(std::vector<KVP> *KeyValuePairs, bool forceUpdate = false) = 0;
};

class WebSocketDataHandlerReceiver
{
  public:
    virtual bool ProcessWebSocketValueAndSendToDatalink(const String& WidgetId, const String& Value) = 0;
};

class WebSocketDataProcessor
{
  public:
    WebSocketDataProcessor( AsyncWebSocket &WebSocket )
                          : m_WebSocket(WebSocket)
    {
      xTaskCreatePinnedToCore( StaticWebSocketDataProcessor_Task,  "WebServer_Task",   10000,  this,  THREAD_PRIORITY_MEDIUM,    &m_WebSocketTaskHandle,    0 );
    }
    virtual ~WebSocketDataProcessor()
    {
      if(m_WebSocketTaskHandle) vTaskDelete(m_WebSocketTaskHandle);
    }
    void RegisterAsWebSocketDataReceiver(const String& Name, WebSocketDataHandlerReceiver *aReceiver);
    void DeRegisterAsWebSocketDataReceiver(const String& Name, WebSocketDataHandlerReceiver *aReceiver);
    void RegisterAsWebSocketDataSender(const String& Name, WebSocketDataHandlerSender *aSender);
    void DeRegisterAsWebSocketDataSender(const String& Name, WebSocketDataHandlerSender *aSender);
    bool ProcessWebSocketValueAndSendToDatalink(const String& WidgetId, const String& Value);
    void UpdateAllDataToClient(uint8_t clientId);
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
    String Encode_Widget_Values_To_JSON(std::vector<KVP> *KeyValuePairs);
    void NotifyClient(uint8_t clientID, const String& TextString);
    void NotifyClients(const String& TextString);
    template<typename T>
    std::vector<T>* AllocateVectorOnHeap(size_t count)
    {
        T* data = static_cast<T*>(heap_caps_malloc(sizeof(T) * count, MALLOC_CAP_SPIRAM));
        if (data)
        {
            // Construct the vector with the allocated memory
            return new std::vector<T>(data, data + count);
        }
        return nullptr;  // Allocation failed
    }
};


template<typename T, size_t COUNT>
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
                        , m_DataItem(t.m_DataItem)
                        , m_Debug(t.m_Debug)
    {
      if(m_IsReceiver) m_WebSocketDataProcessor.RegisterAsWebSocketDataReceiver(m_Name, this);
      if(m_IsSender) m_WebSocketDataProcessor.RegisterAsWebSocketDataSender(m_Name, this);
    }
    WebSocketDataHandler( const String &Name
                        , const std::initializer_list<const char*>& WidgetIds
                        , WebSocketDataProcessor &WebSocketDataProcessor
                        , const bool &IsReceiver
                        , const bool &IsSender
                        , DataItem<T, COUNT> &DataItem
                        , const bool &Debug )
                        : m_Name(Name)
                        , m_WidgetIds(WidgetIds.begin(), WidgetIds.end())
                        , m_WebSocketDataProcessor(WebSocketDataProcessor)
                        , m_IsReceiver(IsReceiver)
                        , m_IsSender(IsSender)
                        , m_DataItem(DataItem)
                        , m_Debug(Debug)
    {
      if(m_IsReceiver) m_WebSocketDataProcessor.RegisterAsWebSocketDataReceiver(m_Name, this);
      if(m_IsSender) m_WebSocketDataProcessor.RegisterAsWebSocketDataSender(m_Name, this);
    }
    
    virtual ~WebSocketDataHandler()
    {
      if(m_IsReceiver) m_WebSocketDataProcessor.DeRegisterAsWebSocketDataReceiver(m_Name, this);
      if(m_IsSender) m_WebSocketDataProcessor.DeRegisterAsWebSocketDataSender(m_Name, this);
    }
    
    bool NewRxValueReceived(T* Object, size_t Count)
    {
      bool ValueChanged = false;
      if(Object && false == m_DataItem.EqualsValue(Object, Count))
      {
        m_DataItem.SetValue(Object, Count);
        ValueChanged = true;
        ESP_LOGD( "WebSocketDataHandler: NewRxValueReceived"
                , "New RX Datalink Value: \tValue: %s \tNew Value: %s"
                , m_DataItem.GetValueAsString(""));
      }
      return ValueChanged;
    }
    
    void SetNewTxValue(T* Object, size_t Count)
    {
      ESP_LOGE( "WebSocketDataHandler: SetNewTxValue", "THIS IS NOT HANDLED YET");
    }
    
    T GetValue()
    {
      T Value;
      m_DataItem.GetValue(&Value, COUNT);
      if(Value)
      {
        return Value;
      }
      else
      {
        return T();
      }
    }
    
    String GetName()
    {
      return m_Name;
    }
  private:
    uint64_t m_Last_Update_Time = millis();
  protected:
    const String m_Name;
    WebSocketDataProcessor &m_WebSocketDataProcessor;
    const bool &m_IsReceiver;
    const bool &m_IsSender;
    std::vector<String> m_WidgetIds;
    DataItem<T, COUNT> &m_DataItem;
    size_t m_OldChangeCount = 0;
    const bool &m_Debug;
    
    virtual void AppendCurrentValueToKVP(std::vector<KVP> *KeyValuePairs, bool forceUpdate = false)
    {
      T CurrentValue[COUNT];
      size_t newChangeCount = m_DataItem.GetValue(CurrentValue, COUNT);
      bool valueChanged = newChangeCount != m_OldChangeCount;
      m_OldChangeCount = newChangeCount;
      if( CurrentValue && (forceUpdate || valueChanged) )
      {
        String CurrentValueString = GetValueAsStringForDataType(CurrentValue, GetDataTypeFromTemplateType<T>(), COUNT, "");
        ESP_LOGD( "WebSocketDataHandler: AppendCurrentValueToKVP", "Pushing New Value \"%s\" to Web Socket", CurrentValueString.c_str());
        for (size_t i = 0; i < m_WidgetIds.size(); i++)
        {
          ESP_LOGD("WebSocketDataHandler: AppendCurrentValueToKVP", "Setting \"%s\" to Value \"%s\"", m_WidgetIds[i].c_str(), CurrentValueString.c_str());
          KeyValuePairs->push_back({ m_WidgetIds[i], CurrentValueString.c_str() });
        }
        m_Last_Update_Time = millis();
      }
    }
    
    virtual bool ProcessWebSocketValueAndSendToDatalink(const String& WidgetId, const String& StringValue)
    {
      bool Found = false;
      for (size_t i = 0; i < m_WidgetIds.size(); i++)
      {
        if( m_WidgetIds[i].equals(WidgetId) )
        {
          Found = true;
          ESP_LOGD( "WebSocketDataHandler: ProcessWebSocketValueAndSendToDatalink"
                  , "Widget ID[%i]: %s  WidgetId: %s"
                  , i , m_WidgetIds[i].c_str(), WidgetId.c_str() );
        }
      }
      if(Found)
      {
        T NewValue[COUNT];
        if (SetValueFromFromStringForDataType(NewValue, StringValue, GetDataTypeFromTemplateType<T>()))
        {
          if(NewValue)
          {
            m_DataItem.SetValue(NewValue, COUNT);
            String NewValueString = GetValueAsStringForDataType(NewValue, GetDataTypeFromTemplateType<T>(), COUNT, "");
            ESP_LOGD( "WebSocketDataHandler: ProcessWebSocketValueAndSendToDatalink"
                    , "Web Socket Value: %s"
                    , NewValueString.c_str());
          }
        }
      }
      return Found;
    }
};

class WebSocket_Compatible_Device_DataHandler: public WebSocketDataHandler<CompatibleDevice_t, 1>
{
  public:
    WebSocket_Compatible_Device_DataHandler( const String &Name 
                                           , const std::initializer_list<const char*>& WidgetIds
                                           , WebSocketDataProcessor &WebSocketDataProcessor
                                           , const bool &IsReceiver
                                           , const bool &IsSender
                                           , DataItem<CompatibleDevice_t, 1> &DataItem
                                           , const bool Debug )
                                           : WebSocketDataHandler<CompatibleDevice_t, 1>( Name
                                                                                        , WidgetIds
                                                                                        , WebSocketDataProcessor
                                                                                        , IsReceiver
                                                                                        , IsSender
                                                                                        , DataItem
                                                                                        , Debug)
    {
    }
    
    virtual ~WebSocket_Compatible_Device_DataHandler()
    {
    }
  protected:
  
    virtual void AppendCurrentValueToKVP(std::vector<KVP> *keyValuePairs, bool forceUpdate = false) override
    {
      std::vector<KVP> keyValuePairVector;
      KVP keyValuePair;
      CompatibleDevice_t currentValue;
      String result;

      size_t newChangeCount = m_DataItem.GetValue(&currentValue, 1);
      bool valueChanged = newChangeCount != this->m_OldChangeCount;
      this->m_OldChangeCount = newChangeCount;

      keyValuePair.Key = currentValue.address;
      keyValuePair.Value = currentValue.name;
      keyValuePairVector.push_back(keyValuePair);
      result = Encode_Compatible_Device_To_JSON(keyValuePairVector);
      if( forceUpdate || valueChanged )
      {
        ESP_LOGI("AppendCurrentValueToKVP", "Encoding Result: \"%s\"", result.c_str());
        for(size_t i = 0; i < m_WidgetIds.size(); i++)
        {
          keyValuePairs->push_back({ m_WidgetIds[i], result.c_str() });
        }
      }
    }
    
    virtual bool ProcessWebSocketValueAndSendToDatalink(const String& widgetId, const String& StringValue) override
    {
      return false;
    }
  private:
    String Encode_Compatible_Device_To_JSON(std::vector<KVP> &keyValuePair)
    {
      JSONVar jSONVars;
      for(int i = 0; i < keyValuePair.size(); ++i)
      { 
        JSONVar compatibleDeviceValues;
        compatibleDeviceValues["ADDRESS"] = keyValuePair[i].Key;
        compatibleDeviceValues["NAME"] = keyValuePair[i].Value;
        jSONVars["CompatibleDevice" + String(i)] = compatibleDeviceValues;
      }
      return JSON.stringify(jSONVars);
    }
};

class WebSocket_BT_Info_ArrayDataHandler: public WebSocketDataHandler<BT_Device_Info_With_Time_Since_Update_t, 1>
{
  public:
    WebSocket_BT_Info_ArrayDataHandler( const String &Name 
                                      , const std::initializer_list<const char*>& WidgetIds
                                      , WebSocketDataProcessor &WebSocketDataProcessor
                                      , const bool &IsReceiver
                                      , const bool &IsSender
                                      , DataItem<BT_Device_Info_With_Time_Since_Update_t, 1> &DataItem
                                      , const bool Debug )
                                      : WebSocketDataHandler<BT_Device_Info_With_Time_Since_Update_t, 1>( Name
                                                                                                     , WidgetIds
                                                                                                     , WebSocketDataProcessor
                                                                                                     , IsReceiver
                                                                                                     , IsSender
                                                                                                     , DataItem
                                                                                                     , Debug)
    {
    }
    
    virtual ~WebSocket_BT_Info_ArrayDataHandler()
    {
    }
  protected:
  
    virtual void AppendCurrentValueToKVP(std::vector<KVP> *KeyValuePairs, bool forceUpdate = false) override
    {   
      BT_Device_Info_With_Time_Since_Update_t CurrentValue;
      size_t newChangeCount = m_DataItem.GetValue(&CurrentValue, 1);
      bool valueChanged = newChangeCount != this->m_OldChangeCount;
      this->m_OldChangeCount = newChangeCount;
      bool found = false;
      bool updated = false;
      unsigned long currentMillis = millis();
      for(size_t i = 0; i < m_ActiveCompatibleDevices.size(); ++i)
      {
        unsigned long elapsedTime;
        unsigned long previousMillis = m_ActiveCompatibleDevices[i].lastUpdateTime;
        if( strcmp(m_ActiveCompatibleDevices[i].name, CurrentValue.name) == 0 &&
            strcmp(m_ActiveCompatibleDevices[i].address, CurrentValue.address) == 0 )
        {
          found = true;
          if(valueChanged)
          {
            ESP_LOGI("","%s: Device Name: %s Address: %s Value Changed: %i", this->m_Name.c_str(), m_ActiveCompatibleDevices[i].name, m_ActiveCompatibleDevices[i].address, newChangeCount);
            m_ActiveCompatibleDevices[i].lastUpdateTime = currentMillis;
            if(m_ActiveCompatibleDevices[i].rssi != CurrentValue.rssi)
            {
              m_ActiveCompatibleDevices[i].rssi = CurrentValue.rssi;
              updated = true;
            }
          }
        }
        if (currentMillis >= previousMillis) { elapsedTime = currentMillis - previousMillis; } 
        else { elapsedTime = (ULONG_MAX - previousMillis) + currentMillis + 1; }
        if(ACTIVE_NAME_TIMEOUT <= elapsedTime)
        {
          ESP_LOGI("WebSocketDataHandler", "Name Timedout: %s", m_ActiveCompatibleDevices[i].name);
          m_ActiveCompatibleDevices.erase(m_ActiveCompatibleDevices.begin()+i);
          --i;
          updated = true;
        }
      }
      if( !found &&
          valueChanged && 
          0 < String(CurrentValue.name).length() && 
          0 < String(CurrentValue.address).length() )
      {
        ESP_LOGI("","%s: New Device Name: %s Address: %s Value Changed: %i", this->m_Name.c_str(), CurrentValue.name, CurrentValue.address, newChangeCount);
        ActiveCompatibleDevice_t NewDevice = ActiveCompatibleDevice_t( CurrentValue.name
                                                                     , CurrentValue.address
                                                                     , CurrentValue.rssi
                                                                     , currentMillis );
        m_ActiveCompatibleDevices.push_back(NewDevice);
        updated = true;
      }

      if(forceUpdate || updated)
      {
        std::vector<KVT> KeyValueTupleVector;
        for(size_t i = 0; i < m_ActiveCompatibleDevices.size(); ++i)
        {
          KVT KeyValueTuple;
          KeyValueTuple.Key = m_ActiveCompatibleDevices[i].address;
          KeyValueTuple.Value1 = m_ActiveCompatibleDevices[i].name;
          KeyValueTuple.Value2 = String(m_ActiveCompatibleDevices[i].rssi).c_str();
          KeyValueTupleVector.push_back(KeyValueTuple);
        }
        if(0 < KeyValueTupleVector.size())
        {
          String result = Encode_SSID_Values_To_JSON(KeyValueTupleVector);
          ESP_LOGD("AppendCurrentValueToKVP", "Encoding Result: \"%s\"", result.c_str());
          for(size_t i = 0; i < m_WidgetIds.size(); i++)
          {
            KeyValuePairs->push_back({ m_WidgetIds[i], result.c_str() });
          }
        }
      }
    }
    
    virtual bool ProcessWebSocketValueAndSendToDatalink(const String& widgetId, const String& stringValue) override
    {
      return false;
    }
  private:
    //Datalink
    std::vector<ActiveCompatibleDevice_t> m_ActiveCompatibleDevices;
    String Encode_SSID_Values_To_JSON(std::vector<KVT> &KeyValueTuple)
    {
      JSONVar JSONVars;
      for(int i = 0; i < KeyValueTuple.size(); ++i)
      { 
        JSONVar CompatibleDeviceValues;
        CompatibleDeviceValues["ADDRESS"] = KeyValueTuple[i].Key;
        CompatibleDeviceValues["NAME"] = KeyValueTuple[i].Value1;
        CompatibleDeviceValues["RSSI"] = KeyValueTuple[i].Value2;
        JSONVars["ActiveCompatibleDevice" + String(i)] = CompatibleDeviceValues;
      }
      return JSON.stringify(JSONVars);
    }
};

#endif
