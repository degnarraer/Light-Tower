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
					 	   , const char* initialValue
					 	   , NamedCallback_t *namedCallback )
						   : LocalDataItem<char, DATAITEM_STRING_LENGTH>( name, initialValue, namedCallback, false )
						   {

						   }
		
		LocalStringDataItem( const String name
					 	   , const char& initialValue
					 	   , NamedCallback_t *namedCallback )
						   : LocalDataItem<char, DATAITEM_STRING_LENGTH>( name, initialValue, namedCallback, false )
						   {

						   }
		virtual ~LocalStringDataItem()
		{
			ESP_LOGI("LocalStringDataItem::~LocalStringDataItem()", "\"%s\": Freeing Memory", m_Name.c_str());
		}

		bool SetValue(const char* value, size_t count)
		{
			assert(value != nullptr && "Value must not be null");
			assert(mp_Value != nullptr && "mp_Value must not be null");
			String newValue = String(value);
			assert(newValue.length() <= count);
			ESP_LOGI( "DataItem: SetValue"
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