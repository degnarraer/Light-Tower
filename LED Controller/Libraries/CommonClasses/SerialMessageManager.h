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
#include <Arduino.h>
#include <vector>
#include "Helpers.h"
#include "DataSerializer.h"

#define MaxQueueCount 10
#define MaxMessageLength 500

template <typename T>
class NewRxTxValueCalleeInterface
{
	public:
		NewRxTxValueCalleeInterface()
		{
			
		}
		virtual ~NewRxTxValueCalleeInterface()
		{
			
		}
		virtual bool NewRxValueReceived(T* object, size_t count) = 0;
		virtual String GetName() = 0;
		virtual size_t GetCount(){ return m_Count;}
	private:
		size_t m_Count = 0;
};

template <typename T>
class NamedCallbackInterface
{
	public:
		NamedCallbackInterface()
		{
			
		}
		virtual ~NamedCallbackInterface()
		{
			
		}
		virtual void RegisterNamedCallback(NamedCallback_t *namedCallback)
		{
			ESP_LOGD("RegisterNamedCallback", "Try Registering callback");
			NamedCallback_t* newNamedCallback = new NamedCallback_t(*namedCallback);
			
			bool IsFound = false;
			for (NamedCallback_t* callback : m_NamedCallbacks)
			{
				if(*newNamedCallback == *callback)
				{
					ESP_LOGE("RegisterNamedCallback", "A callback with this name already exists!");
					IsFound = true;
					break;
				}
			}
			if(false == IsFound)
			{
				ESP_LOGD("RegisterNamedCallback", "NamedCallback Registered");
				m_NamedCallbacks.push_back(newNamedCallback);
			}	
		}
		virtual void DeRegisterNamedCallback(NamedCallback_t* NamedCallback)
		{
			// Find the iterator pointing to the element
			auto it = std::find(m_NamedCallbacks.begin(), m_NamedCallbacks.end(), NamedCallback);

			// Check if the element was found before erasing
			if (it != m_NamedCallbacks.end()) {
				delete *it;
				m_NamedCallbacks.erase(it);
			}
		}
	protected:
		virtual void CallCallbacks(const String& name, T* object)
		{
			ESP_LOGD("NotifyCallee", "CallCallbacks");
			for (NamedCallback_t* namedCallback : m_NamedCallbacks)
			{
				if (namedCallback->Callback) 
				{
					void (*aCallback)(const String&, void*, void*);
					aCallback = namedCallback->Callback;
					void* arg = namedCallback->Arg;
					aCallback(namedCallback->Name, object, arg);
				}
			}
		}
	private:
		std::vector<NamedCallback_t*> m_NamedCallbacks = std::vector<NamedCallback_t*>();
};

