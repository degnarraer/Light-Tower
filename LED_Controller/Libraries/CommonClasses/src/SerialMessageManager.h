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

#pragma once
#include <HardwareSerial.h>
#include "SerialDMA.h"
#include <Arduino.h>
#include <vector>
#include <memory>
#include "Helpers.h"
#include "DataSerializer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/portmacro.h"

#define TASK_DELAY 20
#define MAX_BLOCK_TIME_MS 100 
#define MESSAGE_DELAY 1
#define NULL_POINTER_THREAD_DELAY 100

#define TIME_TO_WAIT_TO_SEND 0
#define TIME_TO_WAIT_TO_RECEIVE 0

#define MAX_QUEUE_COUNT 50
#define MAX_MESSAGE_LENGTH 1000

template <typename T>
class Rx_Value_Caller_Interface;

template <typename T>
class Rx_Value_Callee_Interface
{
	public:
		Rx_Value_Callee_Interface()
		{
			
		}
		virtual ~Rx_Value_Callee_Interface()
		{
			
		}
		virtual bool NewRxValueReceived(const T* values, size_t count, size_t changeCount) = 0;
		virtual std::string GetName() const = 0;
		virtual size_t GetCount(){ return m_Count;}
	private:
		size_t m_Count = 0;
};

template <typename T>
class Named_Callback_Caller_Interface
{
	public:
		Named_Callback_Caller_Interface()
		{
			
		}
		virtual ~Named_Callback_Caller_Interface()
		{
			
		}
		virtual void RegisterNamedCallback(NamedCallback_t *namedCallback)
		{
			ESP_LOGD("RegisterNamedCallback", "Try Registering callback");			
			bool IsFound = false;
			if(namedCallback)
			{
				for (NamedCallback_t* callback : m_NamedCallbacks)
				{
					if(*namedCallback == *callback)
					{
						ESP_LOGE("RegisterNamedCallback", "ERROR! A callback with this name already exists.");
						IsFound = true;
						break;
					}
				}
				if(false == IsFound)
				{
					ESP_LOGD("RegisterNamedCallback", "NamedCallback Registered");
					m_NamedCallbacks.push_back(namedCallback);
				}
			}
		}
		virtual void DeRegisterNamedCallback(NamedCallback_t* NamedCallback)
		{
			auto it = std::find(m_NamedCallbacks.begin(), m_NamedCallbacks.end(), NamedCallback);
			if (it != m_NamedCallbacks.end())
			{
				m_NamedCallbacks.erase(it);
			}
		}
	protected:
		virtual void CallNamedCallbacks(T* object)
		{
			ESP_LOGD("NotifyCallee", "CallCallbacks");
			for (NamedCallback_t* namedCallback : m_NamedCallbacks)
			{
				ESP_LOGD("NotifyCallee", "Calling Callback %s", namedCallback->Name.c_str());
				void (*aCallback)(const std::string&, void*, void*);
				aCallback = namedCallback->Callback;
				void* arg = namedCallback->Arg;
				aCallback(namedCallback->Name, object, arg);
			}
		}
		
	private:
		std::vector<NamedCallback_t*> m_NamedCallbacks = std::vector<NamedCallback_t*>();
};

