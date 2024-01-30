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

#ifndef DataItem_H
#define DataItem_H

#define TIMER_TIME 10000UL
#define TIMER_BUFFER 1000UL

#include "SerialMessageManager.h"
#include <Helpers.h>
#include <Preferences.h>
#include <esp_timer.h>
#include <Arduino.h>
#include <esp_heap_caps.h>

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
class PreferencesWrapper
{
	
	public:
		struct PreferencesWrapperTimerArgs;
		PreferencesWrapper( Preferences *Preferences )
						  : m_Preferences(Preferences){}
		virtual ~PreferencesWrapper()
		{
			if(mp_TimerArgs)
			{
				delete mp_TimerArgs;
			}
		}
		static void Static_Update_Preference(void *arg);
		void Update_Preference(const String &UpdateType, const String& Name, T* ValuePtr, T* InitialValuePtr);
		struct PreferencesWrapperTimerArgs 
		{
			PreferencesWrapperTimerArgs(PreferencesWrapper<T, COUNT>* preferenceWrapper, const String& name, T* value, T* initialValue)
				: PreferenceWrapper(preferenceWrapper), Name(name), Value(value), InitialValue(initialValue) {}

			PreferencesWrapper<T, COUNT>* PreferenceWrapper;
			String Name;
			T* Value;
			T* InitialValue;
		};
	protected:
		void CreatePreferencesTimer(const String& Name, T* Value, T* InitialValue);
		void InitializeNVM(const String& Name, T* ValuePtr, T* InitialValuePtr);
		void HandleLoaded(const String& Name, T* ValuePtr, T* InitialValuePtr);
		void HandleUpdated(const String& Name, T* ValuePtr);
	private:
		Preferences *m_Preferences = nullptr;
		esp_timer_handle_t m_PreferenceTimer;
		PreferencesWrapperTimerArgs* mp_TimerArgs;
		uint64_t m_Preferences_Last_Update = millis();
		bool m_PreferenceTimerActive = false;
};


template <typename T, size_t COUNT>
class DataItem: public NewRxTxValueCallerInterface<T>
			  //, public NewRxTxValueCalleeInterface<T>
			  , public NewRxTxVoidObjectCalleeInterface
			  , public SetupCalleeInterface
			  , public DataTypeFunctions
{
	public:
		DataItem( const String name
				, const T* initialValue
				, const RxTxType_t rxTxType
				, const UpdateStoreType_t updateStoreType
				, const uint16_t rate
				, SerialPortMessageManager &serialPortMessageManager );
		DataItem( const String name
				, const T& initialValue
				, const RxTxType_t rxTxType
				, const UpdateStoreType_t updateStoreType
				, const uint16_t rate
				, SerialPortMessageManager &serialPortMessageManager );
		
		virtual ~DataItem();
		virtual void Setup();
		String GetName();
		size_t GetValue(void* Object, size_t Count);
		String GetValueAsString(const String &Divider);
		void SetNewTxValue(const T* Value, const size_t Count);
		virtual void SetValue(const T *Value, size_t Count);
		size_t GetCount();
		void SetDataLinkEnabled(bool enable);
		bool EqualsValue(T *Object, size_t Count)
		{
			assert(Count == COUNT && "Counts must equal");
			return (memcmp(mp_Value, Object, Count) == 0);
		}
	protected:
		T *mp_Value;
		T *mp_RxValue;
		T *mp_TxValue;
		T *mp_InitialValue;
		const T *mp_InitialValuePtr;
		const String m_Name;
		const RxTxType_t m_RxTxType;
		const UpdateStoreType_t m_UpdateStoreType;
		const uint16_t m_Rate;
		virtual bool DataItem_TX_Now();
		virtual bool NewRXValueReceived(void* Object, size_t Count);
		void DataItem_Try_TX_On_Change();
	private:
		bool m_DataLinkEnabled = true;
		size_t m_ValueChangeCount = 0;
		SerialPortMessageManager &m_SerialPortMessageManager;
		esp_timer_handle_t m_TxTimer;
		size_t m_Count = COUNT;
		void CreateTxTimer();
		void DataItem_Periodic_TX();
		static void StaticDataItem_Periodic_TX(void *arg);
};

