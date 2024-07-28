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
#include "DataItem/SerialDataLinkInterface.h"

class StringDataItem: public LocalStringDataItem
			  		, public SerialDataLinkIntertface<char, DATAITEM_STRING_LENGTH>
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
					  , SerialDataLinkIntertface<char, DATAITEM_STRING_LENGTH>(rxTxType, updateStoreType, rate, serialPortMessageManager)
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
			SerialDataLinkIntertface<char, DATAITEM_STRING_LENGTH>::Setup();
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

		//SerialDataLinkIntertface
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
			bool valueChanged = LocalStringDataItem::SetValue(value, count);
			if(valueChanged)
			{
				DataItem_Try_TX_On_Change();
			}
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
		
		virtual bool NewRxValueReceived(void* object, size_t count)
		{ 
			bool valueUpdated = false;
			char* receivedValue = (char*)object;
			if(strcmp(this->mp_RxValue, receivedValue) != 0)
			{
				ZeroOutCharArray(this->mp_RxValue);
				strcpy(mp_RxValue, receivedValue);
				ESP_LOGI( "DataItem: NewRxValueReceived"
						, "\"%s\" New RX Value Received: \"%s\""
						, m_Name.c_str()
						, receivedValue );
				if( UpdateStoreType_On_Rx == this->m_UpdateStoreType &&
					strcmp(mp_Value, mp_RxValue) != 0 )
				{
					ZeroOutCharArray(mp_Value);
					strcpy(mp_Value, mp_RxValue);
					++m_ValueChangeCount;
					valueUpdated = true;
					CallCallbacks(m_Name, mp_Value);
				}
			}
			if(RxTxType_Rx_Echo_Value == this->m_RxTxType)
			{
				ZeroOutCharArray(this->mp_TxValue);
				strcpy(this->mp_TxValue, this->mp_RxValue);
				DataItem_TX_Now();
			}
			return valueUpdated;
		}

	protected:
	
		void Periodic_TX()
		{
			DataItem_TX_Now();
		}

		void DataItem_Try_TX_On_Change()
		{
			ESP_LOGI("DataItem& DataItem_Try_TX_On_Change", "Data Item: \"%s\": Try TX On Change", LocalDataItem<char, DATAITEM_STRING_LENGTH>::GetName().c_str());
			if(this->m_RxTxType == RxTxType_Tx_On_Change || this->m_RxTxType == RxTxType_Tx_On_Change_With_Heartbeat)
			{
				DataItem_TX_Now();
			}
		}

		virtual bool DataItem_TX_Now()
		{
			bool valueUpdated = false;
			if(this->mp_SerialPortMessageManager->QueueMessageFromData(m_Name, DataType_Char_t, this->mp_TxValue, DATAITEM_STRING_LENGTH))
			{
				if(strcmp(mp_Value, this->mp_TxValue) != 0)
				{
					if(this->m_UpdateStoreType == UpdateStoreType_On_Tx)
					{
						ZeroOutCharArray(mp_Value);
						strcpy(mp_Value, this->mp_TxValue);
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

		void ZeroOutCharArray(char* pChar)
		{
			for (size_t i = 0; i < LocalDataItem::GetCount(); ++i)
			{
				pChar[i] = '\0';
			}
		}
};