template <typename T>
class Rx_Value_Caller_Interface
{
	public:
		Rx_Value_Caller_Interface()
		{
			
		}
		virtual ~Rx_Value_Caller_Interface()
		{
			
		}
		virtual void RegisterForNewRxValueNotification(Rx_Value_Callee_Interface<T>* NewCallee)
		{
			ESP_LOGD("RegisterForNewRxValueNotification", "Try Registering Callee");
			bool IsFound = false;
			for (Rx_Value_Callee_Interface<T>* callee : m_NewRxValueCallees)
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
				m_NewRxValueCallees.push_back(NewCallee);
			}
		}
		virtual void DeRegisterForNewRxValueNotification(Rx_Value_Callee_Interface<T>* Callee)
		{
			auto it = std::find(m_NewRxValueCallees.begin(), m_NewRxValueCallees.end(), Callee);
			if (it != m_NewRxValueCallees.end()) {
				m_NewRxValueCallees.erase(it);
			}
		}
		
	protected:
		virtual void Notify_NewRxValue_Callees(T* values, size_t count, size_t changeCount)
		{
			ESP_LOGD("Notify_NewRxValue_Callees", "Notify Callees");
			for (Rx_Value_Callee_Interface<T>* callee : m_NewRxValueCallees)
			{
				callee->NewRxValueReceived(values, count, changeCount);
			}
		}
	private:
		std::vector<Rx_Value_Callee_Interface<T>*> m_NewRxValueCallees = std::vector<Rx_Value_Callee_Interface<T>*>();
};

class Named_Object_Caller_Interface;
class Named_Object_Callee_Interface
{
	public:
		Named_Object_Callee_Interface( size_t Count )
									 : m_Count(Count)
		{
			
		}
		virtual ~Named_Object_Callee_Interface()
		{
			
		}
		virtual UpdateStatus_t New_Object_From_Sender(const Named_Object_Caller_Interface* sender, const void* object, const size_t changeCount) = 0;
		virtual std::string GetName() const = 0;
		size_t GetCount(){ return m_Count;}
	private:
		size_t m_Count = 0;
};

class Named_Object_Caller_Interface
{
	public:
		Named_Object_Caller_Interface()
		{
			
		}
		virtual ~Named_Object_Caller_Interface()
		{
			
		}
		virtual void RegisterForNewRxValueNotification(Named_Object_Callee_Interface* newCallee);
		virtual void DeRegisterForNewRxValueNotification(Named_Object_Callee_Interface* callee);
		virtual std::string GetName() const = 0;
	protected:
		virtual void Call_Named_Object_Callback(const std::string& name, void* object, const size_t changeCount);
	private:
		std::vector<Named_Object_Callee_Interface*> m_NewValueCallees = std::vector<Named_Object_Callee_Interface*>();
		std::vector<NamedCallback_t*> m_NamedCallbacks = std::vector<NamedCallback_t*>();
};

class SerialPortMessageManager: public Named_Object_Caller_Interface
{
	public:
		SerialPortMessageManager(){}
		SerialPortMessageManager( const std::string& name
								, HardwareSerial *serial
								, DataSerializer *dataSerializer
								, BaseType_t priority )
								: m_Name(name)
								, mp_Serial(serial)
								, mp_DataSerializer(dataSerializer)
								, m_Priority(priority)
		{
		}
		SerialPortMessageManager( const std::string& name
								, int rxPin
								, int txPin
								, int baudRate
								, uart_port_t port
								, DataSerializer *dataSerializer
								, BaseType_t priority )
								: m_Name(name)
								, mp_DataSerializer(dataSerializer)
								, m_Priority(priority)
								, mp_SerialDMA(new SerialDMA(rxPin, txPin, baudRate, StaticOnNewMessage, this, port, priority))
		{
		}
		virtual ~SerialPortMessageManager()
		{
			ESP_LOGD("~SerialPortMessageManager", "Deleting SerialPortMessageManager");
			if(m_RXTaskHandle)
			{
				ESP_LOGD("~SerialPortMessageManager", "Deleting RX Task.");
				vTaskDelete(m_RXTaskHandle);
				m_RXTaskHandle = nullptr;
			}
			if(m_RXQueueTaskHandle)
			{
				ESP_LOGD("~SerialPortMessageManager", "Deleting RX Task.");
				vTaskDelete(m_RXQueueTaskHandle);
				m_RXQueueTaskHandle = nullptr;
			}
			if(m_TXTaskHandle)
			{
				ESP_LOGD("~SerialPortMessageManager", "Deleting TX Task.");
				vTaskDelete(m_TXTaskHandle);
				m_TXTaskHandle = nullptr;
			}
			if(m_TXQueueHandle)
			{
				ESP_LOGD("~SerialPortMessageManager", "Deleting TX Queue");
				vQueueDelete(m_TXQueueHandle);
				m_TXQueueHandle = nullptr;
			}
			if(m_MessageQueueHandle)
			{
				ESP_LOGD("~SerialPortMessageManager", "Deleting RX Queue");
				vQueueDelete(m_MessageQueueHandle);
				m_MessageQueueHandle = nullptr;
			}
			ESP_LOGD("~SerialPortMessageManager", "SerialPortMessageManager Deleted");
		}
		virtual void Setup();
		virtual void QueueNewRXMessage(const std::string &message);
		virtual bool QueueMessageFromDataType(const std::string& Name, DataType_t DataType, void* Object, size_t Count, size_t ChangeCount);
		virtual bool QueueMessage(const std::string& message);
		virtual std::string GetName() const override
		{
			return m_Name;
		}
	private:
		std::string m_Name;
		HardwareSerial *mp_Serial = nullptr;
		SerialDMA *mp_SerialDMA;
		DataSerializer *mp_DataSerializer = nullptr;
		BaseType_t m_Priority = THREAD_PRIORITY_HIGH;
		std::string m_message;
		TaskHandle_t m_RXTaskHandle = nullptr;
		TaskHandle_t m_RXQueueTaskHandle = nullptr;
		TaskHandle_t m_TXTaskHandle = nullptr;
		QueueHandle_t m_TXQueueHandle = nullptr;
		QueueHandle_t m_MessageQueueHandle = nullptr;
	
