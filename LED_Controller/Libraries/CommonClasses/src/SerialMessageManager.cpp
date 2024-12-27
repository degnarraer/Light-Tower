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

#include "SerialMessageManager.h"

//#define TIME_TO_WAIT_FOR_SOUND portMAX_DELAY
#define TIME_TO_WAIT_TO_SEND pdMS_TO_TICKS(0)

void Named_Object_Caller_Interface::RegisterForNewRxValueNotification(Named_Object_Callee_Interface* NewCallee)
{
	ESP_LOGD("RegisterForNewRxValueNotification", "Try Registering Callee");
	bool IsFound = false;
	for (Named_Object_Callee_Interface* callee : m_NewValueCallees)
	{
		if(NewCallee == callee)
		{
			ESP_LOGE("RegisterForNewRxValueNotification", "ERROR! A callee with the name \"%s\" already exists.", NewCallee->GetName().c_str());
			IsFound = true;
			break;
		}
	}
	if(false == IsFound)
	{
		ESP_LOGD("RegisterForNewRxValueNotification", "Callee Registered");
		m_NewValueCallees.push_back(NewCallee);
	}
}

void Named_Object_Caller_Interface::DeRegisterForNewRxValueNotification(Named_Object_Callee_Interface* Callee)
{
	ESP_LOGD("DeRegisterForNewRxValueNotification", "Try DeRegister For New Value Notification");
	auto it = std::find(m_NewValueCallees.begin(), m_NewValueCallees.end(), Callee);
	if (it != m_NewValueCallees.end()) {
		ESP_LOGD("RegisterForNewRxValueNotification", "Callee DeRegistered");
		m_NewValueCallees.erase(it);
	}
}

void Named_Object_Caller_Interface::Call_Named_Object_Callback(const std::string& name, void* object, const size_t changeCount)
{
	ESP_LOGD("Call_Named_Object_Callback", "Notify Callee: \"%s\"", name.c_str());
	bool found = false;
	for (Named_Object_Callee_Interface* callee : m_NewValueCallees)
	{
		if (callee) 
		{
			if (callee->GetName() == name)
			{
				found = true;
				ESP_LOGD("Call_Named_Object_Callback", "Callee Found: \"%s\"", name.c_str());
				callee->New_Object_From_Sender(this, object, changeCount);
				break;
			}
		}
	}
	if(!found) ESP_LOGE("NewRxValueReceived", "ERROR! Rx Value Callee Not Found Found: \"%s\"", name.c_str());
}

void SerialPortMessageManager::Setup()
{
	if(m_TXQueueHandle = xQueueCreate(MaxQueueCount, sizeof(std::string*)))
	ESP_LOGD("Setup", "TX Queue Created.");
	else ESP_LOGE("Setup", "ERROR! Error creating the TX Queue.");

	if(m_MessageQueueHandle = xQueueCreate(MaxQueueCount, sizeof(std::string*)))
	ESP_LOGD("Setup", "RX Queue Created.");
	else ESP_LOGE("Setup", "ERROR! Error creating the RX Queue.");

	if(xTaskCreate( StaticSerialPortMessageManager_RxTask, m_Name.c_str(), 5000, this,  THREAD_PRIORITY_RT,  &m_RXTaskHandle ) == pdPASS)
	ESP_LOGD("Setup", "RX Task Created.");
	else ESP_LOGE("Setup", "ERROR! Error creating the RX Task.");

	if(xTaskCreate( StaticSerialPortMessageManager_RxQueueTask, m_Name.c_str(), 5000, this,  THREAD_PRIORITY_MEDIUM,  &m_RXQueueTaskHandle ) == pdPASS)
	ESP_LOGD("Setup", "RX Queue Task Created.");
	else ESP_LOGE("Setup", "ERROR! Error creating the RX Queue Task.");
	
	if(xTaskCreate( StaticSerialPortMessageManager_TxTask, m_Name.c_str(), 5000, this,  THREAD_PRIORITY_RT,  &m_TXTaskHandle ) == pdPASS)
	ESP_LOGD("Setup", "TX Task Created.");
	else ESP_LOGE("Setup", "ERROR! Error creating the TX Task.");
}

bool SerialPortMessageManager::QueueMessageFromDataType(const std::string& Name, DataType_t DataType, void* Object, size_t Count, size_t ChangeCount)
{
	bool result = false;
	if(nullptr == Object || 0 == Name.length() || 0 == Count)
	{
		ESP_LOGE("QueueMessageFromDataType", "ERROR! Invalid Data.");
	}
	else
	{
		if(mp_DataSerializer)
		{
			std::string message = mp_DataSerializer->SerializeDataItemToJson(Name, DataType, Object, Count, ChangeCount);
			result = QueueMessage( message );
		}
		else
		{
			ESP_LOGE("QueueMessageFromDataType", "ERROR! Null Pointer.");
		}
	}
	return result;
}

