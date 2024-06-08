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

#include "DataItem/LocalDataItem.h"

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
class DataItem: public LocalDataItem<T, COUNT>
			  , public NewRxTxValueCallerInterface<T>
			  , public NewRxTxVoidObjectCalleeInterface
{
	public:
		DataItem( const String name
				, const T* initialValue
				, const RxTxType_t rxTxType
				, const UpdateStoreType_t updateStoreType
				, const uint16_t rate
				, SerialPortMessageManager &serialPortMessageManager
				, NamedCallback_t *namedCallback
				, SetupCallerInterface *setupCallerInterface )
				: LocalDataItem<T, COUNT>( name, initialValue, namedCallback, setupCallerInterface)
				, NewRxTxVoidObjectCalleeInterface(COUNT)
				, m_RxTxType(rxTxType)
				, m_UpdateStoreType(updateStoreType)
				, m_Rate(rate)
				, m_SerialPortMessageManager(serialPortMessageManager)
		{
			ESP_LOGI("DataItem", "DataItem Instantiated: Constructor 1");
			CreateTxTimer();
		}

		DataItem( const String name
				, const T& initialValue
				, const RxTxType_t rxTxType
				, const UpdateStoreType_t updateStoreType
				, const uint16_t rate
				, SerialPortMessageManager &serialPortMessageManager
				, NamedCallback_t *namedCallback
				, SetupCallerInterface *setupCallerInterface )
				: LocalDataItem<T, COUNT>( name, initialValue, namedCallback, setupCallerInterface)
				, NewRxTxVoidObjectCalleeInterface(COUNT)
				, m_RxTxType(rxTxType)
				, m_UpdateStoreType(updateStoreType)
				, m_Rate(rate)
				, m_SerialPortMessageManager(serialPortMessageManager)
				
		{
			ESP_LOGI("DataItem", "DataItem Instantiated: Constructor 2");
			CreateTxTimer();
		}
		
		DataItem( const String name
				, const T* initialValue
				, const RxTxType_t rxTxType
				, const UpdateStoreType_t updateStoreType
				, const uint16_t rate
				, SerialPortMessageManager &serialPortMessageManager
				, NamedCallback_t *namedCallback
				, SetupCallerInterface *setupCallerInterface
				, ValidStringValues_t *validStringValues )
				: LocalDataItem<T, COUNT>( name, initialValue, namedCallback, setupCallerInterface, validStringValues)
				, NewRxTxVoidObjectCalleeInterface(COUNT)
				, m_RxTxType(rxTxType)
				, m_UpdateStoreType(updateStoreType)
				, m_Rate(rate)
				, m_SerialPortMessageManager(serialPortMessageManager)
		{
			ESP_LOGI("DataItem", "DataItem Instantiated: Constructor 3");
			CreateTxTimer();
		}

		DataItem( const String name
				, const T& initialValue
				, const RxTxType_t rxTxType
				, const UpdateStoreType_t updateStoreType
				, const uint16_t rate
				, SerialPortMessageManager &serialPortMessageManager
				, NamedCallback_t *namedCallback
				, SetupCallerInterface *setupCallerInterface
				, ValidStringValues_t *validStringValues )
				: LocalDataItem<T, COUNT>( name, initialValue, namedCallback, setupCallerInterface, validStringValues)
				, NewRxTxVoidObjectCalleeInterface(COUNT)
				, m_RxTxType(rxTxType)
				, m_UpdateStoreType(updateStoreType)
				, m_Rate(rate)
				, m_SerialPortMessageManager(serialPortMessageManager)	
		{
			ESP_LOGI("DataItem", "DataItem Instantiated: Constructor 4");
			CreateTxTimer();
		}

		DataItem( const String name
				, const T& initialValue
				, const RxTxType_t rxTxType
				, const UpdateStoreType_t updateStoreType
				, const uint16_t rate
				, SerialPortMessageManager &serialPortMessageManager
				, NamedCallback_t *namedCallback
				, SetupCallerInterface *setupCallerInterface
				, bool printDelimited )
				: LocalDataItem<T, COUNT>( name, initialValue, namedCallback, setupCallerInterface, printDelimited)
				, NewRxTxVoidObjectCalleeInterface(COUNT)
				, m_RxTxType(rxTxType)
				, m_UpdateStoreType(updateStoreType)
				, m_Rate(rate)
				, m_SerialPortMessageManager(serialPortMessageManager)
		{
			ESP_LOGI("DataItem", "DataItem Instantiated: Constructor 5");
			CreateTxTimer();
		}
		
		virtual ~DataItem()
		{
			ESP_LOGI("DataItem::~DataItem()", "\"%s\": DataItem Freeing Memory", this->GetName().c_str());
			esp_timer_stop(m_TxTimer);
			esp_timer_delete(m_TxTimer);
			if(mp_RxValue) heap_caps_free(mp_RxValue);
			if(mp_TxValue) heap_caps_free(mp_TxValue);
		}
		void Setup()
		{
			ESP_LOGD("DataItem<T, COUNT>::Setup()", "\"%s\": Allocating Memory", m_Name.c_str());
			LocalDataItem<T, COUNT>::Setup();
			mp_RxValue = (T*)heap_caps_malloc(sizeof(T)*COUNT, MALLOC_CAP_SPIRAM);
			mp_TxValue = (T*)heap_caps_malloc(sizeof(T)*COUNT, MALLOC_CAP_SPIRAM);
			if (mp_RxValue && mp_TxValue && this->mp_InitialValuePtr)
			{
				if (std::is_same<T, char>::value)
				{
					String InitialValue = String((char*)this->mp_InitialValuePtr);
					ESP_LOGD( "DataItem<T, COUNT>::Setup()", "\"%s\": Setting initial value: \"%s\""
							, this->m_Name.c_str()
							, InitialValue.c_str());
					for (size_t i = 0; i < COUNT; ++i)
					{
						char value;
						memcpy(&value, this->mp_InitialValuePtr+i, sizeof(char));
						if (i >= InitialValue.length())
						{
							value = '\0';
						}
						memcpy(mp_RxValue+i, &value, sizeof(char));
						memcpy(mp_TxValue+i, &value, sizeof(char));
					}
				}
				else
				{
					ESP_LOGD( "DataItem<T, COUNT>::Setup()", "\"%s\": Setting initial value: \"%s\""
							, m_Name.c_str()
							, GetValueAsStringForDataType(mp_InitialValuePtr, GetDataTypeFromTemplateType<T>(), COUNT, "").c_str());
					for (size_t i = 0; i < COUNT; ++i)
					{
						memcpy(mp_RxValue+i, this->mp_InitialValuePtr, sizeof(T));
						memcpy(mp_TxValue+i, this->mp_InitialValuePtr, sizeof(T));
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
			return LocalDataItem<T, COUNT>::GetName();
		}

		size_t GetCount() const
		{
			return LocalDataItem<T, COUNT>::GetCount();
		}

		void SetNewTxValue(const T* Value, const size_t Count)
		{
			ESP_LOGD("DataItem: SetNewTxValue", "\"%s\" SetNewTxValue to: \"%s\"", m_Name.c_str(), GetValueAsStringForDataType(Value, GetDataTypeFromTemplateType<T>(), COUNT, ""));
			SetValue(Value, Count);
		}
		
		virtual bool SetValueFromString(const String& stringValue) override
		{
			return LocalDataItem<T, COUNT>::SetValueFromString(stringValue);
		}
		
		bool SetValue(const T *value, size_t count)
		{
			bool valueChanged = LocalDataItem<T, COUNT>::SetValue(value, count);
			if(valueChanged)
			{
				DataItem_Try_TX_On_Change();
			}
			return valueChanged;
		}

		bool SetValue(T value)
		{
			bool valueChanged = LocalDataItem<T, COUNT>::SetValue(value);
			if(valueChanged)
			{
				DataItem_Try_TX_On_Change();
			}
			return valueChanged;
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
	protected:
		const RxTxType_t m_RxTxType;
		const UpdateStoreType_t m_UpdateStoreType;
		const uint16_t m_Rate;
		SerialPortMessageManager &m_SerialPortMessageManager;

		T *mp_RxValue;
		T *mp_TxValue;
		
		bool DataItem_TX_Now()
		{
			bool ValueUpdated = false;
			if(m_SerialPortMessageManager.QueueMessageFromData(this->GetName(), DataTypeFunctions::GetDataTypeFromTemplateType<T>(), mp_TxValue, COUNT))
			{				
				if(memcmp(this->mp_Value, mp_TxValue, sizeof(T) * COUNT) != 0)
				{
					if(m_UpdateStoreType == UpdateStoreType_On_Tx)
					{
						LocalDataItem<T, COUNT>::SetValue(mp_TxValue, COUNT);						
						ValueUpdated = true;		
					}
				}
				ESP_LOGD( "DataItem: DataItem_TX_Now", "\"%s\" TX: \"%s\" Value: \"%s\""
						, this->m_SerialPortMessageManager.GetName().c_str()
						, this->m_Name.c_str()
						, this->GetValueAsString().c_str() );
			}
			else
			{
				ESP_LOGE("DataItem: DataItem_TX_Now", "Data Item: \"%s\": Unable to Tx Message", this->m_Name.c_str());
			}
			return ValueUpdated;
		}
		virtual bool NewRxValueReceived(void* Object, size_t Count) override
		{	
			bool ValueUpdated = false;
			T* receivedValue = static_cast<T*>(Object);
			ESP_LOGD( "DataItem: NewRxValueReceived"
					, "\"%s\" RX: \"%s\" Value: \"%s\""
					, this->m_SerialPortMessageManager.GetName().c_str()
					, this->m_Name.c_str()
					, this->GetValueAsString().c_str());
			if(memcmp(mp_RxValue, receivedValue, sizeof(T) * COUNT) != 0)
			{
				memcpy(mp_RxValue, receivedValue, sizeof(T) * COUNT);
				ESP_LOGD( "DataItem: NewRxValueReceived"
						, "Value Changed for: \"%s\" to Value: \"%s\""
						, this->m_Name.c_str()
						, this->GetValueAsString().c_str());
				if( UpdateStoreType_On_Rx == m_UpdateStoreType )
				{
					SetValue(mp_RxValue, COUNT);	
					ValueUpdated = true;
				}
			}
			if(RxTxType_Rx_Echo_Value == this->m_RxTxType)
			{
				memcpy(mp_TxValue, mp_RxValue, sizeof(T) * COUNT);
				ESP_LOGD( "DataItem: NewRxValueReceived"
						, "RX Echo for: \"%s\" with Value: \"%s\""
						, this->m_Name.c_str()
						, this->GetValueAsString().c_str());
				DataItem_TX_Now();
			}
			return ValueUpdated;
		}
		void DataItem_Try_TX_On_Change()
		{
			ESP_LOGI("DataItem& DataItem_Try_TX_On_Change", "Data Item: \"%s\": Try TX On Change", this->m_Name.c_str());
			if(this->m_RxTxType == RxTxType_Tx_On_Change || this->m_RxTxType == RxTxType_Tx_On_Change_With_Heartbeat)
			{
				DataItem_TX_Now();
			}
		}
	private:
		bool m_DataLinkEnabled = true;
		esp_timer_handle_t m_TxTimer;
		esp_timer_create_args_t timerArgs;
		void CreateTxTimer()
		{
			timerArgs.callback = &StaticDataItem_Periodic_TX;
			timerArgs.arg = this;
			timerArgs.name = "Tx_Timer";
			esp_timer_create(&timerArgs, &m_TxTimer);
		}
		void DataItem_Periodic_TX()
		{
			DataItem_TX_Now();
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