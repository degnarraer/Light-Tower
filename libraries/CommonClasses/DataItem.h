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
#include <Preferences.h>

enum RxTxType_t
{
	RxTxType_Tx_Periodic,
	RxTxType_Tx_On_Change,
	RxTxType_Tx_On_Change_With_Heartbeat,
	RxTxType_Rx_Only,
	RxTxType_Rx_Echo_Value,
	RxTxType_Count
};

enum UpdateStoreType_t
{
	UpdateStoreType_On_Tx,
	UpdateStoreType_On_Rx,
	UpdateStoreType_Count
};

template <typename T, int COUNT>
class DataItem: public NewRxTxValueCallerInterface<T>
			  , public NewRxTxVoidObjectCalleeInterface
			  , public SetupCalleeInterface
			  , public DataTypeFunctions
{
	public:
		DataItem( const String name
				, const T *initialValuePointer
				, const RxTxType_t rxTxType
				, const UpdateStoreType_t updateStoreType
				, const uint16_t rate
				, Preferences *preferences
				, SerialPortMessageManager &serialPortMessageManager )
			    : m_Name(name)
				, m_RxTxType(rxTxType)
				, m_UpdateStoreType(updateStoreType)
				, m_Rate(rate)
				, m_Preferences(preferences)
				, m_SerialPortMessageManager(serialPortMessageManager)
		{
			mp_Value =  new T[COUNT];
			for (int i = 0; i < COUNT; ++i)
			{
				mp_Value[i] = initialValuePointer[i];
				mp_RxValue[i] = initialValuePointer[i];
				mp_TxValue[i] = initialValuePointer[i];
			}
			CreateTimer();
			SetDataLinkEnabled(true);
		}
		
		DataItem( const String name
				, const T initialValue
				, const RxTxType_t rxTxType
				, const UpdateStoreType_t updateStoreType
				, const uint16_t rate
				, Preferences *preferences
				, SerialPortMessageManager &serialPortMessageManager )
			    : m_Name(name)
				, m_RxTxType(rxTxType)
				, m_UpdateStoreType(updateStoreType)
				, m_Rate(rate)
				, m_Preferences(preferences)
				, m_SerialPortMessageManager(serialPortMessageManager)
		{
			mp_Value = new T[COUNT];
			mp_RxValue = new T[COUNT];
			mp_TxValue = new T[COUNT];
			for (int i = 0; i < COUNT; ++i)
			{
				mp_Value[i] = T(initialValue);
				mp_RxValue[i] = T(initialValue);
				mp_TxValue[i] = T(initialValue);
			}
			CreateTimer();
			SetDataLinkEnabled(true);
			m_SerialPortMessageManager.RegisterForSetupCall(this);
		}
		
		virtual ~DataItem()
		{
			delete[] mp_Value;
			delete[] mp_RxValue;
			delete[] mp_TxValue;
			esp_timer_stop(m_TxTimer);
			esp_timer_delete(m_TxTimer);
			m_SerialPortMessageManager.DeRegisterForSetupCall(this);
		}
		void Setup()
		{
			InitializeNVM();
			LoadFromNVM();
		}
		void InitializeNVM()
		{
			if(m_Preferences)
			{
				if(1 == COUNT)
				{
					if (false == m_Preferences->isKey(m_Name.c_str()))
					{
						if (std::is_same<T, bool>::value)
						{
							m_Preferences->putBool(m_Name.c_str(), mp_Value[0]); // Assuming bool is stored as an integer
							ESP_LOGI("Dataitem: InitializeNVM", "\"%s\" Initialized BOOL NVM", m_Name.c_str());
						}
						else if (std::is_integral<T>::value)
						{
							m_Preferences->putInt(m_Name.c_str(), mp_Value[0]);
							ESP_LOGI("Dataitem: InitializeNVM", "\"%s\" Initialized INTEGRAL NVM", m_Name.c_str());
						}
						else if (std::is_floating_point<T>::value)
						{
							m_Preferences->putFloat(m_Name.c_str(), mp_Value[0]);
							ESP_LOGI("Dataitem: InitializeNVM", "\"%s\" Initialized FLOAT NVM", m_Name.c_str());
						}
						else if (std::is_same<T, double>::value)
						{
							m_Preferences->putDouble(m_Name.c_str(), mp_Value[0]);
							ESP_LOGI("Dataitem: InitializeNVM", "\"%s\" Initialized DOUBLE NVM", m_Name.c_str());
						}
						else
						{
							ESP_LOGE("Dataitem: InitializeNVM", "\"%s\" data type is not supported for NVM", m_Name.c_str());
						}
					}
					else
					{
						ESP_LOGI("Dataitem: InitializeNVM", "\"%s\" NVM Found", m_Name.c_str());
					}
				}
				else
				{
					ESP_LOGE("Dataitem: InitializeNVM", "Cannot use preferences for items with non 1 COUNT size");
				}
			}
		}
		void LoadFromNVM()
		{
			
		}
		void CreateTimer()
		{
			esp_timer_create_args_t timerArgs;
			timerArgs.callback = &StaticDataItem_TX_Now;
			timerArgs.arg = this;
			timerArgs.name = "Tx_Timer";

			// Create the timer
			esp_timer_create(&timerArgs, &m_TxTimer);
		}
		
		String GetName()
		{
			return m_Name;
		}
		
		T* GetValuePointer()
		{
			return mp_Value;
		}
		
		T GetValue()
		{
			static_assert(COUNT == 1, "Count should be 1 to do this");
			return *mp_Value;
		}
		
		String GetValueAsString()
		{
			return GetValueAsStringForDataType(mp_Value, GetDataTypeFromTemplateType<T>(), COUNT);
		}
		
		void SetNewTxValue(T* Value)
		{
			ESP_LOGD("DataItem: SetNewTxValue", "\"%s\" SetNewTxValue to: \"%s\"", m_Name.c_str(), GetValueAsStringForDataType(Value, GetDataTypeFromTemplateType<T>(), COUNT));
			SetValue(Value);
		}
		
		void SetValue(T Value)
		{
			assert(mp_TxValue != nullptr && "mp_Value must not be null");
			static_assert(COUNT == 1, "Count should be 1 to do this");
			ESP_LOGD( "DataItem: SetValue"
					, "\"%s\" Set Value: \"%s\""
					, m_Name.c_str()
					, GetValueAsStringForDataType(&Value, GetDataTypeFromTemplateType<T>(), COUNT));
			bool ValueChanged = (*mp_Value != Value);
			if(ValueChanged)
			{
				*mp_TxValue = Value;
				DataItem_Try_TX_On_Change();
			}
		}
		
		void SetValue(T *Value)
		{
			assert(Value != nullptr && "Value must not be null");
			assert(mp_Value != nullptr && "mp_Value must not be null");
			assert(COUNT > 0 && "COUNT must be a valid index range for mp_Value");
			ESP_LOGD( "DataItem: SetValue"
					, "\"%s\" Set Value: \"%s\""
					, m_Name.c_str()
					, GetValueAsStringForDataType(Value, GetDataTypeFromTemplateType<T>(), COUNT));
			bool ValueChanged = (memcmp(mp_Value, Value, sizeof(T) * COUNT) != 0);
			if(ValueChanged)
			{
				memcpy(mp_TxValue, Value, sizeof(T) * COUNT);
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
				bool enablePeriodicTX = false;
				bool enablePeriodicRX = false;
				switch(m_RxTxType)
				{
					case RxTxType_Tx_Periodic:
					case RxTxType_Tx_On_Change_With_Heartbeat:
						enablePeriodicTX = true;
						enablePeriodicRX = true;
						break;
					case RxTxType_Tx_On_Change:
					case RxTxType_Rx_Only:
					case RxTxType_Rx_Echo_Value:
						enablePeriodicRX = true;
						break;
					default:
					break;
				}
				if(enablePeriodicTX)
				{
					esp_timer_start_periodic(m_TxTimer, m_Rate * 1000);
					ESP_LOGI("DataItem: SetDataLinkEnabled", "Data Item: \"%s\": Enabled Periodic TX", m_Name.c_str());
				}
				else
				{
					esp_timer_stop(m_TxTimer);
					ESP_LOGI("DataItem: SetDataLinkEnabled", "Data Item: \"%s\": Disabled Periodic TX", m_Name.c_str());
				}
				if(enablePeriodicRX)
				{
					m_SerialPortMessageManager.RegisterForNewValueNotification(this);
					ESP_LOGI("DataItem: SetDataLinkEnabled", "Data Item: \"%s\": Enabled Periodic RX", m_Name.c_str());
				}
				else
				{
					m_SerialPortMessageManager.DeRegisterForNewValueNotification(this);
					ESP_LOGI("DataItem: SetDataLinkEnabled", "Data Item: \"%s\": Disabled Periodic RX", m_Name.c_str());
				}
			}
			else
			{
				esp_timer_stop(m_TxTimer);
				m_SerialPortMessageManager.DeRegisterForNewValueNotification(this);
				ESP_LOGD("SetDataLinkEnabled", "Data Item: \"%s\": Disabled Datalink", m_Name.c_str());
			}
		}
	private:
		Preferences *m_Preferences = nullptr;
		const String m_Name;
		const RxTxType_t m_RxTxType;
		const UpdateStoreType_t m_UpdateStoreType;
		const uint16_t m_Rate;
		T *mp_Value;
		T *mp_RxValue;
		T *mp_TxValue;
		bool m_DataLinkEnabled = true;
		SerialPortMessageManager &m_SerialPortMessageManager;
		esp_timer_handle_t m_TxTimer;
		
		void DataItem_Try_TX_On_Change()
		{
			ESP_LOGI("DataItem& DataItem_Try_TX_On_Change", "Data Item: \"%s\": Try TX On Change", m_Name.c_str());
			bool ValueChanged = (memcmp(mp_TxValue, mp_Value, sizeof(T) * COUNT) != 0);
			if(ValueChanged && (m_RxTxType == RxTxType_Tx_On_Change || m_RxTxType == RxTxType_Tx_On_Change_With_Heartbeat))
			{
				DataItem_TX_Now();
			}
		}
		static void StaticDataItem_TX_Now(void *arg)
		{
			DataItem *aDataItem = static_cast<DataItem*>(arg);
			if(aDataItem)
			{
				aDataItem->DataItem_TX_Now();
			}
		}
		void DataItem_TX_Now()
		{
			if(m_SerialPortMessageManager.QueueMessageFromData(m_Name, GetDataTypeFromTemplateType<T>(), mp_TxValue, COUNT))
			{
				bool TxValueChanged = (memcmp(mp_Value, mp_TxValue, sizeof(T) * COUNT) != 0);
				if(m_UpdateStoreType == UpdateStoreType_On_Tx)
				{
					if(TxValueChanged) memcpy(mp_Value, mp_TxValue, sizeof(T) * COUNT);
				}
				ESP_LOGD("DataItem: DataItem_TX_Now", "Data Item: \"%s\": TX Now: \"%s\"", m_Name.c_str(), GetValueAsStringForDataType(mp_Value, GetDataTypeFromTemplateType<T>(), COUNT).c_str());
			}
			else
			{
				ESP_LOGE("DataItem: DataItem_TX_Now", "Data Item: \"%s\": Unable to Tx Message", m_Name.c_str());
			}
			
		}
		
		void NewRXValueReceived(void* Object)
		{	
			T* receivedValue = static_cast<T*>(Object);
			bool ValueChanged = (memcmp(mp_Value, receivedValue, sizeof(T) * COUNT) != 0);
			if(ValueChanged)
			{
				memcpy(mp_RxValue, receivedValue, sizeof(T) * COUNT);
				ESP_LOGI( "DataItem: NewRXValueReceived"
						, "\"%s\" New RX Value Received: \"%s\""
						, m_Name.c_str()
						, GetValueAsStringForDataType(mp_RxValue, GetDataTypeFromTemplateType<T>(), COUNT));
				
				bool RxValueChanged = (memcmp(mp_Value, mp_RxValue, sizeof(T) * COUNT) != 0);
				if( UpdateStoreType_On_Rx == m_UpdateStoreType )
				{
					if(RxValueChanged) memcpy(mp_Value, mp_RxValue, sizeof(T) * COUNT);
				}
				if(RxTxType_Rx_Echo_Value == m_RxTxType)
				{
					memcpy(mp_TxValue, mp_RxValue, sizeof(T) * COUNT);
					DataItem_TX_Now();
				}
			}
		}
};

#endif