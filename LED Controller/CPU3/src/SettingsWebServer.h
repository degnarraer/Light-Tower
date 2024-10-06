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

#define BLUETOOTH_DEVICE_TIMEOUT 10000

class SettingsWebServerManager: public SetupCallerInterface
{  
  public:
    SettingsWebServerManager( String Title
                            , WebSocketsServer &WebSocket
                            , WebServer &WebServer
                            , IPreferences &preferenceInterface
                            , SerialPortMessageManager &CPU1SerialPortMessageManager
                            , SerialPortMessageManager &CPU2SerialPortMessageManager)
                            : m_WebSocket(WebSocket)
                            , m_WebServer(WebServer)
                            , m_preferenceInterface(preferenceInterface)
                            , m_CPU1SerialPortMessageManager(CPU1SerialPortMessageManager)
                            , m_CPU2SerialPortMessageManager(CPU2SerialPortMessageManager)
    {
      WiFi.onEvent(std::bind( &SettingsWebServerManager::OnWiFiEvent
                            , this
                            , std::placeholders::_1 // event
                            ));

      m_WebSocket.onEvent( std::bind(&SettingsWebServerManager::OnWebSocketEvent
                         , this
                         , std::placeholders::_1 // client
                         , std::placeholders::_2 // type
                         , std::placeholders::_3 // Payload
                         , std::placeholders::_4 // length
                         ));
    }
    
    virtual ~SettingsWebServerManager()
    {
    }
    
    void SetupSettingsWebServerManager()
    {
      InitializeLocalvariables();
      SetupAllSetupCallees();
      InitFileSystem();
      InitWebServer();
      StartWiFi();
    }

    void InitializeLocalvariables()
    {
    }

    void StartWiFi()
    {
      ESP_LOGI("StartWiFi", "Starting Wifi");
      if(m_WiFi_Ready)
      {
        EndWiFi();
      }
      if( Wifi_Mode_t::AccessPoint == m_Wifi_Mode.GetValue() )
      {
        InitWiFi_AccessPoint( m_AP_SSID.GetValueAsString().c_str()
                            , m_AP_Password.GetValueAsString().c_str() );
      }
      else if(Wifi_Mode_t::Station == m_Wifi_Mode.GetValue())
      {
        InitWiFi_AccessPoint_Station( m_AP_SSID.GetValueAsString().c_str()
                                    , m_AP_Password.GetValueAsString().c_str()
                                    , m_STA_SSID.GetValueAsString().c_str()
                                    , m_STA_Password.GetValueAsString().c_str() );
      }
      else
      {
        InitWiFi_AccessPoint_Station( "LED Tower of Power"
                                    , "LEDs Rock"
                                    , ""
                                    , "" );
      }
    }
    
    void EndWiFi()
    {
      ESP_LOGI("EndWiFi", "Ending Wifi");
      EndWebServer();
      WiFi.disconnect(false, true);
      m_WiFi_Ready = false;
    }

