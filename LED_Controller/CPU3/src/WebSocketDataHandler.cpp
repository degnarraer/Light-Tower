#include "WebSocketDataHandler.h"

void WebSocketDataProcessor::UpdateAllDataToClient(uint8_t clientId)
{
    ESP_LOGI("WebSocketDataProcessor::UpdateAllDataToClient", "Sending All Data to Client: %u", clientId);
    std::vector<KVP> signalValues;

    for (int i = 0; i < m_MyTxNotifyees.size(); ++i)
    {
        signalValues.push_back(m_MyTxNotifyees[i]->HandleWebSocketDataRequest());
    }

    if (!signalValues.empty())
    {
        std::string message;
        Encode_Signal_Values_To_JSON(signalValues, message);
        NotifyClient(clientId, message);
    }
}

void WebSocketDataProcessor::WebSocketDataProcessor_WebSocket_TxTask()
{
    const TickType_t xFrequency = 20;
    TickType_t xLastWakeTime = xTaskGetTickCount();

    while (true)
    {
        if (xSemaphoreTakeRecursive(m_Tx_KeyValues_Semaphore, 0) == pdTRUE)
        {
            if (!m_Tx_KeyValues.empty())
            {
                std::vector<KVP> signalValues = std::move(m_Tx_KeyValues);
                if (!signalValues.empty())
                {
                    std::unique_ptr<std::string> message = std::make_unique<std::string>();
                    Encode_Signal_Values_To_JSON(signalValues, *message);
                    std::string* rawMessage = message.release();
                    if (xQueueSend(m_WebSocketMessageQueue, &rawMessage, 0) != pdTRUE) {
                        ESP_LOGW("WebSocketDataProcessor", "Queue is full, dropping message.");
                        delete rawMessage;
                    }
                }
            }
            if(xSemaphoreGiveRecursive(m_Tx_KeyValues_Semaphore) != pdTRUE)
            {
                ESP_LOGE("Web Socket TX Task", "Failed to release semaphore!");
            }
        }
        else
        {
            ESP_LOGW("Semaphore Take Failure", "WARNING! Failed to take Semaphore");
        }
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

void WebSocketDataProcessor::StaticWebSocketDataProcessor_WebSocket_TxTask(void *parameter)
{
    WebSocketDataProcessor *processor = static_cast<WebSocketDataProcessor *>(parameter);
    processor->WebSocketDataProcessor_WebSocket_TxTask();
}

void WebSocketDataProcessor::RegisterForWebSocketRxNotification(const std::string &name, WebSocketDataHandlerReceiver *aReceiver)
{
    auto it = std::find(m_MyRxNotifyees.begin(), m_MyRxNotifyees.end(), aReceiver);
    if (it == m_MyRxNotifyees.end())
    {
        ESP_LOGD("RegisterForWebSocketRxNotification", "Registering %s as Web Socket Data Receiver.", name.c_str());
        m_MyRxNotifyees.push_back(aReceiver);
    }
}

void WebSocketDataProcessor::DeRegisterForWebSocketRxNotification(const std::string &name, WebSocketDataHandlerReceiver *aReceiver)
{
    auto it = std::find(m_MyRxNotifyees.begin(), m_MyRxNotifyees.end(), aReceiver);
    if (it != m_MyRxNotifyees.end())
    {
        ESP_LOGD("DeRegisterForWebSocketRxNotification", "DeRegistering %s as Web Socket Data Receiver.", name.c_str());
        m_MyRxNotifyees.erase(it);
    }
}

void WebSocketDataProcessor::RegisterForWebSocketTxNotification(const std::string &name, WebSocketDataHandlerSender *aSender)
{
    auto it = std::find(m_MyTxNotifyees.begin(), m_MyTxNotifyees.end(), aSender);
    if (it == m_MyTxNotifyees.end())
    {
        ESP_LOGD("RegisterForWebSocketTxNotification", "Registering %s as Web Socket Data Sender.", name.c_str());
        m_MyTxNotifyees.push_back(aSender);
    }
}

void WebSocketDataProcessor::DeRegisterForWebSocketTxNotification(const std::string &name, WebSocketDataHandlerSender *aSender)
{
    auto it = std::find(m_MyTxNotifyees.begin(), m_MyTxNotifyees.end(), aSender);
    if (it != m_MyTxNotifyees.end())
    {
        ESP_LOGD("DeRegisterForWebSocketTxNotification", "DeRegistering %s as Web Socket Data Sender.", name.c_str());
        m_MyTxNotifyees.erase(it);
    }
}

bool WebSocketDataProcessor::ProcessSignalValueAndSendToDatalink(const std::string &signalId, const std::string &value)
{
    bool SignalFound = false;
    for (int i = 0; i < m_MyRxNotifyees.size(); ++i)
    {
        if (m_MyRxNotifyees[i]->GetSignal() == signalId)
        {
            SignalFound = true;
            ESP_LOGD("ProcessSignalValueAndSendToDatalink", "Sending Value: \"%s\" to receiver.", value.c_str());
            m_MyRxNotifyees[i]->HandleWebSocketRxNotification(value);
        }
    }
    return SignalFound;
}

void WebSocketDataProcessor::Encode_Signal_Values_To_JSON(const std::vector<KVP> &keyValuePairs, std::string &result)
{
    if (keyValuePairs.empty())
    {
        result = "[]";
        return;
    }

    JSONVar jsonArray;

    for (size_t i = 0; i < keyValuePairs.size(); ++i)
    {
        if (keyValuePairs[i].Key.empty() || keyValuePairs[i].Value.empty())
        {
            ESP_LOGW("Encode_Signal_Values_To_JSON", "WARNING! Key or Value is empty at index %zu", i);
            continue;
        }

        JSONVar settingValues;
        settingValues["Id"] = keyValuePairs[i].Key.c_str();
        settingValues["Value"] = keyValuePairs[i].Value.c_str();

        jsonArray[i] = settingValues;
    }

    result = std::string(JSON.stringify(jsonArray).c_str());

    if (result.empty())
    {
        ESP_LOGE("Encode_Signal_Values_To_JSON", "ERROR! Failed to serialize JSON array.");
        result = "[]";
    }
}

void WebSocketDataProcessor::NotifyClient(const uint8_t clientID, const std::string &textString)
{
    ESP_LOGD("NotifyClients", "Notify Client: %s", textString.c_str());
    if (!textString.empty())
    {
        m_WebSocket.sendTXT(clientID, textString.c_str(), textString.length());
    }
}

void WebSocketDataProcessor::NotifyClients(const std::string &textString)
{
    ESP_LOGD("NotifyClients", "Notify Clients: %s", textString.c_str());
    if (!textString.empty())
    {
        m_WebSocket.broadcastTXT(textString.c_str(), textString.length());
    }
}
