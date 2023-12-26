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
#define MaxMessageLength 1000

class NewRXValueCallee
{
	public:
		NewRXValueCallee()
		{
			
		}
		virtual ~NewRXValueCallee()
		{
			
		}
		virtual void NewRXValueReceived(void* object) = 0;
		virtual String GetName() = 0;
};

class NewRXValueCaller
{
	public:
		NewRXValueCaller()
		{
			
		}
		virtual ~NewRXValueCaller()
		{
			
		}
		
		void RegisterForNewValueNotification(NewRXValueCallee* NewCallee);
		void DeRegisterForNewValueNotification(NewRXValueCallee* Callee);
		void RegisterNamedCallback(NamedCallback_t* NamedCallback);
		void DeRegisterNamedCallback(NamedCallback_t* NamedCallback);
	protected:
		void NotifyCallee(const String& name, void* object);
		void CallCallbacks(const String& name, void* object);
	private:
		std::vector<NewRXValueCallee*> m_NewValueCallees = std::vector<NewRXValueCallee*>();
		std::vector<NamedCallback_t*> m_NamedCallbacks = std::vector<NamedCallback_t*>();
};

template <typename T>
class GetSerializeInfoCallBack
{
	public:
		GetSerializeInfoCallBack()
		{
			
		}
		virtual ~GetSerializeInfoCallBack()
		{
			
		}
		virtual String GetName() = 0;
		virtual T *GetValue() = 0;
		virtual size_t GetCount() = 0;
};

class SerialPortMessageManager: public NewRXValueCaller
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
		void QueueMessageFromData(String Name, DataType_t DataType, void* Object, size_t Count);
		void QueueMessage(String message);
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