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

#include <Helpers.h>
#include <Preferences.h>
#include <nvs_flash.h>
#include <esp_timer.h>
#include <Arduino.h>
#include <esp_heap_caps.h>
#include "SerialMessageManager.h"
#define DATAITEM_STRING_LENGTH 50
#define DIVIDER "|"

enum RxTxType_t
{
	RxTxType_Tx_Periodic,
	RxTxType_Tx_On_Change,
	RxTxType_Tx_On_Change_With_Heartbeat,
	RxTxType_Rx_Only,
	RxTxType_Rx_Echo_Value,
	RxTxType_Count
};

enum UpdateStoreType_t
{
	UpdateStoreType_On_Tx,
	UpdateStoreType_On_Rx,
	UpdateStoreType_Count
};

template <typename T, size_t COUNT>
class LocalDataItem: public NamedCallbackInterface<T>
				   , public SetupCalleeInterface
				   , public DataTypeFunctions
{
	public:
		LocalDataItem( const String name
					 , const T* initialValue
					 , NamedCallback_t *namedCallback )
					 : m_Name(name)
					 , mp_InitialValuePtr(initialValue)
		{}
		
		LocalDataItem( const String name
					 , const T& initialValue
					 , NamedCallback_t *namedCallback )
					 : m_Name(name)
					 , mp_InitialValuePtr(&initialValue)
		{}
		
		virtual ~LocalDataItem()
		{
			ESP_LOGI("DataItem<T, COUNT>::Setup()", "\"%s\": Freeing Memory", m_Name.c_str());
			if(mp_NamedCallback) this->DeRegisterNamedCallback(mp_NamedCallback);
			heap_caps_free(mp_Value);
			heap_caps_free(mp_InitialValue);
		}
		virtual void Setup()
		{
			ESP_LOGI("DataItem<T, COUNT>::Setup()", "\"%s\": Allocating Memory", m_Name.c_str());
			if(mp_NamedCallback) this->RegisterNamedCallback(mp_NamedCallback);
			mp_Value = (T*)heap_caps_malloc(sizeof(T)*COUNT, MALLOC_CAP_SPIRAM);
			mp_InitialValue = (T*)heap_caps_malloc(sizeof(T)*COUNT, MALLOC_CAP_SPIRAM);
			if (mp_Value && mp_InitialValue && mp_InitialValuePtr)
			{
				if (std::is_same<T, char>::value)
				{
					String InitialValue = String((char*)mp_InitialValuePtr);
					ESP_LOGI( "DataItem<T, COUNT>::Setup()", "\"%s\": Setting initial value: \"%s\""
							, m_Name.c_str()
							, InitialValue.c_str());
					for (size_t i = 0; i < COUNT; ++i)
					{
						char value;
						memcpy(&value, mp_InitialValuePtr+i, sizeof(char));
						if (i >= InitialValue.length())
						{
							value = '\0';
						}
						memcpy(mp_Value+i, &value, sizeof(char));
						this->CallCallbacks(m_Name.c_str(), mp_Value);
						memcpy(mp_InitialValue+i, &value, sizeof(char));
					}
				}
				else
				{
					ESP_LOGI( "DataItem<T, COUNT>::Setup()", "\"%s\": Setting initial value: \"%s\""
							, m_Name.c_str()
							, GetValueAsString().c_str());
					for (size_t i = 0; i < COUNT; ++i)
					{
						memcpy(mp_Value+i, mp_InitialValuePtr, sizeof(T));
						this->CallCallbacks(m_Name.c_str(), mp_Value);
						memcpy(mp_InitialValue+i, mp_InitialValuePtr, sizeof(T));
					}
				}
			}
			else
			{
				ESP_LOGE("DataItem<T, COUNT>::Setup()", "Failed to allocate memory on SPI RAM");
			}
		}
		String GetName()
		{
			return m_Name;
		}
		size_t GetCount()
		{
			return m_Count;
		}
		size_t GetChangeCount()
		{
			return m_ValueChangeCount;
		}
		void GetValue(void* Object, size_t Count)
		{
			assert(Count == COUNT && "Counts must be equal");
			if(mp_Value)
			{
				memcpy(Object, mp_Value, sizeof(T)*Count);
			}
			else
			{
				ESP_LOGE("GetValueAsString", "NULL Pointer!");
				*reinterpret_cast<T**>(Object) = nullptr;
			}
		}

		T* GetValuePointer()
		{
			if(!mp_Value)
			{
				ESP_LOGE("GetValueAsString", "NULL Pointer!");
			}
			return mp_Value;
		}

		T GetValue()
		{
			assert(1 == COUNT && "Count must 1 to use this function");
			if(mp_Value)
			{
				return static_cast<T>(*mp_Value);
			}
			else
			{
				ESP_LOGE("GetValueAsString", "NULL Pointer!");
				return T();
			}
		}

		bool GetStringInitialValue(String &stringValue)
		{
			stringValue = String(*mp_InitialValue);
			return true;
		}
		
		virtual String GetInitialValueAsString()
		{
			String value;
			if(!GetStringInitialValue(value))
			{
				value = "";
			}
			return value;
		}

		bool GetStringValue(String &stringValue)
		{
			if (COUNT == 0)
				return false;

			std::vector<String> valueStrings;

			for (size_t i = 0; i < COUNT; ++i)
			{
				valueStrings.push_back(String(mp_Value[i]));
			}

			stringValue = "";
			
			for (size_t i = 0; i < COUNT - 1; ++i)
			{
				stringValue += valueStrings[i] + DIVIDER;
			}
			stringValue += valueStrings[COUNT - 1];

			return true;
		}

		String& GetValueString()
		{
			if(!GetStringValue(m_StringValue))
			{
				m_StringValue = "";
			}
			return m_StringValue;
		}

		String GetValueAsString()
		{
			String value;
			if(!GetStringValue(value))
			{
				value = "";
			}
			return value;
		}

		bool SetValueFromString(const String& stringValue)
		{
			T value[COUNT];
			std::vector<String> substrings;
			size_t start = 0;
			size_t end = stringValue.indexOf(DIVIDER);
			while (end != -1)
			{
				substrings.push_back(stringValue.substring(start, end - start));
				start = end + 1;
				end = stringValue.indexOf(DIVIDER, start);
			}
			assert(substrings.size() == COUNT && "String did not parse to expected length");
			for (size_t i = 0; i < COUNT; ++i)
			{
				ESP_LOGI("SetValueFromString", "SetValueFromString: %s", substrings[i].c_str());
				value[i] = decodeFromString(substrings[i]);
			}
			return SetValue(value, COUNT);
		}

		T decodeFromString(String str) {
			std::string stdStr = str.c_str();
			std::istringstream iss(stdStr);
			T value;
			iss >> value;
			return value;
		}
		
		bool SetValue(const T *Value, size_t Count)
		{
			assert(Value != nullptr && "Value must not be null");
			assert(mp_Value != nullptr && "mp_Value must not be null");
			assert(COUNT > 0 && "COUNT must be a valid index range for mp_Value");
			assert(COUNT == Count && "Counts must match");
			ESP_LOGD( "LocalDataItem: SetValue"
					, "\"%s\" Set Value: \"%s\""
					, m_Name.c_str()
					, GetValueAsStringForDataType(Value, GetDataTypeFromTemplateType<T>(), COUNT, "").c_str());
			
			bool ValueChanged = (memcmp(mp_Value, &Value, sizeof(T) * COUNT) != 0);
			if(ValueChanged)
			{
				memcpy(mp_Value, &Value, sizeof(T) * COUNT);
				++m_ValueChangeCount;
				this->CallCallbacks(m_Name.c_str(), mp_Value);
			}
			return ValueChanged;
		}

		virtual bool SetValue(T Value)
		{
			assert(COUNT == 1 && "COUNT must be 1 to use this");
			assert(mp_Value != nullptr && "mp_Value must not be null");
			ESP_LOGD( "LocalDataItem: SetValue"
					, "\"%s\" Set Value: \"%s\""
					, this->m_Name.c_str()
					, GetValueAsStringForDataType(Value, GetDataTypeFromTemplateType<T>(), COUNT, "").c_str());
			
			bool ValueChanged = (memcmp(mp_Value, &Value, sizeof(T) * COUNT) != 0);
			if(ValueChanged)
			{
				memcpy(mp_Value, &Value, sizeof(T) * COUNT);
				++this->m_ValueChangeCount;
				this->CallCallbacks(this->m_Name.c_str(), mp_Value);
			}
			return ValueChanged;
		}
		bool EqualsValue(T *Object, size_t Count)
		{
			assert(Count == COUNT && "Counts must equal");
			return (memcmp(mp_Value, Object, Count) == 0);
		}
	protected:
		const String m_Name;
		const T *mp_InitialValuePtr;
		T *mp_Value;
		String m_StringValue;
		T *mp_InitialValue;
		NamedCallback_t *mp_NamedCallback = NULL;
		size_t m_ValueChangeCount = 0;
	private:
		size_t m_Count = COUNT;
};

