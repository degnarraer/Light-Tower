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
class NewRxValue_Caller_Interface;

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
		virtual bool NewRxValueReceived(const NewRxValue_Caller_Interface<T>* sender, const T* values, size_t count) = 0;
		virtual String GetName() const = 0;
		virtual size_t GetCount(){ return m_Count;}
	private:
		size_t m_Count = 0;
};

template <typename T>
class NamedCallback_Caller_Interface
{
	public:
		NamedCallback_Caller_Interface()
		{
			
		}
		virtual ~NamedCallback_Caller_Interface()
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
		virtual void CallNamedCallback(const String& name, T* object)
		{
			ESP_LOGD("NotifyCallee", "CallCallbacks");
			for (NamedCallback_t* namedCallback : m_NamedCallbacks)
			{
				ESP_LOGD("NotifyCallee", "Calling Callback %s", namedCallback->Name.c_str());
				void (*aCallback)(const String&, void*, void*);
				aCallback = namedCallback->Callback;
				void* arg = namedCallback->Arg;
				aCallback(namedCallback->Name, object, arg);
			}
		}
	private:
		std::vector<NamedCallback_t*> m_NamedCallbacks = std::vector<NamedCallback_t*>();
};

template <typename T>
class NewRxValue_Caller_Interface
{
	public:
		NewRxValue_Caller_Interface()
		{
			
		}
		virtual ~NewRxValue_Caller_Interface()
		{
			
		}
		virtual void RegisterForNewRxValueNotification(NewRxTxValueCalleeInterface<T>* NewCallee)
		{
			ESP_LOGI("RegisterForNewRxValueNotification", "Try Registering Callee");
			bool IsFound = false;
			for (NewRxTxValueCalleeInterface<T>* callee : m_NewValueCallees)
			{
				if(NewCallee == callee)
				{
					ESP_LOGE("RegisterForNewRxValueNotification", "ERROR! A callee with this name already exists.");
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
		virtual void DeRegisterForNewRxValueNotification(NewRxTxValueCalleeInterface<T>* Callee)
		{
			// Find the iterator pointing to the element
			auto it = std::find(m_NewValueCallees.begin(), m_NewValueCallees.end(), Callee);

			// Check if the element was found before erasing
			if (it != m_NewValueCallees.end()) {
				m_NewValueCallees.erase(it);
			}
		}
		
	protected:
		virtual void NotifyCallee(const String& name, T* values)
		{
			ESP_LOGD("NotifyCallee", "Notify Callees");
			for (NewRxTxValueCalleeInterface<T>* callee : m_NewValueCallees)
			{
				if (callee) 
				{
					if (callee->GetName().equals(name))
					{
						callee->NewRxValueReceived(this, values, callee->GetCount());
						break;
					}
				}
			}
		}
	private:
		std::vector<NewRxTxValueCalleeInterface<T>*> m_NewValueCallees = std::vector<NewRxTxValueCalleeInterface<T>*>();
};
class NewRxTxVoidObjectCallerInterface;
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
		virtual bool NewRxValueReceived(const NewRxTxVoidObjectCallerInterface* sender, const void* values, size_t count) = 0;
		virtual String GetName() const = 0;
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
		virtual void RegisterForNewRxValueNotification(NewRxTxVoidObjectCalleeInterface* newCallee);
		virtual void DeRegisterForNewRxValueNotification(NewRxTxVoidObjectCalleeInterface* callee);
		virtual void RegisterNamedCallback(NamedCallback_t* namedCallback);
		virtual void DeRegisterNamedCallback(NamedCallback_t* namedCallback);
	protected:
		virtual void NotifyCallee(const String& name, void* object);
		virtual void CallNamedCallback(const String& name, void* object);
	private:
		std::vector<NewRxTxVoidObjectCalleeInterface*> m_NewValueCallees = std::vector<NewRxTxVoidObjectCalleeInterface*>();
		std::vector<NamedCallback_t*> m_NamedCallbacks = std::vector<NamedCallback_t*>();
};

class SerialPortMessageManager: public NewRxTxVoidObjectCallerInterface
{
	public:
		SerialPortMessageManager(){}
		SerialPortMessageManager( const String& name
								, HardwareSerial *serial
								, DataSerializer *dataSerializer
								, BaseType_t coreId = 1 )
								: m_Name(name)
								, mp_Serial(serial)
								, mp_DataSerializer(dataSerializer)
								, m_CoreId(coreId)
		{
		}
		virtual ~SerialPortMessageManager()
		{
			ESP_LOGD("~SerialPortMessageManager", "Deleting SerialPortMessageManager");
			if(m_RXTaskHandle)
			{
				ESP_LOGD("~SerialPortMessageManager", "RX Task Exists.");
				if(eTaskGetState(m_RXTaskHandle) != eDeleted)
				{
					ESP_LOGD("~SerialPortMessageManager", "Deleting RX Task.");
					vTaskDelete(m_RXTaskHandle);
				}
			}
			if(m_TXTaskHandle)
			{
				ESP_LOGD("~SerialPortMessageManager", "TX Task Exists.");
				if(eTaskGetState(m_TXTaskHandle) != eDeleted)
				{
					ESP_LOGD("~SerialPortMessageManager", "Deleting TX Task.");
					vTaskDelete(m_TXTaskHandle);
				}
			}
			if(m_TXQueue)
			{
				ESP_LOGD("~SerialPortMessageManager", "Deleting Queue");
				vQueueDelete(m_TXQueue);
			}
			ESP_LOGD("~SerialPortMessageManager", "SerialPortMessageManager Deleted");
		}
		virtual void SetupSerialPortMessageManager();
		virtual bool QueueMessageFromData(const String& Name, DataType_t DataType, void* Object, size_t Count);
		virtual bool QueueMessage(const String& message);
		virtual String GetName() const 
		{
			return m_Name;
		}
	private:
		String m_Name;
		HardwareSerial *mp_Serial = nullptr;
		DataSerializer *mp_DataSerializer = nullptr;
		BaseType_t  m_CoreId = 1;
		String m_message;
		TaskHandle_t m_RXTaskHandle = nullptr;
		TaskHandle_t m_TXTaskHandle = nullptr;
		QueueHandle_t m_TXQueue = nullptr;
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