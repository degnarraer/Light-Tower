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

#define DATAITEM_STRING_LENGTH 50

class LocalStringDataItem: public LocalDataItem<char, DATAITEM_STRING_LENGTH>
{
	public:
		LocalStringDataItem( const std::string name
					 	   , const std::string &initialValue
					 	   , NamedCallback_t *namedCallback
					 	   , SetupCallerInterface *setupCallerInterface )
						   : LocalDataItem<char, DATAITEM_STRING_LENGTH>( name, initialValue.c_str(), namedCallback, setupCallerInterface )
						   {
						   }

		virtual ~LocalStringDataItem() override
		{
			ESP_LOGI("LocalStringDataItem::~LocalStringDataItem()", "\"%s\": Freeing Memory", m_Name.c_str());
		}

		virtual size_t GetCount() const override
		{
			return LocalDataItem<char, DATAITEM_STRING_LENGTH>::GetCount();
		}

		virtual size_t GetChangeCount() const override
		{
			return LocalDataItem<char, DATAITEM_STRING_LENGTH>::GetChangeCount();
		}

		virtual bool GetInitialValueAsString(String &stringValue) const override
		{
			std::lock_guard<std::recursive_mutex> lock(this->m_ValueMutext);
			if(mp_InitialValue)
			{
				stringValue = String(mp_InitialValue);
				ESP_LOGD("GetInitialValueAsString", "\"%s\": GetInitialValueAsString: \"%s\"", m_Name.c_str(), stringValue.c_str());
				return true;
			}
			else
			{
				ESP_LOGE("GetValueAsString", "ERROR! \"%s\": NULL Pointer.", m_Name.c_str());
				return false;
			}
		}

		virtual String GetInitialValueAsString() const
		{
			std::lock_guard<std::recursive_mutex> lock(this->m_ValueMutext);
			String value;
			if(!GetInitialValueAsString(value))
			{
				ESP_LOGE("GetInitialValueAsString", "ERROR! \"%s\": Unable to Get String Value! Returning Empty String.", m_Name.c_str());
				value = "";
			}
			return value;
		}

		virtual bool GetValueAsString(String &stringValue) const override
		{
			std::lock_guard<std::recursive_mutex> lock(this->m_ValueMutext);
			if(mp_Value)
			{
				stringValue = String(mp_Value);
				ESP_LOGD("GetValueAsString"
						, "\"%s\": GetValueAsString: %s"
						, m_Name.c_str()
						, stringValue.c_str());
				return true;
			}
			else
			{
				return false;
			}
		}

		virtual String GetValueAsString() const override
		{
			std::lock_guard<std::recursive_mutex> lock(this->m_ValueMutext);
			String value;
			if(!GetValueAsString(value))
			{
				value = "";
			}
			return value;
		}

		virtual bool SetValueFromString(const String& stringValue) override
		{
			std::lock_guard<std::recursive_mutex> lock(this->m_ValueMutext);
			ESP_LOGD("LocalStringDataItem::SetValueFromString"
					, "Name: \"%s\" String Value: \"%s\""
					, m_Name.c_str()
					, stringValue.c_str());
			return SetValue(stringValue.c_str(), stringValue.length());
		}

		virtual bool SetValue(const char* value, size_t count) override
		{
			std::lock_guard<std::recursive_mutex> lock(this->m_ValueMutext);
			assert(value != nullptr);
			assert(mp_Value != nullptr);
			assert(count <= DATAITEM_STRING_LENGTH);
			String newValue = String(value);
			ESP_LOGD( "DataItem: SetValue"
					, "\"%s\" Set Value: \"%s\""
					, m_Name.c_str()
					, newValue.c_str() );
			bool valueChanged = false;
			if(0 != strcmp(mp_Value, value))
			{
				valueChanged = true;
			}
			bool validValue = ConfirmValueValidity(value, count);
			bool updateAllowed = UpdateChangeCount(GetChangeCount(), (valueChanged && validValue));
			bool valueUpdateAllowed = updateAllowed & validValue;
			if(valueUpdateAllowed)
			{	
				ZeroOutMemory(mp_Value);
				strcpy(mp_Value, value);
				ESP_LOGI( "LocalDataItem: SetValue", "Set Value to \"%s\"", newValue.c_str());
				this->CallNamedCallbacks(mp_Value);
			}
			return valueUpdateAllowed;
		}

		virtual bool ConfirmValueValidity(const char *values, size_t count) const override
		{
			return LocalDataItem<char, DATAITEM_STRING_LENGTH>::ConfirmValueValidity(values, count);
		}

		virtual String ConvertValueToString(const char* pvalue, size_t count) const override
		{
			return String(pvalue);
		}
};