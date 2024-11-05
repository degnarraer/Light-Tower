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
		virtual String GetName() const = 0;
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
		virtual String GetName() const = 0;
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
		virtual String GetName() const = 0;
	protected:
		virtual void Call_Named_Object_Callback(const String& name, void* object, const size_t changeCount);
	private:
		std::vector<Named_Object_Callee_Interface*> m_NewValueCallees = std::vector<Named_Object_Callee_Interface*>();
		std::vector<NamedCallback_t*> m_NamedCallbacks = std::vector<NamedCallback_t*>();
};

class SerialPortMessageManager: public Named_Object_Caller_Interface
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
		virtual void Setup();
		virtual bool QueueMessageFromDataType(const String& Name, DataType_t DataType, void* Object, size_t Count, size_t ChangeCount);
		virtual bool QueueMessage(const String& message);
		String GetName() const 
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