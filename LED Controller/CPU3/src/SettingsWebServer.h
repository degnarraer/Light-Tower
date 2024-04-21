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
#include "WebSocketDataHandler.h"
#include "DataItem.h"
#include "DataItemWithPreferences.h"

class SettingsWebServerManager
{  
  public:
    SettingsWebServerManager( String Title
                            , AsyncWebSocket &WebSocket
                            , SerialPortMessageManager &CPU1SerialPortMessageManager
                            , SerialPortMessageManager &CPU2SerialPortMessageManager)
                            : m_WebSocket(WebSocket)
                            , m_CPU1SerialPortMessageManager(CPU1SerialPortMessageManager)
                            , m_CPU2SerialPortMessageManager(CPU2SerialPortMessageManager)
    {
    }
    
    virtual ~SettingsWebServerManager()
    {
    }
    
    void SetupSettingsWebServerManager()
    {
      InitializePreferences();
      RegisterForDataItemCallBacks();
    }

    void BeginWebServer()
    {
      InitWiFiAP();
    }
    
    void InitializePreferences()
    {
      if(!m_Preferences.begin("Settings", false))
      {
        ESP_LOGE("InitializePreferences", "Unable to initialize preferences! Resseting Device to Factory Defaults");
        nvs_flash_erase();
        ESP_LOGI("InitializePreferences", "NVS Cleared!");
        nvs_flash_init();
        ESP_LOGI("InitializePreferences", "NVS Initialized");
        ESP.restart();
      }
      else if(m_Preferences.getBool("Pref_Reset", true))
      {
        m_Preferences.clear();
        ESP_LOGI("InitializePreferences", "Preferences Cleared!");
        m_Preferences.putBool("Pref_Reset", false);
      }
    }

    void ClearPreferences()
    {
      m_Preferences.clear();
      ESP_LOGI("Settings Web Server: ClearPreferences", "Preferences Cleared");
    }
    
    void RegisterForDataItemCallBacks()
    {
      m_OuputSourceConnect.RegisterNamedCallback(&m_OuputSourceConnect_Callback);
      m_OuputSourceDisconnect.RegisterNamedCallback(&m_OuputSourceDisconnect_Callback);
      m_ScannedDevice.RegisterNamedCallback(&m_ScannedDevice_Callback);
    }
    
    void OnEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
    {
      switch (type) 
      {
        case WS_EVT_CONNECT:
          Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
          break;
        case WS_EVT_PONG:
          Serial.printf("WebSocket client #%u Pinged Us!\n", client->id());
          break;
        case WS_EVT_DATA:
          HandleWebSocketMessage(client, arg, data, len);
          break;
        case WS_EVT_ERROR:
          Serial.printf("WebSocket client #%u Error. Closing Connection!\n", client->id());
          break;
        case WS_EVT_DISCONNECT:
          Serial.printf("WebSocket client #%u disconnected. Closing Connection.\n", client->id());
          break;
      }
    } 
  private:
    AsyncWebSocket &m_WebSocket;
    SerialPortMessageManager &m_CPU1SerialPortMessageManager;
    SerialPortMessageManager &m_CPU2SerialPortMessageManager;
    WebSocketDataProcessor m_WebSocketDataProcessor = WebSocketDataProcessor(m_WebSocket);
    
    Preferences m_Preferences;
    const char* password = "LEDs Rock";
    struct CallbackArguments 
    {
      void* arg1;
      void* arg2;
    };
    
    //Amplitude Gain
    const float m_AmplitudeGain_InitialValue = 2.0;
    DataItemWithPreferences <float, 1> m_AmplitudeGain = DataItemWithPreferences<float, 1>( "Amp_Gain", m_AmplitudeGain_InitialValue, RxTxType_Tx_On_Change_With_Heartbeat, UpdateStoreType_On_Rx, 5000, &m_Preferences, m_CPU2SerialPortMessageManager);
    WebSocketDataHandler<float, 1> m_Amplitude_Gain_DataHandler = WebSocketDataHandler<float, 1>( "Amplitude Gain Web Socket Handler", {"Amplitude_Gain"}, m_WebSocketDataProcessor, true, true, m_AmplitudeGain, false );    
    
