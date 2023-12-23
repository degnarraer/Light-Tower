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
			Serial.println("Try Callee Registered");
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
				Serial.println("Callee Registered");
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
			Serial.println("Notify Callees");
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
			xTaskCreatePinnedToCore( StaticSerialPortMessageManager_RXLoop, m_Name.c_str(), 5000, this,  configMAX_PRIORITIES - 1,  &m_RXTaskHandle,  0 );
			xTaskCreatePinnedToCore( StaticSerialPortMessageManager_TXLoop, m_Name.c_str(), 5000, this,  configMAX_PRIORITIES - 1,  &m_TXTaskHandle,  0 );
			m_TXQueue = xQueueCreate(10, sizeof(String) );
			if(m_TXQueue == NULL)
			{
				ESP_LOGE("SerialPortMessageManager", "ERROR! Error creating the TX Queue.");
			}
			else
			{
				ESP_LOGI("SerialPortMessageManager", "Created the TX Queue.");
			}
		}
		void CreateAndQueueMessageFromData(String Name, DataType_t DataType, void* Object, size_t Count)
		{
			ESP_LOGD("SerialPortMessageManager", "Serializing Data for: %s Data Type: %i, Pointer: %p Count: %i ", Name.c_str(), DataType, static_cast<void*>(Object), Count);
			String message = m_DataSerializer.SerializeDataToJson(Name, DataType, Object, Count);
			ESP_LOGI("SerialPortMessageManager", "Queueing Message: %s", message.c_str());
			QueueMessage(message);
		}
		
		void QueueMessage(String message)
		{
			String *heapMessage = new String(message);
			if(NULL != m_TXQueue && NULL != heapMessage)
			{
				ESP_LOGD("SerialPortMessageManager", "Send Message: Address: %p Message: %s", static_cast<void*>(heapMessage), heapMessage->c_str());
				if(xQueueSend(m_TXQueue, &heapMessage, 0) != pdTRUE)
				{
					ESP_LOGW("SerialPortMessageManager", "WARNING! Unable to Send Message.");
				}
			}
			else
			{
				ESP_LOGE("SerialPortMessageManager", "Error! NULL Queue!");
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
						NamedObject_t NamedObject;
						m_DataSerializer.DeSerializeJsonToNamedObject(message, NamedObject);
						ESP_LOGI("SerialPortMessageManager", "Named object: %s Address: %p", NamedObject.Name.c_str(), static_cast<void*>(NamedObject.Object));
						if(NamedObject.Object)
						{
							NotifyCallee(NamedObject.Name, NamedObject.Object);
						}
						ESP_LOGI("SerialPortMessageManager", "Message RX: %s", message.c_str() );
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
						ESP_LOGD("SerialPortMessageManager", "Queue Count: %i", QueueCount);
						for(int i = 0; i < QueueCount; ++i)
						{
							String *pmessage;
							if ( xQueueReceive(m_TXQueue, &pmessage, 0) == pdTRUE )
							{
								if (pmessage != nullptr)
								{
									ESP_LOGD("SerialPortMessageManager", "Data TX: Address: %p Message: %s", static_cast<void*>(pmessage), pmessage->c_str());
									m_Serial.println(pmessage->c_str());
									free(pmessage);
								}
								else
								{
									ESP_LOGE("SerialPortMessageManager", "ERROR! Unable to Send Message.");
								}
							}
							else
							{
								ESP_LOGE("SerialPortMessageManager", "ERROR! Unable to Send Message.");
							}
						}
					}
				}
				else
				{
					ESP_LOGE("SerialPortMessageManager", "ERROR! NULL TX Queue.");
				}
			}
		}
};

enum RXTXType_t
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
		DataItem( String name, T &initialValuePointer, RXTXType_t rxTxType, uint16_t rate, SerialPortMessageManager &serialPortMessageManager )
			    : m_Name(name)
				, m_RxTxType(rxTxType)
				, m_Rate(rate)
				, m_SerialPortMessageManager(serialPortMessageManager)
		{
			mp_Value = (T*)malloc(sizeof(T)*COUNT);
			memcpy(mp_Value, initialValuePointer, sizeof(T)*COUNT);
			SetDataLinkEnabled(true);
		}
		DataItem( String name, T initialValue, RXTXType_t rxTxType, uint16_t rate, SerialPortMessageManager &serialPortMessageManager )
			    : m_Name(name)
				, m_RxTxType(rxTxType)
				, m_Rate(rate)
				, m_SerialPortMessageManager(serialPortMessageManager)
		{
			mp_Value = (T*)malloc(sizeof(T)*COUNT);
			memcpy(mp_Value, &initialValue, sizeof(T)*COUNT);
			SetDataLinkEnabled(true);
		}
		virtual ~DataItem()
		{
			free(mp_Value);
			if(m_TXTaskHandle) vTaskDelete(m_TXTaskHandle);
		}
		
		// Templated conversion operator for assignment from a value
		template <typename U>
		DataItem& operator=(const U& value)
		{
			static_assert(std::is_same<T, U>::value, "Types must be the same");
			memcpy(mp_Value, &value, sizeof(T) * COUNT);
			return *this;
		}

		// Templated conversion operator for returning a value
		template <typename U>
		operator U()
		{
			static_assert(std::is_same<T, U>::value, "Types must be the same");
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
			return 1;
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
						enableTX = true;
					break;
					case RxTxType_Rx:
						enableRX = true;
					break;
					default:
					break;
				}
				if(enableTX) xTaskCreatePinnedToCore( StaticDataItem_TX, m_Name.c_str(), 5000, this,  configMAX_PRIORITIES - 1,  &m_TXTaskHandle,  0 );
				if(enableRX) m_SerialPortMessageManager.RegisterForNewValueNotification(this);
				ESP_LOGI("DataItem", "Data Item: %s: Enabled Datalink", m_Name.c_str());
			}
			else
			{
				if(m_TXTaskHandle) vTaskDelete(m_TXTaskHandle);
				m_SerialPortMessageManager.DeRegisterForNewValueNotification(this);
				ESP_LOGI("DataItem", "Data Item: %s: Disabled Datalink", m_Name.c_str());
			}
		}
		void SetValue(T value)
		{
			//m_Value = value;
			if(m_RxTxType == RxTxType_Tx_On_Update)
			{
				//DO SOMETHING
			}
		}
	private:
		String m_Name = "";
		T *mp_Value;
		RXTXType_t m_RxTxType = RxTxType_Rx;
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
			if(m_RxTxType == RxTxType_Tx_On_Update)
			{
				m_Rate = 5000;
			}
			const TickType_t xFrequency = m_Rate;
			TickType_t xLastWakeTime = xTaskGetTickCount();
			while(true)
			{
				vTaskDelayUntil( &xLastWakeTime, xFrequency );
				ESP_LOGD("DataItem", "Data Item: %s: Creating and Queueing Message From Data", m_Name.c_str());
				m_SerialPortMessageManager.CreateAndQueueMessageFromData(m_Name, GetDataTypeFromType<T>(), mp_Value, COUNT);
			}
		}
		void NewRXValueReceived(void* Object)
		{
			ESP_LOGI("DataItem", "Data Item: %s: New Value Received.", m_Name.c_str());
			T* receivedValue = static_cast<T*>(Object);
			// Copy the received data to mp_Value
			memcpy(mp_Value, receivedValue, sizeof(T) * COUNT);
		}
};

#endif