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
#define MaxMessageLength 1000

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
		static void StaticSerialPortMessageManager_RXLoop(void *Parameters);
		void SerialPortMessageManager_RXLoop();
		static void StaticSerialPortMessageManager_TXLoop(void *Parameters);
		void SerialPortMessageManager_TXLoop();
};

enum RxTxType_t
{
	RxTxType_Tx_Periodic = 0,
	RxTxType_Tx_On_Change,
	RxTxType_Tx_On_Change_With_Heartbeat,
	RxTxType_Rx,
	RxTxType_Rx_Echo_Value,
	RxTxType_Count
};

template <typename T, int COUNT>
class DataItem: public NewRXValueCallee
			  , public NewRXValueCaller
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
			mp_Value =  new T[COUNT];
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
			ESP_LOGI("DataItem& operator=(const U& value)");
			static_assert(std::is_same<T, U>::value, "Types must be the same");
			bool valueChanged = false;
			if (*mp_Value != value)
			{
				valueChanged = true;
			}
			memcpy(mp_Value, &value, sizeof(T) * COUNT);
			bool TXNow = false;
			switch(m_RxTxType)
			{
				case RxTxType_Tx_On_Change:
				case RxTxType_Tx_On_Change_With_Heartbeat:
					TXNow = true;
				default:
				break;
			}
			if(valueChanged && TXNow)
			{
				ESP_LOGI("DataItem& operator=(const U& value)", "Value Changed");
				DataItem_TX_Now();
			}
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
					case RxTxType_Tx_On_Change_With_Heartbeat:
						enableTX = true;
					break;
					case RxTxType_Rx:
					case RxTxType_Rx_Echo_Value:
						enableRX = true;
					break;
					default:
					break;
				}
				if(enableTX) xTaskCreatePinnedToCore( StaticDataItem_TX, m_Name.c_str(), 5000, this,  configMAX_PRIORITIES - 1,  &m_TXTaskHandle,  0 );
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
		size_t m_Count = 0;
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
				DataItem_TX_Now();
			}
		}
		void DataItem_TX_Now()
		{
			ESP_LOGD("DataItem_TX", "Data Item: %s: Creating and Queueing Message From Data", m_Name.c_str());
			m_SerialPortMessageManager.QueueMessageFromData(m_Name, GetDataTypeFromType<T>(), mp_Value, COUNT);
		}
		
		void NewRXValueReceived(void* Object)
		{
			bool Should_RX = false;
			bool Should_Echo_Value = false;
			switch(m_RxTxType)
				{
					case RxTxType_Rx:
						Should_RX = true;
					break;
					case RxTxType_Rx_Echo_Value:
						Should_RX = true;
						Should_Echo_Value = true;
					break;
					default:
					break;
				}
			if(Should_RX)
			{
				ESP_LOGD("NewRXValueReceived", "Data Item \"%s\": New Value Received.", m_Name.c_str());
				T* receivedValue = static_cast<T*>(Object);
				memcpy(mp_Value, receivedValue, sizeof(T) * COUNT);
			}
			if(Should_Echo_Value)
			{
				DataItem_TX_Now();
			}
		}
};

#endif