    //FFT Gain
    const float m_FFTGain_InitialValue = 2.0;
    DataItemWithPreferences <float, 1> m_FFTGain = DataItemWithPreferences<float, 1>( "FFT_Gain", m_FFTGain_InitialValue, RxTxType_Tx_On_Change_With_Heartbeat, UpdateStoreType_On_Rx, 5000, &m_Preferences, m_CPU2SerialPortMessageManager);
    WebSocketDataHandler<float, 1> m_FFT_Gain_DataHandler = WebSocketDataHandler<float, 1>( "FFT Gain Web Socket Handler", {"FFT_Gain"}, m_WebSocketDataProcessor, true, true, m_FFTGain, false );

    //Microphone Enable
    //TBD

    //Input Source
    const SoundInputSource_t m_SoundInputSource_InitialValue = SoundInputSource_t::SoundInputSource_OFF;
    DataItemWithPreferences<SoundInputSource_t, 1> m_SoundInputSource = DataItemWithPreferences<SoundInputSource_t, 1>( "Input_Source", m_SoundInputSource_InitialValue, RxTxType_Tx_On_Change_With_Heartbeat, UpdateStoreType_On_Rx, 5000, &m_Preferences, m_CPU1SerialPortMessageManager);
    WebSocketDataHandler<SoundInputSource_t, 1> m_SoundInputSource_DataHandler = WebSocketDataHandler<SoundInputSource_t, 1>( "Sound Input Source Web Socket Handler", {"Sound_Input_Source"}, m_WebSocketDataProcessor, true, true, m_SoundInputSource, false );
    
    //Output Source
    const SoundOutputSource_t m_SoundOuputSource_InitialValue = SoundOutputSource_t::SoundOutputSource_OFF;
    DataItemWithPreferences<SoundOutputSource_t, 1> m_SoundOuputSource = DataItemWithPreferences<SoundOutputSource_t, 1>( "Output_Source", m_SoundOuputSource_InitialValue, RxTxType_Tx_On_Change_With_Heartbeat, UpdateStoreType_On_Rx, 5000, &m_Preferences, m_CPU2SerialPortMessageManager);
    WebSocketDataHandler<SoundOutputSource_t, 1> m_SoundOuputSource_DataHandler = WebSocketDataHandler<SoundOutputSource_t, 1>( "Sound Output Source Web Socket Handler", {"Sound_Output_Source"}, m_WebSocketDataProcessor, true, true, m_SoundOuputSource, false );
    
    //Bluetooth Sink Enable
    const bool m_BluetoothSinkEnable_InitialValue = false;
    DataItemWithPreferences<bool, 1> m_BluetoothSinkEnable = DataItemWithPreferences<bool, 1>( "BT_Sink_En", m_BluetoothSinkEnable_InitialValue, RxTxType_Tx_On_Change_With_Heartbeat, UpdateStoreType_On_Rx, 5000, &m_Preferences, m_CPU1SerialPortMessageManager);
    WebSocketDataHandler<bool, 1> m_BluetoothSinkEnable_DataHandler = WebSocketDataHandler<bool, 1>( "Bluetooth Sink Enable Web Socket Handler", {"BT_Sink_Enable"}, m_WebSocketDataProcessor, true, true, m_BluetoothSinkEnable, false );

    //Sink Name
    const String m_SinkName_InitialValue = "LED Tower of Power";  
    StringDataItemWithPreferences m_BluetoothSinkName = StringDataItemWithPreferences( "BT_Sink_Name", m_SinkName_InitialValue.c_str(), RxTxType_Tx_On_Change_With_Heartbeat, UpdateStoreType_On_Rx, 5000, &m_Preferences, m_CPU1SerialPortMessageManager);
    WebSocketDataHandler<char, 50> m_BluetoothSinkName_DataHandler = WebSocketDataHandler<char, 50>( "Bluetooth Sink Name Web Socket Handler", {"BT_Sink_Name"}, m_WebSocketDataProcessor, true, true, m_BluetoothSinkName, false );

