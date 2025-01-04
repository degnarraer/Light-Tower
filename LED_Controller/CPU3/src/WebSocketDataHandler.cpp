#include "WebSocketDataHandler.h"

WebSocketDataProcessor::WebSocketDataProcessor( WebServer &webServer, WebSocketsServer &webSocket )
                                              : m_WebServer(webServer)
                                              , m_WebSocket(webSocket)
{

}

WebSocketDataProcessor::~WebSocketDataProcessor()
{
    if(m_Message_Queue_Handle)
    {
        vQueueDelete(m_Message_Queue_Handle);
        m_Message_Queue_Handle = nullptr;
    }    
    if(m_Message_Task_Handle)
    {
        vTaskDelete(m_Message_Task_Handle);
        m_Message_Task_Handle = nullptr;
    }

}

void WebSocketDataProcessor::Setup()
{
    m_Message_Queue_Handle = xQueueCreate(50, sizeof(KVP*));
    if(!m_Message_Queue_Handle)
    {
        ESP_LOGE("Setup","ERROR! Unable to create queue.");
    }

    xTaskCreate(Static_Message_Task, "Message Task", 5000, this, THREAD_PRIORITY_HIGH, &m_Message_Task_Handle);
    if(!m_Message_Task_Handle)
    {
        ESP_LOGE("Setup","ERROR! Unable to create task.");
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
        if(m_Message_Queue_Handle)
        {
            KVP* pair_raw = nullptr;
            std::vector<KVP> signalValues;
            while(xQueueReceive(m_Message_Queue_Handle, &pair_raw, pdMS_TO_TICKS(0)) == pdTRUE)
            {
                if(pair_raw)
                {
                    signalValues.push_back(*pair_raw);
                    delete pair_raw;
                }
                else
                {
                    ESP_LOGE("Message_Task","ERROR! Null pointers.");
                }
            }
            if (!signalValues.empty())
            {
                std::string message;
                Encode_Signal_Values_To_JSON(signalValues, message);
                Serial.println(message.c_str());
                NotifyClients(message);
            }
            vTaskDelay(pdMS_TO_TICKS(20));
        }
        else
        {
            ESP_LOGE("Message_Task","ERROR! Null pointers.");
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
}

void WebSocketDataProcessor::Handle_Current_Value_Request(uint8_t clientId)
{
    ESP_LOGI("WebSocketDataProcessor::Handle_Current_Value_Requect", "Sending All Data to Client: %u", clientId);
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

void WebSocketDataProcessor::TxDataToWebSocket(std::string key, std::string value)
{
    KVP *keyValuePair = new KVP(key, value);
    Serial.println(keyValuePair->Value.c_str());
    if(m_Message_Queue_Handle)
    {
        if(xQueueSend(m_Message_Queue_Handle, &keyValuePair, pdMS_TO_TICKS(0)) != pdTRUE)
        {
            delete keyValuePair;
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
        jsonArray[i] = ConvertToJsonVar(keyValuePairs[i]);
    }

    result = std::string(JSON.stringify(jsonArray).c_str());

    if (result.empty())
    {
        ESP_LOGE("Encode_Signal_Values_To_JSON", "ERROR! Failed to serialize JSON array.");
        result = "[]";
    }
}

JSONVar WebSocketDataProcessor::ConvertToJsonVar(KVP pair)
{
    JSONVar jv;
    jv["Id"] = pair.Key.c_str();
    jv["Value"] = pair.Value.c_str();
    return jv;
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