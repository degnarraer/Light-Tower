#include "WebSocketDataHandler.h"

void WebSocketDataProcessor::UpdateAllDataToClient(uint8_t clientId)
{
  ESP_LOGI("WebSocketDataProcessor::UpdateAllDataToClient", "Sending All Data to Client: %i", clientId );
  std::vector<KVP> signalValues = std::vector<KVP>();
  for(int i = 0; i < m_MyTxNotifyees.size(); ++i)
  {
    m_MyTxNotifyees[i]->HandleWebSocketDataRequest();
  }
  if(signalValues.size())
  {
    String message;
    Encode_Signal_Values_To_JSON(signalValues, message);
    NotifyClient(clientId, message);
  }
}

void WebSocketDataProcessor::WebSocketDataProcessor_WebSocket_TxTask()
{
  const TickType_t xFrequency = 20;
  TickType_t xLastWakeTime = xTaskGetTickCount();
  while(true)
  {
    vTaskDelayUntil( &xLastWakeTime, xFrequency );
    std::lock_guard<std::recursive_mutex> lock(m_Tx_KeyValues_Mutex);
    bool hasSize = (m_Tx_KeyValues.size() > 0);
    if(hasSize) ESP_LOGD("WebSocketDataProcessor_WebSocket_TxTask", "Before Size %i", m_Tx_KeyValues.size());
    std::vector<KVP> signalValues = std::move(m_Tx_KeyValues);
    if(hasSize) ESP_LOGD("WebSocketDataProcessor_WebSocket_TxTask", "After Size %i", m_Tx_KeyValues.size());
    std::lock_guard<std::recursive_mutex> unlock(m_Tx_KeyValues_Mutex);
    if(signalValues.size())
    {
      String message;
      Encode_Signal_Values_To_JSON(signalValues, message);
      NotifyClients(message);
    }
  }  
}

void WebSocketDataProcessor::RegisterForWebSocketRxNotification(const String& name, WebSocketDataHandlerReceiver *aReceiver)
{
  auto it = std::find(m_MyRxNotifyees.begin(), m_MyRxNotifyees.end(), aReceiver);
  if (it == m_MyRxNotifyees.end())
  {
    ESP_LOGI("RegisterForWebSocketRxNotification", "Registering %s as Web Socket Data Receiver.", name.c_str());
    m_MyRxNotifyees.push_back(aReceiver);
  }
}

void WebSocketDataProcessor::DeRegisterForWebSocketRxNotification(const String& name, WebSocketDataHandlerReceiver *aReceiver)
{
  auto it = std::find(m_MyRxNotifyees.begin(), m_MyRxNotifyees.end(), aReceiver);
  if (it != m_MyRxNotifyees.end())
  {
    ESP_LOGI("RegisterForWebSocketTxNotification", "DeRegistering %s as Web Socket Data Receiver.", name.c_str());
    m_MyRxNotifyees.erase(it);
  }
}


void WebSocketDataProcessor::RegisterForWebSocketTxNotification(const String& name, WebSocketDataHandlerSender *aSender)
{
  auto it = std::find(m_MyTxNotifyees.begin(), m_MyTxNotifyees.end(), aSender);
  if (it == m_MyTxNotifyees.end())
  {
    ESP_LOGI("RegisterForWebSocketTxNotification", "Registering %s as Web Socket Data Sender.", name.c_str());
    m_MyTxNotifyees.push_back(aSender);
  }
}

void WebSocketDataProcessor::DeRegisterForWebSocketTxNotification(const String& name, WebSocketDataHandlerSender *aSender)
{
  auto it = std::find(m_MyTxNotifyees.begin(), m_MyTxNotifyees.end(), aSender);
  if (it != m_MyTxNotifyees.end())
  {
    ESP_LOGI("RegisterForWebSocketTxNotification", "DeRegistering %s as Web Socket Data Sender.", name.c_str());
    m_MyTxNotifyees.erase(it);
  }
}


bool WebSocketDataProcessor::ProcessSignalValueAndSendToDatalink(const String& signalId, const String& value)
{
  bool SignalFound = false;
  for(int i = 0; i < m_MyRxNotifyees.size(); ++i)
  {
    if(true == m_MyRxNotifyees[i]->GetSignal().equals(signalId))
    {
      SignalFound = true;
      ESP_LOGD("ProcessSignalValueAndSendToDatalink", "Sending Value: \"%s\" to receiver.", value.c_str());
      m_MyRxNotifyees[i]->HandleWebSocketRxNotification(value);
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

void WebSocketDataProcessor::NotifyClient(const uint8_t clientID, const String& textString)
{
  ESP_LOGD("NotifyClients", "Notify Client: %s", textString.c_str());
  if(0 < textString.length())
  {
    m_WebSocket.text(clientID, textString);
  }
}

void WebSocketDataProcessor::NotifyClients(const String& textString)
{
  ESP_LOGI("NotifyClients", "Notify Clients: %s", textString.c_str());
  if(0 < textString.length())
  {
    m_WebSocket.textAll(textString);
  }
}

void WebSocketDataProcessor::StaticWebSocketDataProcessor_WebSocket_TxTask(void * parameter)
{
  WebSocketDataProcessor *processor = (WebSocketDataProcessor*)parameter;
  processor->WebSocketDataProcessor_WebSocket_TxTask();
}