    //Sink Connection State
    const ConnectionStatus_t m_SinkConnectionState_InitialValue = ConnectionStatus_t::Disconnected;
    DataItem<ConnectionStatus_t, 1> m_SinkConnectionState = DataItem<ConnectionStatus_t, 1>( "Sink_Conn_State", m_SinkConnectionState_InitialValue, RxTxType_Rx_Only, UpdateStoreType_On_Rx, 0, m_CPU1SerialPortMessageManager);
    WebSocketDataHandler<ConnectionStatus_t, 1> m_SinkConnectionStatus_DataHandler = WebSocketDataHandler<ConnectionStatus_t, 1>( "BT Sink Connection State Web Socket Handler", {"BT_Sink_Connection_State"}, m_WebSocketDataProcessor, true, true, m_SinkConnectionState, false );    
    
    //Bluetooth Sink Auto Reconnect
    const bool m_BluetoothSinkAutoReConnect_InitialValue = false;
    DataItemWithPreferences<bool, 1> m_BluetoothSinkAutoReConnect = DataItemWithPreferences<bool, 1>( "BT_Sink_AR", m_BluetoothSinkAutoReConnect_InitialValue, RxTxType_Tx_On_Change_With_Heartbeat, UpdateStoreType_On_Rx, 5000, &m_Preferences, m_CPU1SerialPortMessageManager);
    WebSocketDataHandler<bool, 1> m_BluetoothSinkAutoReConnect_DataHandler = WebSocketDataHandler<bool, 1>( "Bluetooth Source Enable Web Socket Handler", {"BT_Sink_Auto_ReConnect"}, m_WebSocketDataProcessor, true, true, m_BluetoothSinkAutoReConnect, false );
    
    //Bluetooth Source Enable
    const bool m_BluetoothSourceEnable_InitialValue = false;
    DataItemWithPreferences<bool, 1> m_BluetoothSourceEnable = DataItemWithPreferences<bool, 1>( "BT_Source_En", m_BluetoothSourceEnable_InitialValue, RxTxType_Tx_On_Change_With_Heartbeat, UpdateStoreType_On_Rx, 5000, &m_Preferences, m_CPU2SerialPortMessageManager);
    WebSocketDataHandler<bool, 1> m_BluetoothSourceEnable_DataHandler = WebSocketDataHandler<bool, 1>( "Bluetooth Source Enable Web Socket Handler", {"BT_Source_Enable"}, m_WebSocketDataProcessor, true, true, m_BluetoothSourceEnable, false );

    //Target Device
    CompatibleDevice_t m_TargetCompatibleDevice_InitialValue = {"", ""};
    DataItem<CompatibleDevice_t, 1> m_TargetCompatibleDevice = DataItem<CompatibleDevice_t, 1>( "Target_Device", m_TargetCompatibleDevice_InitialValue, RxTxType_Tx_On_Change_With_Heartbeat, UpdateStoreType_On_Rx, 5000, m_CPU2SerialPortMessageManager);
    WebSocket_Compatible_Device_DataHandler m_TargetCompatibleDevice_DataHandler = WebSocket_Compatible_Device_DataHandler("BT Target Device Web Socket Data Handler", {"BT_Source_Target_Device"}, m_WebSocketDataProcessor, true, true, m_TargetCompatibleDevice, false );

