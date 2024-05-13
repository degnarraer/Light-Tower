#include "WebSocketDataHandler.h"

void WebSocketDataProcessor::UpdateAllDataToClient(uint8_t clientId)
{
  ESP_LOGI("WebSocketDataProcessor::UpdateAllDataToClient", "Sending All Data to Client: %i", clientId );
  std::vector<KVP> KeyValuePairs = std::vector<KVP>();
  for(int i = 0; i < m_MySenders.size(); ++i)
  {
    m_MySenders[i]->AppendCurrentValueToKVP(KeyValuePairs, true);
  }
  if(KeyValuePairs.size())
  {
    String message;
    Encode_Signal_Values_To_JSON(KeyValuePairs, message);
    NotifyClient(clientId, message);
  }
}

void WebSocketDataProcessor::WebSocketDataProcessor_Task()
{
  const TickType_t xFrequency = 20;
  TickType_t xLastWakeTime = xTaskGetTickCount();
  while(true)
  {
    vTaskDelayUntil( &xLastWakeTime, xFrequency );
    std::vector<KVP> KeyValuePairs = std::vector<KVP>();
    for(int i = 0; i < m_MySenders.size(); ++i)
    {
      m_MySenders[i]->AppendCurrentValueToKVP(KeyValuePairs);
    }
    if(KeyValuePairs.size())
    {
      String message;
      Encode_Signal_Values_To_JSON(KeyValuePairs, message);
      NotifyClients(message);
    }
  }  
}

void WebSocketDataProcessor::RegisterAsWebSocketDataReceiver(const String& Name, WebSocketDataHandlerReceiver *aReceiver)
{
  auto it = std::find(m_MyReceivers.begin(), m_MyReceivers.end(), aReceiver);
  if (it == m_MyReceivers.end())
  {
    ESP_LOGI("RegisterAsWebSocketDataReceiver", "Registering %s as Web Socket Data Receiver.", Name.c_str());
    m_MyReceivers.push_back(aReceiver);
  }
}

void WebSocketDataProcessor::DeRegisterAsWebSocketDataReceiver(const String& Name, WebSocketDataHandlerReceiver *aReceiver)
{
  auto it = std::find(m_MyReceivers.begin(), m_MyReceivers.end(), aReceiver);
  if (it != m_MyReceivers.end())
  {
    ESP_LOGI("RegisterAsWebSocketDataSender", "DeRegistering %s as Web Socket Data Receiver.", Name.c_str());
    m_MyReceivers.erase(it);
  }
}

void WebSocketDataProcessor::RegisterAsWebSocketDataSender(const String& Name, WebSocketDataHandlerSender *aSender)
{
  auto it = std::find(m_MySenders.begin(), m_MySenders.end(), aSender);
  if (it == m_MySenders.end())
  {
    ESP_LOGI("RegisterAsWebSocketDataSender", "Registering %s as Web Socket Data Sender.", Name.c_str());
    m_MySenders.push_back(aSender);
  }
}

void WebSocketDataProcessor::DeRegisterAsWebSocketDataSender(const String& Name, WebSocketDataHandlerSender *aSender)
{
  auto it = std::find(m_MySenders.begin(), m_MySenders.end(), aSender);
  if (it != m_MySenders.end())
  {
    ESP_LOGI("RegisterAsWebSocketDataSender", "DeRegistering %s as Web Socket Data Sender.", Name.c_str());
    m_MySenders.erase(it);
  }
}


bool WebSocketDataProcessor::ProcessSignalValueAndSendToDatalink(const String& SignalId, const String& Value)
{
  bool SignalFound = false;
  for(int i = 0; i < m_MyReceivers.size(); ++i)
  {
    if(true == m_MyReceivers[i]->ProcessSignalValueAndSendToDatalink(SignalId, Value))
    {
      SignalFound = true;
    }
  }
  return SignalFound;
}

void WebSocketDataProcessor::Encode_Signal_Values_To_JSON(std::vector<KVP> &keyValuePairs, String &result)
{
  JSONVar jSONVars;
  for(int i = 0; i < keyValuePairs.size(); ++i)
  {
    JSONVar SettingValues;
    SettingValues["Id"] = keyValuePairs.at(i).Key;
    SettingValues["Value"] = keyValuePairs.at(i).Value;
    jSONVars["SignalValue" + String(i)] = SettingValues; 
  }
  result = JSON.stringify(jSONVars);
}

void WebSocketDataProcessor::NotifyClient(const uint8_t clientID, const String& TextString)
{
  if(0 < TextString.length())
  {
    m_WebSocket.text(clientID, TextString);
  }
}

void WebSocketDataProcessor::NotifyClients(const String& TextString)
{
  if(0 < TextString.length())
  {
    m_WebSocket.textAll(TextString);
  }
}

void WebSocketDataProcessor::UpdateDataForSender(WebSocketDataHandlerSender* sender, bool forceUpdate)
{
  ESP_LOGD("WebSocketDataProcessor::UpdateDataForSender", "Updating Data For DataHandler!");
  std::vector<KVP> KeyValuePairs = std::vector<KVP>();
  sender->AppendCurrentValueToKVP(KeyValuePairs, forceUpdate);
  if(KeyValuePairs.size())
  {
    String message;
    Encode_Signal_Values_To_JSON(KeyValuePairs, message);
    NotifyClients(message);
  }
}

void WebSocketDataProcessor::StaticWebSocketDataProcessor_Task(void * parameter)
{
  WebSocketDataProcessor *processor = (WebSocketDataProcessor*)parameter;
  processor->WebSocketDataProcessor_Task();
}