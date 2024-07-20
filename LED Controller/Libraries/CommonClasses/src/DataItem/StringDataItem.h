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

class StringDataItem: public DataItem<char, DATAITEM_STRING_LENGTH>
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

		virtual ~StringDataItem() override
		{
			ESP_LOGI("StringDataItem::~StringDataItem()", "\"%s\": Freeing Memory", m_Name.c_str());
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
				ESP_LOGE("GetValueAsString", "ERROR! \"%s\": NULL Pointer.", m_Name.c_str());
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
				ESP_LOGE("GetValueAsString", "ERROR! \"%s\": NULL Pointer.", m_Name.c_str());
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
			assert(stringValue.length() <= DATAITEM_STRING_LENGTH);
			ESP_LOGD("StringDataItem::SetValueFromString"
					, "Name: \"%s\" String Value: \"%s\""
					, m_Name.c_str()
					, stringValue.c_str());
			return SetValue(stringValue.c_str(), DATAITEM_STRING_LENGTH);
		}

		virtual bool SetValue(const char* value, size_t count) override
		{
			assert(value != nullptr);
			assert(mp_Value != nullptr);
			assert(count <= DATAITEM_STRING_LENGTH);
			bool valueChanged = LocalDataItem<char, DATAITEM_STRING_LENGTH>::SetValue(value, DATAITEM_STRING_LENGTH);
			if(valueChanged)
			{
				DataItem_Try_TX_On_Change();
			}
			return valueChanged;
		}

	protected:
		virtual bool DataItem_TX_Now()
		{
			bool valueUpdated = false;
			if(mp_SerialPortMessageManager->QueueMessageFromData(m_Name, DataType_Char_t, mp_TxValue, DATAITEM_STRING_LENGTH))
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
				ESP_LOGD("DataItem: DataItem_TX_Now", "TX: \"%s\" Value: \"%s\"", this->m_Name.c_str(), GetValueAsString().c_str());
			}
			else
			{
				ESP_LOGE("DataItem: DataItem_TX_Now", "ERROR! Data Item: \"%s\": Unable to Tx Message.", m_Name.c_str());
			}
			return valueUpdated;
		}

		virtual bool NewRxValueReceived(void* object, size_t count) override
		{ 
			bool valueUpdated = false;
			char* receivedValue = (char*)object;
			if(strcmp(mp_RxValue, receivedValue) != 0)
			{
				ZeroOutCharArray(mp_RxValue);
				strcpy(mp_RxValue, receivedValue);
				ESP_LOGI( "DataItem: NewRxValueReceived"
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