    //Output Source Connect
    const bool m_OuputSourceConnect_InitialValue = false;
    DataItem<bool, 1> m_OuputSourceConnect = DataItem<bool, 1>( "Src_Connect", m_OuputSourceConnect_InitialValue, RxTxType_Tx_On_Change_With_Heartbeat, UpdateStoreType_On_Tx, 5000, m_CPU2SerialPortMessageManager);
    WebSocketDataHandler<bool, 1> m_OuputSourceConnect_DataHandler = WebSocketDataHandler<bool, 1>( "Output Source Connect Web Socket Handler", {"Output_Source_Connect"}, m_WebSocketDataProcessor, true, true, m_OuputSourceConnect, false );
    CallbackArguments m_OuputSourceConnect_CallbackArgs = {&m_TargetCompatibleDevice, &m_TargetCompatibleDevice_InitialValue};
    NamedCallback_t m_OuputSourceConnect_Callback = {"Test Name", &OuputSourceConnect_ValueChanged, &m_OuputSourceConnect_CallbackArgs};
    static void OuputSourceConnect_ValueChanged(const String &Name, void* object, void* arg)
    {
      ESP_LOGI("OuputSourceConnect_ValueChanged", "Ouput Source Connect Value Changed");
      if(arg && object)
      {
        CallbackArguments* arguments = static_cast<CallbackArguments*>(arg);
        if(arguments->arg1 && arguments->arg2 && object)
        {
          bool sourceConnect = *static_cast<bool*>(object);
          if(sourceConnect)
          {
            DataItem<CompatibleDevice_t, 1> *targetCompatibleDevice = static_cast<DataItem<CompatibleDevice_t, 1>*>(arguments->arg1);
            CompatibleDevice_t *initialValue = static_cast<CompatibleDevice_t*>(arguments->arg2);
            if(targetCompatibleDevice && initialValue)
            {
              //targetCompatibleDevice->SetValue(initialValue, 1);
            }
          }
        }
        else
        {
          ESP_LOGE("OuputSourceConnect_ValueChanged", "Invalid Pointer!");
        }
      }
    }

    //Output Source Disconnect
    const bool m_OuputSourceDisconnect_InitialValue = false;
    DataItem<bool, 1> m_OuputSourceDisconnect = DataItem<bool, 1>( "Src_Disconnect", m_OuputSourceDisconnect_InitialValue, RxTxType_Tx_On_Change_With_Heartbeat, UpdateStoreType_On_Tx, 5000, m_CPU2SerialPortMessageManager);
    WebSocketDataHandler<bool, 1> m_OuputSourceDisconnect_DataHandler = WebSocketDataHandler<bool, 1>( "Output Source Disconnect Web Socket Handler", {"Output_Source_Disconnect"}, m_WebSocketDataProcessor, true, true, m_OuputSourceDisconnect, false );
    CallbackArguments m_OuputSourceDisconnect_CallbackArgs = {&m_TargetCompatibleDevice, &m_TargetCompatibleDevice_InitialValue};
    NamedCallback_t m_OuputSourceDisconnect_Callback = {"m_OuputSourceDisconnect_Callback", &OuputSourceDisconnect_ValueChanged, &m_OuputSourceDisconnect_CallbackArgs};
    static void OuputSourceDisconnect_ValueChanged(const String &Name, void* object, void* arg)
    {
      ESP_LOGI("OuputSourceDisconnect_ValueChanged", "Ouput Source Disconnect Value Changed");
      if(arg && object)
      {
        CallbackArguments* arguments = static_cast<CallbackArguments*>(arg);
        if(arguments->arg1 && arguments->arg2 && object)
        {
          bool sourceDisconnect = *static_cast<bool*>(object);
          if(sourceDisconnect)
          {
            DataItem<CompatibleDevice_t, 1> *targetCompatibleDevice = static_cast<DataItem<CompatibleDevice_t, 1>*>(arguments->arg1);
            CompatibleDevice_t *initialValue = static_cast<CompatibleDevice_t*>(arguments->arg2);
            if(targetCompatibleDevice && initialValue)
            {
              //targetCompatibleDevice->SetValue(initialValue, 1);
            }
          }
        }
        else
        {
          ESP_LOGE("OuputSourceDisconnect_ValueChanged", "Invalid Pointer!");
        }
      }
    }

