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

#include <Helpers.h>
#include <Preferences.h>
#include <nvs_flash.h>
#include <esp_timer.h>
#include <Arduino.h>
#include <esp_heap_caps.h>
#include "SerialMessageManager.h"

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

template <typename T, size_t COUNT>
class DataItem: public NewRxTxValueCallerInterface<T>
			  , public NewRxTxVoidObjectCalleeInterface
			  , public SetupCalleeInterface
			  , public DataTypeFunctions
{
	public:
		DataItem( const String name
				, const T* initialValue
				, const RxTxType_t rxTxType
				, const UpdateStoreType_t updateStoreType
				, const uint16_t rate
				, SerialPortMessageManager &serialPortMessageManager )
				: NewRxTxVoidObjectCalleeInterface(COUNT)
				, m_Name(name)
				, mp_InitialValuePtr(initialValue)
				, m_RxTxType(rxTxType)
				, m_UpdateStoreType(updateStoreType)
				, m_Rate(rate)
				, m_SerialPortMessageManager(serialPortMessageManager)
		{
			CreateTxTimer();
			m_SerialPortMessageManager.RegisterForSetupCall(this);
		}
		DataItem( const String name
				, const T& initialValue
				, const RxTxType_t rxTxType
				, const UpdateStoreType_t updateStoreType
				, const uint16_t rate
				, SerialPortMessageManager &serialPortMessageManager )
				: NewRxTxVoidObjectCalleeInterface(COUNT)
				, m_Name(name)
				, mp_InitialValuePtr(&initialValue)
				, m_RxTxType(rxTxType)
				, m_UpdateStoreType(updateStoreType)
				, m_Rate(rate)
				, m_SerialPortMessageManager(serialPortMessageManager)
				
		{
			CreateTxTimer();
			m_SerialPortMessageManager.RegisterForSetupCall(this);
		}
		
		virtual ~DataItem()
		{
			heap_caps_free(mp_Value);
			heap_caps_free(mp_RxValue);
			heap_caps_free(mp_TxValue);
			heap_caps_free(mp_InitialValue);
			esp_timer_stop(m_TxTimer);
			esp_timer_delete(m_TxTimer);
			m_SerialPortMessageManager.DeRegisterForSetupCall(this);
		}
		virtual void Setup()
		{
			ESP_LOGD("DataItem<T, COUNT>::Setup()", "\"%s\": Allocating Memory", m_Name.c_str());
			mp_Value = (T*)heap_caps_malloc(sizeof(T)*COUNT, MALLOC_CAP_SPIRAM);
			mp_RxValue = (T*)heap_caps_malloc(sizeof(T)*COUNT, MALLOC_CAP_SPIRAM);
			mp_TxValue = (T*)heap_caps_malloc(sizeof(T)*COUNT, MALLOC_CAP_SPIRAM);
			mp_InitialValue = (T*)heap_caps_malloc(sizeof(T)*COUNT, MALLOC_CAP_SPIRAM);
			if (mp_Value && mp_RxValue && mp_TxValue && mp_InitialValue && mp_InitialValuePtr)
			{
				if (std::is_same<T, char>::value)
				{
					String InitialValue = String((char*)mp_InitialValuePtr);
					ESP_LOGD( "DataItem<T, COUNT>::Setup()", "\"%s\": Setting initial value: \"%s\""
							, m_Name.c_str()
							, InitialValue.c_str());
					for (size_t i = 0; i < COUNT; ++i)
					{
						char value;
						memcpy(&value, mp_InitialValuePtr+i, sizeof(char));
						if (i >= InitialValue.length())
						{
							value = '\0';
						}
						memcpy(mp_Value+i, &value, sizeof(char));
						this->CallCallbacks(m_Name.c_str(), mp_Value);
						memcpy(mp_RxValue+i, &value, sizeof(char));
						memcpy(mp_TxValue+i, &value, sizeof(char));
						memcpy(mp_InitialValue+i, &value, sizeof(char));
					}
				}
				else
				{
					ESP_LOGD( "DataItem<T, COUNT>::Setup()", "\"%s\": Setting initial value: \"%s\""
							, m_Name.c_str()
							, GetValueAsStringForDataType(mp_InitialValuePtr, GetDataTypeFromTemplateType<T>(), COUNT, "").c_str());
					for (size_t i = 0; i < COUNT; ++i)
					{
						memcpy(mp_Value+i, mp_InitialValuePtr, sizeof(T));
						this->CallCallbacks(m_Name.c_str(), mp_Value);
						memcpy(mp_RxValue+i, mp_InitialValuePtr, sizeof(T));
						memcpy(mp_TxValue+i, mp_InitialValuePtr, sizeof(T));
						memcpy(mp_InitialValue+i, mp_InitialValuePtr, sizeof(T));
					}
				}
				SetDataLinkEnabled(true);
			}
			else
			{
				ESP_LOGE("DataItem<T, COUNT>::Setup()", "Failed to allocate memory on SPI RAM");
			}
		}
		String GetName()
		{
			return m_Name;
		}
		size_t GetValue(void* Object, size_t Count)
		{
			assert(Count == COUNT && "Counts must be equal");
			if(mp_Value)
			{
				memcpy(Object, mp_Value, sizeof(T)*Count);
			}
			else
			{
				*reinterpret_cast<T**>(Object) = nullptr;
			}
			return m_ValueChangeCount;
		}
		T GetValue()
		{
			assert(1 == COUNT && "Count must 1 to use this function");
			if(mp_Value)
			{
				return static_cast<T>(*mp_Value);
			}
			else
			{
				return T();
			}
		}
		String GetValueAsString(const String &Divider)
		{
			return GetValueAsStringForDataType(mp_Value, GetDataTypeFromTemplateType<T>(), COUNT, Divider);
		}
		void SetNewTxValue(const T* Value, const size_t Count)
		{
			ESP_LOGD("DataItem: SetNewTxValue", "\"%s\" SetNewTxValue to: \"%s\"", m_Name.c_str(), GetValueAsStringForDataType(Value, GetDataTypeFromTemplateType<T>(), COUNT, ""));
			SetValue(Value, Count);
		}
		virtual void SetValue(const T *Value, size_t Count)
		{
			assert(Value != nullptr && "Value must not be null");
			assert(mp_Value != nullptr && "mp_Value must not be null");
			assert(COUNT > 0 && "COUNT must be a valid index range for mp_Value");
			assert(COUNT == Count && "Counts must match");
			ESP_LOGD( "DataItem: SetValue"
					, "\"%s\" Set Value: \"%s\""
					, m_Name.c_str()
					, GetValueAsStringForDataType(Value, GetDataTypeFromTemplateType<T>(), COUNT, "").c_str());
			bool ValueChanged = (memcmp(mp_TxValue, Value, sizeof(T) * COUNT) != 0);
			memcpy(mp_TxValue, Value, sizeof(T) * COUNT);
			if(ValueChanged)
			{
				DataItem_Try_TX_On_Change();
			}
		}
		size_t GetCount()
		{
			return m_Count;
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
					ESP_LOGD("DataItem: SetDataLinkEnabled", "Data Item: \"%s\": Enabled Periodic TX", m_Name.c_str());
				}
				else
				{
					esp_timer_stop(m_TxTimer);
					ESP_LOGD("DataItem: SetDataLinkEnabled", "Data Item: \"%s\": Disabled Periodic TX", m_Name.c_str());
				}
				if(enablePeriodicRX)
				{
					m_SerialPortMessageManager.RegisterForNewValueNotification(this);
					ESP_LOGD("DataItem: SetDataLinkEnabled", "Data Item: \"%s\": Enabled Periodic RX", m_Name.c_str());
				}
				else
				{
					m_SerialPortMessageManager.DeRegisterForNewValueNotification(this);
					ESP_LOGD("DataItem: SetDataLinkEnabled", "Data Item: \"%s\": Disabled Periodic RX", m_Name.c_str());
				}
			}
			else
			{
				esp_timer_stop(m_TxTimer);
				m_SerialPortMessageManager.DeRegisterForNewValueNotification(this);
				ESP_LOGD("SetDataLinkEnabled", "Data Item: \"%s\": Disabled Datalink", m_Name.c_str());
			}
		}
		bool EqualsValue(T *Object, size_t Count)
		{
			assert(Count == COUNT && "Counts must equal");
			return (memcmp(mp_Value, Object, Count) == 0);
		}
	protected:
		const String m_Name;
		const T *mp_InitialValuePtr;
		const RxTxType_t m_RxTxType;
		const UpdateStoreType_t m_UpdateStoreType;
		const uint16_t m_Rate;
		
		T *mp_Value;
		T *mp_RxValue;
		T *mp_TxValue;
		T *mp_InitialValue;
		
		virtual bool DataItem_TX_Now()
		{
			bool ValueUpdated = false;
			if(m_SerialPortMessageManager.QueueMessageFromData(m_Name, GetDataTypeFromTemplateType<T>(), mp_TxValue, COUNT))
			{
				bool TxValueChanged = (memcmp(mp_Value, mp_TxValue, sizeof(T) * COUNT) != 0);
				if(m_UpdateStoreType == UpdateStoreType_On_Tx)
				{
					if(TxValueChanged)
					{
						memcpy(mp_Value, mp_TxValue, sizeof(T) * COUNT);
						++m_ValueChangeCount;
						this->CallCallbacks(m_Name.c_str(), mp_Value);
						ValueUpdated = true;
					}
				}
				ESP_LOGI("DataItem: DataItem_TX_Now", "TX Now: \"%s\" Value: \"%s\"", m_Name.c_str(), GetValueAsStringForDataType(mp_TxValue, GetDataTypeFromTemplateType<T>(), COUNT, "").c_str());
			}
			else
			{
				ESP_LOGE("DataItem: DataItem_TX_Now", "Data Item: \"%s\": Unable to Tx Message", m_Name.c_str());
			}
			return ValueUpdated;
		}
		virtual bool NewRXValueReceived(void* Object, size_t Count)
		{	
			bool ValueUpdated = false;
			T* receivedValue = static_cast<T*>(Object);
			bool ValueChanged = (memcmp(mp_RxValue, receivedValue, sizeof(T) * COUNT) != 0);
			ESP_LOGI( "DataItem: NewRXValueReceived"
						, "RX Value: \"%s\" Value: \"%s\""
						, m_Name.c_str()
						, GetValueAsStringForDataType(mp_RxValue, GetDataTypeFromTemplateType<T>(), COUNT, "").c_str());
			if(ValueChanged)
			{
				memcpy(mp_RxValue, receivedValue, sizeof(T) * COUNT);
				ESP_LOGI( "DataItem: NewRXValueReceived"
						, "Value Changed for: \"%s\" to Value: \"%s\""
						, m_Name.c_str()
						, GetValueAsStringForDataType(mp_RxValue, GetDataTypeFromTemplateType<T>(), COUNT, "").c_str());
				
				bool RxValueChanged = (memcmp(mp_Value, mp_RxValue, sizeof(T) * COUNT) != 0);
				if( UpdateStoreType_On_Rx == m_UpdateStoreType )
				{
					if(RxValueChanged)
					{
						memcpy(mp_Value, mp_RxValue, sizeof(T) * COUNT);
						++m_ValueChangeCount;
						this->CallCallbacks(m_Name.c_str(), mp_Value);
						ValueUpdated = true;
					}
				}
			}
			if(RxTxType_Rx_Echo_Value == m_RxTxType)
			{
				memcpy(mp_TxValue, mp_RxValue, sizeof(T) * COUNT);
				ESP_LOGD( "DataItem: NewRXValueReceived"
						, "RX Echo for: \"%s\" with Value: \"%s\""
						, m_Name.c_str()
						, GetValueAsStringForDataType(mp_RxValue, GetDataTypeFromTemplateType<T>(), COUNT, "").c_str());
				DataItem_TX_Now();
			}
			return ValueUpdated;
		}
		void DataItem_Try_TX_On_Change()
		{
			ESP_LOGI("DataItem& DataItem_Try_TX_On_Change", "Data Item: \"%s\": Try TX On Change", m_Name.c_str());
			if(m_RxTxType == RxTxType_Tx_On_Change || m_RxTxType == RxTxType_Tx_On_Change_With_Heartbeat)
			{
				DataItem_TX_Now();
			}
		}
	private:
		bool m_DataLinkEnabled = true;
		size_t m_ValueChangeCount = 0;
		SerialPortMessageManager &m_SerialPortMessageManager;
		esp_timer_handle_t m_TxTimer;
		size_t m_Count = COUNT;
		void CreateTxTimer()
		{
			esp_timer_create_args_t timerArgs;
			timerArgs.callback = &StaticDataItem_Periodic_TX;
			timerArgs.arg = this;
			timerArgs.name = "Tx_Timer";

			// Create the timer
			esp_timer_create(&timerArgs, &m_TxTimer);
		}
		void DataItem_Periodic_TX()
		{
			DataItem_TX_Now();
			//if(m_SerialPortMessageManager.QueueMessageFromData(m_Name, GetDataTypeFromTemplateType<T>(), mp_Value, COUNT))
			//{
			//	ESP_LOGD("DataItem: DataItem_TX_Now", "Data Item: \"%s\": Periodic TX: \"%s\"", m_Name.c_str(), GetValueAsStringForDataType(mp_Value, GetDataTypeFromTemplateType<T>(), COUNT, "").c_str());
			//}
		}
		static void StaticDataItem_Periodic_TX(void *arg)
		{
			DataItem *aDataItem = static_cast<DataItem*>(arg);
			if(aDataItem)
			{
				aDataItem->DataItem_Periodic_TX();
			}
		}
};
class StringDataItem: public DataItem<char, 50>
{
	public:
		StringDataItem( const String name
					  , const char* initialValue
					  , const RxTxType_t rxTxType
					  , const UpdateStoreType_t updateStoreType
					  , const uint16_t rate
					  , SerialPortMessageManager &serialPortMessageManager )
					  : DataItem<char, 50>( name
										  , initialValue
										  , rxTxType
										  , updateStoreType
										  , rate
										  , serialPortMessageManager )
		{
		  
		}
		StringDataItem( const String name
					  , const char& initialValue
					  , const RxTxType_t rxTxType
					  , const UpdateStoreType_t updateStoreType
					  , const uint16_t rate
					  , SerialPortMessageManager &serialPortMessageManager )
					  : DataItem<char, 50>( name
										  , initialValue
										  , rxTxType
										  , updateStoreType
										  , rate
										  , serialPortMessageManager )
		{
		  
		}
		virtual ~StringDataItem()
		{
		}
		virtual void SetValue(const char* Value, size_t Count) override
		{
			assert(Value != nullptr && "Value must not be null");
			assert(mp_Value != nullptr && "mp_Value must not be null");
			String NewValue = String(Value);
			String CurrentValue = String(mp_TxValue);
			assert(NewValue.length() == Count);
			ESP_LOGD( "DataItem: SetValue"
					, "\"%s\" Set Value: \"%s\""
					, m_Name.c_str()
					, NewValue.c_str() );
			bool ValueChanged = CurrentValue.equals(NewValue);
			if(ValueChanged)
			{
				for (size_t i = 0; i < this->GetCount(); ++i)
				{
					mp_TxValue[i] = '\0';
				}
				strcpy(mp_TxValue, Value);
				this->DataItem_Try_TX_On_Change();
			}
		}
	protected:
		virtual bool NewRXValueReceived(void* Object, size_t Count) override 
		{ 
			bool ValueUpdated = false;
			String NewValue = String((char*)Object);
			String CurrentRxValue = String(mp_RxValue);
			String CurrentValue = String(mp_Value);
			bool ValueChanged = CurrentRxValue.equals(NewValue);
			if(ValueChanged)
			{
				for (size_t i = 0; i < this->GetCount(); ++i)
				{
					mp_RxValue[i] = '\0';
				}
				strcpy(mp_RxValue, NewValue.c_str());
				ESP_LOGD( "DataItem: NewRXValueReceived"
						, "\"%s\" New RX Value Received: \"%s\""
						, m_Name.c_str()
						, GetValueAsStringForDataType(mp_RxValue, DataType_Char_t, this->GetCount(), ""));
				
				bool RxValueChanged = CurrentRxValue.equals(CurrentValue);
				if( UpdateStoreType_On_Rx == m_UpdateStoreType )
				{
					if(RxValueChanged)
					{
						for (size_t i = 0; i < this->GetCount(); ++i)
						{
							mp_Value[i] = '\0';
						}
						strcpy(mp_Value, mp_RxValue);
						ValueUpdated = true;
					}
				}
				if(RxTxType_Rx_Echo_Value == m_RxTxType)
				{
					for (size_t i = 0; i < this->GetCount(); ++i)
					{
						mp_TxValue[i] = '\0';
					}
					strcpy(mp_TxValue, mp_RxValue);
					this->DataItem_TX_Now();
				}
			}
			return ValueUpdated;
		}
};