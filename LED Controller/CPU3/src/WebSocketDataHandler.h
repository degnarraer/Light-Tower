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
#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include "Streaming.h"
#include "Helpers.h"
#include "DataTypes.h"
#include "Tunes.h"
#include "DataItem/DataItems.h"
#include "Arduino_JSON.h"
#include "freertos/semphr.h"
#include <cstring>

#define MESSAGE_LENGTH 500
#define BLUETOOTH_DEVICE_TIMEOUT 10000
#define BT_SCANNED_DEVICE_WEB_SOCKET_UPDATE_INTERVAL 1000

class SettingsWebServerManager;

class WebSocketDataHandlerSender
{
  public:
    virtual KVP HandleWebSocketDataRequest() = 0;
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
    WebSocketDataProcessor( WebServer &webServer
                          , WebSocketsServer &webSocket )
                          : m_WebServer(webServer)
                          , m_WebSocket(webSocket)
    {
      m_Tx_KeyValues_Semaphore = xSemaphoreCreateMutex();
      if (m_Tx_KeyValues_Semaphore == nullptr)
      {
          ESP_LOGE("WebSocketDataProcessor", "ERROR! Failed to create semaphore.");
      }
      xTaskCreatePinnedToCore( StaticWebSocketDataProcessor_WebSocket_TxTask,  "WebServer_Task",   10000,  this,  THREAD_PRIORITY_MEDIUM,    &m_WebSocketTaskHandle,    0 );
    }
    virtual ~WebSocketDataProcessor()
    {
      if(m_WebSocketTaskHandle)
      {
        vTaskDelete(m_WebSocketTaskHandle);
        m_WebSocketTaskHandle = nullptr;
      }
      if (m_Tx_KeyValues_Semaphore)
      {
          vSemaphoreDelete(m_Tx_KeyValues_Semaphore);
          m_Tx_KeyValues_Semaphore = nullptr;
      }
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
      if (xSemaphoreTakeRecursive(m_Tx_KeyValues_Semaphore, pdMS_TO_TICKS(5)) == pdTRUE)
      {
        KVP keyValuePair = {key, value};
        m_Tx_KeyValues.push_back(keyValuePair);
        xSemaphoreGiveRecursive(m_Tx_KeyValues_Semaphore);
      }
      else
      {
          ESP_LOGW("Semaphore Take Failure", "WARNING! Failed to take Semaphore");
      }
    }
  private:
    WebServer &m_WebServer;
    WebSocketsServer &m_WebSocket;
    TaskHandle_t m_WebSocketTaskHandle;
    std::vector<WebSocketDataHandlerReceiver*> m_MyRxNotifyees = std::vector<WebSocketDataHandlerReceiver*>();
    std::vector<WebSocketDataHandlerSender*> m_MyTxNotifyees = std::vector<WebSocketDataHandlerSender*>();
    std::vector<KVP> m_Tx_KeyValues = std::vector<KVP>();
    SemaphoreHandle_t m_Tx_KeyValues_Semaphore;
    void WebSocketDataProcessor_WebSocket_TxTask();
    void Encode_Signal_Values_To_JSON(const std::vector<KVP> &signalValue, String &result);
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
                        , m_Name(t.m_Name)
                        , m_Signal(t.m_Signal)
    {
      m_WebSocketDataProcessor.RegisterForWebSocketRxNotification(m_Name, this);
      m_WebSocketDataProcessor.RegisterForWebSocketTxNotification(m_Name, this);
      auto dataItemPtr = dynamic_cast<DataItem<T,COUNT>*>(&m_DataItem);
      if (dataItemPtr)
      {
        dataItemPtr->RegisterForNewRxValueNotification(this);
      }
    }
    WebSocketDataHandler( WebSocketDataProcessor &webSocketDataProcessor
                        , LocalDataItem<T, COUNT> &dataItem )
                        : m_WebSocketDataProcessor(webSocketDataProcessor)
                        , m_DataItem(dataItem)
                        , m_Name(dataItem.GetName() + " Web Socket")
                        , m_Signal(dataItem.GetName())
    {
      m_WebSocketDataProcessor.RegisterForWebSocketRxNotification(m_Name, this);
      m_WebSocketDataProcessor.RegisterForWebSocketTxNotification(m_Name, this);
      auto dataItemPtr = dynamic_cast<DataItem<T,COUNT>*>(&m_DataItem);
      if (dataItemPtr)
      {
        dataItemPtr->RegisterForNewRxValueNotification(this);
      }
    }
    
    virtual ~WebSocketDataHandler()
    {
      m_WebSocketDataProcessor.DeRegisterForWebSocketRxNotification(m_Name, this);
      m_WebSocketDataProcessor.DeRegisterForWebSocketTxNotification(m_Name, this);
      auto dataItemPtr = dynamic_cast<DataItem<T,COUNT>*>(&m_DataItem);
      if (dataItemPtr)
      {
        dataItemPtr->DeRegisterForNewRxValueNotification(this);
      }
    }

