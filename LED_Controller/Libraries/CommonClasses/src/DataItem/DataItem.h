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
#include "DataItem/SerialMessageInterface.h"

template <typename T, size_t COUNT>
class DataItem: public LocalDataItem<T, COUNT>
			  , public SerialMessageInterface<T, COUNT>
{
	public:
		DataItem( const std::string name
				, const T* initialValue
				, const RxTxType_t rxTxType
				, const uint16_t rate )
				: LocalDataItem<T, COUNT>(name, initialValue)
				, SerialMessageInterface<T, COUNT>(rxTxType, rate)
		{
			ESP_LOGD("DataItem", "DataItem Instantiated: Default Constructor 1");
		}

		DataItem( const std::string name
				, const T& initialValue
				, const RxTxType_t rxTxType
				, const uint16_t rate )
				: LocalDataItem<T, COUNT>(name, initialValue)
				, SerialMessageInterface<T, COUNT>(rxTxType, rate)
		{
			ESP_LOGD("DataItem", "DataItem Instantiated: Default Constructor 2");
		}

		DataItem( const std::string name
				, const T* initialValue
				, const RxTxType_t rxTxType
				, const uint16_t rate
				, SerialPortMessageManager *serialPortMessageManager
				, NamedCallback_t *namedCallback
				, SetupCallerInterface *setupCallerInterface )
				: LocalDataItem<T, COUNT>( name, initialValue, namedCallback, setupCallerInterface)
				, SerialMessageInterface<T, COUNT>(rxTxType, rate, serialPortMessageManager)
		{
			ESP_LOGD("DataItem", "DataItem Instantiated: Constructor 3");
		}

		DataItem( const std::string name
				, const T& initialValue
				, const RxTxType_t rxTxType
				, const uint16_t rate
				, SerialPortMessageManager *serialPortMessageManager
				, NamedCallback_t *namedCallback
				, SetupCallerInterface *setupCallerInterface )
				: LocalDataItem<T, COUNT>( name, initialValue, namedCallback, setupCallerInterface)
				, SerialMessageInterface<T, COUNT>(rxTxType, rate, serialPortMessageManager)				
		{
			ESP_LOGD("DataItem", "DataItem Instantiated: Constructor 4");
		}
		
		DataItem( const std::string name
				, const T* initialValue
				, const RxTxType_t rxTxType
				, const uint16_t rate
				, SerialPortMessageManager *serialPortMessageManager
				, NamedCallback_t *namedCallback
				, SetupCallerInterface *setupCallerInterface
				, ValidStringValues_t *validStringValues )
				: LocalDataItem<T, COUNT>( name, initialValue, namedCallback, setupCallerInterface, validStringValues)
				, SerialMessageInterface<T, COUNT>(rxTxType, rate, serialPortMessageManager)
		{
			ESP_LOGD("DataItem", "DataItem Instantiated: Constructor 5");
		}

		DataItem( const std::string name
				, const T& initialValue
				, const RxTxType_t rxTxType
				, const uint16_t rate
				, SerialPortMessageManager *serialPortMessageManager
				, NamedCallback_t *namedCallback
				, SetupCallerInterface *setupCallerInterface
				, ValidStringValues_t *validStringValues )
				: LocalDataItem<T, COUNT>( name, initialValue, namedCallback, setupCallerInterface, validStringValues)
				, SerialMessageInterface<T, COUNT>(rxTxType, rate, serialPortMessageManager)
		{
			ESP_LOGD("DataItem", "DataItem Instantiated: Constructor 6");
		}

		DataItem( const std::string name
				, const T& initialValue
				, const RxTxType_t rxTxType
				, const uint16_t rate
				, SerialPortMessageManager *serialPortMessageManager
				, NamedCallback_t *namedCallback
				, SetupCallerInterface *setupCallerInterface
				, bool printDelimited )
				: LocalDataItem<T, COUNT>( name, initialValue, namedCallback, setupCallerInterface, printDelimited)
				, SerialMessageInterface<T, COUNT>(rxTxType, rate, serialPortMessageManager)
		{
			ESP_LOGD("DataItem", "DataItem Instantiated: Constructor 7");
		}
		
		virtual ~DataItem() override
		{
			ESP_LOGD("DataItem::~DataItem()", "\"%s\": DataItem Freeing Memory", LocalDataItem<T,COUNT>::GetName());				
		}

		//SetupCalleeInterface
		virtual void Setup() override
		{
			ESP_LOGD("DataItem<T, COUNT>::Setup()", "\"%s\": Allocating Memory", LocalDataItem<T,COUNT>::GetName());
			LocalDataItem<T, COUNT>::Setup();
			SerialMessageInterface<T, COUNT>::Setup();
		}

		//SerialMessageInterface
		virtual size_t GetCount() const override
		{
			return LocalDataItem<T, COUNT>::GetCount();
		}
		virtual size_t GetChangeCount() const override
		{
			return LocalDataItem<T, COUNT>::GetChangeCount();
		}
		virtual T* GetValuePointer() const override
		{
			return LocalDataItem<T, COUNT>::GetValuePointer();
		}

		virtual bool EqualsValue(T *object, size_t count) const override
		{
			return LocalDataItem<T, COUNT>::EqualsValue(object, count);
		}

		virtual std::string GetName() const override
		{
			return LocalDataItem<T, COUNT>::GetName();
		}

		virtual std::string GetValueAsString() const override
		{
			return LocalDataItem<T, COUNT>::GetValueAsString();
		}

		virtual DataType_t GetDataType() override
		{
			return LocalDataItem<T, COUNT>::GetDataType();
		}
		
		virtual std::string ConvertValueToString(const T *pvalue, size_t count) const override
		{
			return LocalDataItem<T, COUNT>::ConvertValueToString(pvalue, count);
		}

		virtual size_t ParseStringValueIntoValues(const std::string& stringValue, T* values) override
		{
			return LocalDataItem<T, COUNT>::ParseStringValueIntoValues(stringValue, values);
		}

		virtual UpdateStatus_t SetValue(const T* values, size_t count) override
		{
			ESP_LOGD("SetValue", "Name: \"%s\" SetValue: \"%s\"", this->GetName(), this->ConvertValueToString(values, count).c_str() );
			return this->Set_Tx_Value(values, count);
		}

		virtual UpdateStatus_t SetValue(const T& value) override
		{
			assert(COUNT == 1);
			return this->SetValue(&value, 1);
		}

		virtual UpdateStatus_t SetValueFromString(const std::string& stringValue) override
		{
			ESP_LOGD( "DataItem::SetValueFromString"
					, "Name: \"%s\" String Value: \"%s\""
					, this->GetName()
					, stringValue.c_str() );
			T values[COUNT];
			if(ParseStringValueIntoValues(stringValue, values) == COUNT)
			{
				ESP_LOGD("SetValueFromString", "Name: \"%s\" Setting Tx Value: \"%s\"", this->GetName(), stringValue.c_str() );
				return this->Set_Tx_Value(values, COUNT);
			}
			else
			{
				ESP_LOGE("SetValueFromString", "Name: \"%s\" Count Error!", this->GetName() );
				return UpdateStatus_t();
			}
		}

		virtual bool ConfirmValueValidity(const T* values, size_t count) const override
		{
			return LocalDataItem<T, COUNT>::ConfirmValueValidity(values, count);
		}

		virtual UpdateStatus_t UpdateStore(const T *newValues, const size_t changeCount) override
		{
			return LocalDataItem<T,COUNT>::UpdateStore(newValues, changeCount);
		}
};