    //Scanned Device
    ActiveCompatibleDevice_t m_ScannedDevice_InitialValue = {"", "", 0, 0, 0};
    DataItem<ActiveCompatibleDevice_t, 1> m_ScannedDevice = DataItem<ActiveCompatibleDevice_t, 1>( "Scan_BT_Device", m_ScannedDevice_InitialValue, RxTxType_Rx_Only, UpdateStoreType_On_Rx, 0, m_CPU2SerialPortMessageManager);
    WebSocket_ActiveCompatibleDevice_ArrayDataHandler m_ScannedDevice_DataHandler = WebSocket_ActiveCompatibleDevice_ArrayDataHandler( "Scan BT Device Web Socket Data Handler", {"BT_Source_Target_Devices"}, m_WebSocketDataProcessor, true, true, m_ScannedDevice, false );
    CallbackArguments m_ScannedDevice_CallbackArgs = {&m_WebSocketDataProcessor, &m_ScannedDevice_DataHandler};
    NamedCallback_t m_ScannedDevice_Callback = {"m_ScannedDevice_Callback", &ScannedDevice_ValueChanged, &m_ScannedDevice_CallbackArgs};
    static void ScannedDevice_ValueChanged(const String &Name, void* object, void* arg)
    {
      ESP_LOGI("Manager::ScannedDeviceValueChanged", "Scanned Device Value Changed");
      CallbackArguments* arguments = static_cast<CallbackArguments*>(arg);
      WebSocketDataProcessor* processor = static_cast<WebSocketDataProcessor*>(arguments->arg1);
      WebSocket_ActiveCompatibleDevice_ArrayDataHandler* DataHandler = static_cast<WebSocket_ActiveCompatibleDevice_ArrayDataHandler*>(arguments->arg2);
      processor->UpdateDataForSender(DataHandler, false);
    }
    
    //Bluetooth Source Auto Reconnect
    const bool m_BluetoothSourceAutoReConnect_InitialValue = false;
    DataItemWithPreferences<bool, 1> m_BluetoothSourceAutoReConnect = DataItemWithPreferences<bool, 1>( "BT_Source_AR", m_BluetoothSourceAutoReConnect_InitialValue, RxTxType_Tx_On_Change_With_Heartbeat, UpdateStoreType_On_Rx, 5000, &m_Preferences, m_CPU2SerialPortMessageManager);
    WebSocketDataHandler<bool, 1> m_BluetoothSourceAutoReConnect_DataHandler = WebSocketDataHandler<bool, 1>( "Bluetooth Source Auto Reconnect Web Socket Handler", {"BT_Source_Auto_Reconnect"}, m_WebSocketDataProcessor, true, true, m_BluetoothSourceAutoReConnect, false );

    //Source Connection State
    const ConnectionStatus_t m_SourceConnectionState_InitialValue = ConnectionStatus_t::Disconnected;
    DataItem<ConnectionStatus_t, 1> m_SourceConnectionState = DataItem<ConnectionStatus_t, 1>( "Src_Conn_State", m_SourceConnectionState_InitialValue, RxTxType_Rx_Only, UpdateStoreType_On_Rx, 0, m_CPU2SerialPortMessageManager);
    WebSocketDataHandler<ConnectionStatus_t, 1> m_SourceConnectionState_DataHandler = WebSocketDataHandler<ConnectionStatus_t, 1>( "Source Connection State Web Socket Handler", {"BT_Source_Connection_State"}, m_WebSocketDataProcessor, true, true, m_SourceConnectionState, false );    

    //Source Reset
    const bool m_SourceReset_InitialValue = false;
    DataItemWithPreferences<bool, 1> m_SourceReset = DataItemWithPreferences<bool, 1>( "BT_Src_Reset", m_SourceReset_InitialValue, RxTxType_Tx_On_Change_With_Heartbeat, UpdateStoreType_On_Rx, 5000, &m_Preferences, m_CPU2SerialPortMessageManager);
    WebSocketDataHandler<bool, 1> m_SourceReset_DataHandler = WebSocketDataHandler<bool, 1>( "Source Reset Web Socket Handler", {"BT_Source_Reset"}, m_WebSocketDataProcessor, true, true, m_SourceReset, false );    