class StringDataItem: public DataItem<char, 50>
{
	public:
		StringDataItem( const String name
					  , const char* initialValue
					  , const RxTxType_t rxTxType
					  , const UpdateStoreType_t updateStoreType
					  , const uint16_t rate
					  , SerialPortMessageManager &serialPortMessageManager )
					  : DataItem<char, 50>( name
										  , initialValue
										  , rxTxType
										  , updateStoreType
										  , rate
										  , serialPortMessageManager )
		{
		  
		}
		StringDataItem( const String name
					  , const char& initialValue
					  , const RxTxType_t rxTxType
					  , const UpdateStoreType_t updateStoreType
					  , const uint16_t rate
					  , SerialPortMessageManager &serialPortMessageManager )
					  : DataItem<char, 50>( name
										  , initialValue
										  , rxTxType
										  , updateStoreType
										  , rate
										  , serialPortMessageManager )
		{
		  
		}
		virtual ~StringDataItem()
		{
		}
		virtual void SetValue(const char* Value, size_t Count) override
		{
			assert(Value != nullptr && "Value must not be null");
			assert(mp_Value != nullptr && "mp_Value must not be null");
			String NewValue = String(Value);
			String CurrentValue = String(mp_TxValue);
			assert(NewValue.length() == Count);
			ESP_LOGD( "DataItem: SetValue"
					, "\"%s\" Set Value: \"%s\""
					, m_Name.c_str()
					, NewValue.c_str() );
			bool ValueChanged = CurrentValue.equals(NewValue);
			if(ValueChanged)
			{
				for (size_t i = 0; i < this->GetCount(); ++i)
				{
					mp_TxValue[i] = '\0';
				}
				strcpy(mp_TxValue, Value);
				this->DataItem_Try_TX_On_Change();
			}
		}
	protected:
		virtual bool NewRXValueReceived(void* Object, size_t Count) override 
		{ 
			bool ValueUpdated = false;
			String NewValue = String((char*)Object);
			String CurrentRxValue = String(mp_RxValue);
			String CurrentValue = String(mp_Value);
			bool ValueChanged = CurrentRxValue.equals(NewValue);
			if(ValueChanged)
			{
				for (size_t i = 0; i < this->GetCount(); ++i)
				{
					mp_RxValue[i] = '\0';
				}
				strcpy(mp_RxValue, NewValue.c_str());
				ESP_LOGD( "DataItem: NewRXValueReceived"
						, "\"%s\" New RX Value Received: \"%s\""
						, m_Name.c_str()
						, GetValueAsStringForDataType(mp_RxValue, GetDataTypeFromTemplateType<T>(), COUNT, ""));
				
				bool RxValueChanged = CurrentRxValue.equals(CurrentValue);
				if( UpdateStoreType_On_Rx == m_UpdateStoreType )
				{
					if(RxValueChanged)
					{
						for (size_t i = 0; i < this->GetCount(); ++i)
						{
							mp_Value[i] = '\0';
						}
						strcpy(mp_Value, mp_RxValue);
						ValueUpdated = true;
					}
				}
				if(RxTxType_Rx_Echo_Value == m_RxTxType)
				{
					for (size_t i = 0; i < this->GetCount(); ++i)
					{
						mp_TxValue[i] = '\0';
					}
					strcpy(mp_TxValue, mp_RxValue);
					this->DataItem_TX_Now();
				}
			}
			return ValueUpdated;
		}
};


template <typename T, size_t COUNT>
class DataItemWithPreferences: public DataItem<T, COUNT>
							 , public PreferencesWrapper<T, COUNT>
{
	public:
		DataItemWithPreferences( const String name
							   , const T* initialValue
							   , const RxTxType_t rxTxType
							   , const UpdateStoreType_t updateStoreType
							   , const uint16_t rate
							   , Preferences *preferences
							   , SerialPortMessageManager &serialPortMessageManager );
							   
		DataItemWithPreferences( const String name
							   , const T& initialValue
							   , const RxTxType_t rxTxType
							   , const UpdateStoreType_t updateStoreType
							   , const uint16_t rate
							   , Preferences *preferences
							   , SerialPortMessageManager &serialPortMessageManager );
		
		void Setup() override;
		virtual ~DataItemWithPreferences(){}
		virtual void SetValue(const T *Value, size_t Count)
		{
			DataItem<T, COUNT>::SetValue(Value, Count);
		}
	protected:
		bool DataItem_TX_Now() override;
		virtual bool NewRXValueReceived(void* Object, size_t Count) override;
};

class StringDataItemWithPreferences: public StringDataItem
								   , public PreferencesWrapper<char, 50>
{
	public:
		StringDataItemWithPreferences( const String name
								     , const char* initialValue
								     , const RxTxType_t rxTxType
								     , const UpdateStoreType_t updateStoreType
								     , const uint16_t rate
								     , Preferences *preferences
								     , SerialPortMessageManager &serialPortMessageManager )
								     : PreferencesWrapper<char, 50>(preferences)
									 , StringDataItem( name
													 , initialValue
													 , rxTxType
													 , updateStoreType
													 , rate
													 , serialPortMessageManager )
		{
			
		}
		StringDataItemWithPreferences( const String name
								     , const char& initialValue
								     , const RxTxType_t rxTxType
								     , const UpdateStoreType_t updateStoreType
								     , const uint16_t rate
								     , Preferences *preferences
								     , SerialPortMessageManager &serialPortMessageManager )
								     : PreferencesWrapper<char, 50>(preferences)
									 , StringDataItem( name
													 , initialValue
													 , rxTxType
													 , updateStoreType
													 , rate
													 , serialPortMessageManager )
		{
			
		}
		
		virtual ~StringDataItemWithPreferences(){}
		void SetValue(const char *Value, size_t Count) override
		{
		}
	private:
		bool NewRXValueReceived(void* Object, size_t Count) override
		{
			return false;
		}
};



#endif