    virtual bool NewRxValueReceived(const T* values, size_t count, size_t changeCount) override
    {
      ESP_LOGD( "NewRxValueReceived", "New DataItem Rx Value");
      bool success = false;
      if(IsChangeCountGreater(m_DataItem.GetChangeCount()))
      {
        m_WebSocketDataProcessor.TxDataToWebSocket(m_Signal, m_DataItem.GetValueAsString());
        success = true;
        ESP_LOGD( "NewRxValueReceived", "\"%s\": New Verified DataItem Rx Value: Sent to Web Socket", m_Signal.c_str());
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

    
    virtual KVP HandleWebSocketDataRequest() override
    {
      m_ChangeCount = m_DataItem.GetChangeCount();
      return { m_Signal, m_DataItem.GetValueAsString() };
    }
    
    virtual void HandleWebSocketRxNotification(const String& stringValue) override
    {
      ESP_LOGD( "Web Socket Rx"
              , "\"%s\" WebSocket Rx Signal: \"%s\" Value: \"%s\""
              , m_Name.c_str()
              , m_Signal.c_str()
              , stringValue.c_str());
      m_DataItem.SetValueFromString(stringValue);
      m_ChangeCount = m_DataItem.GetChangeCount();
    }

    void TxDataToWebSocket(String key, String value)
    {
      ESP_LOGD("TxDataToWebSocket", "Key: \"%s\" Value: \"%s\"", key.c_str(), value.c_str());
      m_WebSocketDataProcessor.TxDataToWebSocket(key, value);
      m_ChangeCount = m_DataItem.GetChangeCount();
    }

  protected:
    WebSocketDataProcessor &m_WebSocketDataProcessor;
    LocalDataItem<T, COUNT> &m_DataItem;
    size_t m_ChangeCount;
    const String m_Name;
    const String m_Signal;
		bool IsChangeCountGreater(size_t changeCount)
		{
			return (changeCount > m_ChangeCount) || (m_ChangeCount - changeCount > (SIZE_MAX / 2));
		}
};

class WebSocket_String_DataHandler: public WebSocketDataHandler<char, DATAITEM_STRING_LENGTH>
{
  public:
    WebSocket_String_DataHandler( WebSocketDataProcessor &WebSocketDataProcessor
                                , LocalDataItem<char, DATAITEM_STRING_LENGTH> &DataItem )
                                : WebSocketDataHandler<char, DATAITEM_STRING_LENGTH>( WebSocketDataProcessor
                                                                                    , DataItem )
    {
    }
    
    virtual ~WebSocket_String_DataHandler()
    {
    }
};


class BT_Device_Info_With_Time_Since_Update_WebSocket_DataHandler: public WebSocketDataHandler<BT_Device_Info_With_Time_Since_Update, 1>
{
  public:
    BT_Device_Info_With_Time_Since_Update_WebSocket_DataHandler( WebSocketDataProcessor &WebSocketDataProcessor
                                                               , LocalDataItem<BT_Device_Info_With_Time_Since_Update, 1> &DataItem )
                                                               : WebSocketDataHandler<BT_Device_Info_With_Time_Since_Update, 1>( WebSocketDataProcessor
                                                                                                                               , DataItem )
    {
      m_ActiveDevicesSemaphore = xSemaphoreCreateMutex();
      if (m_ActiveDevicesSemaphore == nullptr)
      {
          ESP_LOGE("WebSocketDataProcessor", "ERROR! Failed to create semaphore.");
      }
    }
    
    virtual ~BT_Device_Info_With_Time_Since_Update_WebSocket_DataHandler()
    {
      StopTrackingDevices();
      if (m_ActiveDevicesSemaphore != nullptr)
      {
          vSemaphoreDelete(m_ActiveDevicesSemaphore);
          m_ActiveDevicesSemaphore = nullptr;
      }
    }

    bool NewRxValueReceived(const BT_Device_Info_With_Time_Since_Update* values, size_t count, size_t changeCount) override
    {
      ESP_LOGD( "NewRxValueReceived", "New DataItem Rx Value");
      bool success = false;
      if(this->IsChangeCountGreater(m_DataItem.GetChangeCount()))
      {
        for(size_t i = 0; i < count; ++i)
        {
          ActiveCompatibleDeviceReceived(values[i]);
        }
        success = true;
      }
      m_ChangeCount = m_DataItem.GetChangeCount();
      return success;
    }

    void StartTrackingDevices()
    {
      CreateUpdateTask();
    }

    void StopTrackingDevices()
    {
      DestroyUpdateTask();
    }

  private:
    void CreateUpdateTask()
    {
      if(!m_ActiveDeviceUpdateTask)
      {
        if( xTaskCreatePinnedToCore( Static_UpdateActiveCompatibleDevices, "Update Active Devices", 5000,  this, THREAD_PRIORITY_MEDIUM, &m_ActiveDeviceUpdateTask, 0 ) == pdTRUE )
        {
          ESP_LOGI("CreateUpdateTask", "Started Device Tracking Task");
        }
        else
        {
          ESP_LOGE("CreateUpdateTask", "ERROR! Unable to create task.");
        }
      }
    }

    void DestroyUpdateTask()
    {
      if(m_ActiveDeviceUpdateTask)
      {
        vTaskDelete(m_ActiveDeviceUpdateTask);
        m_ActiveDeviceUpdateTask = nullptr;
      }
    }

    void ActiveCompatibleDeviceReceived(const BT_Device_Info_With_Time_Since_Update &device)
    {
      if (xSemaphoreTakeRecursive(m_ActiveDevicesSemaphore, pdMS_TO_TICKS(5)) == pdTRUE)
      {
        ActiveBluetoothDevice_t newdevice( device.name
                                          , device.address
                                          , device.rssi
                                          , millis()
                                          , device.timeSinceUpdate );
        auto it = std::find(m_ActiveDevices.begin(), m_ActiveDevices.end(), newdevice);
        if (it != m_ActiveDevices.end())
        {
          *it = newdevice;
          ESP_LOGD("ScannedDevice_ValueChanged", "Device Updated: \"%s\"", device.toString().c_str());
        }
        else 
        {
          if(device.timeSinceUpdate < BLUETOOTH_DEVICE_TIMEOUT)
          {
            ESP_LOGI("ScannedDevice_ValueChanged", "New Device: \"%s\"", device.toString().c_str());
            m_ActiveDevices.push_back(newdevice);
          }
        }
        xSemaphoreGiveRecursive(m_ActiveDevicesSemaphore);
      }
      else
      {
          ESP_LOGW("Semaphore Take Failure", "WARNING! Failed to take Semaphore");
      }
    }

    static void Static_UpdateActiveCompatibleDevices(void * parameter)
    {
      BT_Device_Info_With_Time_Since_Update_WebSocket_DataHandler *handler = (BT_Device_Info_With_Time_Since_Update_WebSocket_DataHandler*)parameter;
      const TickType_t xFrequency = 1000;
      TickType_t xLastWakeTime = xTaskGetTickCount();
      unsigned long lastWebSocketUpdateTime = 0;
      while(true)
      {
        vTaskDelayUntil( &xLastWakeTime, xFrequency );
        handler->CleanActiveCompatibleDevices();
        unsigned long currentTime = millis();
        if(currentTime - lastWebSocketUpdateTime >= BT_SCANNED_DEVICE_WEB_SOCKET_UPDATE_INTERVAL)
        {
          handler->SendActiveCompatibleDevicesToWebSocket();
        }
      }
    }

    void CleanActiveCompatibleDevices()
    {
      if (xSemaphoreTakeRecursive(m_ActiveDevicesSemaphore, pdMS_TO_TICKS(5)) == pdTRUE)
      {
        ESP_LOGV("CleanActiveCompatibleDevices", "Cleaning Stale Devices.");
        for (auto it = m_ActiveDevices.begin(); it != m_ActiveDevices.end();)
        {
            ActiveBluetoothDevice_t& device = *it;
            if (device.timeSinceUpdate > BLUETOOTH_DEVICE_TIMEOUT || 
                millis() - device.lastUpdateTime > BLUETOOTH_DEVICE_TIMEOUT)
            {
                ESP_LOGI("CleanActiveCompatibleDevices", "Removing Device: \"%s\"", device.toString().c_str());
                it = m_ActiveDevices.erase(it);
            }
            else
            {
                ++it;
            }
        }
        xSemaphoreGiveRecursive(m_ActiveDevicesSemaphore);
      }
      else
      {
          ESP_LOGW("Semaphore Take Failure", "WARNING! Failed to take Semaphore");
      }
    }

    void SendActiveCompatibleDevicesToWebSocket()
    {
        const size_t preAllocSize = 1024;
        std::string jsonString;
        jsonString.reserve(preAllocSize);
        if (xSemaphoreTakeRecursive(m_ActiveDevicesSemaphore, pdMS_TO_TICKS(5)) == pdTRUE)
        {
          std::vector<ActiveBluetoothDevice_t> tempVector = m_ActiveDevices;
          jsonString += "{\"Devices\":[";
          bool firstDevice = true;
          for (const auto& device : tempVector)
          {
              if (!firstDevice)
              {
                  jsonString += ",";
              }
              firstDevice = false;
              jsonString += "{";
              jsonString += "\"Name\":\"" + std::string(device.name) + "\",";
              jsonString += "\"Address\":\"" + std::string(device.address) + "\",";
              jsonString += "\"RSSI\":\"" + std::to_string(device.rssi) + "\"";
              jsonString += "}";
          }
          jsonString += "]}";
          xSemaphoreGiveRecursive(m_ActiveDevicesSemaphore);
        }
        else
        {
            ESP_LOGW("Semaphore Take Failure", "WARNING! Failed to take Semaphore");
        }
        
        ESP_LOGD("SendActiveCompatibleDevicesToWebSocket", "JSON: %s", jsonString.c_str());

        if (!jsonString.empty())
        {
            String key = this->GetSignal();
            this->TxDataToWebSocket(key, jsonString.c_str());
        }
    }
    private:
      std::vector<ActiveBluetoothDevice_t> m_ActiveDevices;
      SemaphoreHandle_t m_ActiveDevicesSemaphore;
      TaskHandle_t m_ActiveDeviceUpdateTask;
};
