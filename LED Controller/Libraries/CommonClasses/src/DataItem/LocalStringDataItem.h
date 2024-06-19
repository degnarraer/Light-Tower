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
		LocalStringDataItem( const String name
					 	   , const String &initialValue
					 	   , NamedCallback_t *namedCallback
					 	   , SetupCallerInterface *setupCallerInterface )
						   : LocalDataItem<char, DATAITEM_STRING_LENGTH>( name, initialValue.c_str(), namedCallback, setupCallerInterface, NULL )
						   {
						   }

		virtual ~LocalStringDataItem()
		{
			ESP_LOGI("LocalStringDataItem::~LocalStringDataItem()", "\"%s\": Freeing Memory", m_Name.c_str());
		}

		virtual bool GetInitialValueAsString(String &stringValue) const override
		{
			if(mp_InitialValue)
			{
				stringValue = String(mp_InitialValue);
				ESP_LOGD("GetInitialValueAsString", "\"%s\": GetInitialValueAsString: \"%s\"", m_Name.c_str(), stringValue.c_str());
				return true;
			}
			else
			{
				ESP_LOGE("GetValueAsString", "\"%s\": NULL Pointer!", m_Name.c_str());
				return false;
			}
		}

		virtual String GetInitialValueAsString() const
		{
			String value;
			if(!GetInitialValueAsString(value))
			{
				ESP_LOGE("GetInitialValueAsString", "\"%s\": Unable to Get String Value! Returning Empty String.", m_Name.c_str());
				value = "";
			}
			return value;
		}

		virtual bool GetValueAsString(String &stringValue) const override
		{
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
			String value;
			if(!GetValueAsString(value))
			{
				value = "";
			}
			return value;
		}

		virtual bool SetValueFromString(const String& stringValue) override
		{
			ESP_LOGD("LocalStringDataItem::SetValueFromString"
					, "\"%s\": String Value: \"%s\""
					, m_Name.c_str()
					, stringValue.c_str());
			return SetValue(stringValue.c_str(), stringValue.length());
		}

		virtual bool SetValue(const char* value, size_t count)
		{
			assert(value != nullptr);
			assert(mp_Value != nullptr);
			assert(count <= DATAITEM_STRING_LENGTH);
			String newValue = String(value);
			ESP_LOGD( "DataItem: SetValue"
					, "\"%s\" Set Value: \"%s\""
					, m_Name.c_str()
					, newValue.c_str() );
					
			bool valueChanged = (strcmp(mp_Value, value) != 0);
			if(valueChanged)
			{	
				ZeroOutCharArray(mp_Value);
				strcpy(mp_Value, value);
				++m_ValueChangeCount;
				CallCallbacks(m_Name.c_str(), mp_Value);
			}
			return valueChanged;
		}
	protected:
		void ZeroOutCharArray(char* pChar)
		{
			for (size_t i = 0; i < this->GetCount(); ++i)
			{
				pChar[i] = '\0';
			}
		}
};