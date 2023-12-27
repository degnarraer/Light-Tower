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

template<typename T>
class WebSocketDataHandler: public WebSocketDataHandlerReceiver
                          , public WebSocketDataHandlerSender
                          , public NewRxTxValueCalleeInterface<T>
{
  public:
    WebSocketDataHandler()
    {
    }
    WebSocketDataHandler( const WebSocketDataHandler &t )
                        : m_Name(t.m_Name)
                        , mp_WidgetId(t.mp_WidgetId)
                        , m_NumberOfWidgets(t.m_NumberOfWidgets)
                        , m_NewRxValue(t.m_NewRxValue)
                        , m_Value(t.m_Value)
                        , m_Debug(t.m_Debug)
    {
      m_NewRxValue.RegisterForNewValueNotification(this);
    }
    WebSocketDataHandler( const String &Name
                        , String *WidgetId
                        , const size_t NumberOfWidgets
                        , NewRxValueCallerInterface<T> &NewRxValue
                        , const bool &Debug )
                        : m_Name(Name)
                        , mp_WidgetId(WidgetId)
                        , m_NewRxValue(NewRxValue)
                        , m_NumberOfWidgets(NumberOfWidgets)
                        , m_Debug(Debug)
    {
      m_NewRxValue.RegisterForNewValueNotification(this);
    }
    
    virtual ~WebSocketDataHandler()
    {
      m_NewRxValue.DeRegisterForNewValueNotification(this);
    }
    
    void NewRxValueReceived(T* object)
    {
      SetValue(*object);
    }
    void SetNewTxValue(T* Object)
    {
      SetValue(*Object);
    }
    void SetValue(T Value)
    {
      m_Value = Value;
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
    String *mp_WidgetId;
    const size_t &m_NumberOfWidgets;
    T m_Value;
    T m_OldValue;
    NewRxValueCallerInterface<T> &m_NewRxValue;
    const bool &m_Debug;
    
    virtual void CheckForNewDataLinkValueAndSendToWebSocket(std::vector<KVP> &KeyValuePairs)
    {
      if(NULL != mp_WidgetId)
      {
        if(m_OldValue != m_Value)
        {
          for (size_t i = 0; i < m_NumberOfWidgets; i++)
          {
            if(true == m_Debug) Serial << "Sending " << String(m_Value) << " to Web Socket\n";
            KeyValuePairs.push_back({ mp_WidgetId[i], String(m_Value) });
          }
          m_OldValue = m_Value;
        }
      }
    }
    
    virtual bool ProcessWebSocketValueAndSendToDatalink(String WidgetId, String Value)
    {
      String InputId = WidgetId;
      bool Found = false;
      if(NULL != mp_WidgetId)
      {
        for (size_t i = 0; i < m_NumberOfWidgets; i++)
        {
          if( mp_WidgetId[i].equals(InputId) )
          {
            Found = true;
            // SEND TO DATALINK
          }
        }
      }
      return Found;
    }
};

class WebSocketSSIDArrayDataHandler: public WebSocketDataHandler<String>
{
  public:
    WebSocketSSIDArrayDataHandler( const String &Name 
                                 , String *WidgetIds
                                 , const size_t NumberOfWidgets
                                 , NewRxValueCallerInterface<String> &NewRxValue
                                 , const bool Debug )
                                 : WebSocketDataHandler<String>( Name
                                                               , WidgetIds
                                                               , NumberOfWidgets
                                                               , NewRxValue
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
                            , String *WidgetIds
                            , const size_t NumberOfWidgets
                            , NewRxValueCallerInterface<String> &NewRxValue
                            , const bool Debug )
                            : WebSocketDataHandler<String>( Name
                                                          , WidgetIds
                                                          , NumberOfWidgets
                                                          , NewRxValue
                                                          , Debug)
    {
    }
    
    virtual ~WebSocketSSIDDataHandler()
    {
    }
    
  protected:
  
    void CheckForNewDataLinkValueAndSendToWebSocket(std::vector<KVP> &KeyValuePairs) override
    {
      /*
      String ValueCopy;
      char Buffer[m_DataItem->TotalByteCount];
      if(NULL != m_DataItem && NULL != mp_WidgetId)
      {
        if(true == GetValueFromQueue(&Buffer, m_DataItem->QueueHandle_TX, m_DataItem->Name, m_ReadUntilEmpty, m_TicksToWait, m_PullError))
        {
          if(xSemaphoreTake(mySemaphore, portMAX_DELAY) == pdTRUE)
          {
            m_Value = String(Buffer);
            ValueCopy = m_Value;
            xSemaphoreGive(mySemaphore);
            for (size_t i = 0; i < m_NumberOfWidgets; i++)
            {
              if(true == m_Debug) Serial << m_DataItem->Name << " Sending " << ValueCopy << " to Web Socket\n";
              KeyValuePairs.push_back({ mp_WidgetId[i], ValueCopy });
            }
          }
        }
      }
      */
    }

    bool ProcessWebSocketValueAndSendToDatalink(String WidgetId, String Value) override
    {
      bool Found = false;
      String InputId = WidgetId;
      m_Value = Value;
      if(NULL != mp_WidgetId)
      {
        SSID_Info_t SSID_Info = SSID_Info_t(m_Value);
        for (size_t i = 0; i < m_NumberOfWidgets; i++)
        {
          mp_WidgetId[i].trim();
          InputId.trim();
          if( mp_WidgetId[i].equals(InputId) )
          {
            Found = true;
            if(true == m_Debug) Serial << " Sending " << m_Value << " to Web Socket\n";
            //PushValueToQueue(&SSID_Info, m_DataItem->QueueHandle_RX, m_DataItem->Name, 0, m_PushError);
          }
        }
      }
      return Found;
    }
};
#endif
