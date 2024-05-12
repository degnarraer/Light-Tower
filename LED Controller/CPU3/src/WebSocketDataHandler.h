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

#include "Arduino.h"
#include <freertos/portmacro.h>
#include "Streaming.h"
#include "Helpers.h"
#include "DataTypes.h"
#include "Tunes.h"
#include "DataItem.h"
#include "ESPAsyncWebServer.h"
#include "AsyncTCP.h"
#include "Arduino_JSON.h"
#include <mutex>

#define MESSAGE_LENGTH 500

class SettingsWebServerManager;
class WebSocketDataHandlerSender
{
  public:
    virtual void AppendCurrentValueToKVP(std::vector<KVP> *KeyValuePairs, bool forceUpdate = false) = 0;
};

class WebSocketDataHandlerReceiver
{
  public:
    virtual bool  ProcessSignalValueAndSendToDatalink(const String& SignalId, const String& stringValue) = 0;
};

class WebSocketDataProcessor
{
  public:
    WebSocketDataProcessor( AsyncWebServer &webServer
                          , AsyncWebSocket &webSocket )
                          : m_WebServer(webServer)
                          , m_WebSocket(webSocket)
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
    bool ProcessSignalValueAndSendToDatalink(const String& SignalId, const String& Value);
    void UpdateAllDataToClient(uint8_t clientId);
    void UpdateDataForSender(WebSocketDataHandlerSender* sender, bool forceUpdate);
    static void StaticWebSocketDataProcessor_Task(void * parameter);
  private:
    AsyncWebServer &m_WebServer;
    AsyncWebSocket &m_WebSocket;
    TaskHandle_t m_WebSocketTaskHandle;
    char m_Message[MESSAGE_LENGTH];
    std::vector<WebSocketDataHandlerReceiver*> m_MyReceivers = std::vector<WebSocketDataHandlerReceiver*>();
    std::vector<WebSocketDataHandlerSender*> m_MySenders = std::vector<WebSocketDataHandlerSender*>();
    void WebSocketDataProcessor_Task();
    void Encode_Signal_Values_To_JSON(std::vector<KVP> *KeyValuePairs, char *result);
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
                        , m_Signal(t.m_Signal)
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
                        , const String &Signal
                        , WebSocketDataProcessor &WebSocketDataProcessor
                        , const bool &IsReceiver
                        , const bool &IsSender
                        , LocalDataItem<T, COUNT> &DataItem
                        , const bool &Debug )
                        : m_Name(Name)
                        , m_Signal(Signal)
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

    virtual void AppendCurrentValueToKVP(std::vector<KVP> *KeyValuePairs, bool forceUpdate = false) override
    {
      T currentValue[COUNT];
      String currentValueString;
      if( (ValueChanged(currentValue) || forceUpdate) && m_DataItem.GetStringValue(currentValueString, "") )
      {
        ESP_LOGI( "WebSocketDataHandler: AppendCurrentValueToKVP", "\"%s\": Pushing New Value \"%s\" to Web Socket",m_DataItem.GetName().c_str(), currentValueString.c_str());
        KeyValuePairs->push_back({ m_Signal.c_str(), currentValueString.c_str() });
        m_Last_Update_Time = millis();
      }
    }
    
    virtual bool ProcessSignalValueAndSendToDatalink(const String& SignalId, const String& stringValue) override
    {
      bool found = m_Signal.equals(SignalId);      
      if(found)
      {
        T newValue[COUNT];
        if (SetValueFromStringForDataType(newValue, stringValue, GetDataTypeFromTemplateType<T>()))
        {
          m_DataItem.SetValue(newValue, COUNT);
          String newValueString = GetValueAsStringForDataType(newValue, GetDataTypeFromTemplateType<T>(), COUNT, "");
          ESP_LOGD( "WebSocketDataHandler: ProcessSignalValueAndSendToDatalink"
                  , "Web Socket Value: %s"
                  , newValueString.c_str());
        }
      }
      return found;
    }
  protected:
    const String m_Name;
    const String &m_Signal;
    WebSocketDataProcessor &m_WebSocketDataProcessor;
    const bool &m_IsReceiver;
    const bool &m_IsSender;
    LocalDataItem<T, COUNT> &m_DataItem;
    const bool &m_Debug;
    size_t m_ChangeCount = 0;

    bool ValueChanged(void* object)
    {
      size_t newChangeCount = m_DataItem.GetValue(object, COUNT);
      bool valueChanged = newChangeCount != m_ChangeCount;
      m_ChangeCount = newChangeCount;
      return valueChanged;
    }
  private:
    uint64_t m_Last_Update_Time = millis();
};

class WebSocket_String_DataHandler: public WebSocketDataHandler<char, DATAITEM_STRING_LENGTH>
{
  public:
    WebSocket_String_DataHandler( const String &Name 
                                , const String &signalIds
                                , WebSocketDataProcessor &WebSocketDataProcessor
                                , const bool &IsReceiver
                                , const bool &IsSender
                                , LocalDataItem<char, DATAITEM_STRING_LENGTH> &DataItem
                                , const bool Debug )
                                : WebSocketDataHandler<char, DATAITEM_STRING_LENGTH>( Name
                                                                                    , signalIds
                                                                                    , WebSocketDataProcessor
                                                                                    , IsReceiver
                                                                                    , IsSender
                                                                                    , DataItem
                                                                                    , Debug)
    {
    }
    
