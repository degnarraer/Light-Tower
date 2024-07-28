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
#include "DataItem/SerialDataLinkInterface.h"

template <typename T, size_t COUNT>
class DataItem: public LocalDataItem<T, COUNT>
			  , public SerialDataLinkIntertface<T, COUNT>
{
	public:
		DataItem( const String name
				, const T* initialValue
				, const RxTxType_t rxTxType
				, const UpdateStoreType_t updateStoreType
				, const uint16_t rate )
				: LocalDataItem<T, COUNT>(name, initialValue)
				, SerialDataLinkIntertface<T, COUNT>(rxTxType, updateStoreType, rate)
		{
			ESP_LOGI("DataItem", "DataItem Instantiated: Default Constructor 1");
		}

		DataItem( const String name
				, const T& initialValue
				, const RxTxType_t rxTxType
				, const UpdateStoreType_t updateStoreType
				, const uint16_t rate )
				: LocalDataItem<T, COUNT>(name, initialValue)
				, SerialDataLinkIntertface<T, COUNT>(rxTxType, updateStoreType, rate)
		{
			ESP_LOGI("DataItem", "DataItem Instantiated: Default Constructor 2");
		}

		DataItem( const String name
				, const T* initialValue
				, const RxTxType_t rxTxType
				, const UpdateStoreType_t updateStoreType
				, const uint16_t rate
				, SerialPortMessageManager *serialPortMessageManager
				, NamedCallback_t *namedCallback
				, SetupCallerInterface *setupCallerInterface )
				: LocalDataItem<T, COUNT>( name, initialValue, namedCallback, setupCallerInterface)
				, SerialDataLinkIntertface<T, COUNT>(rxTxType, updateStoreType, rate, serialPortMessageManager)
		{
			ESP_LOGI("DataItem", "DataItem Instantiated: Constructor 1");
		}

		DataItem( const String name
				, const T& initialValue
				, const RxTxType_t rxTxType
				, const UpdateStoreType_t updateStoreType
				, const uint16_t rate
				, SerialPortMessageManager *serialPortMessageManager
				, NamedCallback_t *namedCallback
				, SetupCallerInterface *setupCallerInterface )
				: LocalDataItem<T, COUNT>( name, initialValue, namedCallback, setupCallerInterface)
				, SerialDataLinkIntertface<T, COUNT>(rxTxType, updateStoreType, rate, serialPortMessageManager)				
		{
			ESP_LOGI("DataItem", "DataItem Instantiated: Constructor 2");
		}
		
		DataItem( const String name
				, const T* initialValue
				, const RxTxType_t rxTxType
				, const UpdateStoreType_t updateStoreType
				, const uint16_t rate
				, SerialPortMessageManager *serialPortMessageManager
				, NamedCallback_t *namedCallback
				, SetupCallerInterface *setupCallerInterface
				, ValidStringValues_t *validStringValues )
				: LocalDataItem<T, COUNT>( name, initialValue, namedCallback, setupCallerInterface, validStringValues)
				, SerialDataLinkIntertface<T, COUNT>(rxTxType, updateStoreType, rate, serialPortMessageManager)
		{
			ESP_LOGI("DataItem", "DataItem Instantiated: Constructor 3");
		}

		DataItem( const String name
				, const T& initialValue
				, const RxTxType_t rxTxType
				, const UpdateStoreType_t updateStoreType
				, const uint16_t rate
				, SerialPortMessageManager *serialPortMessageManager
				, NamedCallback_t *namedCallback
				, SetupCallerInterface *setupCallerInterface
				, ValidStringValues_t *validStringValues )
				: LocalDataItem<T, COUNT>( name, initialValue, namedCallback, setupCallerInterface, validStringValues)
				, SerialDataLinkIntertface<T, COUNT>(rxTxType, updateStoreType, rate, serialPortMessageManager)
		{
			ESP_LOGI("DataItem", "DataItem Instantiated: Constructor 4");
		}

		DataItem( const String name
				, const T& initialValue
				, const RxTxType_t rxTxType
				, const UpdateStoreType_t updateStoreType
				, const uint16_t rate
				, SerialPortMessageManager *serialPortMessageManager
				, NamedCallback_t *namedCallback
				, SetupCallerInterface *setupCallerInterface
				, bool printDelimited )
				: LocalDataItem<T, COUNT>( name, initialValue, namedCallback, setupCallerInterface, printDelimited)
				, SerialDataLinkIntertface<T, COUNT>(rxTxType, updateStoreType, rate, serialPortMessageManager)
		{
			ESP_LOGI("DataItem", "DataItem Instantiated: Constructor 5");
		}
		
		virtual ~DataItem() override
		{
			ESP_LOGI("DataItem::~DataItem()", "\"%s\": DataItem Freeing Memory", LocalDataItem<T,COUNT>::GetName().c_str());				
		}

		//SetupCalleeInterface
		virtual void Setup() override
		{
			ESP_LOGD("DataItem<T, COUNT>::Setup()", "\"%s\": Allocating Memory", LocalDataItem<T,COUNT>::GetName().c_str());
			LocalDataItem<T, COUNT>::Setup();
			SerialDataLinkIntertface<T, COUNT>::Setup();
		}

		//SerialDataLinkIntertface
		virtual size_t GetCount() const override
		{
			return LocalDataItem<T, COUNT>::GetCount();
		}
		virtual T* GetValuePointer() const override
		{
			return LocalDataItem<T, COUNT>::GetValuePointer();
		}
		virtual bool EqualsValue(T *object, size_t count) const override
		{
			return LocalDataItem<T, COUNT>::EqualsValue(object, count);
		}
		virtual String GetName() const override
		{
			return LocalDataItem<T, COUNT>::GetName();
		}
		virtual String GetValueAsString() const override
		{
			return LocalDataItem<T, COUNT>::GetValueAsString();
		}
		virtual DataType_t GetDataType() override
		{
			return LocalDataItem<T, COUNT>::GetDataType();
		}
		
		virtual bool SetValue(const T *value, size_t count) override
		{
			bool valueChanged = LocalDataItem<T, COUNT>::SetValue(value, count);
			if(valueChanged)
			{
				DataItem_Try_TX_On_Change();
			}
			return valueChanged;
		}

		//Local DataItem Override
		virtual bool SetValue(T value) override
		{
			bool valueChanged = LocalDataItem<T, COUNT>::SetValue(value);
			if(valueChanged)
			{
				DataItem_Try_TX_On_Change();
			}
			return valueChanged;
		}
		
		void Periodic_TX()
		{
			DataItem_TX_Now();
		}

		virtual bool NewRxValueReceived(void* Object, size_t Count) override
		{	
			bool ValueUpdated = false;
			T* receivedValue = static_cast<T*>(Object);
			ESP_LOGD( "DataItem: NewRxValueReceived"
					, "\"%s\" RX: \"%s\" Value: \"%s\""
					, this->mp_SerialPortMessageManager->GetName().c_str()
					, LocalDataItem<T,COUNT>::GetName().c_str()
					, this->GetValueAsString().c_str());
			if(memcmp(this->mp_RxValue, receivedValue, sizeof(T) * COUNT) != 0)
			{
				memcpy(this->mp_RxValue, receivedValue, sizeof(T) * COUNT);
				ESP_LOGD( "DataItem: NewRxValueReceived"
						, "Value Changed for: \"%s\" to Value: \"%s\""
						, LocalDataItem<T,COUNT>::GetName().c_str()
						, this->GetValueAsString().c_str());
				if( UpdateStoreType_On_Rx == this->m_UpdateStoreType )
				{
					SetValue(this->mp_RxValue, COUNT);	
					ValueUpdated = true;
				}
			}
			if(RxTxType_Rx_Echo_Value == this->m_RxTxType)
			{
				memcpy(this->mp_TxValue, this->mp_RxValue, sizeof(T) * COUNT);
				ESP_LOGD( "DataItem: NewRxValueReceived"
						, "RX Echo for: \"%s\" with Value: \"%s\""
						, LocalDataItem<T,COUNT>::GetName().c_str()
						, this->GetValueAsString().c_str());
				DataItem_TX_Now();
			}
			return ValueUpdated;
		}
		
		void DataItem_Try_TX_On_Change()
		{
			ESP_LOGI("DataItem& DataItem_Try_TX_On_Change", "Data Item: \"%s\": Try TX On Change", LocalDataItem<T,COUNT>::GetName().c_str());
			if(this->m_RxTxType == RxTxType_Tx_On_Change || this->m_RxTxType == RxTxType_Tx_On_Change_With_Heartbeat)
			{
				DataItem_TX_Now();
			}
		}
		bool DataItem_TX_Now()
		{
			bool ValueUpdated = false;
			if(this->mp_SerialPortMessageManager->QueueMessageFromData(LocalDataItem<T,COUNT>::GetName(), DataTypeFunctions::GetDataTypeFromTemplateType<T>(), this->mp_TxValue, COUNT))
			{				
				if(memcmp(this->mp_Value, this->mp_TxValue, sizeof(T) * COUNT) != 0)
				{
					if(this->m_UpdateStoreType == UpdateStoreType_On_Tx)
					{
						LocalDataItem<T, COUNT>::SetValue(this->mp_TxValue, COUNT);						
						ValueUpdated = true;		
					}
				}
				ESP_LOGD( "DataItem: DataItem_TX_Now", "\"%s\" TX: \"%s\" Value: \"%s\""
						, this->mp_SerialPortMessageManager->GetName().c_str()
						, LocalDataItem<T,COUNT>::GetName().c_str()
						, this->GetValueAsString().c_str() );
			}
			else
			{
				ESP_LOGE("DataItem: DataItem_TX_Now", "ERROR! Data Item: \"%s\": Unable to Tx Message.", LocalDataItem<T,COUNT>::GetName().c_str());
			}
			return ValueUpdated;
		}
};