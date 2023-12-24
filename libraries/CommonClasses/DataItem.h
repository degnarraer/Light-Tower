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

#ifndef DataItem_H
#define DataItem_H
#define MaxQueueCount 10
#define MaxStringLength 100

#include <vector>
#include "DataSerializer.h"
#include <Helpers.h>


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
		
		void RegisterForNewValueNotification(NewRXValueCallee* NewCallee)
		{
			ESP_LOGI("RegisterForNewValueNotification", "Try Registering Callee");
			bool IsFound = false;
			for (NewRXValueCallee* callee : m_NewValueCallees)
			{
				if(NewCallee == callee)
				{
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
		void DeRegisterForNewValueNotification(NewRXValueCallee* Callee)
		{
			// Find the iterator pointing to the element
			auto it = std::find(m_NewValueCallees.begin(), m_NewValueCallees.end(), Callee);

			// Check if the element was found before erasing
			if (it != m_NewValueCallees.end()) {
				m_NewValueCallees.erase(it);
			}
		}
	protected:
		void NotifyCallee(const String& name, void* object)
		{
			ESP_LOGI("NotifyCallee", "Notify Callees");
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
	private:
		std::vector<NewRXValueCallee*> m_NewValueCallees = std::vector<NewRXValueCallee*>();
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
								, HardwareSerial &serial
								, DataSerializer &dataSerializer )
								: m_Name(Name)
								, m_Serial(serial)
								, m_DataSerializer(dataSerializer)
		{
		}
		virtual ~SerialPortMessageManager()
		{
			if(m_RXTaskHandle) vTaskDelete(m_RXTaskHandle);
			if(m_TXTaskHandle) vTaskDelete(m_TXTaskHandle);
		}
		void SetupSerialPortMessageManager()
		{
			xTaskCreatePinnedToCore( StaticSerialPortMessageManager_RXLoop, m_Name.c_str(), 10000, this,  configMAX_PRIORITIES - 1,  &m_RXTaskHandle,  0 );
			xTaskCreatePinnedToCore( StaticSerialPortMessageManager_TXLoop, m_Name.c_str(), 10000, this,  configMAX_PRIORITIES - 1,  &m_TXTaskHandle,  0 );
			m_TXQueue = xQueueCreate(MaxQueueCount, sizeof(char) * MaxStringLength );
			if(NULL == m_TXQueue)
			{
				ESP_LOGE("SetupSerialPortMessageManager", "ERROR! Error creating the TX Queue.");
			}
			else
			{
				ESP_LOGI("SetupSerialPortMessageManager", "Created the TX Queue.");
			}
		}
		void QueueMessageFromData(String Name, DataType_t DataType, void* Object, size_t Count)
		{
			
			if(nullptr == Object || 0 == Name.length() || 0 == Count)
			{
				ESP_LOGE("QueueMessageFromData", "Error Invalid Data!");
			}
			else
			{
				ESP_LOGI("QueueMessageFromData", "Serializing Data for: \"%s\" Data Type: \"%i\", Pointer: \"%p\" Count: \"%i\" ", Name.c_str(), DataType, static_cast<void*>(Object), Count);
				String message = m_DataSerializer.SerializeDataToJson(Name, DataType, Object, Count);
				if(message.length() > 0 && message.length() <= MaxStringLength)
				{
					QueueMessage(message);
				}
				else
				{
					ESP_LOGE("QueueMessageFromData", "Error! Invalide String Length!");
				}
			}
			
		}
		
		void QueueMessage(String message)
		{
			if(nullptr != m_TXQueue)
			{
				ESP_LOGI("QueueMessage", "Queue Message: \"%s\"", message.c_str());
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
	private:
		HardwareSerial &m_Serial;
		DataSerializer &m_DataSerializer;
		String m_Name = "";
		TaskHandle_t m_RXTaskHandle;
		TaskHandle_t m_TXTaskHandle;
		QueueHandle_t m_TXQueue;
		static void StaticSerialPortMessageManager_RXLoop(void *Parameters)
		{
			SerialPortMessageManager* aSerialPortMessageManager = (SerialPortMessageManager*)Parameters;
			aSerialPortMessageManager->SerialPortMessageManager_RXLoop();
		}
		void SerialPortMessageManager_RXLoop()
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
						ESP_LOGI("SerialPortMessageManager", "Message RX: \"%s\"", message.c_str());
						NamedObject_t NamedObject;
						m_DataSerializer.DeSerializeJsonToNamedObject(message, NamedObject);
						if(NamedObject.Object)
						{
							ESP_LOGI("SerialPortMessageManager", "DeSerialized Named object: \"%s\" Address: \"%p\"", NamedObject.Name.c_str(), static_cast<void*>(NamedObject.Object));
							NotifyCallee(NamedObject.Name, NamedObject.Object);
						}
						else
						{
							ESP_LOGI("SerialPortMessageManager", "DeSerialized Named object failed");
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
		
		static void StaticSerialPortMessageManager_TXLoop(void *Parameters)
		{
			SerialPortMessageManager* aSerialPortMessageManager = (SerialPortMessageManager*)Parameters;
			aSerialPortMessageManager->SerialPortMessageManager_TXLoop();
		}
		void SerialPortMessageManager_TXLoop()
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
							char message[MaxStringLength];
							if ( xQueueReceive(m_TXQueue, message, 0) == pdTRUE )
							{
								ESP_LOGD("SerialPortMessageManager_TXLoop", "Data TX: Address: \"%p\" Message: \"%s\"", static_cast<void*>(message), String(message).c_str());
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
};

enum RxTxType_t
{
	RxTxType_Tx_Periodic = 0,
	RxTxType_Tx_On_Update,
	RxTxType_Rx,
	RxTxType_Count
};

template <typename T, int COUNT>
class DataItem: public NewRXValueCallee
			  , public GetSerializeInfoCallBack<T>
			  , public CommonUtils
{
	public:
		DataItem( String name, T &initialValuePointer, RxTxType_t rxTxType, uint16_t rate, SerialPortMessageManager &serialPortMessageManager )
			    : m_Name(name)
				, m_RxTxType(rxTxType)
				, m_Rate(rate)
				, m_SerialPortMessageManager(serialPortMessageManager)
		{
			mp_Value = new T[COUNT];
			for (int i = 0; i < COUNT; ++i)
			{
				mp_Value[i].DeepCopy(initialValuePointer[i]);
			}
			SetDataLinkEnabled(true);
		}
		
		DataItem( String name, T initialValue, RxTxType_t rxTxType, uint16_t rate, SerialPortMessageManager &serialPortMessageManager )
			    : m_Name(name)
				, m_RxTxType(rxTxType)
				, m_Rate(rate)
				, m_SerialPortMessageManager(serialPortMessageManager)
		{
			mp_Value = new T[COUNT];
			for (int i = 0; i < COUNT; ++i)
			{
				mp_Value[i] = T(initialValue);
			}
			SetDataLinkEnabled(true);
		}
		
		virtual ~DataItem()
		{
			delete[] mp_Value;
			if(m_TXTaskHandle) vTaskDelete(m_TXTaskHandle);
		}
		
		// Templated conversion operator for assignment from a value
		template <typename U>
		DataItem& operator=(const U& value)
		{
			static_assert(std::is_same<T, U>::value, "Types must be the same");
			ESP_LOGI("DataItem& operator=(const U& value)");
			memcpy(mp_Value, &value, sizeof(T) * COUNT);
			return *this;
		}

		// Templated conversion operator for returning a value
		template <typename U>
		operator U()
		{
			static_assert(std::is_same<T, U>::value, "Types must be the same");
			ESP_LOGI("operator U()");
			return mp_Value[0];
		}
		
		operator NewRXValueCallee*()
		{
			return this;
		}
		
		String GetName()
		{
			return m_Name;
		}
		
		T *GetValue()
		{
			return mp_Value;
		}
		
		size_t GetCount()
		{
			return COUNT;
		}
		
		void SetDataLinkEnabled(bool enable)
		{
			m_DataLinkEnabled = enable;
			if(m_DataLinkEnabled)
			{
				bool enableTX = false;
				bool enableRX = false;
				switch(m_RxTxType)
				{
					case RxTxType_Tx_Periodic:
						enableTX = true;
					break;
					case RxTxType_Tx_On_Update:
						m_Rate = 5000;
						enableTX = true;
					break;
					case RxTxType_Rx:
						enableRX = true;
					break;
					default:
					break;
				}
				if(enableTX) xTaskCreatePinnedToCore( StaticDataItem_TX, m_Name.c_str(), 10000, this,  configMAX_PRIORITIES - 1,  &m_TXTaskHandle,  0 );
				if(enableRX) m_SerialPortMessageManager.RegisterForNewValueNotification(this);
				ESP_LOGI("SetDataLinkEnabled", "Data Item: \"%s\": Enabled Datalink", m_Name.c_str());
			}
			else
			{
				if(m_TXTaskHandle) vTaskDelete(m_TXTaskHandle);
				m_SerialPortMessageManager.DeRegisterForNewValueNotification(this);
				ESP_LOGI("SetDataLinkEnabled", "Data Item: \"%s\": Disabled Datalink", m_Name.c_str());
			}
		}
	private:
		String m_Name = "";
		T *mp_Value;
		RxTxType_t m_RxTxType = RxTxType_Rx;
		bool m_DataLinkEnabled = true;
		uint16_t m_Rate = 0;
		size_t m_Count = 1;
		SerialPortMessageManager &m_SerialPortMessageManager;
		TaskHandle_t m_TXTaskHandle;
		static void StaticDataItem_TX(void *Parameters)
		{
			DataItem* aDataItem = (DataItem*)Parameters;
			aDataItem->DataItem_TX();
		}
		
		void DataItem_TX()
		{
			const TickType_t xFrequency = m_Rate;
			TickType_t xLastWakeTime = xTaskGetTickCount();
			while(true)
			{
				vTaskDelayUntil( &xLastWakeTime, xFrequency );
				ESP_LOGD("DataItem_TX", "Data Item: %s: Creating and Queueing Message From Data", m_Name.c_str());
				m_SerialPortMessageManager.QueueMessageFromData(m_Name, GetDataTypeFromType<T>(), mp_Value, COUNT);
			}
		}
		
		void NewRXValueReceived(void* Object)
		{
			if(Object)
			{
				ESP_LOGI("NewRXValueReceived", "Data Item \"%s\": New Value Received.", m_Name.c_str());
				T* receivedValue = static_cast<T*>(Object);
				memcpy(mp_Value, receivedValue, sizeof(T) * COUNT);
			}
		}
};

#endif