bool SerialPortMessageManager::QueueMessage(const std::string& message)
{
	bool result = false;
	if(m_TXQueueHandle)
	{	
		if( message.length() > 0 && message.length() <= MaxMessageLength )
		{
			std::string *p_txMessage = new std::string;
			*p_txMessage = message;
			if( xQueueSend(m_TXQueueHandle, &p_txMessage, TIME_TO_WAIT_TO_SEND) == pdTRUE )
			{
				ESP_LOGD("QueueMessage", "\"%s\" Queued Message: \"%s\"", m_Name, p_txMessage->c_str());
				result = true;
			}
			else
			{
				ESP_LOGW("QueueMessage", "WARNING! \"%s\" Unable to Queue Message.", m_Name);
				delete p_txMessage;
			}
		}
	}
	else
	{
		ESP_LOGE("QueueMessage", "ERROR! NULL Queue.");
	}
	return result;
}

void SerialPortMessageManager::SerialPortMessageManager_RxTask()
{
    ESP_LOGD("Setup", "Starting RX Task.");
	const TickType_t xFrequency = 20;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    
    while (true)
    {
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
        if (mp_Serial && m_MessageQueueHandle)
        {
			size_t available = mp_Serial->available();
            for(int i = 0; i < available; ++i)
            {
                char character = mp_Serial->read();
                if (m_message.length() >= MaxMessageLength)
                {
                    ESP_LOGE("SerialPortMessageManager_RxTask", "ERROR! Message RX Overrun: \"%s\".", m_message.c_str());
                    m_message.clear();
                    break;
                }
                m_message += character;
                if (m_message.size() >= 1 && character == '\n')
                {
					m_message = trim(m_message);
					std::string *p_rxMessage = new std::string;
					*p_rxMessage = m_message;
					ESP_LOGD("SerialPortMessageManager_RxTask", "Rx from: \"%s\" Message: \"%s\"", m_Name, m_message.c_str());
					if( xQueueSend(m_MessageQueueHandle, &p_rxMessage, pdMS_TO_TICKS(0)) != pdTRUE )
					{
        				static LogWithRateLimit SerialPortMessageManager_RxTask_QueueFail_RLL(1000, ESP_LOG_WARN);
						SerialPortMessageManager_RxTask_QueueFail_RLL.Log(ESP_LOG_WARN, "SerialPortMessageManager_RxTask", "RX Message Dropped.");
						delete p_rxMessage;
					}
                    m_message.clear();
                }
            }
        }
        else
        {
            ESP_LOGE("SerialPortMessageManager_RxTask", "ERROR! Null Pointer.");
            vTaskDelay(100);
        }
    }
}

void SerialPortMessageManager::SerialPortMessageManager_TxTask()
{
	ESP_LOGD("Setup", "Starting TX Task.");
	const TickType_t xFrequency = 20;
	TickType_t xLastWakeTime = xTaskGetTickCount();
	while(true)
	{
		vTaskDelayUntil( &xLastWakeTime, xFrequency );
		if(m_TXQueueHandle)
		{
			size_t messages = uxQueueMessagesWaiting(m_TXQueueHandle);
			for(size_t i = 0; i < messages; ++i)
			{
				std::string *p_Tx_Message;
				if ( xQueueReceive(m_TXQueueHandle, &p_Tx_Message, 0) == pdTRUE )
				{
					if(p_Tx_Message)
					{
						if (p_Tx_Message->length() > MaxMessageLength)
						{
							ESP_LOGW("SerialPortMessageManager_TxTask", "\"%s\" WARNING! Message exceeds MaxMessageLength. Truncating.",m_Name);
							(*p_Tx_Message)[MaxMessageLength - 1] = '\0';
						}
						ESP_LOGD("SerialPortMessageManager_TxTask", "\"%s\" Data TX: \"%s\"",m_Name, p_Tx_Message->c_str());
						mp_Serial->println(p_Tx_Message->c_str());
						delete p_Tx_Message;
					}
				}
				else
				{
					ESP_LOGE("SerialPortMessageManager_TxTask", "\"%s\" ERROR! Unable to Send Message.",m_Name);
				}
			}
		}
		else
		{
			ESP_LOGE("SerialPortMessageManager_TxTask", "ERROR! NULL TX Queue.");
		}
	}
}