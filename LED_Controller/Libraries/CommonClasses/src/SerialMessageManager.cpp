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

void Named_Object_Caller_Interface::RegisterForNewRxValueNotification(Named_Object_Callee_Interface* NewCallee)
{
	ESP_LOGD("RegisterForNewRxValueNotification", "Try Registering Callee");
	bool IsFound = false;
	for (Named_Object_Callee_Interface* callee : m_NewValueCallees)
	{
		if(!callee) 
		{
			ESP_LOGE("Call_Named_Object_Callback", "ERROR! Invalid Callee");
			continue;
		}
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
		if(!callee) 
		{
			ESP_LOGE("Call_Named_Object_Callback", "ERROR! Invalid Callee.");
			continue;
		}
		if (callee->GetName() == name)
		{
			found = true;
			ESP_LOGD("Call_Named_Object_Callback", "Callee Found: \"%s\"", name.c_str());
			callee->New_Object_From_Sender(this, object, changeCount);
			break;
		}
	}
	if(!found) ESP_LOGE("NewRxValueReceived", "ERROR! Rx Value Callee Not Found Found: \"%s\"", name.c_str());
}

void SerialPortMessageManager::Setup()
{
	ESP_LOGD("Setup", "SerialPortMessageManager Setup.");
	if(mp_SerialDMA)
	{
		mp_SerialDMA->begin();
	}
	if(m_TXQueueHandle = xQueueCreate(MAX_QUEUE_COUNT, sizeof(std::string*)))
	ESP_LOGD("Setup", "TX Queue Created.");
	else ESP_LOGE("Setup", "ERROR! Error creating the TX Queue.");

	if(m_MessageQueueHandle = xQueueCreate(MAX_QUEUE_COUNT, sizeof(std::string*)))
	ESP_LOGD("Setup", "RX Queue Created.");
	else ESP_LOGE("Setup", "ERROR! Error creating the RX Queue.");

	if(xTaskCreate( StaticSerialPortMessageManager_RxQueueTask, m_Name.c_str(), 5000, this,  m_Priority,  &m_RXQueueTaskHandle ) == pdPASS)
	ESP_LOGD("Setup", "RX Queue Task Created.");
	else ESP_LOGE("Setup", "ERROR! Error creating the RX Queue Task.");
	
	if(xTaskCreate( StaticSerialPortMessageManager_TxTask, m_Name.c_str(), 5000, this,  m_Priority,  &m_TXTaskHandle ) == pdPASS)
	ESP_LOGD("Setup", "TX Task Created.");
	else ESP_LOGE("Setup", "ERROR! Error creating the TX Task.");
}

bool SerialPortMessageManager::QueueMessageFromDataType(const std::string& Name, DataType_t DataType, void* Object, size_t Count, size_t ChangeCount)
{
	bool result = false;
	if(nullptr == mp_DataSerializer || nullptr == Object || 0 == Name.length() || 0 == Count)
	{
		ESP_LOGE("QueueMessageFromDataType", "ERROR! Invalid Data.");
		return result;
	}
	std::string message = mp_DataSerializer->SerializeDataItemToJson(Name, DataType, Object, Count, ChangeCount);
	result = QueueMessage( message );
	return result;
}

bool SerialPortMessageManager::QueueMessage(const std::string& message)
{
	bool result = false;
	if( message.length() > 0 && message.length() <= MAX_MESSAGE_LENGTH )
	{
		std::string *p_txMessage = new std::string;
		*p_txMessage = message;
		if( xQueueSend(m_TXQueueHandle, &p_txMessage, SEMAPHORE_MEDIUM_BLOCK) == pdTRUE )
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
	else
	{
		ESP_LOGW("QueueMessage", "WARNING! \"%s\" Message too long.", m_Name);
	}
	return result;
}

void SerialPortMessageManager::QueueNewRXMessage(const std::string &message)
{
    std::unique_ptr<std::string> sp_rxMessage = std::make_unique<std::string>(message);
    trim(*sp_rxMessage);
    std::string *p_rxMessage_raw = sp_rxMessage.release();

    ESP_LOGD("SerialPortMessageManager_RxTask", "Rx from: \"%s\" Message: \"%s\"", m_Name, p_rxMessage_raw->c_str());

    // Use xQueueSendFromISR inside an interrupt
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	// Inside an interrupt, use xQueueSendFromISR
	if (xQueueSendFromISR(m_MessageQueueHandle, &p_rxMessage_raw, &xHigherPriorityTaskWoken) != pdTRUE)
	{
		// Log failure
		static LogWithRateLimit SerialPortMessageManager_RxTask_QueueFail_RLL(1000, ESP_LOG_WARN);
		SerialPortMessageManager_RxTask_QueueFail_RLL.Log(ESP_LOG_WARN, "SerialPortMessageManager_RxTask", "RX Message Dropped.");
		delete p_rxMessage_raw;
	}
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken); // Yield to higher priority task if needed
}

void SerialPortMessageManager::SerialPortMessageManager_TxTask()
{
	while(true)
	{
		std::string *rawMessage;
		if(xQueueReceive(m_TXQueueHandle, &rawMessage, SEMAPHORE_MEDIUM_BLOCK) == pdTRUE )
		{
			std::unique_ptr<std::string> sp_Tx_Message(rawMessage);
			if (sp_Tx_Message->length() > MAX_MESSAGE_LENGTH)
			{
				ESP_LOGW("SerialPortMessageManager_TxTask", "\"%s\" WARNING! Message exceeds MAX_MESSAGE_LENGTH. Skipping.",m_Name);
				continue;
			}
			ESP_LOGD("SerialPortMessageManager_TxTask", "\"%s\" Data TX: \"%s\"",m_Name, sp_Tx_Message->c_str());
			if(mp_SerialDMA)
			{
				mp_SerialDMA->write(sp_Tx_Message->c_str());
			}
		}
	}
}