template <typename T>
class NewRxTxValueCallerInterface
{
	public:
		NewRxTxValueCallerInterface()
		{
			
		}
		virtual ~NewRxTxValueCallerInterface()
		{
			
		}
		virtual void SetNewTxValue(const T* object, const size_t count) = 0;
		virtual void RegisterForNewValueNotification(NewRxTxValueCalleeInterface<T>* NewCallee)
		{
			ESP_LOGI("RegisterForNewValueNotification", "Try Registering Callee");
			bool IsFound = false;
			for (NewRxTxValueCalleeInterface<T>* callee : m_NewValueCallees)
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
		virtual void DeRegisterForNewValueNotification(NewRxTxValueCalleeInterface<T>* Callee)
		{
			// Find the iterator pointing to the element
			auto it = std::find(m_NewValueCallees.begin(), m_NewValueCallees.end(), Callee);

			// Check if the element was found before erasing
			if (it != m_NewValueCallees.end()) {
				m_NewValueCallees.erase(it);
			}
		}
		
	protected:
		virtual void NotifyCallee(const String& name, T* object)
		{
			ESP_LOGD("NotifyCallee", "Notify Callees");
			for (NewRxTxValueCalleeInterface<T>* callee : m_NewValueCallees)
			{
				if (callee) 
				{
					if (callee->GetName().equals(name))
					{
						callee->NewRxValueReceived(object, callee->GetCount());
						break;
					}
				}
			}
		}
	private:
		std::vector<NewRxTxValueCalleeInterface<T>*> m_NewValueCallees = std::vector<NewRxTxValueCalleeInterface<T>*>();
};

class NewRxTxVoidObjectCalleeInterface
{
	public:
		NewRxTxVoidObjectCalleeInterface( size_t Count )
										: m_Count(Count)
		{
			
		}
		virtual ~NewRxTxVoidObjectCalleeInterface()
		{
			
		}
		virtual bool NewRxValueReceived(void* object, size_t Count) = 0;
		virtual String GetName() = 0;
		virtual size_t GetCount(){ return m_Count;}
	private:
		size_t m_Count = 0;
};

class NewRxTxVoidObjectCallerInterface
{
	public:
		NewRxTxVoidObjectCallerInterface()
		{
			
		}
		virtual ~NewRxTxVoidObjectCallerInterface()
		{
			
		}
		virtual void RegisterForNewValueNotification(NewRxTxVoidObjectCalleeInterface* NewCallee);
		virtual void DeRegisterForNewValueNotification(NewRxTxVoidObjectCalleeInterface* Callee);
		virtual void RegisterNamedCallback(NamedCallback_t* NamedCallback);
		virtual void DeRegisterNamedCallback(NamedCallback_t* NamedCallback);
	protected:
		virtual void NotifyCallee(const String& name, void* object);
		virtual void CallCallbacks(const String& name, void* object);
	private:
		std::vector<NewRxTxVoidObjectCalleeInterface*> m_NewValueCallees = std::vector<NewRxTxVoidObjectCalleeInterface*>();
		std::vector<NamedCallback_t*> m_NamedCallbacks = std::vector<NamedCallback_t*>();
};

class SerialPortMessageManager: public NewRxTxVoidObjectCallerInterface
{
	public:
		SerialPortMessageManager( const String& Name
								, HardwareSerial &Serial
								, DataSerializer &DataSerializer
								, BaseType_t coreId = 1 )
								: m_Name(Name)
								, m_Serial(Serial)
								, m_DataSerializer(DataSerializer)
								, m_CoreId(coreId)
		{
		}
		virtual ~SerialPortMessageManager()
		{
			if(m_RXTaskHandle && eTaskGetState(m_RXTaskHandle) != eDeleted)
			{
				ESP_LOGI("~SerialPortMessageManager", "Deleting RX Task.");
				vTaskDelete(m_RXTaskHandle);
        		m_RXTaskHandle = NULL;
			}
			if(m_TXTaskHandle && eTaskGetState(m_TXTaskHandle) != eDeleted)
			{
				ESP_LOGI("~SerialPortMessageManager", "Deleting TX Task.");
				vTaskDelete(m_TXTaskHandle);
        		m_TXTaskHandle = NULL;
			}
		}
		virtual void SetupSerialPortMessageManager();
		virtual bool QueueMessageFromData(const String& Name, DataType_t DataType, void* Object, size_t Count);
		virtual bool QueueMessage(const String& message);
		virtual String GetName()
		{
			return m_Name;
		}
	private:
		String m_Name;
		HardwareSerial &m_Serial;
		DataSerializer &m_DataSerializer;
		BaseType_t  m_CoreId = 1;
		String m_message;
		TaskHandle_t m_RXTaskHandle;
		TaskHandle_t m_TXTaskHandle;
		QueueHandle_t m_TXQueue;
		static void StaticSerialPortMessageManager_RxTask(void *Parameters)
		{
			SerialPortMessageManager* aSerialPortMessageManager = (SerialPortMessageManager*)Parameters;
			aSerialPortMessageManager->SerialPortMessageManager_RxTask();
		}
		virtual void SerialPortMessageManager_RxTask();
		static void StaticSerialPortMessageManager_TxTask(void *Parameters)
		{
			SerialPortMessageManager* aSerialPortMessageManager = (SerialPortMessageManager*)Parameters;
			aSerialPortMessageManager->SerialPortMessageManager_TxTask();
		}
		virtual void SerialPortMessageManager_TxTask();
};