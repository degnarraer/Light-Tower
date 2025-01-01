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
#include "SettingsWebServer.h"


SettingsWebServerManager::SettingsWebServerManager( String Title
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
    
SettingsWebServerManager::~SettingsWebServerManager()
{
}
    
void SettingsWebServerManager::Setup()
{
  InitializeLocalvariables();
  SetupAllSetupCallees();
  InitFileSystem();
  InitWebServer();
  StartWiFi();
}

void SettingsWebServerManager::InitializeLocalvariables()
{
  m_WebSocketDataProcessor.Setup();
}

void SettingsWebServerManager::StartWiFi()
{
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
    InitWiFi_AccessPoint( "LED Tower of Power"
                        , "LEDs Rock" );
  }
}
    
void SettingsWebServerManager::EndWiFi()
{
  ESP_LOGI("EndWiFi", "Ending Wifi");
  EndWebServer();
  WiFi.disconnect(false, true);
  m_WiFi_Ready = false;
}

// Init the web server to use the local SPIFFS memory and serve up index.html file.
void SettingsWebServerManager::InitWebServer()
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
void SettingsWebServerManager::InitFileSystem()
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
void SettingsWebServerManager::OnWiFiEvent(WiFiEvent_t event)
{
  switch (event) 
  {
      case SYSTEM_EVENT_WIFI_READY:
        ESP_LOGI("Wifi Event", "Wifi Ready!");
        m_WiFi_Ready = true;
        TryBeginWebServer();
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
        m_Station_Connected = true;
        TryBeginWebServer();
      break;
      case SYSTEM_EVENT_STA_DISCONNECTED:
        ESP_LOGI("Wifi Event", "Station disconnected.");
        m_Station_Connected = false;
        TryEndWebServer();
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
        TryBeginWebServer();
      break;
      case SYSTEM_EVENT_AP_STOP:
        ESP_LOGI("Wifi Event", "Access Point stopped!");
        m_AccessPoint_Running = false;
        TryEndWebServer();
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

void SettingsWebServerManager::OnWebSocketEvent(uint8_t clientID, WStype_t type, uint8_t *payload, size_t length)
{
  switch (type) 
  {
      case WStype_CONNECTED:
      {
          IPAddress ip = m_WebSocket.remoteIP(clientID);
          ESP_LOGI("OnWebSocketEvent", "WebSocket client #%u connected from %s", clientID, ip.toString().c_str());
          break;
      }
      case WStype_PONG:
          ESP_LOGI("OnWebSocketEvent", "WebSocket client #%u Pinged Us!", clientID);
          break;
      case WStype_TEXT:
          ESP_LOGI("OnWebSocketEvent", "WebSocket client #%u sent Text #%s", clientID, String((char*)payload, length).c_str());
          HandleWebSocketMessage(clientID, type, payload, length);
          break;
      case WStype_ERROR:
          ESP_LOGI("OnWebSocketEvent", "WebSocket client #%u Error. Closing Connection!", clientID);
          break;
      case WStype_DISCONNECTED:
          ESP_LOGI("OnWebSocketEvent", "WebSocket client #%u disconnected. Closing Connection.", clientID);
          break;
  }
}