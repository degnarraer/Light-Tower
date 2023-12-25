#include "DataItem.h"

void NewRXValueCaller::RegisterForNewValueNotification(NewRXValueCallee* NewCallee)
{
	ESP_LOGI("RegisterForNewValueNotification", "Try Registering Callee");
	bool IsFound = false;
	for (NewRXValueCallee* callee : m_NewValueCallees)
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

void NewRXValueCaller::DeRegisterForNewValueNotification(NewRXValueCallee* Callee)
{
	// Find the iterator pointing to the element
	auto it = std::find(m_NewValueCallees.begin(), m_NewValueCallees.end(), Callee);

	// Check if the element was found before erasing
	if (it != m_NewValueCallees.end()) {
		m_NewValueCallees.erase(it);
	}
}

void NewRXValueCaller::RegisterNamedCallback(NamedCallback_t* NamedCallback)
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

void NewRXValueCaller::DeRegisterNamedCallback(NamedCallback_t* NamedCallback)
{
	// Find the iterator pointing to the element
	auto it = std::find(m_NamedCallbacks.begin(), m_NamedCallbacks.end(), NamedCallback);

	// Check if the element was found before erasing
	if (it != m_NamedCallbacks.end()) {
		m_NamedCallbacks.erase(it);
	}
}

void NewRXValueCaller::NotifyCallee(const String& name, void* object)
{
	ESP_LOGD("NotifyCallee", "Notify Callees");
	for (NewRXValueCallee* callee : m_NewValueCallees)
	{
		if (callee) 
		{
			if (callee->GetName().equals(name))
			{
				callee->NewRXValueReceived(object);
				break;
			}
		}
	}
}
void NewRXValueCaller::CallCallbacks(const String& name, void* object)
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
	xTaskCreatePinnedToCore( StaticSerialPortMessageManager_RXLoop, m_Name.c_str(), 5000, this,  configMAX_PRIORITIES - 1,  &m_RXTaskHandle,  0 );
	xTaskCreatePinnedToCore( StaticSerialPortMessageManager_TXLoop, m_Name.c_str(), 5000, this,  configMAX_PRIORITIES - 1,  &m_TXTaskHandle,  0 );
	m_TXQueue = xQueueCreate(MaxQueueCount, sizeof(char) * MaxMessageLength );
	if(NULL == m_TXQueue)
	{
		ESP_LOGE("SetupSerialPortMessageManager", "ERROR! Error creating the TX Queue.");
	}
	else
	{
		ESP_LOGI("SetupSerialPortMessageManager", "Created the TX Queue.");
	}
}
void SerialPortMessageManager::QueueMessageFromData(String Name, DataType_t DataType, void* Object, size_t Count)
{
	
	if(nullptr == Object || 0 == Name.length() || 0 == Count)
	{
		ESP_LOGE("QueueMessageFromData", "Error Invalid Data!");
	}
	else
	{
		ESP_LOGD("QueueMessageFromData", "Serializing Data for: \"%s\" Data Type: \"%i\", Pointer: \"%p\" Count: \"%i\" ", Name.c_str(), DataType, static_cast<void*>(Object), Count);
		String message = m_DataSerializer.SerializeDataToJson(Name, DataType, Object, Count);
		if(message.length() == 0)
		{
			ESP_LOGE("QueueMessageFromData", "Error! 0 String Length!");
		}
		else if (message.length() >= MaxMessageLength)
		{
			ESP_LOGE("QueueMessageFromData", "Error! Message Length of %i is Greater than %i.",message.length(), MaxMessageLength);
		}
		else
		{
			QueueMessage(message);
		}
	}
	
}
void SerialPortMessageManager::QueueMessage(String message)
{
	if(nullptr != m_TXQueue)
	{
		ESP_LOGD("QueueMessage", "Queue Message: \"%s\"", message.c_str());
		if(xQueueSend(m_TXQueue, message.c_str(), 0) != pdTRUE)
		{
			ESP_LOGW("QueueMessage", "WARNING! Unable to Queue Message.");
		}
	}
	else
	{
		ESP_LOGE("QueueMessage", "Error! NULL Queue!");
	}
}

void SerialPortMessageManager::StaticSerialPortMessageManager_RXLoop(void *Parameters)
{
	SerialPortMessageManager* aSerialPortMessageManager = (SerialPortMessageManager*)Parameters;
	aSerialPortMessageManager->SerialPortMessageManager_RXLoop();
}
void SerialPortMessageManager::SerialPortMessageManager_RXLoop()
{
	const TickType_t xFrequency = 20;
	TickType_t xLastWakeTime = xTaskGetTickCount();
	String message = "";
	char character;
	while(true)
	{
		vTaskDelayUntil( &xLastWakeTime, xFrequency );
		while (m_Serial.available())
		{
			character = m_Serial.read();
			if(character == '\n')
			{
				message.concat(character);
				ESP_LOGD("SerialPortMessageManager", "Message RX: \"%s\"", message.c_str());
				NamedObject_t NamedObject;
				m_DataSerializer.DeSerializeJsonToNamedObject(message, NamedObject);
				if(NamedObject.Object)
				{
					ESP_LOGD("SerialPortMessageManager", "DeSerialized Named object: \"%s\" Address: \"%p\"", NamedObject.Name.c_str(), static_cast<void*>(NamedObject.Object));
					NotifyCallee(NamedObject.Name, NamedObject.Object);
				}
				else
				{
					if(NamedObject.Name.length() > 0)
					{
						ESP_LOGW("SerialPortMessageManager", "DeSerialized Named object %s failed", NamedObject.Name);
					}
					else
					{
						ESP_LOGW("SerialPortMessageManager", "DeSerialized Named object failed");
					}
				}
				message = "";
			}
			else
			{
				message.concat(character);
			}
		}
	}
}

void SerialPortMessageManager::StaticSerialPortMessageManager_TXLoop(void *Parameters)
{
	SerialPortMessageManager* aSerialPortMessageManager = (SerialPortMessageManager*)Parameters;
	aSerialPortMessageManager->SerialPortMessageManager_TXLoop();
}
void SerialPortMessageManager::SerialPortMessageManager_TXLoop()
{
	const TickType_t xFrequency = 20;
	TickType_t xLastWakeTime = xTaskGetTickCount();
	while(true)
	{
		vTaskDelayUntil( &xLastWakeTime, xFrequency );
		if(NULL != m_TXQueue)
		{
			size_t QueueCount = uxQueueMessagesWaiting(m_TXQueue);
			if(QueueCount > 0)
			{
				ESP_LOGD("SerialPortMessageManager_TXLoop", "Queue Count: %i", QueueCount);
				for(int i = 0; i < QueueCount; ++i)
				{
					char message[MaxMessageLength];
					if ( xQueueReceive(m_TXQueue, message, 0) == pdTRUE )
					{
						ESP_LOGI("SerialPortMessageManager_TXLoop", "Data TX: Address: \"%p\" Message: \"%s\"", static_cast<void*>(message), String(message).c_str());
						m_Serial.println(String(message).c_str());
					}
					else
					{
						ESP_LOGE("SerialPortMessageManager_TXLoop", "ERROR! Unable to Send Message.");
					}
				}
			}
		}
		else
		{
			ESP_LOGE("SerialPortMessageManager_TXLoop", "ERROR! NULL TX Queue.");
		}
	}
}