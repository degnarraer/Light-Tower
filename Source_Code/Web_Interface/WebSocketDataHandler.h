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
#include <Arduino_JSON.h>
#include "Tunes.h"

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
    WebSocketDataHandler()
    {
    }
    WebSocketDataHandler(const WebSocketDataHandler &t)
    {
      m_DataItem = t.m_DataItem;
      m_WidgetId = t.m_WidgetId;
      m_Value = t.m_Value;
      m_ReadUntilEmpty = t.m_ReadUntilEmpty;
      m_TicksToWait = t.m_TicksToWait;
      mySemaphore = t.mySemaphore;
    }
    WebSocketDataHandler( DataItem_t *DataItem
                        , String *WidgetId
                        , const size_t NumberOfWidgets
                        , bool ReadUntilEmpty
                        , TickType_t TicksToWait
                        , bool Debug )
                        : m_DataItem(DataItem) 
                        , m_WidgetId(WidgetId)
                        , m_NumberOfWidgets(NumberOfWidgets)
                        , m_ReadUntilEmpty(ReadUntilEmpty)
                        , m_TicksToWait(TicksToWait)
                        , m_Debug(Debug)
    {
      mySemaphore = xSemaphoreCreateRecursiveMutex();
      xSemaphoreGive(mySemaphore);
    }
    virtual ~WebSocketDataHandler()
    {
    }
    
    void SetValue(T Value)
    {
      if(xSemaphoreTake(mySemaphore, portMAX_DELAY) == pdTRUE)
      {
        m_Value = Value;
        xSemaphoreGive(mySemaphore);
      }
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
    UBaseType_t m_TaskPriority;
    uint8_t m_CoreId;
    bool m_PushError = false;
    bool m_PullError = false;
    bool m_Debug = false;
    SemaphoreHandle_t mySemaphore;
    
    virtual void CheckForNewDataLinkValueAndSendToWebSocket(std::vector<KVP> &KeyValuePairs)
    {
      if(NULL != m_DataItem && NULL != m_WidgetId)
      {
        if(xSemaphoreTake(mySemaphore, portMAX_DELAY) == pdTRUE)
        {
          bool ValueReceived = GetValueFromQueue(&m_Value, m_DataItem->QueueHandle_TX, m_DataItem->Name, m_ReadUntilEmpty, m_TicksToWait, m_PullError);
          T ValueCopy = m_Value;
          xSemaphoreGive(mySemaphore);
          
          if(true == ValueReceived)
          {
            for (size_t i = 0; i < m_NumberOfWidgets; i++)
            {
              if(true == m_Debug) Serial << m_DataItem->Name << " Sending " << String(ValueCopy) << " to Web Socket\n";
              KeyValuePairs.push_back({ m_WidgetId[i], String(ValueCopy) });
            }
          }
        }
      }
    }
    
    virtual bool ProcessWebSocketValueAndSendToDatalink(String WidgetId, String Value)
    {
      String InputId = WidgetId;
      bool Found = false;
      if(NULL != m_DataItem && NULL != m_WidgetId)
      {
        for (size_t i = 0; i < m_NumberOfWidgets; i++)
        {
          if( m_WidgetId[i].equals(InputId) )
          {
            Found = true;
            T ValueCopy;
            if( true == SetDataItemValueFromValueString(&ValueCopy, Value, m_DataItem->DataType) )
            {
              if(xSemaphoreTake(mySemaphore, portMAX_DELAY) == pdTRUE)
              {
                m_Value = ValueCopy;
                xSemaphoreGive(mySemaphore);
                if(true == m_Debug) Serial << m_DataItem->Name << " Sending " << String(ValueCopy) << " to Datalink\n";
                //PushValueToQueue(&ValueCopy, m_DataItem->QueueHandle_RX, m_DataItem->Name, 0, m_PushError);
              }
            }
          }
        }
      }
      return Found;
    }
};

class WebSocketSSIDArrayDataHandler: public WebSocketDataHandler<String>
{
  public:
    WebSocketSSIDArrayDataHandler( DataItem_t *DataItem
                                 , String *WidgetIds
                                 , const size_t NumberOfWidgets
                                 , bool ReadUntilEmpty
                                 , TickType_t TicksToWait
                                 , bool Debug )
                                 : WebSocketDataHandler<String>(DataItem, WidgetIds, NumberOfWidgets, ReadUntilEmpty, TicksToWait, Debug)
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
          KeyValuePairs.push_back({ m_WidgetId[i], Encode_SSID_Values_To_JSON(KeyValueTupleVector) });
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
    WebSocketSSIDDataHandler( DataItem_t *DataItem
                            , String *WidgetIds
                            , const size_t NumberOfWidgets
                            , bool ReadUntilEmpty
                            , TickType_t TicksToWait
                            , bool Debug )
                            : WebSocketDataHandler<String>(DataItem, WidgetIds, NumberOfWidgets, ReadUntilEmpty, TicksToWait, Debug)
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
      if(NULL != m_DataItem && NULL != m_WidgetId)
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
              KeyValuePairs.push_back({ m_WidgetId[i], ValueCopy });
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
      if(xSemaphoreTake(mySemaphore, portMAX_DELAY) == pdTRUE)
      {
        m_Value = Value;
        String ValueCopy = m_Value;
        xSemaphoreGive(mySemaphore);
        if(NULL != m_DataItem && NULL != m_WidgetId)
        {
          SSID_Info_t SSID_Info = SSID_Info_t(ValueCopy);
          for (size_t i = 0; i < m_NumberOfWidgets; i++)
          {
            m_WidgetId[i].trim();
            InputId.trim();
            if( m_WidgetId[i].equals(InputId) )
            {
              Found = true;
              if(true == m_Debug) Serial << m_DataItem->Name << " Sending " << ValueCopy << " to Web Socket\n";
              //PushValueToQueue(&SSID_Info, m_DataItem->QueueHandle_RX, m_DataItem->Name, 0, m_PushError);
            }
          }
        }
      }
      return Found;
    }
};
#endif
