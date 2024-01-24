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


void SetupCallerInterface::RegisterForSetupCall(SetupCalleeInterface* NewCallee)
{
	ESP_LOGI("RegisterForSetupCall", "Try Registering Callee");
	bool IsFound = false;
	for (SetupCalleeInterface* callee : m_SetupCallees)
	{
		if(NewCallee == callee)
		{
			ESP_LOGE("RegisterForSetupCall", "A callee with this name already exists!");
			IsFound = true;
			break;
		}
	}
	if(false == IsFound)
	{
		ESP_LOGI("RegisterForSetupCall", "Callee Registered");
		m_SetupCallees.push_back(NewCallee);
	}
}
void SetupCallerInterface::DeRegisterForSetupCall(SetupCalleeInterface* Callee)
{
	auto it = std::find(m_SetupCallees.begin(), m_SetupCallees.end(), Callee);
	if (it != m_SetupCallees.end())
	{
		m_SetupCallees.erase(it);
	}
}

void SetupCallerInterface::SetupAllSetupCallees()
{
	ESP_LOGD("SetupCallerInterface", "Setup All Setup Callees");
	for (SetupCalleeInterface* callee : m_SetupCallees)
	{
		if (callee) 
		{
			callee->Setup();
		}
	}
}

void NewRxTxVoidObjectCallerInterface::RegisterForNewValueNotification(NewRxTxVoidObjectCalleeInterface* NewCallee)
{
	ESP_LOGI("RegisterForNewValueNotification", "Try Registering Callee");
	bool IsFound = false;
	for (NewRxTxVoidObjectCalleeInterface* callee : m_NewValueCallees)
	{
		if(NewCallee == callee)
		{
			ESP_LOGE("RegisterForNewValueNotification", "A callee with this name already exists!");
			IsFound = true;
			break;
		}
	}
	if(false == IsFound)
	{
		ESP_LOGI("RegisterForNewValueNotification", "Callee Registered");
		m_NewValueCallees.push_back(NewCallee);
	}
}

void NewRxTxVoidObjectCallerInterface::DeRegisterForNewValueNotification(NewRxTxVoidObjectCalleeInterface* Callee)
{
	// Find the iterator pointing to the element
	auto it = std::find(m_NewValueCallees.begin(), m_NewValueCallees.end(), Callee);

	// Check if the element was found before erasing
	if (it != m_NewValueCallees.end()) {
		m_NewValueCallees.erase(it);
	}
}

void NewRxTxVoidObjectCallerInterface::RegisterNamedCallback(NamedCallback_t* NamedCallback)
{
	ESP_LOGI("RegisterNamedCallback", "Try Registering callback");
	bool IsFound = false;
	for (NamedCallback_t* callback : m_NamedCallbacks)
	{
		if(NamedCallback == callback)
		{
			ESP_LOGE("RegisterNamedCallback", "A callback with this name already exists!");
			IsFound = true;
			break;
		}
	}
	if(false == IsFound)
	{
		ESP_LOGI("RegisterNamedCallback", "NamedCallback Registered");
		m_NamedCallbacks.push_back(NamedCallback);
	}
	
}

void NewRxTxVoidObjectCallerInterface::DeRegisterNamedCallback(NamedCallback_t* NamedCallback)
{
	// Find the iterator pointing to the element
	auto it = std::find(m_NamedCallbacks.begin(), m_NamedCallbacks.end(), NamedCallback);

	// Check if the element was found before erasing
	if (it != m_NamedCallbacks.end()) {
		m_NamedCallbacks.erase(it);
	}
}

void NewRxTxVoidObjectCallerInterface::NotifyCallee(const String& name, void* object)
{
	ESP_LOGD("NotifyCallee", "Notify Callees");
	for (NewRxTxVoidObjectCalleeInterface* callee : m_NewValueCallees)
	{
		if (callee) 
		{
			if (callee->GetName().equals(name))
			{
				callee->NewRXValueReceived(object, callee->GetCount());
				break;
			}
		}
	}
}

void NewRxTxVoidObjectCallerInterface::CallCallbacks(const String& name, void* object)
{
	ESP_LOGD("NotifyCallee", "CallCallbacks");
	for (NamedCallback_t* namedCallback : m_NamedCallbacks)
	{
		if (namedCallback->Callback) 
		{
			void (*aCallback)(const String&, void*);
			aCallback(namedCallback->Name, object);
			break;	
		}
	}
}