    virtual ~WebSocket_String_DataHandler()
    {
    }
  protected:
    virtual void AppendCurrentValueToKVP(std::vector<KVP> *keyValuePairs, bool forceUpdate = false) override
    { 
      char currentValue[DATAITEM_STRING_LENGTH];
      String CurrentValueString;// = m_DataItem.GetValueAsString("");
      if( (ValueChanged(currentValue) || forceUpdate) && CurrentValueString.length() > 0 )
      {
        ESP_LOGI( "WebSocket_Compatible_Device_DataHandler: AppendCurrentValueToKVP", "\"%s\": Pushing New Value \"%s\" to Web Socket",m_DataItem.GetName().c_str(), CurrentValueString.c_str());
        keyValuePairs->push_back({ m_Signal, currentValue });
      }
    }
    
    virtual bool ProcessSignalValueAndSendToDatalink(const String& signalId, const String& stringValue) override
    {
      bool found = false;
      if(signalId.equals(m_DataItem.GetName()))
      {
        found = true;
        m_DataItem.SetValue(stringValue.c_str(), DATAITEM_STRING_LENGTH);
      }
      return found;
    }
  private:
    //Datalink

};

class WebSocket_Compatible_Device_DataHandler: public WebSocketDataHandler<CompatibleDevice_t, 1>
{
  public:
    WebSocket_Compatible_Device_DataHandler( const String &Name 
                                           , const String &Signal
                                           , WebSocketDataProcessor &WebSocketDataProcessor
                                           , const bool &IsReceiver
                                           , const bool &IsSender
                                           , DataItem<CompatibleDevice_t, 1> &DataItem
                                           , const bool Debug )
                                           : WebSocketDataHandler<CompatibleDevice_t, 1>( Name
                                                                                        , Signal
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
      
      size_t newChangeCount = m_DataItem.GetValue(&currentValue, 1);
      bool valueChanged = newChangeCount != this->m_ChangeCount;
      this->m_ChangeCount = newChangeCount;

      keyValuePair.Key = currentValue.address;
      keyValuePair.Value = currentValue.name;
      keyValuePairVector.push_back(keyValuePair);
      if(keyValuePairVector.size())
      {
        String result; // = Encode_Compatible_Device_To_JSON(keyValuePairVector);
        if( (forceUpdate || valueChanged) && result.length() > 0 )
        {
          String CurrentValueString;// = m_DataItem.GetValueAsString("");
          ESP_LOGI( "WebSocket_Compatible_Device_DataHandler: AppendCurrentValueToKVP", "\"%s\": Pushing New Value \"%s\" to Web Socket",m_DataItem.GetName().c_str(), CurrentValueString.c_str());
          keyValuePairs->push_back({ m_Signal, result.c_str() });
        }
      }
    }
    
