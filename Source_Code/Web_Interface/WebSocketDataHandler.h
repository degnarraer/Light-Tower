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
      xSemaphoreTake(mySemaphore, portMAX_DELAY);
      m_Value = Value;
      xSemaphoreGive(mySemaphore);
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
        xSemaphoreTake(mySemaphore, portMAX_DELAY);
        bool ValueReceived = GetValueFromQueue(&m_Value, m_DataItem->QueueHandle_TX, m_DataItem->Name.c_str(), m_ReadUntilEmpty, m_TicksToWait, m_PullError);
        T ValueCopy = m_Value;
        xSemaphoreGive(mySemaphore);
        
        if(true == ValueReceived)
        {
          for (size_t i = 0; i < m_NumberOfWidgets; i++)
          {
            if(true == m_Debug) Serial << m_DataItem->Name.c_str() << " Sending " << String(ValueCopy).c_str() << " to Web Socket\n";
            KeyValuePairs.push_back({ m_WidgetId[i].c_str(), String(ValueCopy).c_str() });
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
        T ValueCopy;
        bool ConversionSuccesful = ConvertStringToDataBufferFromDataType(&ValueCopy, Value, m_DataItem->DataType);
        xSemaphoreTake(mySemaphore, portMAX_DELAY);
        m_Value = ValueCopy;
        xSemaphoreGive(mySemaphore);
        if( true == ConversionSuccesful )
        {
          for (size_t i = 0; i < m_NumberOfWidgets; i++)
          {
            if( m_WidgetId[i].equals(InputId) )
            {
                Found = true;
                if(true == m_Debug) Serial << m_DataItem->Name.c_str() << " Sending " << String(ValueCopy).c_str() << " to Datalink\n";
                PushValueToQueue(&ValueCopy, m_DataItem->QueueHandle_RX, m_DataItem->Name.c_str(), 0, m_PushError);
            }
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
      String ValueCopy;
      char Buffer[m_DataItem->TotalByteCount];
      if(NULL != m_DataItem && NULL != m_WidgetId)
      {
        if(true == GetValueFromQueue(&Buffer, m_DataItem->QueueHandle_TX, m_DataItem->Name.c_str(), m_ReadUntilEmpty, m_TicksToWait, m_PullError))
        {
          xSemaphoreTake(mySemaphore, portMAX_DELAY);
          m_Value = String(Buffer);
          ValueCopy = m_Value;
          xSemaphoreGive(mySemaphore);
          for (size_t i = 0; i < m_NumberOfWidgets; i++)
          {
            if(true == m_Debug) Serial << m_DataItem->Name.c_str() << " Sending " << ValueCopy.c_str() << " to Web Socket\n";
            KeyValuePairs.push_back({ m_WidgetId[i].c_str(), ValueCopy.c_str() });
          }
        }
      }
    }
    
    bool ProcessWebSocketValueAndSendToDatalink(String WidgetId, String Value) override
    {
      bool Found = false;
      String InputId = WidgetId;
      xSemaphoreTake(mySemaphore, portMAX_DELAY);
      m_Value = Value;
      String ValueCopy = m_Value;
      xSemaphoreGive(mySemaphore);
      
      if(NULL != m_DataItem && NULL != m_WidgetId)
      {
        Wifi_Info_t WifiInfo = Wifi_Info_t(ValueCopy);
        for (size_t i = 0; i < m_NumberOfWidgets; i++)
        {
          m_WidgetId[i].trim();
          InputId.trim();
          if( m_WidgetId[i].equals(InputId) )
          {
            Found = true;
            if(true == m_Debug) Serial << m_DataItem->Name.c_str() << " Sending " << ValueCopy.c_str() << " to Web Socket\n";
            PushValueToQueue(&WifiInfo, m_DataItem->QueueHandle_RX, m_DataItem->Name.c_str(), 0, m_PushError);
          }
        }
      }
      return Found;
    }
};
#endif
