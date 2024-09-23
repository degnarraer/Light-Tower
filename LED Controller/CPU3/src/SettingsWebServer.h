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
#include <ESPmDNS.h>

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
      InitWebServer();
      WiFi.onEvent(std::bind( &SettingsWebServerManager::onWiFiEvent
                            , this
                            , std::placeholders::_1 ));
      m_WebSocket.onEvent(std::bind( &SettingsWebServerManager::OnEvent
                                   , this
                                   , std::placeholders::_1
                                   , std::placeholders::_2
                                   , std::placeholders::_3
                                   , std::placeholders::_4
                                   , std::placeholders::_5
                                   , std::placeholders::_6 ));
      m_WebServer.addHandler(&m_WebSocket);
    }

    void StartWiFi()
    {
      if(Wifi_Mode_t::AccessPoint == m_Wifi_Mode.GetValue())
      {
        InitWiFi_AccessPoint( m_AP_SSID.GetValueAsString().c_str()
                            , m_AP_Password.GetValueAsString().c_str()
                            , m_Host_Name.GetValueAsString().c_str() );
      }
      else if(Wifi_Mode_t::Station == m_Wifi_Mode.GetValue())
      {
        InitWiFi_AccessPoint_Station( m_AP_SSID.GetValueAsString().c_str()
                                    , m_AP_Password.GetValueAsString().c_str()
                                    , m_STA_SSID.GetValueAsString().c_str()
                                    , m_STA_Password.GetValueAsString().c_str()
                                    , m_Host_Name.GetValueAsString().c_str() );
      }
      else
      {
        InitWiFi_AccessPoint("LED Tower of Power", "LEDs Rock", "LTOP");
      }
    }
    
    void EndWiFi()
    {
      EndWebServer();
      WiFi.disconnect();
    }

    void InitializePreferences()
    {
      if(!m_preferenceInterface.begin("Settings", false))
      {
        ESP_LOGE("InitializePreferences", "ERROR! Unable to initialize preferences. Resseting Device to Factory Defaults.");
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
        ESP_LOGE("Settings_Web_Server", "ERROR! Unable to mount SPIFFS.");
      }
    }

    // Event handler to capture Wi-Fi events
    void onWiFiEvent(WiFiEvent_t event)
    {
      switch (event) 
      {
          case SYSTEM_EVENT_WIFI_READY:
            ESP_LOGI("WiFiEvent", "Wifi Ready!");
            break;
          case SYSTEM_EVENT_SCAN_DONE:
            ESP_LOGI("WiFiEvent", "Scan Done!");
            break;
          case SYSTEM_EVENT_STA_START:
            ESP_LOGI("WiFiEvent", "Station started!");
            m_Station_Running = true;
            BeginWebServer();
            break;
          case SYSTEM_EVENT_STA_STOP:
            ESP_LOGI("WiFiEvent", "Station stopped!");
            m_Station_Running = false;
            if(!m_Station_Running && !m_AccessPoint_Running)
            {
              EndWebServer();
            }
            break;
          case SYSTEM_EVENT_STA_CONNECTED:
            ESP_LOGI("WiFiEvent", "Station connected");
            break;
          case SYSTEM_EVENT_STA_DISCONNECTED:
            ESP_LOGI("WiFiEvent", "Station disconnected");
            WiFi.reconnect();
            break;
          case SYSTEM_EVENT_STA_AUTHMODE_CHANGE:
            ESP_LOGI("WiFiEvent", "Station Auth Mode Change");
            break;
          case SYSTEM_EVENT_STA_GOT_IP:
            ESP_LOGI("WiFiEvent", "Station Got IP address: \"%s\"", WiFi.localIP().toString().c_str());
            break;
          case SYSTEM_EVENT_STA_LOST_IP:
            ESP_LOGI("WiFiEvent", "Station Lost IP address: \"%s\"", WiFi.localIP().toString().c_str());
            break;
          case SYSTEM_EVENT_STA_BSS_RSSI_LOW:
            ESP_LOGI("WiFiEvent", "Station RSSI low: \"%i\"", WiFi.RSSI());
            break;
          case SYSTEM_EVENT_STA_WPS_ER_SUCCESS:
            //ESP_LOGI("WiFiEvent", "Station WPS ER Success!");
            break;
          case SYSTEM_EVENT_STA_WPS_ER_FAILED:
            //ESP_LOGI("WiFiEvent", "Station WPS ER failed!");
            break;
          case SYSTEM_EVENT_STA_WPS_ER_TIMEOUT:
            ESP_LOGI("WiFiEvent", "Station WPS ER timeout!");
            break;
          case SYSTEM_EVENT_STA_WPS_ER_PIN:
            ESP_LOGI("WiFiEvent", "Station WPS ER Pin!");
            break;
          case SYSTEM_EVENT_STA_WPS_ER_PBC_OVERLAP:
            ESP_LOGI("WiFiEvent", "Station WPS ER overlap!");
            break;
          case SYSTEM_EVENT_AP_START:
            ESP_LOGI("WiFiEvent", "Access Point start!");
            m_AccessPoint_Running = true;
            BeginWebServer();
            break;
          case SYSTEM_EVENT_AP_STOP:
            ESP_LOGI("WiFiEvent", "Access Point stop!");
            m_AccessPoint_Running = false;
            if(!m_Station_Running && !m_AccessPoint_Running)
            {
              EndWebServer();
            }
            break;
          case SYSTEM_EVENT_AP_STACONNECTED:
            ESP_LOGI("WiFiEvent", "Station Connected to the Access Point.");
            break;
          case SYSTEM_EVENT_AP_STADISCONNECTED:
            ESP_LOGI("WiFiEvent", "Station Disconnected from the Access Point.");
            break;
          case SYSTEM_EVENT_AP_STAIPASSIGNED:
          {
            ip_event_got_ip_t* ip_event = (ip_event_got_ip_t*) event;
            IPAddress assignedIP(ip_event->ip_info.ip.addr);
            ESP_LOGI("WiFiEvent", "Assigned IP address: \"%s\"", assignedIP.toString().c_str());
            break;
          }
          case SYSTEM_EVENT_AP_PROBEREQRECVED:
            ESP_LOGI("WiFiEvent", "Prob Error Received");
            break;
          case SYSTEM_EVENT_ACTION_TX_STATUS:
            ESP_LOGI("WiFiEvent", "Action Tx Status");
            break;
          case SYSTEM_EVENT_ROC_DONE:
            ESP_LOGI("WiFiEvent", "ROC Done");
            break;
          case SYSTEM_EVENT_STA_BEACON_TIMEOUT:
            ESP_LOGI("WiFiEvent", "Beacon Timeout");
            break;
          case SYSTEM_EVENT_FTM_REPORT:
            ESP_LOGI("WiFiEvent", "FTM Report");
            break;
          case SYSTEM_EVENT_GOT_IP6:
            ESP_LOGI("WiFiEvent", "Got IP6");
            break;
          case SYSTEM_EVENT_ETH_START:
            ESP_LOGI("WiFiEvent", "ETH Start");
            break;
          case SYSTEM_EVENT_ETH_STOP:
            ESP_LOGI("WiFiEvent", "ETH Stop");
            break;
          case SYSTEM_EVENT_ETH_CONNECTED:
            ESP_LOGI("WiFiEvent", "ETH Connected");
            break;
          case SYSTEM_EVENT_ETH_DISCONNECTED:
            ESP_LOGI("WiFiEvent", "ETH Disconnected");
            break;
          case SYSTEM_EVENT_ETH_GOT_IP:
            ESP_LOGI("WiFiEvent", "ETH Got IP");
            break;
          case SYSTEM_EVENT_ETH_LOST_IP:
            ESP_LOGI("WiFiEvent", "ETH Lost IP");
            break;
          case SYSTEM_EVENT_MAX:
            ESP_LOGI("WiFiEvent", "Event Max");
            break;
          default:
            ESP_LOGE("WiFiEvent", "Unhandled Event!");
            break;
      }
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
    WebSocketDataProcessor m_WebSocketDataProcessor{m_WebServer, m_WebSocket};
    bool m_AccessPoint_Running = false;
    bool m_Station_Running = false;
    bool m_Web_Server_Running = false;

    struct CallbackArguments 
    {
      void* arg1;
      void* arg2;
    };

    const ValidStringValues_t validBoolValues = { "0", "1" };

    //Wifi Mode
    const ValidStringValues_t m_Wifi_Mode_ValidValues = { "Station", "AccessPoint" };
    CallbackArguments m_Wifi_Mode_CallbackArgs = { this };
    NamedCallback_t m_Wifi_Mode_Callback = { "Wifi Mode Callback"
                                           , &Wifi_Mode_ValueChanged
                                           , &m_Wifi_Mode_CallbackArgs };
    const Wifi_Mode_t m_Wifi_Mode_InitialValue = Wifi_Mode_t::AccessPoint;
    LocalDataItemWithPreferences<Wifi_Mode_t, 1> m_Wifi_Mode = LocalDataItemWithPreferences<Wifi_Mode_t, 1>( "WIFI_Mode"
                                                                                                           , m_Wifi_Mode_InitialValue
                                                                                                           , &m_preferenceInterface
                                                                                                           , &m_Wifi_Mode_Callback
                                                                                                           , this
                                                                                                           , &m_Wifi_Mode_ValidValues );
    WebSocketDataHandler<Wifi_Mode_t, 1> m_Wifi_Mode_DataHandler = WebSocketDataHandler<Wifi_Mode_t, 1>( m_WebSocketDataProcessor, m_Wifi_Mode, false );
    static void Wifi_Mode_ValueChanged(const String &Name, void* object, void* arg)
    {
      if(arg && object)
      {
        CallbackArguments* pArguments = static_cast<CallbackArguments*>(arg);
        assert((pArguments->arg1) && "Null Pointers!");
        SettingsWebServerManager* pSettingWebServer = static_cast<SettingsWebServerManager*>(object);
        Wifi_Mode_t* pWifi_Mode = static_cast<Wifi_Mode_t*>(object);
        ESP_LOGI("Wifi_Mode_ValueChanged", "Wifi Mode Changed: %i", *pWifi_Mode);
        //TBD THIS NEED COMPLETED
      }
    }

    //WIFI Host Name
    CallbackArguments m_Host_Name_CallbackArgs = { this };
    NamedCallback_t m_Host_Name_Callback = { "Host Name"
                                          , &Host_Name_ValueChanged
                                          , &m_Host_Name_CallbackArgs };
    const String m_Host_Name_InitialValue = "LTOP";
    LocalStringDataItemWithPreferences m_Host_Name = LocalStringDataItemWithPreferences( "Host_Name"
                                                                                       , m_Host_Name_InitialValue
                                                                                       , &m_preferenceInterface
                                                                                       , &m_Host_Name_Callback
                                                                                       , this );
    WebSocket_String_DataHandler m_Host_Name_DataHandler = WebSocket_String_DataHandler( m_WebSocketDataProcessor, m_Host_Name, false );
    static void Host_Name_ValueChanged(const String &Name, void* object, void* arg)
    {
      if(arg && object)
      {
        CallbackArguments* pArguments = static_cast<CallbackArguments*>(arg);
        assert((pArguments->arg1) && "Null Pointers!");
        SettingsWebServerManager* pSettingWebServer = static_cast<SettingsWebServerManager*>(object);
        char* pHostName = static_cast<char*>(object);
        ESP_LOGI("Host_Name_ValueChanged", "Host Name Changed: %s", pHostName);
        //TBD THIS NEED COMPLETED
      }
    }

    //WIFI Station SSID
    CallbackArguments m_STA_SSID_CallbackArgs = { this };
    NamedCallback_t m_STA_SSID_Callback = { "Station SSID Callback"
                                          , &STA_SSID_ValueChanged
                                          , &m_STA_SSID_CallbackArgs };
    const String m_STA_SSID_InitialValue = "";
    LocalStringDataItemWithPreferences m_STA_SSID = LocalStringDataItemWithPreferences( "STA_SSID"
                                                                                      , m_STA_SSID_InitialValue
                                                                                      , &m_preferenceInterface
                                                                                      , &m_STA_SSID_Callback
                                                                                      , this );
    WebSocket_String_DataHandler m_STA_SSID_DataHandler = WebSocket_String_DataHandler( m_WebSocketDataProcessor, m_STA_SSID, false );
    static void STA_SSID_ValueChanged(const String &Name, void* object, void* arg)
    {
      if(arg && object)
      {
        CallbackArguments* pArguments = static_cast<CallbackArguments*>(arg);
        assert((pArguments->arg1) && "Null Pointers!");
        SettingsWebServerManager* pSettingWebServer = static_cast<SettingsWebServerManager*>(object);
        char* pSSID = static_cast<char*>(object);
        ESP_LOGI("STA_SSID_ValueChanged", "Station SSID Changed: %s", pSSID);
        //TBD THIS NEED COMPLETED
      }
    }

    //WIFI Access Point Password
    CallbackArguments m_STA_Password_CallbackArgs = { this };
    NamedCallback_t m_STA_Password_Callback = { "Station Password Callback"
                                              , &STA_Password_ValueChanged
                                              , &m_STA_Password_CallbackArgs };
    const String m_STA_Password_InitialValue = "";
    LocalStringDataItemWithPreferences m_STA_Password = LocalStringDataItemWithPreferences( "STA_Password"
                                                                                          , m_STA_Password_InitialValue
                                                                                          , &m_preferenceInterface
                                                                                          , &m_STA_Password_Callback
                                                                                          , this );
    WebSocket_String_DataHandler m_STA_Password_DataHandler = WebSocket_String_DataHandler( m_WebSocketDataProcessor, m_STA_Password, false );
    static void STA_Password_ValueChanged(const String &Name, void* object, void* arg)
    {
      if(arg && object)
      {
        CallbackArguments* pArguments = static_cast<CallbackArguments*>(arg);
        assert((pArguments->arg1) && "Null Pointers!");
        SettingsWebServerManager* pSettingWebServer = static_cast<SettingsWebServerManager*>(object);
        char* pPassword = static_cast<char*>(object);
        ESP_LOGI("STA_Password_ValueChanged", "Station Password Changed: %s", pPassword);
      }
    }

    //WIFI Access Point SSID
    CallbackArguments m_AP_SSID_CallbackArgs = { this };
    NamedCallback_t m_AP_SSID_Callback = { "Access Point SSID Callback"
                                      , &AP_SSID_ValueChanged
                                      , &m_AP_SSID_CallbackArgs };
    const String m_AP_SSID_InitialValue = "LED Tower of Power";
    LocalStringDataItemWithPreferences m_AP_SSID = LocalStringDataItemWithPreferences( "AP_SSID"
                                                                                     , m_AP_SSID_InitialValue
                                                                                     , &m_preferenceInterface
                                                                                     , &m_AP_SSID_Callback
                                                                                     , this );
    WebSocket_String_DataHandler m_AP_SSID_DataHandler = WebSocket_String_DataHandler( m_WebSocketDataProcessor, m_AP_SSID, false );
    static void AP_SSID_ValueChanged(const String &Name, void* object, void* arg)
    {
      if(arg && object)
      {
        CallbackArguments* pArguments = static_cast<CallbackArguments*>(arg);
        assert((pArguments->arg1) && "Null Pointers!");
        SettingsWebServerManager* pSettingWebServer = static_cast<SettingsWebServerManager*>(object);
        char* pSSID = static_cast<char*>(object);
        ESP_LOGI("SSID_ValueChanged", "Access Point SSID Changed: %s", pSSID);
        //TBD THIS NEED COMPLETED
      }
    }

    //WIFI Access Point Password
    CallbackArguments m_AP_Password_CallbackArgs = { this };
    NamedCallback_t m_AP_Password_Callback = { "Access Point Password Callback"
                                          , &AP_Password_ValueChanged
                                          , &m_AP_Password_CallbackArgs };
    const String m_AP_Password_InitialValue = "LEDs Rock";
    LocalStringDataItemWithPreferences m_AP_Password = LocalStringDataItemWithPreferences( "AP_Password"
                                                                                         , m_AP_Password_InitialValue
                                                                                         , &m_preferenceInterface
                                                                                         , &m_AP_Password_Callback
                                                                                         , this );
    WebSocket_String_DataHandler m_AP_Password_DataHandler = WebSocket_String_DataHandler( m_WebSocketDataProcessor, m_AP_Password, false );
    static void AP_Password_ValueChanged(const String &Name, void* object, void* arg)
    {
      if(arg && object)
      {
        CallbackArguments* pArguments = static_cast<CallbackArguments*>(arg);
        assert((pArguments->arg1) && "Null Pointers!");
        SettingsWebServerManager* pSettingWebServer = static_cast<SettingsWebServerManager*>(object);
        char* pPassword = static_cast<char*>(object);
        ESP_LOGI("Password_ValueChanged", "Access Point Password Changed: %s", pPassword);
      }
    }

    //Amplitude Gain
    const float m_AmplitudeGain_InitialValue = 2.0;
    DataItemWithPreferences <float, 1> m_AmplitudeGain = DataItemWithPreferences<float, 1>( "Amp_Gain"
                                                                                          , m_AmplitudeGain_InitialValue
                                                                                          , RxTxType_Tx_On_Change_With_Heartbeat
                                                                                          , 5000
                                                                                          , &m_preferenceInterface
                                                                                          , &m_CPU2SerialPortMessageManager
                                                                                          , nullptr
                                                                                          , this );
    WebSocketDataHandler<float, 1> m_Amplitude_Gain_DataHandler = WebSocketDataHandler<float, 1>( m_WebSocketDataProcessor
                                                                                                , m_AmplitudeGain
                                                                                                , false );    
    
    //FFT Gain
    const float m_FFTGain_InitialValue = 2.0;
    DataItemWithPreferences <float, 1> m_FFTGain = DataItemWithPreferences<float, 1>( "FFT_Gain"
                                                                                    , m_FFTGain_InitialValue
                                                                                    , RxTxType_Tx_On_Change_With_Heartbeat
                                                                                    , 5000
                                                                                    , &m_preferenceInterface
                                                                                    , &m_CPU2SerialPortMessageManager
                                                                                    , nullptr
                                                                                    , this );
    WebSocketDataHandler<float, 1> m_FFT_Gain_DataHandler = WebSocketDataHandler<float, 1>( m_WebSocketDataProcessor
                                                                                          , m_FFTGain
                                                                                          , false );

    //Microphone Enable
    //TBD

    //Input Source
    const ValidStringValues_t validInputSourceValues = { "OFF", "Microphone", "Bluetooth" };
    DataItemWithPreferences<SoundInputSource_t, 1> m_SoundInputSource = DataItemWithPreferences<SoundInputSource_t, 1>( "Input_Source"
                                                                                                                      , SoundInputSource_t::OFF
                                                                                                                      , RxTxType_Tx_On_Change_With_Heartbeat
                                                                                                                      , 5000
                                                                                                                      , &m_preferenceInterface
                                                                                                                      , &m_CPU1SerialPortMessageManager
                                                                                                                      , nullptr
                                                                                                                      , this
                                                                                                                      , &validInputSourceValues );
    WebSocketDataHandler<SoundInputSource_t, 1> m_SoundInputSource_DataHandler = WebSocketDataHandler<SoundInputSource_t, 1>( m_WebSocketDataProcessor
                                                                                                                            , m_SoundInputSource
                                                                                                                            , false );
    
    //Output Source
    const ValidStringValues_t validOutputSourceValues = { "OFF", "Bluetooth" };
    DataItemWithPreferences<SoundOutputSource_t, 1> m_SoundOuputSource = DataItemWithPreferences<SoundOutputSource_t, 1>( "Output_Source"
                                                                                                                        , SoundOutputSource_t::OFF
                                                                                                                        , RxTxType_Tx_On_Change_With_Heartbeat
                                                                                                                        , 5000
                                                                                                                        , &m_preferenceInterface
                                                                                                                        , &m_CPU2SerialPortMessageManager
                                                                                                                        , nullptr
                                                                                                                        , this
                                                                                                                        , &validOutputSourceValues);
    WebSocketDataHandler<SoundOutputSource_t, 1> m_SoundOuputSource_DataHandler = WebSocketDataHandler<SoundOutputSource_t, 1>( m_WebSocketDataProcessor
                                                                                                                              , m_SoundOuputSource
                                                                                                                              , false );
    
    //Bluetooth Sink Enable
    const bool m_BluetoothSinkEnable_InitialValue = false;
    DataItemWithPreferences<bool, 1> m_BluetoothSinkEnable = DataItemWithPreferences<bool, 1>( "BT_Sink_En", m_BluetoothSinkEnable_InitialValue, RxTxType_Tx_On_Change_With_Heartbeat, 5000, &m_preferenceInterface, &m_CPU1SerialPortMessageManager, nullptr, this, nullptr);
    WebSocketDataHandler<bool, 1> m_BluetoothSinkEnable_DataHandler = WebSocketDataHandler<bool, 1>( m_WebSocketDataProcessor, m_BluetoothSinkEnable, false );

    //Sink Name
    const String m_SinkName_InitialValue = "LED Tower of Power";  
    StringDataItemWithPreferences m_SinkName = StringDataItemWithPreferences( "Sink_Name", m_SinkName_InitialValue, RxTxType_Tx_On_Change_With_Heartbeat, 5000, &m_preferenceInterface, &m_CPU1SerialPortMessageManager, nullptr, this);
    WebSocket_String_DataHandler m_SinkName_DataHandler = WebSocket_String_DataHandler( m_WebSocketDataProcessor, m_SinkName, false );

    //Source Name
    const String m_SourceName_InitialValue = "";  
    StringDataItemWithPreferences m_SourceName = StringDataItemWithPreferences( "Source_Name", m_SourceName_InitialValue, RxTxType_Rx_Only, 0, &m_preferenceInterface, &m_CPU2SerialPortMessageManager, nullptr, this);
    WebSocket_String_DataHandler m_SourceName_DataHandler = WebSocket_String_DataHandler( m_WebSocketDataProcessor, m_SourceName, false );

    //Sink Connection State
    const ConnectionStatus_t m_SinkConnectionState_InitialValue = ConnectionStatus_t::Disconnected;
    DataItem<ConnectionStatus_t, 1> m_SinkConnectionState = DataItem<ConnectionStatus_t, 1>( "Sink_Conn_State", m_SinkConnectionState_InitialValue, RxTxType_Rx_Only, 0, &m_CPU1SerialPortMessageManager, nullptr, this);
    WebSocketDataHandler<ConnectionStatus_t, 1> m_SinkConnectionStatus_DataHandler = WebSocketDataHandler<ConnectionStatus_t, 1>( m_WebSocketDataProcessor, m_SinkConnectionState, false );    
    
    //Bluetooth Sink Auto Reconnect
    const bool m_BluetoothSinkAutoReConnect_InitialValue = false;
    DataItemWithPreferences<bool, 1> m_BluetoothSinkAutoReConnect = DataItemWithPreferences<bool, 1>( "BT_Sink_AR", m_BluetoothSinkAutoReConnect_InitialValue, RxTxType_Tx_On_Change_With_Heartbeat, 5000, &m_preferenceInterface, &m_CPU1SerialPortMessageManager, nullptr, this, nullptr);
    WebSocketDataHandler<bool, 1> m_BluetoothSinkAutoReConnect_DataHandler = WebSocketDataHandler<bool, 1>( m_WebSocketDataProcessor, m_BluetoothSinkAutoReConnect, false );
    
    //Bluetooth Source Enable
    const bool m_BluetoothSourceEnable_InitialValue = false;
    DataItemWithPreferences<bool, 1> m_BluetoothSourceEnable = DataItemWithPreferences<bool, 1>( "BT_Source_En", m_BluetoothSourceEnable_InitialValue, RxTxType_Tx_On_Change_With_Heartbeat, 5000, &m_preferenceInterface, &m_CPU2SerialPortMessageManager, nullptr, this, nullptr);
    WebSocketDataHandler<bool, 1> m_BluetoothSourceEnable_DataHandler = WebSocketDataHandler<bool, 1>( m_WebSocketDataProcessor, m_BluetoothSourceEnable, false );

    //Target Device
    CompatibleDevice_t m_TargetCompatibleDevice_InitialValue = {"", ""};
    DataItem<CompatibleDevice_t, 1> m_TargetCompatibleDevice = DataItem<CompatibleDevice_t, 1>( "Target_Device", m_TargetCompatibleDevice_InitialValue, RxTxType_Tx_On_Change_With_Heartbeat, 5000, &m_CPU2SerialPortMessageManager, nullptr, this);
    WebSocketDataHandler<CompatibleDevice_t, 1> m_TargetCompatibleDevice_DataHandler = WebSocketDataHandler<CompatibleDevice_t, 1>( m_WebSocketDataProcessor, m_TargetCompatibleDevice, false );

    //Sink Connect
    CallbackArguments m_SinkConnect_CallbackArgs = {this};
    NamedCallback_t m_SinkConnect_Callback = {"m_SinkConnect_Callback", &SinkConnect_ValueChanged, &m_SinkConnect_CallbackArgs};
    const bool m_SinkConnect_InitialValue = false;
    DataItem<bool, 1> m_SinkConnect = DataItem<bool, 1>( "Sink_Connect", m_SinkConnect_InitialValue, RxTxType_Tx_On_Change_With_Heartbeat, 5000, &m_CPU1SerialPortMessageManager, &m_SinkConnect_Callback, this);
    WebSocketDataHandler<bool, 1> m_SinkConnect_DataHandler = WebSocketDataHandler<bool, 1>( m_WebSocketDataProcessor, m_SinkConnect, false );
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
    DataItem<bool, 1> m_SinkDisconnect = DataItem<bool, 1>( "Sink_Disconnect", m_SinkDisconnect_InitialValue, RxTxType_Tx_On_Change_With_Heartbeat, 5000, &m_CPU1SerialPortMessageManager, &m_SinkDisconnect_Callback, this);
    WebSocketDataHandler<bool, 1> m_SinkDisconnect_DataHandler = WebSocketDataHandler<bool, 1>( m_WebSocketDataProcessor, m_SinkDisconnect, false );
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
    DataItem<bool, 1> m_SourceConnect = DataItem<bool, 1>( "Src_Connect", m_SourceConnect_InitialValue, RxTxType_Tx_On_Change_With_Heartbeat, 5000, &m_CPU2SerialPortMessageManager, &m_SourceConnect_Callback, this);
    WebSocketDataHandler<bool, 1> m_SourceConnect_DataHandler = WebSocketDataHandler<bool, 1>( m_WebSocketDataProcessor, m_SourceConnect, false );
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
          ESP_LOGE("SourceConnect_ValueChanged", "ERROR! Invalid Pointer.");
        }
      }
    }

    //Output Source Disconnect
    CallbackArguments m_SourceDisconnect_CallbackArgs = {&m_TargetCompatibleDevice, &m_TargetCompatibleDevice_InitialValue};
    NamedCallback_t m_SourceDisconnect_Callback = {"m_SourceDisconnect_Callback", &SourceDisconnect_ValueChanged, &m_SourceDisconnect_CallbackArgs};
    const bool m_SourceDisconnect_InitialValue = false;
    DataItem<bool, 1> m_SourceDisconnect = DataItem<bool, 1>( "Src_Disconnect", m_SourceDisconnect_InitialValue, RxTxType_Tx_On_Change_With_Heartbeat, 5000, &m_CPU2SerialPortMessageManager, &m_SourceDisconnect_Callback, this);
    WebSocketDataHandler<bool, 1> m_SourceDisconnect_DataHandler = WebSocketDataHandler<bool, 1>( m_WebSocketDataProcessor, m_SourceDisconnect, false );
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
          ESP_LOGE("OuputSourceDisconnect_ValueChanged", "ERROR! Invalid Pointer.");
        }
      }
    }

    //Scanned Device
    CallbackArguments m_ScannedDevice_CallbackArgs = {&m_WebSocketDataProcessor, &m_ScannedDevice_DataHandler};
    NamedCallback_t m_ScannedDevice_Callback = {"m_ScannedDevice_Callback", &ScannedDevice_ValueChanged, &m_ScannedDevice_CallbackArgs};
    ActiveCompatibleDevice_t m_ScannedDevice_InitialValue = {"", "", 0, 0, 0};
    DataItem<ActiveCompatibleDevice_t, 1> m_ScannedDevice = DataItem<ActiveCompatibleDevice_t, 1>( "Scan_BT_Device", m_ScannedDevice_InitialValue, RxTxType_Rx_Only, 0, &m_CPU2SerialPortMessageManager, &m_ScannedDevice_Callback, this);
    WebSocketDataHandler<ActiveCompatibleDevice_t,1> m_ScannedDevice_DataHandler = WebSocketDataHandler<ActiveCompatibleDevice_t,1> ( m_WebSocketDataProcessor, m_ScannedDevice, false );
    static void ScannedDevice_ValueChanged(const String &Name, void* object, void* arg)
    {
      ESP_LOGD("Manager::ScannedDeviceValueChanged", "Scanned Device Value Changed");
      CallbackArguments* arguments = static_cast<CallbackArguments*>(arg);
      WebSocketDataProcessor* processor = static_cast<WebSocketDataProcessor*>(arguments->arg1);
      WebSocketDataHandler<ActiveCompatibleDevice_t,1> * DataHandler = static_cast<WebSocketDataHandler<ActiveCompatibleDevice_t,1> *>(arguments->arg2);
      //DELETE ME processor->UpdateDataForSender(DataHandler, false);
    }
    
    //Bluetooth Source Auto Reconnect
    const bool m_BluetoothSourceAutoReConnect_InitialValue = false;
    DataItemWithPreferences<bool, 1> m_BluetoothSourceAutoReConnect = DataItemWithPreferences<bool, 1>( "BT_Source_AR", m_BluetoothSourceAutoReConnect_InitialValue, RxTxType_Tx_On_Change_With_Heartbeat, 5000, &m_preferenceInterface, &m_CPU2SerialPortMessageManager, nullptr, this, &validBoolValues);
    WebSocketDataHandler<bool, 1> m_BluetoothSourceAutoReConnect_DataHandler = WebSocketDataHandler<bool, 1>( m_WebSocketDataProcessor, m_BluetoothSourceAutoReConnect, false );

    //Source Connection State
    const ConnectionStatus_t m_SourceConnectionState_InitialValue = ConnectionStatus_t::Disconnected;
    DataItem<ConnectionStatus_t, 1> m_SourceConnectionState = DataItem<ConnectionStatus_t, 1>( "Src_Conn_State", m_SourceConnectionState_InitialValue, RxTxType_Rx_Only, 0, &m_CPU2SerialPortMessageManager, nullptr, this);
    WebSocketDataHandler<ConnectionStatus_t, 1> m_SourceConnectionState_DataHandler = WebSocketDataHandler<ConnectionStatus_t, 1>( m_WebSocketDataProcessor, m_SourceConnectionState, false );    

    //Source Reset
    const bool m_SourceReset_InitialValue = false;
    DataItemWithPreferences<bool, 1> m_SourceReset = DataItemWithPreferences<bool, 1>( "BT_Src_Reset", m_SourceReset_InitialValue, RxTxType_Tx_On_Change_With_Heartbeat, 5000, &m_preferenceInterface, &m_CPU2SerialPortMessageManager, nullptr, this, &validBoolValues);
    WebSocketDataHandler<bool, 1> m_SourceReset_DataHandler = WebSocketDataHandler<bool, 1>( m_WebSocketDataProcessor, m_SourceReset, false );    
    
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
            ESP_LOGE("SettingsWebServer: HandleWebSocketMessage", "ERROR! Parsing failed for Input: %s.", WebSocketData.c_str());
          }
          else
          {
            if(HandleSignalValue(jsonObject)){}
            else if(HandleJSONValue(jsonObject)){}
            else ESP_LOGE("SettingsWebServer: HandleWebSocketMessage", "ERROR! Unknown Web Socket Message: %s.", WebSocketData.c_str());
          }
        }
      }
    }
    
    bool HandleSignalValue(JSONVar &jsonObject)
    {
      bool result = false;
      if(jsonObject.hasOwnProperty("SignalValue"))
      {
        result = true;
        JSONVar signalValue = jsonObject["SignalValue"];
        if (signalValue.hasOwnProperty("Id") && signalValue.hasOwnProperty("Value"))
        {
          const String Id = signalValue["Id"];
          const String Value = signalValue["Value"];
          ESP_LOGD( "SettingsWebServer: HandleWebSocketMessage", "Signal Value Message Received. ID: \"%s\" Value: \"%s\""
                  , Id.c_str()
                  , Value.c_str() );
          if(!m_WebSocketDataProcessor.ProcessSignalValueAndSendToDatalink(Id, Value))
          {
            ESP_LOGE("SettingsWebServer: HandleWebSocketMessage", "ERROR! Unknown Signal Value Object: %s.", Id.c_str());
          }
        }
        else
        {
          ESP_LOGD("SettingsWebServer: HandleWebSocketMessage", "Unknown JSON Object: %s", JSON.stringify(signalValue).c_str());
        }
      }
      return result;
    }

    bool HandleJSONValue(JSONVar &jsonObject)
    {
      bool result = false;
      if(jsonObject.hasOwnProperty("JSONValue"))
      {
        result = true;
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
            ESP_LOGE("SettingsWebServer: HandleWebSocketMessage", "ERROR! Unknown JSON Object: %s.", Id.c_str());
          }
        }
        else
        {
          ESP_LOGD("SettingsWebServer: HandleWebSocketMessage", "Known JSON Object: %s", JSON.stringify(jSONValue).c_str());
        }
      }
      return result;
    }
    
    void BeginWebServer()
    {
      m_WebServer.begin();
      m_Web_Server_Running = true;
    }

    void EndWebServer()
    {
      m_WebServer.end();
      m_Web_Server_Running = false;
    }

    void Start_DNS_Server(const char* myHostName)
    {
      if (MDNS.begin(myHostName))
      {
        ESP_LOGI( "SettingsWebServer: InitWifiClient", "Started DNS Server with Host Name: \"%s\"", myHostName);
      }
      else
      {
        ESP_LOGE( "SettingsWebServer: InitWifiClient", "Unable to start DNS Server with Host Name: \"%s\"", myHostName);
      }
      MDNS.addService("http", "tcp", 80);
    }

    void InitWiFi_Station(const char* staSSID, const char* staPassword, const char* myHostName)
    {
      ESP_LOGI( "SettingsWebServer: InitWifiClient", "Starting Wifi Station Mode: SSID: \"%s\" Password: \"%s\" Host Name: \"%s\"", staSSID, staPassword, myHostName);
      WiFi.mode(WIFI_STA);
      WiFi.setHostname(myHostName);
      WiFi.begin(staSSID, staPassword);
    }

    bool InitWiFi_AccessPoint(const char* apSSID, const char* apPassword, const char* myHostName)
    {
      ESP_LOGI( "SettingsWebServer: InitWifiClient", "Starting Wifi Access Point Mode: SSID: \"%s\" Password: \"%s\" Host Name: \"%s\"", apSSID, apPassword, myHostName);
      WiFi.mode(WIFI_AP);
      IPAddress Ip(192, 168, 0, 1);
      IPAddress NMask(255, 255, 255, 0);
      WiFi.softAPConfig(Ip, Ip, NMask);
      WiFi.softAP(apSSID, apPassword);
      Start_DNS_Server(myHostName);
    }

    void InitWiFi_AccessPoint_Station(const char* apSSID, const char* apPassword, const char* staSSID, const char* staPassword, const char* myHostName)
    {
      ESP_LOGI( "SettingsWebServer: InitWifiClient", "Starting Wifi Access Point + Station Mode: Access Point SSID: \"%s\" Access Point Password: \"%s\" Station SSID: \"%s\" Station Password: \"%s\" Host Name: \"%s\"", apSSID, apPassword, staSSID, staPassword, myHostName);
      WiFi.mode(WIFI_AP_STA);
      WiFi.begin(staSSID, staPassword);
      IPAddress Ip(192, 168, 0, 1);
      IPAddress NMask(255, 255, 255, 0);
      WiFi.softAPConfig(Ip, Ip, NMask);
      WiFi.softAP(apSSID, apPassword);
      Start_DNS_Server(myHostName);
    }
};