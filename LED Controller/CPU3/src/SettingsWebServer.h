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
#include "SPIFFS.h"
#include "DataItem/DataItems.h"

class SettingsWebServerManager: public SetupCallerInterface
{  
  public:
    SettingsWebServerManager( String Title
                            , AsyncWebSocket &WebSocket
                            , AsyncWebServer &WebServer
                            , IPreferences &preferenceInterface
                            , SerialPortMessageManager &CPU1SerialPortMessageManager
                            , SerialPortMessageManager &CPU2SerialPortMessageManager)
                            : m_WebSocket(WebSocket)
                            , m_WebServer(WebServer)
                            , m_preferenceInterface(preferenceInterface)
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
      SetupAllSetupCallees();
      InitFileSystem();
    }

    void SetupWifi()
    {
      InitWiFiAP();
    }

    void InitializePreferences()
    {
      if(!m_preferenceInterface.begin("Settings", false))
      {
        ESP_LOGE("InitializePreferences", "Unable to initialize preferences! Resseting Device to Factory Defaults");
        nvs_flash_erase();
        ESP_LOGI("InitializePreferences", "NVS Cleared!");
        nvs_flash_init();
        ESP_LOGI("InitializePreferences", "NVS Initialized");
        ESP.restart();
      }
      else if(m_preferenceInterface.getBool("Pref_Reset", true))
      {
        m_preferenceInterface.clear();
        ESP_LOGI("InitializePreferences", "Preferences Cleared!");
        m_preferenceInterface.putBool("Pref_Reset", false);
      }
    }

    // Init the web server to use the local SPIFFS memory and serve up index.html file.
    void InitWebServer()
    {
      // Web Server Root URL
      m_WebServer.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
      {
        request->send(SPIFFS, "/index.html", "text/html");
      });
      m_WebServer.serveStatic("/", SPIFFS, "/").setCacheControl("no-cache, no-store, must-revalidate");
    }
    
    // Initialize SPIFFS
    void InitFileSystem()
    {
      if (SPIFFS.begin())
      {
        ESP_LOGI("Settings_Web_Server", "SPIFFS mounted successfully");
        File root = SPIFFS.open("/");
        File file = root.openNextFile();
        while(file)
        {
          ESP_LOGD("Settings_Web_Server", "FILE: %s", file.name());
          file = root.openNextFile();
        }
      }
      else
      {
        ESP_LOGE("Settings_Web_Server", "An error has occurred while mounting SPIFFS");
      }
    }

    void BeginWebServer()
    {
      m_WebServer.begin();
    }

    void OnEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
    {
      SettingsWebServerManager *instance = reinterpret_cast<SettingsWebServerManager*>(arg);
      if (instance)
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
    } 
  private:
    AsyncWebSocket &m_WebSocket;
    AsyncWebServer &m_WebServer;
    IPreferences &m_preferenceInterface; 
    SerialPortMessageManager &m_CPU1SerialPortMessageManager;
    SerialPortMessageManager &m_CPU2SerialPortMessageManager;
    WebSocketDataProcessor m_WebSocketDataProcessor = WebSocketDataProcessor(m_WebServer, m_WebSocket);

    struct CallbackArguments 
    {
      void* arg1;
      void* arg2;
    };

    const ValidStringValues_t validBoolValues = { "0", "1" };

    //WIFI SSID
    CallbackArguments m_SSID_CallbackArgs = { this };
    NamedCallback_t m_SSID_Callback = { "SSID Callback"
                                      , &SSID_ValueChanged
                                      , &m_SSID_CallbackArgs };
    LocalStringDataItemWithPreferences m_SSID = LocalStringDataItemWithPreferences( "SSID"
                                                                                  , "LED Tower of Power"
                                                                                  , &m_preferenceInterface
                                                                                  , &m_SSID_Callback
                                                                                  , this );
    WebSocket_String_DataHandler m_SSID_DataHandler = WebSocket_String_DataHandler( "SSID Web Socket Handler", "SSID", m_WebSocketDataProcessor, true, true, m_SSID, false );
    static void SSID_ValueChanged(const String &Name, void* object, void* arg)
    {
      ESP_LOGI("SSID_ValueChanged", "CALLED");
      if(arg && object)
      {
        CallbackArguments* pArguments = static_cast<CallbackArguments*>(arg);
        assert((pArguments->arg1) && "Null Pointers!");
        SettingsWebServerManager* pSettingWebServer = static_cast<SettingsWebServerManager*>(object);
        char* pSSID = static_cast<char*>(object);
        ESP_LOGI("SSID_ValueChanged", "SSID Changed: %s", pSSID);
        //TBD THIS NEED COMPLETED
      }
    }

    //WIFI Password
    CallbackArguments m_Password_CallbackArgs = { this };
    NamedCallback_t m_Password_Callback = { "Password Callback"
                                          , &Password_ValueChanged
                                          , &m_Password_CallbackArgs };
    const String m_Password_InitialValue = "LEDs Rock";
    LocalStringDataItemWithPreferences m_Password = LocalStringDataItemWithPreferences( "Password"
                                                                                      , m_Password_InitialValue
                                                                                      , &m_preferenceInterface
                                                                                      , &m_Password_Callback
                                                                                      , this );
    WebSocket_String_DataHandler m_Password_DataHandler = WebSocket_String_DataHandler( "Password Web Socket Handler", "Password", m_WebSocketDataProcessor, true, true, m_Password, false );
    static void Password_ValueChanged(const String &Name, void* object, void* arg)
    {
      ESP_LOGI("Password_ValueChanged", "CALLED");
      if(arg && object)
      {
        CallbackArguments* pArguments = static_cast<CallbackArguments*>(arg);
        assert((pArguments->arg1) && "Null Pointers!");
        SettingsWebServerManager* pSettingWebServer = static_cast<SettingsWebServerManager*>(object);
        char* pPassword = static_cast<char*>(object);
        ESP_LOGI("Password_ValueChanged", "Password Changed: %s", pPassword);
      }
    }

    //Amplitude Gain
    const float m_AmplitudeGain_InitialValue = 2.0;
    DataItemWithPreferences <float, 1> m_AmplitudeGain = DataItemWithPreferences<float, 1>( "Amp_Gain"
                                                                                          , m_AmplitudeGain_InitialValue
                                                                                          , RxTxType_Tx_On_Change_With_Heartbeat
                                                                                          , UpdateStoreType_On_Rx
                                                                                          , 5000
                                                                                          , &m_preferenceInterface
                                                                                          , &m_CPU2SerialPortMessageManager
                                                                                          , NULL
                                                                                          , this );
    WebSocketDataHandler<float, 1> m_Amplitude_Gain_DataHandler = WebSocketDataHandler<float, 1>( "Amplitude Gain Web Socket Handler"
                                                                                                , "Amplitude_Gain"
                                                                                                , m_WebSocketDataProcessor
                                                                                                , true
                                                                                                , true
                                                                                                , m_AmplitudeGain
                                                                                                , false );    
    
    //FFT Gain
    const float m_FFTGain_InitialValue = 2.0;
    DataItemWithPreferences <float, 1> m_FFTGain = DataItemWithPreferences<float, 1>( "FFT_Gain"
                                                                                    , m_FFTGain_InitialValue
                                                                                    , RxTxType_Tx_On_Change_With_Heartbeat
                                                                                    , UpdateStoreType_On_Rx
                                                                                    , 5000
                                                                                    , &m_preferenceInterface
                                                                                    , &m_CPU2SerialPortMessageManager
                                                                                    , NULL
                                                                                    , this );
    WebSocketDataHandler<float, 1> m_FFT_Gain_DataHandler = WebSocketDataHandler<float, 1>( "FFT Gain Web Socket Handler"
                                                                                          , "FFT_Gain"
                                                                                          , m_WebSocketDataProcessor
                                                                                          , true
                                                                                          , true
                                                                                          , m_FFTGain
                                                                                          , false );

    //Microphone Enable
    //TBD

    //Input Source
    const ValidStringValues_t validInputSourceValues = { "OFF", "Microphone", "Bluetooth" };
    const SoundInputSource_t m_SoundInputSource_InitialValue = SoundInputSource_t::OFF;
    DataItemWithPreferences<SoundInputSource_t, 1> m_SoundInputSource = DataItemWithPreferences<SoundInputSource_t, 1>( "Input_Source"
                                                                                                                      , m_SoundInputSource_InitialValue
                                                                                                                      , RxTxType_Tx_On_Change_With_Heartbeat
                                                                                                                      , UpdateStoreType_On_Rx
                                                                                                                      , 5000
                                                                                                                      , &m_preferenceInterface
                                                                                                                      , &m_CPU1SerialPortMessageManager
                                                                                                                      , NULL
                                                                                                                      , this
                                                                                                                      , &validInputSourceValues );
    WebSocketDataHandler<SoundInputSource_t, 1> m_SoundInputSource_DataHandler = WebSocketDataHandler<SoundInputSource_t, 1>( "Sound Input Source Web Socket Handler"
                                                                                                                            , "Sound_Input_Source"
                                                                                                                            , m_WebSocketDataProcessor
                                                                                                                            , true
                                                                                                                            , true
                                                                                                                            , m_SoundInputSource
                                                                                                                            , false );
    
    //Output Source
    const ValidStringValues_t validOutputSourceValues = { "OFF", "Bluetooth" };
    const SoundOutputSource_t m_SoundOuputSource_InitialValue = SoundOutputSource_t::OFF;
    DataItemWithPreferences<SoundOutputSource_t, 1> m_SoundOuputSource = DataItemWithPreferences<SoundOutputSource_t, 1>( "Output_Source", m_SoundOuputSource_InitialValue, RxTxType_Tx_On_Change_With_Heartbeat, UpdateStoreType_On_Rx, 5000, &m_preferenceInterface, &m_CPU2SerialPortMessageManager, NULL, this, &validOutputSourceValues);
    WebSocketDataHandler<SoundOutputSource_t, 1> m_SoundOuputSource_DataHandler = WebSocketDataHandler<SoundOutputSource_t, 1>( "Sound Output Source Web Socket Handler", "Sound_Output_Source", m_WebSocketDataProcessor, true, true, m_SoundOuputSource, false );
    
    //Bluetooth Sink Enable
    const bool m_BluetoothSinkEnable_InitialValue = false;
    DataItemWithPreferences<bool, 1> m_BluetoothSinkEnable = DataItemWithPreferences<bool, 1>( "BT_Sink_En", m_BluetoothSinkEnable_InitialValue, RxTxType_Tx_On_Change_With_Heartbeat, UpdateStoreType_On_Rx, 5000, &m_preferenceInterface, &m_CPU1SerialPortMessageManager, NULL, this, NULL);
    WebSocketDataHandler<bool, 1> m_BluetoothSinkEnable_DataHandler = WebSocketDataHandler<bool, 1>( "Sink Enable Web Socket Handler", "BT_Sink_Enable", m_WebSocketDataProcessor, true, true, m_BluetoothSinkEnable, false );

    //Sink Name
    const String m_SinkName_InitialValue = "LED Tower of Power";  
    StringDataItemWithPreferences m_SinkName = StringDataItemWithPreferences( "Sink_Name", m_SinkName_InitialValue, RxTxType_Tx_On_Change_With_Heartbeat, UpdateStoreType_On_Rx, 5000, &m_preferenceInterface, &m_CPU1SerialPortMessageManager, NULL, this);
    WebSocket_String_DataHandler m_SinkName_DataHandler = WebSocket_String_DataHandler( "Sink Name Web Socket Handler", "Sink_Name", m_WebSocketDataProcessor, true, true, m_SinkName, false );

    //Source Name
    const String m_SourceName_InitialValue = "";  
    StringDataItemWithPreferences m_SourceName = StringDataItemWithPreferences( "Source_Name", m_SourceName_InitialValue, RxTxType_Rx_Only, UpdateStoreType_On_Rx, 0, &m_preferenceInterface, &m_CPU2SerialPortMessageManager, NULL, this);
    WebSocket_String_DataHandler m_SourceName_DataHandler = WebSocket_String_DataHandler( "Sink Name Web Socket Handler", "Source_Name", m_WebSocketDataProcessor, true, true, m_SourceName, false );

    //Sink Connection State
    const ConnectionStatus_t m_SinkConnectionState_InitialValue = ConnectionStatus_t::Disconnected;
    DataItem<ConnectionStatus_t, 1> m_SinkConnectionState = DataItem<ConnectionStatus_t, 1>( "Sink_Conn_State", m_SinkConnectionState_InitialValue, RxTxType_Rx_Only, UpdateStoreType_On_Rx, 0, &m_CPU1SerialPortMessageManager, NULL, this);
    WebSocketDataHandler<ConnectionStatus_t, 1> m_SinkConnectionStatus_DataHandler = WebSocketDataHandler<ConnectionStatus_t, 1>( "BT Sink Connection State Web Socket Handler", "BT_Sink_Connection_State", m_WebSocketDataProcessor, true, true, m_SinkConnectionState, false );    
    
    //Bluetooth Sink Auto Reconnect
    const bool m_BluetoothSinkAutoReConnect_InitialValue = false;
    DataItemWithPreferences<bool, 1> m_BluetoothSinkAutoReConnect = DataItemWithPreferences<bool, 1>( "BT_Sink_AR", m_BluetoothSinkAutoReConnect_InitialValue, RxTxType_Tx_On_Change_With_Heartbeat, UpdateStoreType_On_Rx, 5000, &m_preferenceInterface, &m_CPU1SerialPortMessageManager, NULL, this, NULL);
    WebSocketDataHandler<bool, 1> m_BluetoothSinkAutoReConnect_DataHandler = WebSocketDataHandler<bool, 1>( "Source Enable Web Socket Handler", "BT_Sink_Auto_ReConnect", m_WebSocketDataProcessor, true, true, m_BluetoothSinkAutoReConnect, false );
    
    //Bluetooth Source Enable
    const bool m_BluetoothSourceEnable_InitialValue = false;
    DataItemWithPreferences<bool, 1> m_BluetoothSourceEnable = DataItemWithPreferences<bool, 1>( "BT_Source_En", m_BluetoothSourceEnable_InitialValue, RxTxType_Tx_On_Change_With_Heartbeat, UpdateStoreType_On_Rx, 5000, &m_preferenceInterface, &m_CPU2SerialPortMessageManager, NULL, this, NULL);
    WebSocketDataHandler<bool, 1> m_BluetoothSourceEnable_DataHandler = WebSocketDataHandler<bool, 1>( "Source Enable Web Socket Handler", "BT_Source_Enable", m_WebSocketDataProcessor, true, true, m_BluetoothSourceEnable, false );

    //Target Device
    CompatibleDevice_t m_TargetCompatibleDevice_InitialValue = {"", ""};
    DataItem<CompatibleDevice_t, 1> m_TargetCompatibleDevice = DataItem<CompatibleDevice_t, 1>( "Target_Device", m_TargetCompatibleDevice_InitialValue, RxTxType_Tx_On_Change_With_Heartbeat, UpdateStoreType_On_Rx, 5000, &m_CPU2SerialPortMessageManager, NULL, this);
    WebSocket_Compatible_Device_DataHandler m_TargetCompatibleDevice_DataHandler = WebSocket_Compatible_Device_DataHandler("BT Target Device Web Socket Data Handler", "BT_Source_Target_Device", m_WebSocketDataProcessor, true, true, m_TargetCompatibleDevice, false );

    //Sink Connect
    CallbackArguments m_SinkConnect_CallbackArgs = {this};
    NamedCallback_t m_SinkConnect_Callback = {"m_SinkConnect_Callback", &SinkConnect_ValueChanged, &m_SinkConnect_CallbackArgs};
    const bool m_SinkConnect_InitialValue = false;
    DataItem<bool, 1> m_SinkConnect = DataItem<bool, 1>( "Sink_Connect", m_SinkConnect_InitialValue, RxTxType_Tx_On_Change_With_Heartbeat, UpdateStoreType_On_Tx, 5000, &m_CPU1SerialPortMessageManager, &m_SinkConnect_Callback, this);
    WebSocketDataHandler<bool, 1> m_SinkConnect_DataHandler = WebSocketDataHandler<bool, 1>( "Sink Connect Web Socket Handler", "Sink_Connect", m_WebSocketDataProcessor, true, true, m_SinkConnect, false );
    static void SinkConnect_ValueChanged(const String &Name, void* object, void* arg)
    {
      ESP_LOGI("SinkConnect_ValueChanged", "Sink Connect Value Changed");
      if(arg && object)
      {
        CallbackArguments* arguments = static_cast<CallbackArguments*>(arg);
        if(arguments->arg1 && object)
        {
          bool sinkConnect = *static_cast<bool*>(object);
          if(sinkConnect)
          {
          }
        }
      }
    }

    //Sink Disconnect
    CallbackArguments m_SinkDisconnect_CallbackArgs = {this};
    NamedCallback_t m_SinkDisconnect_Callback = {"m_SinkDisconnect_Callback", &SinkDisconnect_ValueChanged, &m_SinkDisconnect_CallbackArgs};
    const bool m_SinkDisconnect_InitialValue = false;
    DataItem<bool, 1> m_SinkDisconnect = DataItem<bool, 1>( "Sink_Disconnect", m_SinkDisconnect_InitialValue, RxTxType_Tx_On_Change_With_Heartbeat, UpdateStoreType_On_Tx, 5000, &m_CPU1SerialPortMessageManager, &m_SinkDisconnect_Callback, this);
    WebSocketDataHandler<bool, 1> m_SinkDisconnect_DataHandler = WebSocketDataHandler<bool, 1>( "Sink Disconnect Web Socket Handler", {"Sink_Disconnect"}, m_WebSocketDataProcessor, true, true, m_SinkDisconnect, false );
    static void SinkDisconnect_ValueChanged(const String &Name, void* object, void* arg)
    {
      ESP_LOGI("SinkDisconnect_ValueChanged", "Sink DisConnect Value Changed");
      if(arg && object)
      {
        CallbackArguments* arguments = static_cast<CallbackArguments*>(arg);
        if(arguments->arg1 && arguments->arg2 && object)
        {
          bool sinkDisconnect = *static_cast<bool*>(object);
          if(sinkDisconnect)
          {
          }
        }
      }
    }

    //Output Source Connect
    CallbackArguments m_SourceConnect_CallbackArgs = {&m_TargetCompatibleDevice, &m_TargetCompatibleDevice_InitialValue};
    NamedCallback_t m_SourceConnect_Callback = {"Test Name", &SourceConnect_ValueChanged, &m_SourceConnect_CallbackArgs};
    const bool m_SourceConnect_InitialValue = false;
    DataItem<bool, 1> m_SourceConnect = DataItem<bool, 1>( "Src_Connect", m_SourceConnect_InitialValue, RxTxType_Tx_On_Change_With_Heartbeat, UpdateStoreType_On_Tx, 5000, &m_CPU2SerialPortMessageManager, &m_SourceConnect_Callback, this);
    WebSocketDataHandler<bool, 1> m_SourceConnect_DataHandler = WebSocketDataHandler<bool, 1>( "Output Source Connect Web Socket Handler", {"Source_Connect"}, m_WebSocketDataProcessor, true, true, m_SourceConnect, false );
    static void SourceConnect_ValueChanged(const String &Name, void* object, void* arg)
    {
      ESP_LOGI("SourceConnect_ValueChanged", "Source Connect Value Changed");
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
          ESP_LOGE("SourceConnect_ValueChanged", "Invalid Pointer!");
        }
      }
    }

    //Output Source Disconnect
    CallbackArguments m_SourceDisconnect_CallbackArgs = {&m_TargetCompatibleDevice, &m_TargetCompatibleDevice_InitialValue};
    NamedCallback_t m_SourceDisconnect_Callback = {"m_SourceDisconnect_Callback", &SourceDisconnect_ValueChanged, &m_SourceDisconnect_CallbackArgs};
    const bool m_SourceDisconnect_InitialValue = false;
    DataItem<bool, 1> m_SourceDisconnect = DataItem<bool, 1>( "Src_Disconnect", m_SourceDisconnect_InitialValue, RxTxType_Tx_On_Change_With_Heartbeat, UpdateStoreType_On_Tx, 5000, &m_CPU2SerialPortMessageManager, &m_SourceDisconnect_Callback, this);
    WebSocketDataHandler<bool, 1> m_SourceDisconnect_DataHandler = WebSocketDataHandler<bool, 1>( "Output Source Disconnect Web Socket Handler", {"Source_Disconnect"}, m_WebSocketDataProcessor, true, true, m_SourceDisconnect, false );
    static void SourceDisconnect_ValueChanged(const String &Name, void* object, void* arg)
    {
      ESP_LOGI("SourceDisconnect_ValueChanged", "Source Disconnect Value Changed");
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
    CallbackArguments m_ScannedDevice_CallbackArgs = {&m_WebSocketDataProcessor, &m_ScannedDevice_DataHandler};
    NamedCallback_t m_ScannedDevice_Callback = {"m_ScannedDevice_Callback", &ScannedDevice_ValueChanged, &m_ScannedDevice_CallbackArgs};
    ActiveCompatibleDevice_t m_ScannedDevice_InitialValue = {"", "", 0, 0, 0};
    DataItem<ActiveCompatibleDevice_t, 1> m_ScannedDevice = DataItem<ActiveCompatibleDevice_t, 1>( "Scan_BT_Device", m_ScannedDevice_InitialValue, RxTxType_Rx_Only, UpdateStoreType_On_Rx, 0, &m_CPU2SerialPortMessageManager, &m_ScannedDevice_Callback, this);
    WebSocket_ActiveCompatibleDevice_ArrayDataHandler m_ScannedDevice_DataHandler = WebSocket_ActiveCompatibleDevice_ArrayDataHandler( "Scan BT Device Web Socket Data Handler", "BT_Source_Target_Devices", m_WebSocketDataProcessor, true, true, m_ScannedDevice, false );
    static void ScannedDevice_ValueChanged(const String &Name, void* object, void* arg)
    {
      ESP_LOGD("Manager::ScannedDeviceValueChanged", "Scanned Device Value Changed");
      CallbackArguments* arguments = static_cast<CallbackArguments*>(arg);
      WebSocketDataProcessor* processor = static_cast<WebSocketDataProcessor*>(arguments->arg1);
      WebSocket_ActiveCompatibleDevice_ArrayDataHandler* DataHandler = static_cast<WebSocket_ActiveCompatibleDevice_ArrayDataHandler*>(arguments->arg2);
      processor->UpdateDataForSender(DataHandler, false);
    }
    
    //Bluetooth Source Auto Reconnect
    const bool m_BluetoothSourceAutoReConnect_InitialValue = false;
    DataItemWithPreferences<bool, 1> m_BluetoothSourceAutoReConnect = DataItemWithPreferences<bool, 1>( "BT_Source_AR", m_BluetoothSourceAutoReConnect_InitialValue, RxTxType_Tx_On_Change_With_Heartbeat, UpdateStoreType_On_Rx, 5000, &m_preferenceInterface, &m_CPU2SerialPortMessageManager, NULL, this, &validBoolValues);
    WebSocketDataHandler<bool, 1> m_BluetoothSourceAutoReConnect_DataHandler = WebSocketDataHandler<bool, 1>( "Bluetooth Source Auto Reconnect Web Socket Handler", "BT_Source_Auto_Reconnect", m_WebSocketDataProcessor, true, true, m_BluetoothSourceAutoReConnect, false );

    //Source Connection State
    const ConnectionStatus_t m_SourceConnectionState_InitialValue = ConnectionStatus_t::Disconnected;
    DataItem<ConnectionStatus_t, 1> m_SourceConnectionState = DataItem<ConnectionStatus_t, 1>( "Src_Conn_State", m_SourceConnectionState_InitialValue, RxTxType_Rx_Only, UpdateStoreType_On_Rx, 0, &m_CPU2SerialPortMessageManager, NULL, this);
    WebSocketDataHandler<ConnectionStatus_t, 1> m_SourceConnectionState_DataHandler = WebSocketDataHandler<ConnectionStatus_t, 1>( "Source Connection State Web Socket Handler", "BT_Source_Connection_State", m_WebSocketDataProcessor, true, true, m_SourceConnectionState, false );    

    //Source Reset
    const bool m_SourceReset_InitialValue = false;
    DataItemWithPreferences<bool, 1> m_SourceReset = DataItemWithPreferences<bool, 1>( "BT_Src_Reset", m_SourceReset_InitialValue, RxTxType_Tx_On_Change_With_Heartbeat, UpdateStoreType_On_Rx, 5000, &m_preferenceInterface, &m_CPU2SerialPortMessageManager, NULL, this, &validBoolValues);
    WebSocketDataHandler<bool, 1> m_SourceReset_DataHandler = WebSocketDataHandler<bool, 1>( "Source Reset Web Socket Handler", "BT_Source_Reset", m_WebSocketDataProcessor, true, true, m_SourceReset, false );    
    
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
          JSONVar jsonObject = JSON.parse(WebSocketData);
          if (JSON.typeof(jsonObject) == "undefined")
          {
            ESP_LOGE("SettingsWebServer: HandleWebSocketMessage", "Parsing failed for Input: %s", WebSocketData.c_str());
          }
          else
          {
            if(jsonObject.hasOwnProperty("SignalValue"))
            {
              JSONVar signalValue = jsonObject["SignalValue"];
              if (signalValue.hasOwnProperty("Id") && signalValue.hasOwnProperty("Value"))
              {
                const String Id = signalValue["Id"];
                const String Value = signalValue["Value"];
                ESP_LOGI( "SettingsWebServer: HandleWebSocketMessage", "Web Socket Signal Value Data Received. Id: \"%s\" Value: \"%s\""
                        , Id.c_str()
                        , Value.c_str() );
                if(!m_WebSocketDataProcessor.ProcessSignalValueAndSendToDatalink(Id.c_str(), Value.c_str()))
                {
                  ESP_LOGE("SettingsWebServer: HandleWebSocketMessage", "Unknown Signal Value Object: %s", Id.c_str());
                }
              }
              else
              {
                  ESP_LOGD("SettingsWebServer: HandleWebSocketMessage", "Unknown JSON Object: %s", JSON.stringify(signalValue).c_str());
              }
            }
            else if(jsonObject.hasOwnProperty("JSONValue"))
            {
              JSONVar jSONValue = jsonObject["JSONValue"];
              if (jSONValue.hasOwnProperty("Id") && jSONValue.hasOwnProperty("Value"))
              {
                const String Id = jSONValue["Id"];
                const String Value = jSONValue["Value"];
                ESP_LOGI( "SettingsWebServer: HandleWebSocketMessage", "Web Socket JSON Data Received. Id: \"%s\" Value: \"%s\""
                        , Id.c_str()
                        , Value.c_str());
                if(!m_WebSocketDataProcessor.ProcessSignalValueAndSendToDatalink(Id.c_str(), Value.c_str()))
                {
                  ESP_LOGE("SettingsWebServer: HandleWebSocketMessage", "Unknown JSON Object: %s", Id.c_str());
                }
              }
              else
              
              {
                ESP_LOGD("SettingsWebServer: HandleWebSocketMessage", "Known JSON Object: %s", JSON.stringify(jSONValue).c_str());
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
      ESP_LOGI( "SettingsWebServer: InitWiFiAP", "Initializing Wifi Client");
      if(0 == m_SSID.GetValueAsString().length() || 0 == m_Password.GetValueAsString().length())
      {
        //WiFi.disconnect();
        WiFi.mode(WIFI_STA);
        bool connected = WiFi.begin(m_SSID.GetValueAsString().c_str(), m_Password.GetValueAsString().c_str());
        ESP_LOGI("SettingsWebServer: InitWifiClient", "Connecting to WiFi ..");
        while (WiFi.status() != WL_CONNECTED) {
          ESP_LOGI("SettingsWebServer: InitWifiClient", "Connecting...");
          delay(1000);
        }
        IPAddress ipAddress = WiFi.localIP();
        if(connected)
        {
          InitWebServer();
          BeginWebServer();
          ESP_LOGI( "SettingsWebServer: InitWifiClient"
                  , "Wifi Started! IP Address: %i.%i.%i.%i"
                  , ipAddress[0]
                  , ipAddress[1]
                  , ipAddress[2]
                  , ipAddress[3] );
        }
        else
        {
          ESP_LOGE( "SettingsWebServer: InitWifiClient"
                  , "Wifi failed to start!");
        }
      }
      else
      {
          ESP_LOGE( "SettingsWebServer: InitWifiClient"
                  , "Null Pointers!");
      }
    }
    
    void InitWiFiAP()
    {
      ESP_LOGI( "SettingsWebServer: InitWiFiAP", "Initializing Wifi Access Point");
      if(m_SSID.GetValueAsString().length(), m_Password.GetValueAsString().length())
      {
        // Setup ESP32 as Access Point
        IPAddress Ip(192, 168, 0, 1);
        IPAddress NMask(255, 255, 255, 0);
        //WiFi.disconnect();
        WiFi.softAPConfig(Ip, Ip, NMask);
        ESP_LOGI( "SettingsWebServer: InitWifiClient", "Starting Access Point: SSID: \"%s\" Password: \"%s\"", m_SSID.GetValueAsString().c_str(), m_Password.GetValueAsString().c_str());
        bool connected = WiFi.softAP(m_SSID.GetValueAsString().c_str(), m_Password.GetValueAsString().c_str());
        IPAddress ipAddress = WiFi.softAPIP();
        if(connected)
        {
          InitWebServer();
          BeginWebServer();
          ESP_LOGI( "SettingsWebServer: InitWifiClient"
                  , "Wifi AP Started! IP Address: %i.%i.%i.%i"
                  , ipAddress[0]
                  , ipAddress[1]
                  , ipAddress[2]
                  , ipAddress[3] );
        }
        else
        {
          ESP_LOGE( "SettingsWebServer: InitWifiClient"
                  , "Wifi failed to start!");
        }
      }
      else
      {
          ESP_LOGE( "SettingsWebServer: InitWifiClient"
                  , "Null Pointers!");
      }
    }
};