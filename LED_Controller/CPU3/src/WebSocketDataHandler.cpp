#include "WebSocketDataHandler.h"

WebSocketDataProcessor::WebSocketDataProcessor( WebServer &webServer, WebSocketsServer &webSocket )
                                              : m_WebServer(webServer)
                                              , m_WebSocket(webSocket)
{

}

WebSocketDataProcessor::~WebSocketDataProcessor()
{
    if(m_Message_Task_Handle)
    {
        vTaskDelete(m_Message_Task_Handle);
        m_Message_Task_Handle = nullptr;
    }
    if(m_Message_Queue_Handle)
    {
        vQueueDelete(m_Message_Queue_Handle);
        m_Message_Queue_Handle = nullptr;
    }
}

void WebSocketDataProcessor::Setup()
{
    m_Message_Queue_Handle = xQueueCreate(WEB_SOCKET_QUEUE_SIZE, sizeof(KVP*));
    if(m_Message_Queue_Handle)
    {
        if(xTaskCreate(Static_Message_Task, "Message Task", 5000, this, WEB_SOCKET_TX_TASK_PRIORITY, &m_Message_Task_Handle) != pdTRUE)
        {
            ESP_LOGE("Setup","ERROR! Unable to create task.");
            vQueueDelete(m_Message_Queue_Handle);
            m_Message_Queue_Handle = nullptr;
        }
    }
    else
    {
        ESP_LOGE("Setup","ERROR! Unable to create queue.");
    }
}

void WebSocketDataProcessor::Static_Message_Task(void* pvParameters)
{
    WebSocketDataProcessor* aProcessor = static_cast<WebSocketDataProcessor*>(pvParameters);
    aProcessor->Message_Task();
}

void WebSocketDataProcessor::Message_Task()
{
    while(true)
    {
        if (!m_Message_Queue_Handle)
        {
            ESP_LOGE("Message_Task", "Queue handle is null. Cleaning up and terminating task.");
            vTaskDelay(pdMS_TO_TICKS(100)); // Allow some time for logs to flush
            vTaskDelete(nullptr);
        }
        KVP* pair_raw = nullptr;
        std::vector<KVP> signalValues;
        while(xQueueReceive(m_Message_Queue_Handle, &pair_raw, SEMAPHORE_NO_BLOCK) == pdTRUE)
        {
            signalValues.push_back(*pair_raw);
            delete pair_raw;
        }
        if (!signalValues.empty())
        {
            std::string message;
            Encode_Signal_Values_To_JSON(signalValues, message);
            NotifyClients(message);
        }
        vTaskDelay(pdMS_TO_TICKS(WEB_SOCKET_TX_TASK_DELAY));
    }
}

void WebSocketDataProcessor::Handle_Current_Value_Request(uint8_t clientId)
{
    ESP_LOGI("WebSocketDataProcessor::Handle_Current_Value_Requect", "Sending All Data to Client: %u", clientId);
    std::vector<KVP> signalValues;
    for (const auto &notifyee : m_MyTxNotifyees)
    {
        auto value = notifyee->HandleWebSocketDataRequest();
        if (!value.Key.empty() && !value.Value.empty())
        {
            signalValues.push_back(value);
        }
    }
    if (!signalValues.empty())
    {
        std::string message;
        Encode_Signal_Values_To_JSON(signalValues, message);
        NotifyClient(clientId, message);
    }
}

void WebSocketDataProcessor::TxDataToWebSocket(const std::string &key, const std::string &value)
{
    auto keyValuePair = std::make_unique<KVP>(key, value);
    if (m_Message_Queue_Handle)
    {
        auto rawPtr = keyValuePair.release();
        if (xQueueSend(m_Message_Queue_Handle, &rawPtr, SEMAPHORE_SHORT_BLOCK) != pdTRUE)
        {
            delete rawPtr;
            ESP_LOGW("TxDataToWebSocket", "Unable to queue message");
        }
    }
    else
    {
        ESP_LOGE("TxDataToWebSocket", "ERROR! Null pointer.");
    }
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

bool WebSocketDataProcessor::Handle_Signal_Value_RX(const std::string &signalId, const std::string &value)
{
    bool SignalFound = false;
    for (int i = 0; i < m_MyRxNotifyees.size(); ++i)
    {
        if (m_MyRxNotifyees[i]->GetSignal() == signalId)
        {
            SignalFound = true;
            ESP_LOGD("Handle_Signal_Value_RX", "Sending Value: \"%s\" to receiver.", value.c_str());
            m_MyRxNotifyees[i]->HandleWebSocketRxNotification(value);
        }
    }
    return SignalFound;
}

void WebSocketDataProcessor::Encode_Signal_Values_To_JSON(const std::vector<KVP> &keyValuePairs, std::string &result)
{
    if (keyValuePairs.empty())
    {
        result = "";
        return;
    }
    JSONVar jsonArray;
    size_t items = 0;
    for (const auto &pair : keyValuePairs)
    {
        if (pair.Key.empty() || pair.Value.empty())
        {
            ESP_LOGW("Encode_Signal_Values_To_JSON", 
                     "Skipping empty key or value in pair: Key=\"%s\", Value=\"%s\"", 
                     pair.Key.c_str(), pair.Value.c_str());
            continue;
        }
        JSONVar value;
        value["Id"] = pair.Key.c_str();
        value["Value"] = pair.Value.c_str();
        jsonArray[items++] = value;
    }
    if (jsonArray.length() == 0)
    {
        ESP_LOGW("Encode_Signal_Values_To_JSON", "No valid key-value pairs found for JSON encoding.");
        result = "";
        return;
    }
    result = JSON.stringify(jsonArray).c_str();
    if (result.empty())
    {
        ESP_LOGE("Encode_Signal_Values_To_JSON", "Failed to serialize JSON array. Returning empty array.");
        result = "";
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