void SerialPortMessageManager::SetupSerialPortMessageManager()
{
	if(xTaskCreatePinnedToCore( StaticSerialPortMessageManager_RxTask, m_Name.c_str(), 10000, this,  THREAD_PRIORITY_HIGH,  &m_RXTaskHandle,  1 ) != pdPASS)
	ESP_LOGE("SetupSerialPortMessageManager", "ERROR! Error creating the RX Task.");
	else ESP_LOGI("SetupSerialPortMessageManager", "RX Task Created.");
	
	if(xTaskCreatePinnedToCore( StaticSerialPortMessageManager_TxTask, m_Name.c_str(), 10000, this,  THREAD_PRIORITY_HIGH,  &m_TXTaskHandle,  1 ) != pdPASS)
	ESP_LOGE("SetupSerialPortMessageManager", "ERROR! Error creating the TX Task.");
	else ESP_LOGI("SetupSerialPortMessageManager", "TX Task Created.");
	
	m_TXQueue = xQueueCreate(MaxQueueCount, sizeof(char) * MaxMessageLength );
	if(NULL == m_TXQueue) ESP_LOGE("SetupSerialPortMessageManager", "ERROR! Error creating the TX Queue.");
	else ESP_LOGI("SetupSerialPortMessageManager", "TX Queue Created.");
	SetupAllSetupCallees();
}
bool SerialPortMessageManager::QueueMessageFromData(const String& Name, DataType_t DataType, void* Object, size_t Count)
{
	bool result = false;
	if(nullptr == Object || 0 == Name.length() || 0 == Count)
	{
		ESP_LOGE("QueueMessageFromData", "Error Invalid Data!");
	}
	else
	{
		ESP_LOGD("QueueMessageFromData", "Serializing Data for: \"%s\" Data Type: \"%i\", Pointer: \"%p\" Count: \"%i\" ", Name.c_str(), DataType, static_cast<void*>(Object), Count);
		String message = m_DataSerializer.SerializeDataToJson(Name, DataType, Object, Count);
		result = QueueMessage( message.c_str() );
	}
	return result;
}
bool SerialPortMessageManager::QueueMessage(const String& message)
{
	bool result = false;
	if(m_TXQueue)
	{	
		if( message.length() > 0 &&
			message.length() <= MaxMessageLength && 
			xQueueSend(m_TXQueue, message.c_str(), 0) == pdTRUE )
		{
			ESP_LOGD("QueueMessage", "\"%s\" Queued Message: \"%s\"", m_Name.c_str(), message.c_str());
			result = true;
		}
		else
		{
			ESP_LOGW("QueueMessage", "WARNING! \"%s\" Unable to Queue Message.", m_Name.c_str());
		}
	}
	else
	{
		ESP_LOGE("QueueMessage", "Error! NULL Queue!");
	}
	return result;
}

void SerialPortMessageManager::SerialPortMessageManager_RxTask()
{
	ESP_LOGI("SetupSerialPortMessageManager", "Starting RX Task.");
	const TickType_t xFrequency = 10;
	TickType_t xLastWakeTime = xTaskGetTickCount();
	while(true)
	{
		vTaskDelayUntil( &xLastWakeTime, xFrequency );
		while (m_Serial.available())
		{
			char character = m_Serial.read();
			if(m_message.length() > MaxMessageLength)
			{
				ESP_LOGE("SerialPortMessageManager", "Message RX Overrun: \"%s\"", m_message.c_str());
				m_message = "";
			}
			if(character == '\n')
			{
				ESP_LOGD("SerialPortMessageManager", "\"%s\" Message RX: \"%s\"", m_Name.c_str(), m_message.c_str());
				
				NamedObject_t NamedObject;
				m_DataSerializer.DeSerializeJsonToNamedObject(m_message.c_str(), NamedObject);
				if(NamedObject.Object)
				{
					ESP_LOGD("SerialPortMessageManager", "\"%s\" DeSerialized Named object: \"%s\" Address: \"%p\"", m_Name.c_str(), NamedObject.Name.c_str(), static_cast<void*>(NamedObject.Object));
					NotifyCallee(NamedObject.Name, NamedObject.Object);
				}
				else
				{
					ESP_LOGW("SerialPortMessageManager", "\"%s\" DeSerialized Named object failed", m_Name.c_str());
				}
				m_message = "";
			}
			else
			{
				m_message += character;
			}
		}
	}
}

void SerialPortMessageManager::SerialPortMessageManager_TxTask()
{
	ESP_LOGI("SetupSerialPortMessageManager", "Starting TX Task.");
	const TickType_t xFrequency = 20;
	TickType_t xLastWakeTime = xTaskGetTickCount();
	while(true)
	{
		vTaskDelayUntil( &xLastWakeTime, xFrequency );
		if(NULL != m_TXQueue)
		{
			size_t QueueCount = uxQueueMessagesWaiting(m_TXQueue);
			if(QueueCount) ESP_LOGD("SerialPortMessageManager_TxTask", "Queue Count: %i", QueueCount);
			for(size_t i = 0; i < QueueCount; ++i)
			{
				char message[MaxMessageLength] = "\0";
				if ( xQueueReceive(m_TXQueue, message, 0) == pdTRUE )
				{
					if (strlen(message) > MaxMessageLength)
                    {
                        ESP_LOGW("SerialPortMessageManager_TxTask", "WARNING! Message exceeds MaxMessageLength. Truncating.");
                        message[MaxMessageLength - 1] = '\0';
                    }
					ESP_LOGD("SerialPortMessageManager_TxTask", "Data TX: Address: \"%p\" Message: \"%s\"", static_cast<void*>(message), message);
					m_Serial.println(message);
				}
				else
				{
					ESP_LOGE("SerialPortMessageManager_TxTask", "ERROR! Unable to Send Message.");
				}
			}
		}
		else
		{
			ESP_LOGE("SerialPortMessageManager_TxTask", "ERROR! NULL TX Queue.");
		}
	}
}