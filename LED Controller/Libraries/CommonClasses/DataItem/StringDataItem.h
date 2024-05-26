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
					  , const char* initialValue
					  , const RxTxType_t rxTxType
					  , const UpdateStoreType_t updateStoreType
					  , const uint16_t rate
					  , SerialPortMessageManager &serialPortMessageManager
					  , NamedCallback_t *namedCallback )
					  : DataItem<char, DATAITEM_STRING_LENGTH>( name
															  , initialValue
															  , rxTxType
															  , updateStoreType
															  , rate
															  , serialPortMessageManager
															  , namedCallback )
		{
		  
		}
		
		StringDataItem( const String name
					  , const char& initialValue
					  , const RxTxType_t rxTxType
					  , const UpdateStoreType_t updateStoreType
					  , const uint16_t rate
					  , SerialPortMessageManager &serialPortMessageManager
					  , NamedCallback_t *namedCallback )
					  : DataItem<char, DATAITEM_STRING_LENGTH>( name
										     				  , initialValue
										     				  , rxTxType
										     				  , updateStoreType
										     				  , rate
										     				  , serialPortMessageManager
										     				  , namedCallback )
		{
		  
		}

		virtual ~StringDataItem()
		{
			ESP_LOGI("StringDataItem::~StringDataItem()", "\"%s\": Freeing Memory", m_Name.c_str());
		}

		bool SetValue(const char* Value, size_t Count)
		{
			assert(Value != nullptr && "Value must not be null");
			assert(mp_Value != nullptr && "mp_Value must not be null");
			String NewValue = String(Value);
			String CurrentValue = String(mp_TxValue);
			assert(NewValue.length() <= Count);
			bool ValueChanged = !NewValue.equals(CurrentValue);
			if(ValueChanged)
			{	
				ESP_LOGI( "DataItem: SetValue"
						, "\"%s\" Set Value: \"%s\""
						, m_Name.c_str()
						, NewValue.c_str() );
				this->ZeroOutCharArray(mp_TxValue);
				strcpy(mp_TxValue, Value);
				this->DataItem_Try_TX_On_Change();
			}
			return ValueChanged;
		}
	protected:
		bool DataItem_TX_Now()
		{
			bool ValueUpdated = false;
			if(m_SerialPortMessageManager.QueueMessageFromData(m_Name, DataType_Char_t, mp_TxValue, DATAITEM_STRING_LENGTH))
			{
				if(strcmp(mp_Value, mp_TxValue) != 0)
				{
					if(m_UpdateStoreType == UpdateStoreType_On_Tx)
					{
						ZeroOutCharArray(mp_Value);
						strcpy(mp_Value, mp_TxValue);
						ValueUpdated = true;
						++m_ValueChangeCount;
						CallCallbacks(m_Name.c_str(), mp_Value);
					}
				}
				ESP_LOGD("DataItem: DataItem_TX_Now", "TX: \"%s\" Value: \"%s\"", this->m_Name.c_str(), GetValueAsStringForDataType(mp_TxValue, GetDataTypeFromTemplateType<T>(), COUNT, "").c_str());
			}
			else
			{
				ESP_LOGE("DataItem: DataItem_TX_Now", "Data Item: \"%s\": Unable to Tx Message", m_Name.c_str());
			}
			return ValueUpdated;
		}

		bool NewRXValueReceived(void* Object, size_t Count)
		{ 
			bool ValueUpdated = false;
			char* receivedValue = (char*)Object;
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
					ValueUpdated = true;
					CallCallbacks(m_Name.c_str(), mp_Value);
				}
			}
			if(RxTxType_Rx_Echo_Value == m_RxTxType)
			{
				ZeroOutCharArray(mp_TxValue);
				strcpy(mp_TxValue, mp_RxValue);
				DataItem_TX_Now();
			}
			return ValueUpdated;
		}

		void ZeroOutCharArray(char* pChar)
		{
			for (size_t i = 0; i < LocalDataItem::GetCount(); ++i)
			{
				pChar[i] = '\0';
			}
		}
};