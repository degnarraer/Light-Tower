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

#include "DataItem/DataItem.h"
#include "DataItem/LocalStringDataItem.h"
#include "DataItem/SerialMessageInterface.h"

class StringDataItem: public LocalStringDataItem
			  		, public SerialMessageInterface<char, DATAITEM_STRING_LENGTH>
{
	public:
		StringDataItem( const std::string name
					  , const std::string &initialValue
					  , const RxTxType_t rxTxType
					  , const uint16_t rate
					  , SerialPortMessageManager *serialPortMessageManager
					  , NamedCallback_t *namedCallback
					  , SetupCallerInterface *setupCallerInterface )
					  : LocalStringDataItem( name
										   , initialValue
										   , namedCallback
										   , setupCallerInterface )
					  , SerialMessageInterface<char, DATAITEM_STRING_LENGTH>(rxTxType, rate, serialPortMessageManager)
		{
		  
		}

		virtual ~StringDataItem() override
		{
			ESP_LOGI("DataItem::~DataItem()", "\"%s\": DataItem Freeing Memory", LocalStringDataItem::GetName().c_str());				
		}

		virtual void Setup() override
		{
			ESP_LOGD("DataItem<T, COUNT>::Setup()", "\"%s\": Allocating Memory", LocalStringDataItem::GetName().c_str());
			LocalStringDataItem::Setup();
			SerialMessageInterface<char, DATAITEM_STRING_LENGTH>::Setup();
		}

		//DataItemInterface
		virtual String GetName() const override
		{
			return LocalStringDataItem::GetName();
		}
		virtual size_t GetCount() const override
		{
			return LocalStringDataItem::GetCount();
		}

		virtual size_t GetChangeCount() const override
		{
			std::lock_guard<std::recursive_mutex> lock(this->m_ValueMutext);
			return LocalStringDataItem::GetChangeCount();
		}

		//SerialMessageInterface
		virtual char* GetValuePointer() const override
		{
			return LocalStringDataItem::GetValuePointer();
		}
		virtual bool EqualsValue(char *object, size_t count) const override
		{
			std::lock_guard<std::recursive_mutex> lock(this->m_ValueMutext);
			return LocalStringDataItem::EqualsValue(object, count);
		}
		virtual String GetValueAsString() const override
		{
			std::lock_guard<std::recursive_mutex> lock(this->m_ValueMutext);
			return LocalStringDataItem::GetValueAsString();
		}
		virtual DataType_t GetDataType() override
		{
			return LocalStringDataItem::GetDataType();
		}
		bool UpdateStore(const char *newValues, const size_t changeCount)
		{
			std::lock_guard<std::recursive_mutex> lock(this->m_ValueMutext);
			return LocalStringDataItem::UpdateStore(newValues, changeCount);
		}
		String ConvertValueToString(const char *values, size_t count) const
		{
			std::lock_guard<std::recursive_mutex> lock(this->m_ValueMutext);
			return LocalStringDataItem::ConvertValueToString(values, count);
		}
		size_t ParseStringValueIntoValues(const String& stringValue, char *values)
		{
			std::lock_guard<std::recursive_mutex> lock(this->m_ValueMutext);
			return LocalStringDataItem::ParseStringValueIntoValues(stringValue, values);
		}
		virtual bool SetValue(const char* value, size_t count) override
		{
			std::lock_guard<std::recursive_mutex> lock(this->m_ValueMutext);
			bool valueChanged = Set_Tx_Value(value, count);
			return valueChanged;
		}

		virtual bool GetInitialValueAsString(String &stringValue) const override
		{
			std::lock_guard<std::recursive_mutex> lock(this->m_ValueMutext);
			return LocalStringDataItem::GetInitialValueAsString(stringValue);
		}

		String GetInitialValueAsString() const override
		{
			std::lock_guard<std::recursive_mutex> lock(this->m_ValueMutext);
			return LocalStringDataItem::GetInitialValueAsString();
		}

		virtual bool SetValueFromString(const String& stringValue) override
		{
			std::lock_guard<std::recursive_mutex> lock(this->m_ValueMutext);
			assert(stringValue.length() <= DATAITEM_STRING_LENGTH);
			ESP_LOGD("StringDataItem::SetValueFromString"
					, "Name: \"%s\" String Value: \"%s\""
					, this->m_Name.c_str()
					, stringValue.c_str());
			return this->Set_Tx_Value(stringValue.c_str(), stringValue.length());
		}
		
		virtual bool ConfirmValueValidity(const char *values, size_t count) const override
		{
			return LocalStringDataItem::ConfirmValueValidity(values, count);
		}
};