class LocalStringDataItem: public LocalDataItem<char, DATAITEM_STRING_LENGTH>
{
	public:
		LocalStringDataItem( const String name
					 	   , const char* initialValue
					 	   , NamedCallback_t *namedCallback )
						   : LocalDataItem<char, DATAITEM_STRING_LENGTH>( name, initialValue, namedCallback)
						   {

						   }
		
		LocalStringDataItem( const String name
					 	   , const char& initialValue
					 	   , NamedCallback_t *namedCallback )
						   : LocalDataItem<char, DATAITEM_STRING_LENGTH>( name, initialValue, namedCallback)
						   {

						   }
		virtual ~LocalStringDataItem()
		{
			ESP_LOGI("LocalStringDataItem::~LocalStringDataItem()", "\"%s\": Freeing Memory", m_Name.c_str());
		}

		virtual void Setup() override
		{
			LocalDataItem::Setup();
		}

		bool SetValue(const char* Value, size_t Count)
		{
			assert(Value != nullptr && "Value must not be null");
			assert(mp_Value != nullptr && "mp_Value must not be null");
			String NewValue = String(Value);
			assert(NewValue.length() <= Count);
			ESP_LOGI( "DataItem: SetValue"
					, "\"%s\" Set Value3A: \"%s\""
					, m_Name.c_str()
					, NewValue.c_str() );
					
			bool ValueChanged = (strcmp(mp_Value, Value) != 0);
			if(ValueChanged)
			{	
				ZeroOutCharArray(mp_Value);
				strcpy(mp_Value, Value);
				++m_ValueChangeCount;
				this->CallCallbacks(m_Name.c_str(), mp_Value);
			}
			return ValueChanged;
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


template <typename T, size_t COUNT>
class DataItem: public LocalDataItem<T, COUNT>
			  , public NewRxTxValueCallerInterface<T>
			  , public NewRxTxVoidObjectCalleeInterface
{
	public:
		DataItem( const String name
				, const T* initialValue
				, const RxTxType_t rxTxType
				, const UpdateStoreType_t updateStoreType
				, const uint16_t rate
				, SerialPortMessageManager &serialPortMessageManager
				, NamedCallback_t *namedCallback )
				: LocalDataItem<T, COUNT>( name, initialValue, namedCallback)
				, NewRxTxVoidObjectCalleeInterface(COUNT)
				, m_RxTxType(rxTxType)
				, m_UpdateStoreType(updateStoreType)
				, m_Rate(rate)
				, m_SerialPortMessageManager(serialPortMessageManager)
		{
			CreateTxTimer();
			m_SerialPortMessageManager.RegisterForSetupCall(this);
		}
		DataItem( const String name
				, const T& initialValue
				, const RxTxType_t rxTxType
				, const UpdateStoreType_t updateStoreType
				, const uint16_t rate
				, SerialPortMessageManager &serialPortMessageManager
				, NamedCallback_t *namedCallback )
				: LocalDataItem<T, COUNT>( name, initialValue, namedCallback)
				, NewRxTxVoidObjectCalleeInterface(COUNT)
				, m_RxTxType(rxTxType)
				, m_UpdateStoreType(updateStoreType)
				, m_Rate(rate)
				, m_SerialPortMessageManager(serialPortMessageManager)
				
		{
			CreateTxTimer();
			m_SerialPortMessageManager.RegisterForSetupCall(this);
		}
		
		virtual ~DataItem()
		{
			ESP_LOGI("DataItem::~DataItem()", "\"%s\": Freeing Memory", this->GetName().c_str());
			heap_caps_free(mp_RxValue);
			heap_caps_free(mp_TxValue);
			esp_timer_stop(this->m_TxTimer);
			esp_timer_delete(this->m_TxTimer);
			m_SerialPortMessageManager.DeRegisterForSetupCall(this);
		}
		void Setup()
		{
			ESP_LOGD("DataItem<T, COUNT>::Setup()", "\"%s\": Allocating Memory", m_Name.c_str());
			LocalDataItem<T, COUNT>::Setup();
			mp_RxValue = (T*)heap_caps_malloc(sizeof(T)*COUNT, MALLOC_CAP_SPIRAM);
			mp_TxValue = (T*)heap_caps_malloc(sizeof(T)*COUNT, MALLOC_CAP_SPIRAM);
			if (mp_RxValue && mp_TxValue && this->mp_InitialValuePtr)
			{
				if (std::is_same<T, char>::value)
				{
					String InitialValue = String((char*)this->mp_InitialValuePtr);
					ESP_LOGD( "DataItem<T, COUNT>::Setup()", "\"%s\": Setting initial value: \"%s\""
							, this->m_Name.c_str()
							, InitialValue.c_str());
					for (size_t i = 0; i < COUNT; ++i)
					{
						char value;
						memcpy(&value, this->mp_InitialValuePtr+i, sizeof(char));
						if (i >= InitialValue.length())
						{
							value = '\0';
						}
						memcpy(mp_RxValue+i, &value, sizeof(char));
						memcpy(mp_TxValue+i, &value, sizeof(char));
					}
				}
				else
				{
					ESP_LOGD( "DataItem<T, COUNT>::Setup()", "\"%s\": Setting initial value: \"%s\""
							, m_Name.c_str()
							, GetValueAsStringForDataType(mp_InitialValuePtr, GetDataTypeFromTemplateType<T>(), COUNT, "").c_str());
					for (size_t i = 0; i < COUNT; ++i)
					{
						memcpy(mp_RxValue+i, this->mp_InitialValuePtr, sizeof(T));
						memcpy(mp_TxValue+i, this->mp_InitialValuePtr, sizeof(T));
					}
				}
				SetDataLinkEnabled(true);
			}
			else
			{
				ESP_LOGE("DataItem<T, COUNT>::Setup()", "Failed to allocate memory on SPI RAM");
			}
		}
		String GetName()
		{
			return LocalDataItem<T, COUNT>::GetName();
		}
		void SetNewTxValue(const T* Value, const size_t Count)
		{
			ESP_LOGD("DataItem: SetNewTxValue", "\"%s\" SetNewTxValue to: \"%s\"", m_Name.c_str(), GetValueAsStringForDataType(Value, GetDataTypeFromTemplateType<T>(), COUNT, ""));
			SetValue(Value, Count);
		}

		bool SetValue(const T *value, size_t count)
		{
			bool valueChanged = LocalDataItem<T, COUNT>::SetValue(value, count);
			if(valueChanged)
			{
				DataItem_Try_TX_On_Change();
			}
			return valueChanged;
		}

		bool SetValue(T value)
		{
			bool valueChanged = LocalDataItem<T, COUNT>::SetValue(value);
			if(valueChanged)
			{
				DataItem_Try_TX_On_Change();
			}
			return valueChanged;
		}

		void SetDataLinkEnabled(bool enable)
		{
			m_DataLinkEnabled = enable;
			if(m_DataLinkEnabled)
			{
				bool enablePeriodicTX = false;
				bool enablePeriodicRX = false;
				switch(m_RxTxType)
				{
					case RxTxType_Tx_Periodic:
					case RxTxType_Tx_On_Change_With_Heartbeat:
						enablePeriodicTX = true;
						enablePeriodicRX = true;
						break;
					case RxTxType_Tx_On_Change:
					case RxTxType_Rx_Only:
					case RxTxType_Rx_Echo_Value:
						enablePeriodicRX = true;
						break;
					default:
					break;
				}
				if(enablePeriodicTX)
				{
					esp_timer_start_periodic(m_TxTimer, m_Rate * 1000);
					ESP_LOGD("DataItem: SetDataLinkEnabled", "Data Item: \"%s\": Enabled Periodic TX", this->m_Name.c_str());
				}
				else
				{
					esp_timer_stop(m_TxTimer);
					ESP_LOGD("DataItem: SetDataLinkEnabled", "Data Item: \"%s\": Disabled Periodic TX", this->m_Name.c_str());
				}
				if(enablePeriodicRX)
				{
					m_SerialPortMessageManager.RegisterForNewValueNotification(this);
					ESP_LOGD("DataItem: SetDataLinkEnabled", "Data Item: \"%s\": Enabled Periodic RX", this->m_Name.c_str());
				}
				else
				{
					m_SerialPortMessageManager.DeRegisterForNewValueNotification(this);
					ESP_LOGD("DataItem: SetDataLinkEnabled", "Data Item: \"%s\": Disabled Periodic RX", this->m_Name.c_str());
				}
			}
			else
			{
				esp_timer_stop(m_TxTimer);
				m_SerialPortMessageManager.DeRegisterForNewValueNotification(this);
				ESP_LOGD("SetDataLinkEnabled", "Data Item: \"%s\": Disabled Datalink", this->m_Name.c_str());
			}
		}
	protected:
		const RxTxType_t m_RxTxType;
		const UpdateStoreType_t m_UpdateStoreType;
		const uint16_t m_Rate;
		SerialPortMessageManager &m_SerialPortMessageManager;

		T *mp_RxValue;
		T *mp_TxValue;
		
		virtual bool DataItem_TX_Now()
		{
			bool ValueUpdated = false;
			if(m_SerialPortMessageManager.QueueMessageFromData(this->GetName(), DataTypeFunctions::GetDataTypeFromTemplateType<T>(), mp_TxValue, COUNT))
			{				
				if(memcmp(this->mp_Value, mp_TxValue, sizeof(T) * COUNT) != 0)
				{
					if(m_UpdateStoreType == UpdateStoreType_On_Tx)
					{
						LocalDataItem<T, COUNT>::SetValue(mp_TxValue, COUNT);						
						ValueUpdated = true;		
					}
				}
				ESP_LOGI( "DataItem: DataItem_TX_Now", "TX: \"%s\" Value: \"%s\""
						, this->m_Name.c_str()
						, this->GetValueAsString().c_str() );
			}
			else
			{
				ESP_LOGE("DataItem: DataItem_TX_Now", "Data Item: \"%s\": Unable to Tx Message", this->m_Name.c_str());
			}
			return ValueUpdated;
		}
		virtual bool NewRXValueReceived(void* Object, size_t Count)
		{	
			bool ValueUpdated = false;
			T* receivedValue = static_cast<T*>(Object);
			ESP_LOGD( "DataItem: NewRXValueReceived"
						, "RX: \"%s\" Value: \"%s\""
						, this->m_Name.c_str()
						, this->GetValueAsString().c_str());
			if(memcmp(mp_RxValue, receivedValue, sizeof(T) * COUNT) != 0)
			{
				memcpy(mp_RxValue, receivedValue, sizeof(T) * COUNT);
				ESP_LOGD( "DataItem: NewRXValueReceived"
						, "Value Changed for: \"%s\" to Value: \"%s\""
						, this->m_Name.c_str()
						, this->GetValueAsString().c_str());
				if( UpdateStoreType_On_Rx == m_UpdateStoreType )
				{
					LocalDataItem<T, COUNT>::SetValue(mp_RxValue, COUNT);	
					ValueUpdated = true;
				}
			}
			if(RxTxType_Rx_Echo_Value == this->m_RxTxType)
			{
				memcpy(mp_TxValue, mp_RxValue, sizeof(T) * COUNT);
				ESP_LOGD( "DataItem: NewRXValueReceived"
						, "RX Echo for: \"%s\" with Value: \"%s\""
						, m_Name.c_str()
						, this->GetValueAsString().c_str());
				DataItem_TX_Now();
			}
			return ValueUpdated;
		}
		void DataItem_Try_TX_On_Change()
		{
			ESP_LOGI("DataItem& DataItem_Try_TX_On_Change", "Data Item: \"%s\": Try TX On Change", this->m_Name.c_str());
			if(this->m_RxTxType == RxTxType_Tx_On_Change || this->m_RxTxType == RxTxType_Tx_On_Change_With_Heartbeat)
			{
				DataItem_TX_Now();
			}
		}
	private:
		bool m_DataLinkEnabled = true;
		esp_timer_handle_t m_TxTimer;
		void CreateTxTimer()
		{
			esp_timer_create_args_t timerArgs;
			timerArgs.callback = &StaticDataItem_Periodic_TX;
			timerArgs.arg = this;
			timerArgs.name = "Tx_Timer";

			// Create the timer
			esp_timer_create(&timerArgs, &this->m_TxTimer);
		}
		void DataItem_Periodic_TX()
		{
			DataItem_TX_Now();
		}
		static void StaticDataItem_Periodic_TX(void *arg)
		{
			DataItem *aDataItem = static_cast<DataItem*>(arg);
			if(aDataItem)
			{
				aDataItem->DataItem_Periodic_TX();
			}
		}
};
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
		virtual bool DataItem_TX_Now() override
		{
			bool ValueUpdated = false;
			if(m_SerialPortMessageManager.QueueMessageFromData(m_Name, DataType_Char_t, mp_TxValue, DATAITEM_STRING_LENGTH))
			{
				if(strcmp(mp_Value, mp_TxValue) != 0)
				{
					if(m_UpdateStoreType == UpdateStoreType_On_Tx)
					{
						this->ZeroOutCharArray(mp_Value);
						strcpy(mp_Value, mp_TxValue);
						ValueUpdated = true;
						++m_ValueChangeCount;
						this->CallCallbacks(m_Name.c_str(), mp_Value);
					}
				}
				ESP_LOGD("DataItem: DataItem_TX_Now", "TX: \"%s\" Value: \"%s\"", this->m_Name.c_str(), GetValueAsStringForDataType(mp_TxValue, GetDataTypeFromTemplateType<T>(), COUNT, "").c_str());
			}
			else
			{
				ESP_LOGE("DataItem: DataItem_TX_Now", "Data Item: \"%s\": Unable to Tx Message", this->m_Name.c_str());
			}
			return ValueUpdated;
		}
		virtual bool NewRXValueReceived(void* Object, size_t Count) override 
		{ 
			bool ValueUpdated = false;
			char* receivedValue = (char*)Object;
			if(strcmp(mp_RxValue, receivedValue) != 0)
			{
				this->ZeroOutCharArray(mp_RxValue);
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
					this->CallCallbacks(this->m_Name.c_str(), mp_Value);
				}
			}
			if(RxTxType_Rx_Echo_Value == m_RxTxType)
			{
				ZeroOutCharArray(mp_TxValue);
				strcpy(mp_TxValue, mp_RxValue);
				this->DataItem_TX_Now();
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