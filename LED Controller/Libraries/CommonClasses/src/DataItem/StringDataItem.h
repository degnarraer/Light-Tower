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
		StringDataItem( const String name
					  , const String &initialValue
					  , const RxTxType_t rxTxType
					  , const UpdateStoreType_t updateStoreType
					  , const uint16_t rate
					  , SerialPortMessageManager *serialPortMessageManager
					  , NamedCallback_t *namedCallback
					  , SetupCallerInterface *setupCallerInterface )
					  : LocalStringDataItem( name
										   , initialValue
										   , namedCallback
										   , setupCallerInterface )
					  , SerialMessageInterface<char, DATAITEM_STRING_LENGTH>(rxTxType, updateStoreType, rate, serialPortMessageManager)
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

		//SerialMessageInterface
		virtual char* GetValuePointer() const override
		{
			return LocalStringDataItem::GetValuePointer();
		}
		virtual bool EqualsValue(char *object, size_t count) const override
		{
			return LocalStringDataItem::EqualsValue(object, count);
		}
		virtual String GetValueAsString() const override
		{
			return LocalStringDataItem::GetValueAsString();
		}
		virtual DataType_t GetDataType() override
		{
			return LocalStringDataItem::GetDataType();
		}
		
		virtual bool SetValue(const char* value, size_t count) override
		{
			bool valueChanged = Set_Tx_Value(value, count);
			return valueChanged;
		}

		virtual bool GetInitialValueAsString(String &stringValue) const override
		{
			if(this->mp_InitialValue)
			{
				stringValue = String(this->mp_InitialValue);
				ESP_LOGD("GetInitialValueAsString", "\"%s\": GetInitialValueAsString: \"%s\"", this->m_Name.c_str(), stringValue.c_str());
				return true;
			}
			else
			{
				ESP_LOGE("GetValueAsString", "ERROR! \"%s\": NULL Pointer.", this->m_Name.c_str());
				return false;
			}
		}

		String GetInitialValueAsString() const override
		{
			String value;
			if(!GetInitialValueAsString(value))
			{
				ESP_LOGE("GetInitialValueAsString", "ERROR! \"%s\": Unable to Get String Value! Returning Empty String.", m_Name.c_str());
				value = "";
			}
			return value;
		}

		virtual bool SetValueFromString(const String& stringValue) override
		{
			assert(stringValue.length() <= DATAITEM_STRING_LENGTH);
			ESP_LOGD("StringDataItem::SetValueFromString"
					, "Name: \"%s\" String Value: \"%s\""
					, this->m_Name.c_str()
					, stringValue.c_str());
			return SetValue(stringValue.c_str(), DATAITEM_STRING_LENGTH);
		}

	protected:

		void ZeroOutCharArray(char* pChar)
		{
			for (size_t i = 0; i < LocalDataItem::GetCount(); ++i)
			{
				pChar[i] = '\0';
			}
		}
};