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

class StringDataItem: public DataItem<char, DATAITEM_STRING_LENGTH>
{
	public:
		StringDataItem( const String name
					  , const String &initialValue
					  , const RxTxType_t rxTxType
					  , const UpdateStoreType_t updateStoreType
					  , const uint16_t rate
					  , SerialPortMessageManager &serialPortMessageManager
					  , NamedCallback_t *namedCallback
					  , SetupCallerInterface *setupCallerInterface )
					  : DataItem<char, DATAITEM_STRING_LENGTH>( name
															  , initialValue.c_str()
															  , rxTxType
															  , updateStoreType
															  , rate
															  , serialPortMessageManager
															  , namedCallback
															  , setupCallerInterface )
		{
		  
		}

		virtual ~StringDataItem()
		{
			ESP_LOGI("StringDataItem::~StringDataItem()", "\"%s\": Freeing Memory", m_Name.c_str());
		}

		virtual bool GetStringInitialValue(String &stringValue) override
		{
			if(mp_InitialValue)
			{
				stringValue = String(mp_InitialValue);
				ESP_LOGD("GetStringInitialValue", "\"%s\": GetStringInitialValue: \"%s\"", m_Name.c_str(), stringValue.c_str());
				return true;
			}
			else
			{
				ESP_LOGE("GetValueAsString", "\"%s\": NULL Pointer!", m_Name.c_str());
				return false;
			}
		}

		virtual bool GetStringValue(String &stringValue) override
		{
			if(mp_Value)
			{
				stringValue = String(mp_Value);
				ESP_LOGD("GetStringValue"
						, "\"%s\": GetStringValue: %s"
						, m_Name.c_str()
						, stringValue.c_str());
				return true;
			}
			else
			{
				ESP_LOGE("GetStringValue", "\"%s\": NULL Pointer!", m_Name.c_str());
				return false;
			}
		}

		virtual String& GetValueString() override
		{
			if(!GetStringValue(m_StringValue))
			{
				m_StringValue = "";
			}
			return m_StringValue;
		}

		virtual String GetValueAsString() override
		{
			String value;
			if(!GetStringValue(value))
			{
				value = "";
			}
			return value;
		}

		virtual bool SetValueFromString(const String& stringValue) override
		{
			assert(stringValue.length() <= DATAITEM_STRING_LENGTH && "String too long!");
			ESP_LOGD("StringDataItem::SetValueFromString"
					, "\"%s\": String Value: \"%s\""
					, m_Name.c_str()
					, stringValue.c_str());
			return SetValue(stringValue.c_str(), DATAITEM_STRING_LENGTH);
		}

		virtual bool SetValue(const char* value, size_t count) override
		{
			assert(value != nullptr && "Value must not be null");
			assert(mp_Value != nullptr && "mp_Value must not be null");
			String newValue = String(value);
			String currentValue = String(mp_TxValue);
			assert(newValue.length() <= count);
			bool valueChanged = !newValue.equals(currentValue);
			if(valueChanged)
			{	
				ESP_LOGD( "DataItem: SetValue"
						, "\"%s\" Set Value: \"%s\""
						, m_Name.c_str()
						, newValue.c_str() );
				this->ZeroOutCharArray(mp_TxValue);
				strcpy(mp_TxValue, value);
				this->DataItem_Try_TX_On_Change();
			}
			return valueChanged;
		}

	protected:
		bool DataItem_TX_Now()
		{
			bool valueUpdated = false;
			if(m_SerialPortMessageManager.QueueMessageFromData(m_Name, DataType_Char_t, mp_TxValue, DATAITEM_STRING_LENGTH))
			{
				if(strcmp(mp_Value, mp_TxValue) != 0)
				{
					if(m_UpdateStoreType == UpdateStoreType_On_Tx)
					{
						ZeroOutCharArray(mp_Value);
						strcpy(mp_Value, mp_TxValue);
						valueUpdated = true;
						++m_ValueChangeCount;
						CallCallbacks(m_Name, mp_Value);
					}
				}
				ESP_LOGD("DataItem: DataItem_TX_Now", "TX: \"%s\" Value: \"%s\"", this->m_Name.c_str(), GetValueAsStringForDataType(mp_TxValue, GetDataTypeFromTemplateType<T>(), COUNT, "").c_str());
			}
			else
			{
				ESP_LOGE("DataItem: DataItem_TX_Now", "Data Item: \"%s\": Unable to Tx Message", m_Name.c_str());
			}
			return valueUpdated;
		}

		bool NewRXValueReceived(void* object, size_t count)
		{ 
			bool valueUpdated = false;
			char* receivedValue = (char*)object;
			if(strcmp(mp_RxValue, receivedValue) != 0)
			{
				ZeroOutCharArray(mp_RxValue);
				strcpy(mp_RxValue, receivedValue);
				ESP_LOGI( "DataItem: NewRXValueReceived"
						, "\"%s\" New RX Value Received: \"%s\""
						, m_Name.c_str()
						, receivedValue );
				if( UpdateStoreType_On_Rx == m_UpdateStoreType &&
					strcmp(mp_Value, mp_RxValue) != 0 )
				{
					ZeroOutCharArray(mp_Value);
					strcpy(mp_Value, mp_RxValue);
					++m_ValueChangeCount;
					valueUpdated = true;
					CallCallbacks(m_Name, mp_Value);
				}
			}
			if(RxTxType_Rx_Echo_Value == m_RxTxType)
			{
				ZeroOutCharArray(mp_TxValue);
				strcpy(mp_TxValue, mp_RxValue);
				DataItem_TX_Now();
			}
			return valueUpdated;
		}

		void ZeroOutCharArray(char* pChar)
		{
			for (size_t i = 0; i < LocalDataItem::GetCount(); ++i)
			{
				pChar[i] = '\0';
			}
		}
};