    bool ProcessSignalValueAndSendToDatalink(const String& SignalId, const String& stringValue) override
    {
      bool found = m_Signal.equals(SignalId);
      if(found)
      {
        ESP_LOGI("WebSocket_Compatible_Device_DataHandler: ProcessSignalValueAndSendToDatalink", "New Web Socket Value for \"%s\": \"%s\"", SignalId.c_str(), stringValue.c_str());
        JSONVar jSONObject = JSON.parse(stringValue);
        if (JSON.typeof(jSONObject) == "undefined")
        {
          ESP_LOGE("WebSocket_Compatible_Device_DataHandler: ProcessSignalValueAndSendToDatalink", "Error parsing JSON");
          return false;
        }
        String name = jSONObject["Name"];
        String address = jSONObject["Address"];
        CompatibleDevice_t newValue(name.c_str(), address.c_str());
        m_DataItem.SetValue(&newValue, 1);
      }
      return found;
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

class WebSocket_ActiveCompatibleDevice_ArrayDataHandler: public WebSocketDataHandler<ActiveCompatibleDevice_t, 1>
{
  public:
    WebSocket_ActiveCompatibleDevice_ArrayDataHandler( const String &Name 
                                                     , const String &Signal
                                                     , WebSocketDataProcessor &WebSocketDataProcessor
                                                     , const bool &IsReceiver
                                                     , const bool &IsSender
                                                     , DataItem<ActiveCompatibleDevice_t, 1> &DataItem
                                                     , const bool Debug )
                                                     : WebSocketDataHandler<ActiveCompatibleDevice_t, 1>( Name
                                                                                                        , Signal
                                                                                                        , WebSocketDataProcessor
                                                                                                        , IsReceiver
                                                                                                        , IsSender
                                                                                                        , DataItem
                                                                                                        , Debug)
    {
    }
    
    virtual ~WebSocket_ActiveCompatibleDevice_ArrayDataHandler()
    {
    }
  protected:
  
    virtual void AppendCurrentValueToKVP(std::vector<KVP> *KeyValuePairs, bool forceUpdate = false) override
    {   
      ActiveCompatibleDevice_t activeCompatibleDevice;
      bool valueChanged = false; // = ValueChanged(&activeCompatibleDevice);
      bool found = false;
      bool updated = false;
      unsigned long currentMillis = millis();
      for(int i = 0; false; ++i ) //size_t i = 0; i < m_ActiveCompatibleDevices.size(); ++i)
      {
        unsigned long elapsedTime;
        unsigned long previousMillis = m_ActiveCompatibleDevices[i].lastUpdateTime;
        m_ActiveCompatibleDevices[i].lastUpdateTime = currentMillis;
        if( strcmp(m_ActiveCompatibleDevices[i].address, activeCompatibleDevice.address) == 0 )
        {
          found = true;
          if(valueChanged)
          {
            if(strcmp(m_ActiveCompatibleDevices[i].name, activeCompatibleDevice.name) == 0)
            {
              strcpy(m_ActiveCompatibleDevices[i].name, activeCompatibleDevice.name);
              updated = true;
            }
            if(m_ActiveCompatibleDevices[i].rssi != activeCompatibleDevice.rssi)
            {
              m_ActiveCompatibleDevices[i].rssi = activeCompatibleDevice.rssi;
              updated = true;
            }
            if(m_ActiveCompatibleDevices[i].timeSinceUpdate != activeCompatibleDevice.timeSinceUpdate)
            {
              m_ActiveCompatibleDevices[i].timeSinceUpdate = activeCompatibleDevice.timeSinceUpdate;
              updated = true;
            }
            ESP_LOGI( "WebSocket_ActiveCompatibleDevice_ArrayDataHandler: AppendCurrentValueToKVP","%s: \n************* \nDevice Name: %s \nAddress: %s \nRSSI: %i \nUpdate Time: %lu \nTime Since Update: %lu \nChange Count: %i"
                    , this->m_Name.c_str()
                    , m_ActiveCompatibleDevices[i].name
                    , m_ActiveCompatibleDevices[i].address
                    , m_ActiveCompatibleDevices[i].rssi
                    , m_ActiveCompatibleDevices[i].lastUpdateTime
                    , m_ActiveCompatibleDevices[i].timeSinceUpdate
                    , m_ChangeCount );
          }
          else
          {
            if (currentMillis >= previousMillis) { elapsedTime = currentMillis - previousMillis; } 
            else { elapsedTime = (ULONG_MAX - previousMillis) + currentMillis + 1; }
            m_ActiveCompatibleDevices[i].timeSinceUpdate += elapsedTime;
          }
        }
        else
        {
          if (currentMillis >= previousMillis) { elapsedTime = currentMillis - previousMillis; } 
          else { elapsedTime = (ULONG_MAX - previousMillis) + currentMillis + 1; }
          m_ActiveCompatibleDevices[i].timeSinceUpdate += elapsedTime;
        }
        if(ACTIVE_NAME_TIMEOUT < m_ActiveCompatibleDevices[i].timeSinceUpdate)
        {
          ESP_LOGI("WebSocket_ActiveCompatibleDevice_ArrayDataHandler: AppendCurrentValueToKVP", "Name Timedout: %s", m_ActiveCompatibleDevices[i].name);
          m_ActiveCompatibleDevices.erase(m_ActiveCompatibleDevices.begin()+i);
          --i;
          updated = true;
        }
      }
      if( !found &&
          valueChanged && 
          0 < String(activeCompatibleDevice.name).length() && 
          0 < String(activeCompatibleDevice.address).length() &&
          ACTIVE_NAME_TIMEOUT >= activeCompatibleDevice.timeSinceUpdate + ACTIVE_NAME_TIMEOUT/2)
      {
        ESP_LOGI( "WebSocket_ActiveCompatibleDevice_ArrayDataHandler: AppendCurrentValueToKVP","%s: \n************* \nNew Device Name: %s \nAddress: %s \nRSSI: %i \nUpdate Time: %lu \nTime Since Update: %lu \nChange Count: %i"
                , this->m_Name.c_str()
                , activeCompatibleDevice.name
                , activeCompatibleDevice.address
                , activeCompatibleDevice.rssi
                , activeCompatibleDevice.lastUpdateTime
                , activeCompatibleDevice.timeSinceUpdate
                , m_ChangeCount);
        ActiveCompatibleDevice_t NewDevice = ActiveCompatibleDevice_t( activeCompatibleDevice.name
                                                                     , activeCompatibleDevice.address
                                                                     , activeCompatibleDevice.rssi
                                                                     , currentMillis
                                                                     , activeCompatibleDevice.timeSinceUpdate );
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
        String result; // = Encode_SSID_Values_To_JSON(KeyValueTupleVector);
        ESP_LOGI("WebSocket_ActiveCompatibleDevice_ArrayDataHandler: AppendCurrentValueToKVP", "Encoding Result: \"%s\"", result.c_str());
        if(result.length() > 0)
        {
          KeyValuePairs->push_back({ m_Signal, result.c_str() });
        }
      }
    }
    
    virtual bool ProcessSignalValueAndSendToDatalink(const String& SignalId, const String& stringValue) override
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
