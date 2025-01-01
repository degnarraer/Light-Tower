#include "WebSocketDataHandler.h"

WebSocketDataProcessor::WebSocketDataProcessor(WebServer &webServer, WebSocketsServer &webSocket)
    : m_WebServer(webServer), m_WebSocket(webSocket), m_WebServerTaskHandle(nullptr), m_WebServerTxTaskHandle(nullptr)
{
}

WebSocketDataProcessor::~WebSocketDataProcessor()
{
    if (m_WebServerTaskHandle)
    {
        vTaskDelete(m_WebServerTaskHandle);
        m_WebServerTaskHandle = nullptr;
    }
    if (m_WebServerTxTaskHandle)
    {
        vTaskDelete(m_WebServerTxTaskHandle);
        m_WebServerTxTaskHandle = nullptr;
    }
    if (m_Tx_KeyValues_Semaphore)
    {
        vSemaphoreDelete(m_Tx_KeyValues_Semaphore);
        m_Tx_KeyValues_Semaphore = nullptr;
    }
}

void WebSocketDataProcessor::Setup()
{
    // Create binary semaphore for mutual exclusion
    m_Tx_KeyValues_Semaphore = xSemaphoreCreateRecursiveMutex();
    if (m_Tx_KeyValues_Semaphore == nullptr)
    {
        ESP_LOGE("WebSocketDataProcessor", "ERROR! Failed to create semaphore.");
        return;
    }

    // Create message queue for WebSocket transmission
    m_WebSocketMessageQueue = xQueueCreate(WEB_SOCKET_TX_QUEUE_SIZE, sizeof(std::string*));
    if (m_WebSocketMessageQueue)
        ESP_LOGD("SetupWebSocket", "Created Queue");
    else
        ESP_LOGE("SetupWebSocket", "Failed to create queue");

    // Create tasks
    xTaskCreate(Static_WebSocket_Data_Processor_TxTask, "Web Server Task", 2500, this, WEB_SOCKET_TX_TASK_PRIORITY, &m_WebServerTaskHandle);
    xTaskCreate(Static_WebSocket_Transmission_Task, "Web Server Tx Task", 2500, this, WEB_SOCKET_TX_TASK_PRIORITY, &m_WebServerTxTaskHandle);
}

void WebSocketDataProcessor::Handle_Current_Value_Request(uint8_t clientId)
{
    ESP_LOGI("WebSocketDataProcessor::Handle_Current_Value_Request", "Sending All Data to Client: %u", clientId);
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

void WebSocketDataProcessor::Tx_Data_To_WebSocket(std::string key, std::string value)
{
    if (xSemaphoreTakeRecursive(m_Tx_KeyValues_Semaphore, pdMS_TO_TICKS(WEB_SOCKET_TX_WAIT)) == pdTRUE)
    {
        KVP keyValuePair = {key, value};
        m_Tx_KeyValues.push_back(keyValuePair);
        xSemaphoreGiveRecursive(m_Tx_KeyValues_Semaphore);
    }
    else
    {
        ESP_LOGW("Semaphore Take Failure", "WARNING! Failed to take Semaphore in Tx_Data_To_WebSocket.");
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

void WebSocketDataProcessor::Static_WebSocket_Data_Processor_TxTask(void *parameter)
{
    WebSocketDataProcessor *processor = static_cast<WebSocketDataProcessor *>(parameter);
    processor->WebSocket_Data_Processor_TxTask();
}

void WebSocketDataProcessor::WebSocket_Data_Processor_TxTask()
{
    while (true)
    {
        bool messageToSend = false;
        std::unique_ptr<std::string> message = std::make_unique<std::string>();
        if (xSemaphoreTakeRecursive(m_Tx_KeyValues_Semaphore, pdMS_TO_TICKS(WEB_SOCKET_TX_WAIT)) == pdTRUE)
        {
            if (!m_Tx_KeyValues.empty())
            {
                std::vector<KVP> signalValues = std::move(m_Tx_KeyValues);
                if (!signalValues.empty())
                {
                    Encode_Signal_Values_To_JSON(signalValues, *message);
                    messageToSend = true;
                }
            }
            xSemaphoreGiveRecursive(m_Tx_KeyValues_Semaphore);
        }
        else
        {
            ESP_LOGW("WebSocketDataProcessor", "Failed to acquire semaphore.");
        }
        if(messageToSend)
        {
            std::string *rawMessage = message.release();
            if (xQueueSend(m_WebSocketMessageQueue, &rawMessage, pdMS_TO_TICKS(WEB_SOCKET_TX_WAIT)) != pdTRUE)
            {
                ESP_LOGW("WebSocketDataProcessor", "Queue is full, dropping message.");
                delete rawMessage;
            }
        }
    }
}

void WebSocketDataProcessor::Static_WebSocket_Transmission_Task(void *parameter)
{
    WebSocketDataProcessor *processor = static_cast<WebSocketDataProcessor *>(parameter);
    processor->WebSocket_Transmission_Task();
}

void WebSocketDataProcessor::WebSocket_Transmission_Task()
{
    while (true)
    {
        std::string *message_raw = nullptr;
        while(xQueueReceive(m_WebSocketMessageQueue, &message_raw, portMAX_DELAY) == pdTRUE)
        {
            std::unique_ptr<std::string> sp_message(message_raw);
            if (sp_message)
            {
                NotifyClients(*sp_message);
            }
            vTaskDelay(pdMS_TO_TICKS(WEB_SOCKET_PER_MESSAGE_DELAY));
        }
        vTaskDelay(pdMS_TO_TICKS(WEB_SOCKET_TX_TASK_DELAY));
    }
}