    void HandleWebSocketMessage(AsyncWebSocketClient *client, void *arg, uint8_t *data, size_t len)
    {
      AwsFrameInfo *info = (AwsFrameInfo*)arg;
      data[len] = 0;
      if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
      {
        String WebSocketData = String((char*)data);
        ESP_LOGD("SettingsWebServer: HandleWebSocketMessage", "WebSocket Data from Client: %i, Data: %s", client->id(), WebSocketData.c_str());
        if ( WebSocketData.equals("Hello I am here!") )
        {
          ESP_LOGI("SettingsWebServer: HandleWebSocketMessage", "New Client Message: \"Hello I am here!\"");
          m_WebSocketDataProcessor.UpdateAllDataToClient(client->id());
          return;
        }
        else
        {
          DynamicJsonDocument doc(10000); // Adjust the size based on your needs
          DeserializationError error = deserializeJson(doc, WebSocketData);
    
          if (error)
          {
            ESP_LOGE("SettingsWebServer: HandleWebSocketMessage", "Error parsing JSON: %s", error.c_str());
          }
          else
          {
            if(doc["WidgetValue"])
            {
              const JsonVariant widgetValue = doc["WidgetValue"];
              if (widgetValue.is<JsonObject>() && widgetValue["Id"].is<String>() && widgetValue["Value"].is<String>())
              {
                ESP_LOGD( "SettingsWebServer: HandleWebSocketMessage", "Web Socket Widget Value Data Received. Id: \"%s\" Value: \"%s\""
                        , widgetValue["Id"].as<String>().c_str()
                        , widgetValue["Value"].as<String>().c_str() );
                if(!m_WebSocketDataProcessor.ProcessWidgetValueAndSendToDatalink(widgetValue["Id"].as<String>(), widgetValue["Value"].as<String>()))
                {
                  ESP_LOGE("SettingsWebServer: HandleWebSocketMessage", "Unknown Widget Value Object: %s", widgetValue["Id"].as<String>().c_str());
                }
              }
              else
              {
                  ESP_LOGD("SettingsWebServer: HandleWebSocketMessage", "Known JSON Object: %s", widgetValue.as<String>());
              }
            }
            else if(doc["JSONValue"])
            {
              const JsonVariant jSONValue = doc["JSONValue"];
              if (jSONValue.is<JsonObject>() && jSONValue["Id"].is<String>() && jSONValue["Value"].is<JsonObject>())
              {
                ESP_LOGD( "SettingsWebServer: HandleWebSocketMessage", "Web Socket JSON Data Received. Id: \"%s\" Value: \"%s\""
                        , jSONValue["Id"].as<String>().c_str()
                        , jSONValue["Value"].as<String>().c_str() );
                if(!m_WebSocketDataProcessor.ProcessWidgetValueAndSendToDatalink(jSONValue["Id"].as<String>(), jSONValue["Value"].as<String>()))
                {
                  ESP_LOGE("SettingsWebServer: HandleWebSocketMessage", "Unknown JSON Object: %s", jSONValue["Id"].as<String>().c_str());
                }
              }
              else
              {
                ESP_LOGD("SettingsWebServer: HandleWebSocketMessage", "Known JSON Object: %s", jSONValue.as<String>());
              }
            }
            else
            {
              ESP_LOGE("SettingsWebServer: HandleWebSocketMessage", "Unknown Web Socket Message: %s", WebSocketData.c_str());
            }
          }
        }
      }
    }
        
    // Initialize WiFi Client
    void InitWiFiClient()
    {
      WiFi.mode(WIFI_STA);
      WiFi.begin(m_BluetoothSinkName.GetValueAsString("").c_str(), password);
      ESP_LOGI("SettingsWebServer: InitWifiClient", "Connecting to WiFi ..");
      while (WiFi.status() != WL_CONNECTED) {
        ESP_LOGI("SettingsWebServer: InitWifiClient", "Connecting...");
        delay(1000);
      }
      IPAddress ipAddress = WiFi.localIP();
      ESP_LOGI( "SettingsWebServer: InitWifiClient"
              , "Connected! IP Address: %i.%i.%i.%i"
              , ipAddress[0]
              , ipAddress[1]
              , ipAddress[2]
              , ipAddress[3] );
    }
    
    void InitWiFiAP()
    {
        // Setup ESP32 as Access Point
      IPAddress Ip(192, 168, 0, 1);
      IPAddress NMask(255, 255, 255, 0);
      
      WiFi.softAPConfig(Ip, Ip, NMask);
      WiFi.softAP(m_BluetoothSinkName.GetValueAsString("").c_str(), password);
      IPAddress ipAddress = WiFi.softAPIP();
      ESP_LOGI( "SettingsWebServer: InitWifiClient"
              , "Connected! IP Address: %i.%i.%i.%i"
              , ipAddress[0]
              , ipAddress[1]
              , ipAddress[2]
              , ipAddress[3] );
    }
};