    // Init the web server to use the local SPIFFS memory and serve up index.html file.
    void InitWebServer()
    {
      // Serve the root path with the index.html file
      m_WebServer.on("/", HTTP_GET, [this]()
      {
          File file = SPIFFS.open("/index.html", "r"); // Open the index.html file
          if (!file)
          {
              ESP_LOGE("InitWebServer", "Failed to open index.html for reading");
              m_WebServer.send(404, "text/plain", "File Not Found"); // Send 404 if file not found
              return;
          }
          m_WebServer.streamFile(file, "text/html"); // Stream the index.html file
          file.close(); // Close the file after streaming
      });

      // Serve all other static files from SPIFFS
      m_WebServer.serveStatic("/", SPIFFS, "/"); // Serve static files
      
      // Set cache control for static files
      m_WebServer.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
      m_WebServer.sendHeader("Pragma", "no-cache");
      m_WebServer.sendHeader("Expires", "0");

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
    void OnWiFiEvent(WiFiEvent_t event)
    {
      switch (event) 
      {
          case SYSTEM_EVENT_WIFI_READY:
            ESP_LOGI("Wifi Event", "Wifi Ready!");
            m_WiFi_Ready = true;
            BeginWebServer();
            StartDNSServer();
          break;
          case SYSTEM_EVENT_SCAN_DONE:
            ESP_LOGI("Wifi Event", "Scan Done!");
          break;
          case SYSTEM_EVENT_STA_START:
            ESP_LOGI("Wifi Event", "Station started!");
            m_Station_Running = true;
          break;
          case SYSTEM_EVENT_STA_STOP:
            ESP_LOGI("Wifi Event", "Station stopped!");
            m_Station_Running = false;
          break;
          case SYSTEM_EVENT_STA_CONNECTED:
            ESP_LOGI("Wifi Event", "Station connected.");
          break;
          case SYSTEM_EVENT_STA_DISCONNECTED:
            ESP_LOGI("Wifi Event", "Station disconnected.");
          break;
          case SYSTEM_EVENT_STA_AUTHMODE_CHANGE:
            ESP_LOGI("Wifi Event", "Station Auth Mode Change.");
          break;
          case SYSTEM_EVENT_STA_GOT_IP:
            ESP_LOGI("Wifi Event", "Station Got IP address: \"%s\".", WiFi.localIP().toString().c_str());
          break;
          case SYSTEM_EVENT_STA_LOST_IP:
            ESP_LOGI("Wifi Event", "Station Lost IP address: \"%s\".", WiFi.localIP().toString().c_str());
          break;
          case SYSTEM_EVENT_STA_BSS_RSSI_LOW:
            ESP_LOGI("Wifi Event", "Station RSSI low: \"%i\".", WiFi.RSSI());
          break;
          case SYSTEM_EVENT_STA_WPS_ER_SUCCESS:
            ESP_LOGI("Wifi Event", "Station WPS ER Success!");
          break;
          case SYSTEM_EVENT_STA_WPS_ER_FAILED:
            ESP_LOGI("Wifi Event", "Station WPS ER failed!");
          break;
          case SYSTEM_EVENT_STA_WPS_ER_TIMEOUT:
            ESP_LOGI("Wifi Event", "Station WPS ER timeout.");
          break;
          case SYSTEM_EVENT_STA_WPS_ER_PIN:
            ESP_LOGI("Wifi Event", "Station WPS ER Pin.");
          break;
          case SYSTEM_EVENT_STA_WPS_ER_PBC_OVERLAP:
            ESP_LOGI("Wifi Event", "Station WPS ER overlap.");
          break;
          case SYSTEM_EVENT_AP_START:
            ESP_LOGI("Wifi Event", "Access Point started!");
            m_AccessPoint_Running = true;
          break;
          case SYSTEM_EVENT_AP_STOP:
            ESP_LOGI("Wifi Event", "Access Point stopped!");
            m_AccessPoint_Running = false;
          break;
          case SYSTEM_EVENT_AP_STACONNECTED:
            ESP_LOGI("Wifi Event", "Station Connected to the Access Point.");
          break;
          case SYSTEM_EVENT_AP_STADISCONNECTED:
            ESP_LOGI("Wifi Event", "Station Disconnected from the Access Point.");
          break;
          case SYSTEM_EVENT_AP_STAIPASSIGNED:
          {
            ip_event_got_ip_t* ip_event = (ip_event_got_ip_t*) &event;
            IPAddress assignedIP = IPAddress(ip_event->ip_info.ip.addr);
            ESP_LOGI("Wifi Event", "Assigned IP address: \"%s\".", assignedIP.toString().c_str());
          }
          break;
          case SYSTEM_EVENT_AP_PROBEREQRECVED:
            ESP_LOGI("Wifi Event", "Prob Error Received.");
          break;
          case SYSTEM_EVENT_ACTION_TX_STATUS:
            ESP_LOGI("Wifi Event", "Action Tx Status.");
          break;
          case SYSTEM_EVENT_ROC_DONE:
            ESP_LOGI("Wifi Event", "ROC Done.");
          break;
          case SYSTEM_EVENT_STA_BEACON_TIMEOUT:
            ESP_LOGI("Wifi Event", "Beacon Timeout.");
          break;
          case SYSTEM_EVENT_FTM_REPORT:
            ESP_LOGI("Wifi Event", "FTM Report.");
          break;
          case SYSTEM_EVENT_GOT_IP6:
            ESP_LOGI("Wifi Event", "Got IP6.");
          break;
          case SYSTEM_EVENT_ETH_START:
            ESP_LOGI("Wifi Event", "ETH Start.");
          break;
          case SYSTEM_EVENT_ETH_STOP:
            ESP_LOGI("Wifi Event", "ETH Stop.");
          break;
          case SYSTEM_EVENT_ETH_CONNECTED:
            ESP_LOGI("Wifi Event", "ETH Connected.");
          break;
          case SYSTEM_EVENT_ETH_DISCONNECTED:
            ESP_LOGI("Wifi Event", "ETH Disconnected.");
          break;
          case SYSTEM_EVENT_ETH_GOT_IP:
            ESP_LOGI("Wifi Event", "ETH Got IP.");
          break;
          case SYSTEM_EVENT_ETH_LOST_IP:
            ESP_LOGI("Wifi Event", "ETH Lost IP.");
          break;
          case SYSTEM_EVENT_MAX:
            ESP_LOGI("Wifi Event", "Event Max.");
          break;
          default:
            ESP_LOGE("Wifi Event", "Unhandled Event!");
          break;
      }
    }

    void OnWebSocketEvent(uint8_t clientID, WStype_t type, uint8_t *payload, size_t length)
    {
      switch (type) 
      {
          case WStype_CONNECTED:
          {
              IPAddress ip = m_WebSocket.remoteIP(clientID);
              ESP_LOGI("OnWebSocketEvent", "WebSocket client #%u connected from %s\n", clientID, ip.toString().c_str());
              break;
          }
          case WStype_PONG:
              ESP_LOGI("OnWebSocketEvent", "WebSocket client #%u Pinged Us!\n", clientID);
              break;
          case WStype_TEXT:
              ESP_LOGI("OnWebSocketEvent", "WebSocket client #%u sent Text #%s\n", clientID, String((char*)payload, length).c_str());
              HandleWebSocketMessage(clientID, type, payload, length);
              break;
          case WStype_ERROR:
              ESP_LOGI("OnWebSocketEvent", "WebSocket client #%u Error. Closing Connection!\n", clientID);
              break;
          case WStype_DISCONNECTED:
              ESP_LOGI("OnWebSocketEvent", "WebSocket client #%u disconnected. Closing Connection.\n", clientID);
              break;
      }
    }
  private:
    WebSocketsServer &m_WebSocket;
    WebServer &m_WebServer;
    IPreferences &m_preferenceInterface; 
    SerialPortMessageManager &m_CPU1SerialPortMessageManager;
    SerialPortMessageManager &m_CPU2SerialPortMessageManager;
    WebSocketDataProcessor m_WebSocketDataProcessor{m_WebServer, m_WebSocket};
    bool m_WiFi_Ready = false;
    bool m_AccessPoint_Running = false;
    bool m_Station_Running = false;
    bool m_Web_Server_Running = false;
    const int maxClients = 4;

    struct CallbackArguments 
    {
      void* arg1;
      void* arg2;
      void* arg3;
      void* arg4;
    };

    const ValidStringValues_t validBoolValues = { "0", "1" };

    //Wifi ReStart

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
    WebSocketDataHandler<Wifi_Mode_t, 1> m_Wifi_Mode_DataHandler = WebSocketDataHandler<Wifi_Mode_t, 1>( m_WebSocketDataProcessor, m_Wifi_Mode );
    static void Wifi_Mode_ValueChanged(const String &Name, void* object, void* arg)
    {
      ESP_LOGI("Wifi_Mode_ValueChanged", "Wifi Mode ValueChanged");
      if(object && arg)
      {
        Wifi_Mode_t wifiMode = *static_cast<Wifi_Mode_t*>(object);
        ESP_LOGI("Wifi_Mode_ValueChanged", "Wifi Mode Changed: \"%s\"", Wifi_Mode::ToString(wifiMode).c_str());
        CallbackArguments* pArguments = static_cast<CallbackArguments*>(arg);
        if((pArguments->arg1) && (pArguments->arg2))
        {
          SettingsWebServerManager* pSettingWebServer = static_cast<SettingsWebServerManager*>(pArguments->arg1);
          bool wifiReady = *static_cast<bool*>(pArguments->arg2);
          if(wifiReady)
          {
            //pSettingWebServer->StartWiFi();
          }
        }
      }
    }

    //WIFI Host Name
    CallbackArguments m_Host_Name_CallbackArgs = { this, &m_WiFi_Ready };
    NamedCallback_t m_Host_Name_Callback = { "Host Name"
                                           , &Host_Name_ValueChanged
                                           , &m_Host_Name_CallbackArgs };
    const std::string m_Host_Name_InitialValue = "LTOP";
    LocalStringDataItemWithPreferences m_Host_Name = LocalStringDataItemWithPreferences( "Host_Name"
                                                                                       , m_Host_Name_InitialValue
                                                                                       , &m_preferenceInterface
                                                                                       , &m_Host_Name_Callback
                                                                                       , this );
    WebSocket_String_DataHandler m_Host_Name_DataHandler = WebSocket_String_DataHandler( m_WebSocketDataProcessor, m_Host_Name );
    static void Host_Name_ValueChanged(const String &Name, void* object, void* arg)
    {
      if(object && arg)
      {
        char* pHostName = static_cast<char*>(object);
        ESP_LOGI("Host_Name_ValueChanged", "Host Name Changed: \"%s\"", pHostName);
        CallbackArguments* pArguments = static_cast<CallbackArguments*>(arg);
        if((pArguments->arg1) && (pArguments->arg2))
        {
          SettingsWebServerManager* pSettingWebServer = static_cast<SettingsWebServerManager*>(pArguments->arg1);
          bool wifiReady = *static_cast<bool*>(pArguments->arg2);
          if(wifiReady)
          {
            //pSettingWebServer->StartWiFi();
          }
        }
      }
    }

    //WIFI Station SSID
    CallbackArguments m_STA_SSID_CallbackArgs = { this, &m_WiFi_Ready };
    NamedCallback_t m_STA_SSID_Callback = { "Station SSID Callback"
                                          , &STA_SSID_ValueChanged
                                          , &m_STA_SSID_CallbackArgs };
    const std::string m_STA_SSID_InitialValue = "";
    LocalStringDataItemWithPreferences m_STA_SSID = LocalStringDataItemWithPreferences( "STA_SSID"
                                                                                      , m_STA_SSID_InitialValue
                                                                                      , &m_preferenceInterface
                                                                                      , &m_STA_SSID_Callback
                                                                                      , this );
    WebSocket_String_DataHandler m_STA_SSID_DataHandler = WebSocket_String_DataHandler( m_WebSocketDataProcessor, m_STA_SSID );
    static void STA_SSID_ValueChanged(const String &Name, void* object, void* arg)
    {
      if(object && arg)
      {
        char* pStationSSID = static_cast<char*>(object);
        ESP_LOGI("STA_SSID_ValueChanged", "Station SSID Changed: \"%s\"", pStationSSID);
        CallbackArguments* pArguments = static_cast<CallbackArguments*>(arg);
        if((pArguments->arg1) && (pArguments->arg2))
        {
          SettingsWebServerManager* pSettingWebServer = static_cast<SettingsWebServerManager*>(pArguments->arg1);
          bool wifiReady = *static_cast<bool*>(pArguments->arg2);
          if(wifiReady)
          {
            //pSettingWebServer->StartWiFi();
          }
        }
      }
    }

    //WIFI Access Point Password
    CallbackArguments m_STA_Password_CallbackArgs = { this, &m_WiFi_Ready };
    NamedCallback_t m_STA_Password_Callback = { "Station Password Callback"
                                              , &STA_Password_ValueChanged
                                              , &m_STA_Password_CallbackArgs };
    const std::string m_STA_Password_InitialValue = "";
    LocalStringDataItemWithPreferences m_STA_Password = LocalStringDataItemWithPreferences( "STA_Password"
                                                                                          , m_STA_Password_InitialValue
                                                                                          , &m_preferenceInterface
                                                                                          , &m_STA_Password_Callback
                                                                                          , this );
    WebSocket_String_DataHandler m_STA_Password_DataHandler = WebSocket_String_DataHandler( m_WebSocketDataProcessor, m_STA_Password );
    static void STA_Password_ValueChanged(const String &Name, void* object, void* arg)
    {
      if(object && arg)
      {
        CallbackArguments* pArguments = static_cast<CallbackArguments*>(arg);
        if(pArguments->arg1 && pArguments->arg2)
        {
          SettingsWebServerManager* pSettingWebServer = static_cast<SettingsWebServerManager*>(object);
          char* pPassword = static_cast<char*>(object);
          ESP_LOGI("STA_Password_ValueChanged", "Station Password Changed: %s", pPassword);
          bool wifiReady = *static_cast<bool*>(pArguments->arg2);
          if(wifiReady)
          {
            //pSettingWebServer->StartWiFi();
          }
        }
      }
    }

    //WIFI Access Point SSID
    CallbackArguments m_AP_SSID_CallbackArgs = { this, &m_WiFi_Ready };
    NamedCallback_t m_AP_SSID_Callback = { "Access Point SSID Callback"
                                         , &AP_SSID_ValueChanged
                                         , &m_AP_SSID_CallbackArgs };
    const std::string m_AP_SSID_InitialValue = "LED Tower of Power";
    LocalStringDataItemWithPreferences m_AP_SSID = LocalStringDataItemWithPreferences( "AP_SSID"
                                                       
                                                                                     , m_AP_SSID_InitialValue
                                                                                     , &m_preferenceInterface
                                                                                     , &m_AP_SSID_Callback
                                                                                     , this );
    WebSocket_String_DataHandler m_AP_SSID_DataHandler = WebSocket_String_DataHandler( m_WebSocketDataProcessor, m_AP_SSID );
    static void AP_SSID_ValueChanged(const String &Name, void* object, void* arg)
    {
      if(object && arg)
      {
        CallbackArguments* pArguments = static_cast<CallbackArguments*>(arg);
        if(pArguments->arg1 && pArguments->arg2)
        {
          SettingsWebServerManager* pSettingWebServer = static_cast<SettingsWebServerManager*>(object);
          char* pSSID = static_cast<char*>(object);
          ESP_LOGI("SSID_ValueChanged", "Access Point SSID Changed: %s", pSSID);
          
        }
      }
    }

    //WIFI Access Point Password
    CallbackArguments m_AP_Password_CallbackArgs = { this, &m_WiFi_Ready };
    NamedCallback_t m_AP_Password_Callback = { "Access Point Password Callback"
                                          , &AP_Password_ValueChanged
                                          , &m_AP_Password_CallbackArgs };
    const std::string m_AP_Password_InitialValue = "LEDs Rock";
    LocalStringDataItemWithPreferences m_AP_Password = LocalStringDataItemWithPreferences( "AP_Password"
                                                                                         , m_AP_Password_InitialValue
                                                                                         , &m_preferenceInterface
                                                                                         , &m_AP_Password_Callback
                                                                                         , this );
    WebSocket_String_DataHandler m_AP_Password_DataHandler = WebSocket_String_DataHandler( m_WebSocketDataProcessor, m_AP_Password );
    static void AP_Password_ValueChanged(const String &Name, void* object, void* arg)
    {
      if(object && arg)
      {
        CallbackArguments* pArguments = static_cast<CallbackArguments*>(arg);
        if(pArguments->arg1 && pArguments->arg2)
        {
          SettingsWebServerManager* pSettingWebServer = static_cast<SettingsWebServerManager*>(object);
          char* pPassword = static_cast<char*>(object);
          ESP_LOGI("Password_ValueChanged", "Access Point Password Changed: %s", pPassword);
          bool wifiReady = *static_cast<bool*>(pArguments->arg2);
          if(wifiReady)
          {
            //pSettingWebServer->StartWiFi();
          }
        }
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
    WebSocketDataHandler<float, 1> m_Amplitude_Gain_DataHandler = WebSocketDataHandler<float, 1>( m_WebSocketDataProcessor, m_AmplitudeGain );    
    
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
    WebSocketDataHandler<float, 1> m_FFT_Gain_DataHandler = WebSocketDataHandler<float, 1>( m_WebSocketDataProcessor, m_FFTGain );

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
    WebSocketDataHandler<SoundInputSource_t, 1> m_SoundInputSource_DataHandler = WebSocketDataHandler<SoundInputSource_t, 1>( m_WebSocketDataProcessor, m_SoundInputSource );
    
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
    WebSocketDataHandler<SoundOutputSource_t, 1> m_SoundOuputSource_DataHandler = WebSocketDataHandler<SoundOutputSource_t, 1>( m_WebSocketDataProcessor, m_SoundOuputSource );
    
    //Bluetooth Sink Enable
    const bool m_BluetoothSinkEnable_InitialValue = false;
    DataItemWithPreferences<bool, 1> m_BluetoothSinkEnable = DataItemWithPreferences<bool, 1>( "BT_Sink_En", m_BluetoothSinkEnable_InitialValue, RxTxType_Tx_On_Change_With_Heartbeat, 5000, &m_preferenceInterface, &m_CPU1SerialPortMessageManager, nullptr, this, nullptr);
    WebSocketDataHandler<bool, 1> m_BluetoothSinkEnable_DataHandler = WebSocketDataHandler<bool, 1>( m_WebSocketDataProcessor, m_BluetoothSinkEnable );

    //Sink Name
    const std::string m_SinkName_InitialValue = "LED Tower of Power";  
    StringDataItemWithPreferences m_SinkName = StringDataItemWithPreferences( "Sink_Name", m_SinkName_InitialValue, RxTxType_Tx_On_Change_With_Heartbeat, 5000, &m_preferenceInterface, &m_CPU1SerialPortMessageManager, nullptr, this);
    WebSocket_String_DataHandler m_SinkName_DataHandler = WebSocket_String_DataHandler( m_WebSocketDataProcessor, m_SinkName );

    //Source Name
    const std::string m_SourceName_InitialValue = "";  
    StringDataItemWithPreferences m_SourceName = StringDataItemWithPreferences( "Source_Name", m_SourceName_InitialValue, RxTxType_Rx_Only, 0, &m_preferenceInterface, &m_CPU2SerialPortMessageManager, nullptr, this);
    WebSocket_String_DataHandler m_SourceName_DataHandler = WebSocket_String_DataHandler( m_WebSocketDataProcessor, m_SourceName );

    //Sink Connection State
    const ConnectionStatus_t m_SinkConnectionState_InitialValue = ConnectionStatus_t::Disconnected;
    DataItem<ConnectionStatus_t, 1> m_SinkConnectionState = DataItem<ConnectionStatus_t, 1>( "Sink_Conn_State", m_SinkConnectionState_InitialValue, RxTxType_Rx_Only, 0, &m_CPU1SerialPortMessageManager, nullptr, this);
    WebSocketDataHandler<ConnectionStatus_t, 1> m_SinkConnectionStatus_DataHandler = WebSocketDataHandler<ConnectionStatus_t, 1>( m_WebSocketDataProcessor, m_SinkConnectionState );    
    
    //Bluetooth Sink Auto Reconnect
    const bool m_BluetoothSinkAutoReConnect_InitialValue = false;
    DataItemWithPreferences<bool, 1> m_BluetoothSinkAutoReConnect = DataItemWithPreferences<bool, 1>( "BT_Sink_AR", m_BluetoothSinkAutoReConnect_InitialValue, RxTxType_Tx_On_Change_With_Heartbeat, 5000, &m_preferenceInterface, &m_CPU1SerialPortMessageManager, nullptr, this, nullptr);
    WebSocketDataHandler<bool, 1> m_BluetoothSinkAutoReConnect_DataHandler = WebSocketDataHandler<bool, 1>( m_WebSocketDataProcessor, m_BluetoothSinkAutoReConnect );
    
    //Bluetooth Source Enable
    const bool m_BluetoothSourceEnable_InitialValue = false;
    DataItemWithPreferences<bool, 1> m_BluetoothSourceEnable = DataItemWithPreferences<bool, 1>( "BT_Source_En", m_BluetoothSourceEnable_InitialValue, RxTxType_Tx_On_Change_With_Heartbeat, 5000, &m_preferenceInterface, &m_CPU2SerialPortMessageManager, nullptr, this, nullptr);
    WebSocketDataHandler<bool, 1> m_BluetoothSourceEnable_DataHandler = WebSocketDataHandler<bool, 1>( m_WebSocketDataProcessor, m_BluetoothSourceEnable );

    //Target Device
    CompatibleDevice_t m_TargetCompatibleDevice_InitialValue = {"", ""};
    DataItem<CompatibleDevice_t, 1> m_TargetCompatibleDevice = DataItem<CompatibleDevice_t, 1>( "Target_Device", m_TargetCompatibleDevice_InitialValue, RxTxType_Tx_On_Change_With_Heartbeat, 5000, &m_CPU2SerialPortMessageManager, nullptr, this);
    WebSocketDataHandler<CompatibleDevice_t, 1> m_TargetCompatibleDevice_DataHandler = WebSocketDataHandler<CompatibleDevice_t, 1>( m_WebSocketDataProcessor, m_TargetCompatibleDevice );

    //Sink Connect
    CallbackArguments m_SinkConnect_CallbackArgs = {this};
    NamedCallback_t m_SinkConnect_Callback = {"m_SinkConnect_Callback", &SinkConnect_ValueChanged, &m_SinkConnect_CallbackArgs};
    const bool m_SinkConnect_InitialValue = false;
    DataItem<bool, 1> m_SinkConnect = DataItem<bool, 1>( "Sink_Connect", m_SinkConnect_InitialValue, RxTxType_Tx_On_Change_With_Heartbeat, 5000, &m_CPU1SerialPortMessageManager, &m_SinkConnect_Callback, this);
    WebSocketDataHandler<bool, 1> m_SinkConnect_DataHandler = WebSocketDataHandler<bool, 1>( m_WebSocketDataProcessor, m_SinkConnect );
    static void SinkConnect_ValueChanged(const String &Name, void* object, void* arg)
    {
      ESP_LOGI("SinkConnect_ValueChanged", "Sink Connect Value Changed");
      if(object && arg)
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
    WebSocketDataHandler<bool, 1> m_SinkDisconnect_DataHandler = WebSocketDataHandler<bool, 1>( m_WebSocketDataProcessor, m_SinkDisconnect );
    static void SinkDisconnect_ValueChanged(const String &Name, void* object, void* arg)
    {
      ESP_LOGI("SinkDisconnect_ValueChanged", "Sink DisConnect Value Changed");
      if(object && arg)
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

    //Source Restart
    CallbackArguments m_SourceRestart_CallbackArgs = {this};
    NamedCallback_t m_SourceRestart_Callback = {"SourceRestart Callback", &SourceRestart_ValueChanged, &m_SourceRestart_CallbackArgs};
    const bool m_SourceRestart_InitialValue = false;
    LocalDataItem<bool, 1> m_SourceRestart = LocalDataItem<bool, 1>( "Wifi_Restart", m_SourceRestart_InitialValue, &m_SourceRestart_Callback, this);
    WebSocketDataHandler<bool, 1> m_SourceRestart_DataHandler = WebSocketDataHandler<bool, 1>( m_WebSocketDataProcessor, m_SourceRestart );
    static void SourceRestart_ValueChanged(const String &Name, void* object, void* arg)
    {
      ESP_LOGI("SourceRestart_ValueChanged", "Source Restart Value Changed");
      if(object && arg)
      {
        CallbackArguments* arguments = static_cast<CallbackArguments*>(arg);
        if(arguments->arg1 && object)
        {
          SettingsWebServerManager* pSettingWebServer = static_cast<SettingsWebServerManager*>(arguments->arg1);
          bool sourceRestart = *static_cast<bool*>(object);
          if(sourceRestart)
          {
            pSettingWebServer->StartWiFi();
          }
        }
      }
    }

    //Output Source Connect
    CallbackArguments m_SourceConnect_CallbackArgs = {&m_TargetCompatibleDevice, &m_TargetCompatibleDevice_InitialValue};
    NamedCallback_t m_SourceConnect_Callback = {"Test Name", &SourceConnect_ValueChanged, &m_SourceConnect_CallbackArgs};
    const bool m_SourceConnect_InitialValue = false;
    DataItem<bool, 1> m_SourceConnect = DataItem<bool, 1>( "Src_Connect", m_SourceConnect_InitialValue, RxTxType_Tx_On_Change_With_Heartbeat, 5000, &m_CPU2SerialPortMessageManager, &m_SourceConnect_Callback, this);
    WebSocketDataHandler<bool, 1> m_SourceConnect_DataHandler = WebSocketDataHandler<bool, 1>( m_WebSocketDataProcessor, m_SourceConnect );
    static void SourceConnect_ValueChanged(const String &Name, void* object, void* arg)
    {
      ESP_LOGI("SourceConnect_ValueChanged", "Source Connect Value Changed");
      if(object && arg)
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
    WebSocketDataHandler<bool, 1> m_SourceDisconnect_DataHandler = WebSocketDataHandler<bool, 1>( m_WebSocketDataProcessor, m_SourceDisconnect );
    static void SourceDisconnect_ValueChanged(const String &Name, void* object, void* arg)
    {
      ESP_LOGI("SourceDisconnect_ValueChanged", "Source Disconnect Value Changed");
      if(object && arg)
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
    CallbackArguments m_ScannedDevice_CallbackArgs = {this, &m_WebSocketDataProcessor};
    BT_Device_Info_With_Time_Since_Update m_ScannedDevice_InitialValue = {"", "", 0, 0, };
    DataItem<BT_Device_Info_With_Time_Since_Update, 1> m_ScannedDevice = DataItem<BT_Device_Info_With_Time_Since_Update, 1>( "Scan_BT_Device", m_ScannedDevice_InitialValue, RxTxType_Rx_Only, 0, &m_CPU2SerialPortMessageManager, nullptr, this);
    BT_Device_Info_With_Time_Since_Update_WebSocket_DataHandler m_ScannedDevice_DataHandler = BT_Device_Info_With_Time_Since_Update_WebSocket_DataHandler(m_WebSocketDataProcessor, m_ScannedDevice);

    //Bluetooth Source Auto Reconnect
    const bool m_BluetoothSourceAutoReConnect_InitialValue = false;
    DataItemWithPreferences<bool, 1> m_BluetoothSourceAutoReConnect = DataItemWithPreferences<bool, 1>( "BT_Source_AR", m_BluetoothSourceAutoReConnect_InitialValue, RxTxType_Tx_On_Change_With_Heartbeat, 5000, &m_preferenceInterface, &m_CPU2SerialPortMessageManager, nullptr, this, &validBoolValues);
    WebSocketDataHandler<bool, 1> m_BluetoothSourceAutoReConnect_DataHandler = WebSocketDataHandler<bool, 1>( m_WebSocketDataProcessor, m_BluetoothSourceAutoReConnect );

    //Source Connection State
    const ConnectionStatus_t m_SourceConnectionState_InitialValue = ConnectionStatus_t::Disconnected;
    DataItem<ConnectionStatus_t, 1> m_SourceConnectionState = DataItem<ConnectionStatus_t, 1>( "Src_Conn_State", m_SourceConnectionState_InitialValue, RxTxType_Rx_Only, 0, &m_CPU2SerialPortMessageManager, nullptr, this);
    WebSocketDataHandler<ConnectionStatus_t, 1> m_SourceConnectionState_DataHandler = WebSocketDataHandler<ConnectionStatus_t, 1>( m_WebSocketDataProcessor, m_SourceConnectionState );    

    //Source Reset
    const bool m_SourceReset_InitialValue = false;
    DataItemWithPreferences<bool, 1> m_SourceReset = DataItemWithPreferences<bool, 1>( "BT_Src_Reset", m_SourceReset_InitialValue, RxTxType_Tx_On_Change_With_Heartbeat, 5000, &m_preferenceInterface, &m_CPU2SerialPortMessageManager, nullptr, this, &validBoolValues);
    WebSocketDataHandler<bool, 1> m_SourceReset_DataHandler = WebSocketDataHandler<bool, 1>( m_WebSocketDataProcessor, m_SourceReset );    
    void HandleWebSocketMessage(uint8_t clientID, WStype_t type, uint8_t *payload, size_t length)
    {
        // Handle text messages
        if (type == WStype_TEXT) 
        {
            payload[length] = 0;

            String WebSocketData = String((char*)payload);
            Serial.printf("WebSocket Data from Client %u: %s\n", clientID, WebSocketData.c_str());

            if (WebSocketData.equals("Hello I am here!")) 
            {
                Serial.printf("New Client Message from %u: \"Hello I am here!\"\n", clientID);
                m_WebSocketDataProcessor.UpdateAllDataToClient(clientID);
                return;
            } 
            else 
            {
                JSONVar jsonObject = JSON.parse(WebSocketData);
                if (JSON.typeof(jsonObject) == "undefined") 
                {
                    Serial.printf("ERROR! Parsing failed for Input from Client %u: %s.\n", clientID, WebSocketData.c_str());
                } 
                else 
                {
                    if (HandleSignalValue(jsonObject)) {} 
                    else if (HandleJSONValue(jsonObject)){} 
                    else{ ESP_LOGE("HandleWebSocketMessage", "ERROR! Unknown WebSocket Message from Client %u: %s.\n", clientID, WebSocketData.c_str()); }
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
          ESP_LOGD( "HandleWebSocketMessage", "Signal Value Message Received. ID: \"%s\" Value: \"%s\""
                  , Id.c_str()
                  , Value.c_str() );
          if(!m_WebSocketDataProcessor.ProcessSignalValueAndSendToDatalink(Id, Value))
          {
            ESP_LOGE("HandleWebSocketMessage", "ERROR! Unknown Signal Value Object: %s.", Id.c_str());
          }
        }
        else
        {
          ESP_LOGD("HandleWebSocketMessage", "Unknown JSON Object: %s", JSON.stringify(signalValue).c_str());
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
          ESP_LOGI( "HandleWebSocketMessage", "Web Socket JSON Data Received. Id: \"%s\" Value: \"%s\""
                  , Id.c_str()
                  , Value.c_str());
          if(!m_WebSocketDataProcessor.ProcessSignalValueAndSendToDatalink(Id.c_str(), Value.c_str()))
          {
            ESP_LOGE("HandleWebSocketMessage", "ERROR! Unknown JSON Object: %s.", Id.c_str());
          }
        }
        else
        {
          ESP_LOGD("HandleWebSocketMessage", "Known JSON Object: %s", JSON.stringify(jSONValue).c_str());
        }
      }
      return result;
    }
    
    void BeginWebServer()
    {
      ESP_LOGI("BeginWebServer", "Starting Web Server");
      m_WebServer.begin();
      m_WebSocket.begin();
      m_Web_Server_Running = true;
    }

    void TryEndWebServer()
    {
      ESP_LOGI("BeginWebServer", "Try Ending Web Server");
      if(!m_Station_Running && !m_AccessPoint_Running)
      {
        EndWebServer();
      }
    }

    void EndWebServer()
    {
      ESP_LOGI("BeginWebServer", "Ending Web Server");
      if(m_Web_Server_Running)
      {
        m_WebServer.stop();
        m_WebSocket.close();
        m_Web_Server_Running = false;
        ESP_LOGI("BeginWebServer", "Web Server Stopped");
      }
    }

    void StartDNSServer()
    {
      String hostName = m_Host_Name.GetValueAsString();
      if (MDNS.begin(hostName.c_str()))
      {
        ESP_LOGI( "StartDNSServer", "Started DNS Server with Host Name: \"%s\"", hostName.c_str());
      }
      else
      {
        ESP_LOGE( "StartDNSServer", "Unable to start DNS Server with Host Name: \"%s\"", hostName.c_str());
      }
      if(MDNS.addService("http", "tcp", 80))
      {
        ESP_LOGI( "StartDNSServer", "DNS added TCP service.");
      }
      else
      {
        ESP_LOGE( "StartDNSServer", "Unable to TCP service to DNS Server.");
      }
    }

    void TryEndDNSServer()
    {
      if(!m_Station_Running && !m_AccessPoint_Running)
      {
        End_DNS_Server();
      }
    }

    void End_DNS_Server()
    {
      ESP_LOGI( "InitWifiClient", "Ending DNS Server");
      MDNS.end();
    }

    void InitWiFi_AccessPoint(const char* apSSID, const char* apPassword)
    {
      ESP_LOGI( "InitWifiClient", "Starting Wifi Access Point Mode: Access Point SSID: \"%s\" Access Point Password: \"%s\"", apSSID, apPassword);
      WiFi.mode(WIFI_AP);
      IPAddress Ip(192, 168, 0, 1);
      IPAddress NMask(255, 255, 255, 0);
      WiFi.softAPConfig(Ip, Ip, NMask);
      WiFi.softAP(apSSID, apPassword);
    }

    void InitWiFi_AccessPoint_Station(const char* apSSID, const char* apPassword, const char* staSSID, const char* staPassword)
    {
      ESP_LOGI( "InitWifiClient", "Starting Wifi Access Point + Station Mode: Access Point SSID: \"%s\" Access Point Password: \"%s\" Station SSID: \"%s\" Station Password: \"%s\"", apSSID, apPassword, staSSID, staPassword);
      WiFi.mode(WIFI_AP_STA);
      WiFi.begin(staSSID, staPassword);
      IPAddress Ip(192, 168, 0, 1);
      IPAddress NMask(255, 255, 255, 0);
      WiFi.softAPConfig(Ip, Ip, NMask);
      WiFi.softAP(apSSID, apPassword);
    }
};