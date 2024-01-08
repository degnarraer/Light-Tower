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

#ifndef SerialMessageManager_H
#define SerialMessageManager_H

#include <vector>
#include "DataSerializer.h"
#include <Helpers.h>

#define MaxQueueCount 10
#define MaxMessageLength 250

class SetupCalleeInterface
{
	public:
		SetupCalleeInterface(){}
		virtual ~SetupCalleeInterface(){}
		virtual void Setup() = 0;
};
class SetupCallerInterface
{
	public:
		SetupCallerInterface(){}
		virtual ~SetupCallerInterface(){}
		void RegisterForSetupCall(SetupCalleeInterface* NewCallee);
		void DeRegisterForSetupCall(SetupCalleeInterface* Callee);
	protected:
		void SetupAllSetupCallees();
	private:
		std::vector<SetupCalleeInterface*> m_SetupCallees = std::vector<SetupCalleeInterface*>();
};

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
		size_t GetCount(){ return m_Count;}
	private:
		size_t m_Count = 0;
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
		virtual void SetNewTxValue(T* object) = 0;
		void RegisterForNewValueNotification(NewRxTxValueCalleeInterface<T>* NewCallee)
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
		void DeRegisterForNewValueNotification(NewRxTxValueCalleeInterface<T>* Callee)
		{
			// Find the iterator pointing to the element
			auto it = std::find(m_NewValueCallees.begin(), m_NewValueCallees.end(), Callee);

			// Check if the element was found before erasing
			if (it != m_NewValueCallees.end()) {
				m_NewValueCallees.erase(it);
			}
		}
		void RegisterNamedCallback(NamedCallback_t* NamedCallback)
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
		void DeRegisterNamedCallback(NamedCallback_t* NamedCallback)
		{
			// Find the iterator pointing to the element
			auto it = std::find(m_NamedCallbacks.begin(), m_NamedCallbacks.end(), NamedCallback);

			// Check if the element was found before erasing
			if (it != m_NamedCallbacks.end()) {
				m_NamedCallbacks.erase(it);
			}
		}
	protected:
		void NotifyCallee(const String& name, T* object)
		{
			ESP_LOGD("NotifyCallee", "Notify Callees");
			for (NewRxTxValueCalleeInterface<T>* callee : m_NewValueCallees)
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
		void CallCallbacks(const String& name, T* object)
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
	private:
		std::vector<NewRxTxValueCalleeInterface<T>*> m_NewValueCallees = std::vector<NewRxTxValueCalleeInterface<T>*>();
		std::vector<NamedCallback_t*> m_NamedCallbacks = std::vector<NamedCallback_t*>();
};

class NewRxTxVoidObjectCalleeInterface
{
	public:
		NewRxTxVoidObjectCalleeInterface()
		{
			
		}
		virtual ~NewRxTxVoidObjectCalleeInterface()
		{
			
		}
		virtual bool NewRXValueReceived(void* object, size_t Count) = 0;
		virtual String GetName() = 0;
		size_t GetCount(){ return m_Count;}
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
		
		void RegisterForTXNotification(NewRxTxVoidObjectCalleeInterface* NewCallee, size_t Rate);
		void RegisterForNewValueNotification(NewRxTxVoidObjectCalleeInterface* NewCallee);
		void DeRegisterForNewValueNotification(NewRxTxVoidObjectCalleeInterface* Callee);
		void RegisterNamedCallback(NamedCallback_t* NamedCallback);
		void DeRegisterNamedCallback(NamedCallback_t* NamedCallback);
	protected:
		void NotifyCallee(const String& name, void* object);
		void CallCallbacks(const String& name, void* object);
	private:
		std::vector<NewRxTxVoidObjectCalleeInterface*> m_NewValueCallees = std::vector<NewRxTxVoidObjectCalleeInterface*>();
		std::vector<NamedCallback_t*> m_NamedCallbacks = std::vector<NamedCallback_t*>();
};

class SerialPortMessageManager: public NewRxTxVoidObjectCallerInterface
							  , public SetupCallerInterface
{
	public:
		SerialPortMessageManager( String Name
								, HardwareSerial &Serial
								, DataSerializer &DataSerializer )
								: m_Name(Name)
								, m_Serial(Serial)
								, m_DataSerializer(DataSerializer)
		{
		}
		virtual ~SerialPortMessageManager()
		{
			if(m_RXTaskHandle)
			{
				ESP_LOGI("~SerialPortMessageManager", "Deleted the RX Task.");
				vTaskDelete(m_RXTaskHandle);
			}
			if(m_TXTaskHandle)
			{
				ESP_LOGI("~SerialPortMessageManager", "Deleted the TX Task.");
				vTaskDelete(m_TXTaskHandle);
			}
		}
		void SetupSerialPortMessageManager();
		bool QueueMessageFromData(String Name, DataType_t DataType, void* Object, size_t Count);
		bool QueueMessage(String message);
	private:
		HardwareSerial &m_Serial;
		DataSerializer &m_DataSerializer;
		String m_Name = "";
		TaskHandle_t m_RXTaskHandle;
		TaskHandle_t m_TXTaskHandle;
		QueueHandle_t m_TXQueue;
		static void StaticSerialPortMessageManager_RxTask(void *Parameters)
		{
			SerialPortMessageManager* aSerialPortMessageManager = (SerialPortMessageManager*)Parameters;
			aSerialPortMessageManager->SerialPortMessageManager_RxTask();
		}
		void SerialPortMessageManager_RxTask();
		static void StaticSerialPortMessageManager_TxTask(void *Parameters)
		{
			SerialPortMessageManager* aSerialPortMessageManager = (SerialPortMessageManager*)Parameters;
			aSerialPortMessageManager->SerialPortMessageManager_TxTask();
		}
		void SerialPortMessageManager_TxTask();
};

#endif