		static void StaticOnNewMessage(const std::string& message, void* arg)
		{
    		SerialPortMessageManager *manager = static_cast<SerialPortMessageManager*>(arg);
			manager->OnNewMessage(message);
		} 
		
		void OnNewMessage(const std::string& message)
		{
            ESP_LOGD("OnNewMessage", "%s", message.c_str());
    		QueueNewRXMessage(message);
		}

		static void StaticSerialPortMessageManager_RxQueueTask(void *Parameters)
		{
			SerialPortMessageManager* aSerialPortMessageManager = (SerialPortMessageManager*)Parameters;
			aSerialPortMessageManager->SerialPortMessageManager_RxQueueTask();
		}

		virtual void SerialPortMessageManager_RxQueueTask()
		{
			while (true)
			{
				std::string *p_rxMessage;
				if(xQueueReceive(m_MessageQueueHandle, &p_rxMessage, portMAX_DELAY) == pdTRUE)
				{
					std::unique_ptr<std::string> sp_rxMessage(p_rxMessage);
					NamedObject_t NamedObject;
					if (mp_DataSerializer->DeSerializeJsonToNamedObject(p_rxMessage->c_str(), NamedObject))
					{
						ESP_LOGD("SerialPortMessageManager", "\"%s\" DeSerialized Named object: \"%s\" Address: \"%p\"", m_Name, NamedObject.Name.c_str(), static_cast<void*>(NamedObject.Object));
						this->Call_Named_Object_Callback(NamedObject.Name, NamedObject.Object, NamedObject.ChangeCount);
					}
					else
					{
						ESP_LOGW("SerialPortMessageManager", "WARNING! \"%s\" DeSerialized Named object failed", m_Name.c_str());
					}
				}
			}
		}

		static void StaticSerialPortMessageManager_TxTask(void *Parameters)
		{
			SerialPortMessageManager* aSerialPortMessageManager = (SerialPortMessageManager*)Parameters;
			aSerialPortMessageManager->SerialPortMessageManager_TxTask();
		}
		virtual void SerialPortMessageManager_TxTask();
		
		std::string trim(const std::string &str) {
			size_t start = str.find_first_not_of(" \t\n\r");
			size_t end = str.find_last_not_of(" \t\n\r");
			if (start == std::string::npos || end == std::string::npos)
			{
				return "";
			}
			return str.substr(start, end - start + 1);
		}
};