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

#include "SerialMessageManager.h"
#include <Helpers.h>


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
class DataItem: public NewRxTxValueCallerInterface<T>
			  , public NewRxTxVoidObjectCalleeInterface
			  , public DataTypeFunctions
{
	public:
		DataItem( String name
				, T &initialValuePointer
				, RxTxType_t rxTxType
				, uint16_t rate
				, size_t StackSize
				, SerialPortMessageManager &serialPortMessageManager )
			    : m_Name(name)
				, m_RxTxType(rxTxType)
				, m_Rate(rate)
				, m_StackSize(StackSize)
				, m_SerialPortMessageManager(serialPortMessageManager)
		{
			mp_Value =  new T[COUNT];
			for (int i = 0; i < COUNT; ++i)
			{
				mp_Value[i].DeepCopy(initialValuePointer[i]);
			}
			SetDataLinkEnabled(true);
		}
		
		DataItem( String name
				, T initialValue
				, RxTxType_t rxTxType
				, uint16_t rate
				, size_t StackSize
				, SerialPortMessageManager &serialPortMessageManager )
			    : m_Name(name)
				, m_RxTxType(rxTxType)
				, m_Rate(rate)
				, m_StackSize(StackSize)
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
			ESP_LOGD("DataItem& operator=(const U& value)");
			static_assert(std::is_same<T, U>::value, "Types must be the same");
			bool valueChanged = false;
			if (memcmp(mp_Value, &value, sizeof(T) * COUNT) != 0)
			{
				valueChanged = true;
				memcpy(mp_Value, &value, sizeof(T) * COUNT);
				DataItem_Try_TX_On_Change();
			}
			return *this;
		}

		// Templated conversion operator for returning a value
		template <typename U>
		operator U()
		{
			static_assert(std::is_same<T, U>::value, "Types must be the same");
			ESP_LOGD("operator U()");
			return mp_Value[0];
		}
		template <typename U>
		operator U*()
		{
			static_assert(std::is_same<T, U>::value, "Types must be the same");
			ESP_LOGD("operator U*()");
			return mp_Value;
		}
		
		operator NewRxTxValueCallerInterface<T>*()
		{
			return this;
		}
		
		operator NewRxTxVoidObjectCalleeInterface*()
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
		
		void SetNewTxValue(T* Value)
		{
			ESP_LOGD("DataItem: SetNewTxValue", "%s SetNewTxValue to: %s", m_Name.c_str(), GetDataItemValueAsString(Value, GetDataTypeFromType<T>(), COUNT));
			SetValue(Value);
		}
		
		void SetValue(T* Value)
		{
			ESP_LOGD("DataItem: SetValue", "%s SetValue to: %s", m_Name.c_str(), GetDataItemValueAsString(Value, GetDataTypeFromType<T>(), COUNT));
			assert(Value != nullptr && "Value must not be null");
			assert(mp_Value != nullptr && "mp_Value must not be null");
			assert(COUNT > 0 && COUNT <= sizeof(T) && "COUNT must be a valid index range for mp_Value");
			bool valueChanged = false;
			if (memcmp(mp_Value, Value, sizeof(T) * COUNT) != 0)
			{
				valueChanged = true;
				memcpy(mp_Value, Value, sizeof(T) * COUNT);
				ESP_LOGI("DataItem", "%s Value Changed to: %s", m_Name.c_str(), GetDataItemValueAsString(mp_Value, GetDataTypeFromType<T>(), COUNT));
				DataItem_Try_TX_On_Change();
			}
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
				if(enableTX) xTaskCreatePinnedToCore( StaticDataItem_TX, m_Name.c_str(), m_StackSize, this,  configMAX_PRIORITIES - 1,  &m_TXTaskHandle,  0 );
				if(enableRX) m_SerialPortMessageManager.RegisterForNewValueNotification(this);
				ESP_LOGD("SetDataLinkEnabled", "Data Item: \"%s\": Enabled Datalink", m_Name.c_str());
			}
			else
			{
				if(m_TXTaskHandle) vTaskDelete(m_TXTaskHandle);
				m_SerialPortMessageManager.DeRegisterForNewValueNotification(this);
				ESP_LOGD("SetDataLinkEnabled", "Data Item: \"%s\": Disabled Datalink", m_Name.c_str());
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
		size_t m_StackSize = 0;
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
		
		void DataItem_Try_TX_On_Change()
		{
			bool TXNow = false;
			switch(m_RxTxType)
			{
				case RxTxType_Tx_On_Change:
				case RxTxType_Tx_On_Change_With_Heartbeat:
					TXNow = true;
				default:
				break;
			}
			if(TXNow)
			{
				ESP_LOGI("DataItem& operator=(const U& value)", "Value Changed and TX Now");
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
			T* receivedValue = static_cast<T*>(Object);
			switch(m_RxTxType)
			{
				case RxTxType_Rx:
				case RxTxType_Rx_Echo_Value:
				{
					ESP_LOGD("DataItem: NewRXValueReceived", "%s New RX Value Received: %s", m_Name.c_str(), GetDataItemValueAsString(receivedValue, GetDataTypeFromType<T>(), COUNT));
					bool valueChanged = false;
					if (memcmp(mp_Value, receivedValue, sizeof(T) * COUNT) != 0)
					{
						valueChanged = true;
						memcpy(mp_Value, receivedValue, sizeof(T) * COUNT);
						ESP_LOGI("DataItem", "%s Value Changed to: %s", m_Name.c_str(), GetDataItemValueAsString(mp_Value, GetDataTypeFromType<T>(), COUNT));
						DataItem_Try_TX_On_Change();
					}
				}
				break;
				default:
					ESP_LOGW("DataItem: NewRXValueReceived", "%s Unhandled RX", m_Name.c_str());
				break;
			}
		}
};

#endif