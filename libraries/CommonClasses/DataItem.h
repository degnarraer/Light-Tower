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

#include "SerialMessageManager.h"
#include <Helpers.h>
#include <Preferences.h>
#include <esp_timer.h>

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

template <typename T, int COUNT>
class DataItem: public NewRxTxValueCallerInterface<T>
			  , public NewRxTxVoidObjectCalleeInterface
			  , public SetupCalleeInterface
			  , public DataTypeFunctions
{
	public:
		
		
		DataItem( const String name
				, const T *initialValuePointer
				, const RxTxType_t rxTxType
				, const UpdateStoreType_t updateStoreType
				, const uint16_t rate
				, SerialPortMessageManager &serialPortMessageManager );
		
		DataItem( const String name
				, const T initialValue
				, const RxTxType_t rxTxType
				, const UpdateStoreType_t updateStoreType
				, const uint16_t rate
				, SerialPortMessageManager &serialPortMessageManager );
		
		virtual ~DataItem();
		virtual void Setup();
		String GetName();
		T* GetValuePointer();
		T GetValue();
		String GetValueAsString();
		void SetNewTxValue(T* Value);
		void SetValue(T Value);
		void SetValue(T *Value);
		size_t GetCount();
		void SetDataLinkEnabled(bool enable);
	protected:
		T *mp_Value;
		T *mp_RxValue;
		T *mp_TxValue;
		T *mp_InitialValue;
		const String m_Name;
		const RxTxType_t m_RxTxType;
		const UpdateStoreType_t m_UpdateStoreType;
		const uint16_t m_Rate;
		virtual bool DataItem_TX_Now();
		virtual bool NewRXValueReceived(void* Object);
		void DataItem_Try_TX_On_Change();
	private:
		bool m_DataLinkEnabled = true;
		SerialPortMessageManager &m_SerialPortMessageManager;
		esp_timer_handle_t m_TxTimer;
		
		void CreateTxTimer();
		void DataItem_Periodic_TX();
		static void StaticDataItem_Periodic_TX(void *arg);
};

template <typename T, int COUNT>
class DataItemWithPreferences: public DataItem<T, COUNT>
{
	public:
		DataItemWithPreferences( const String name
							   , const T *initialValuePointer
							   , const RxTxType_t rxTxType
							   , const UpdateStoreType_t updateStoreType
							   , const uint16_t rate
							   , Preferences *preferences
							   , SerialPortMessageManager &serialPortMessageManager )
							   : m_Preferences(preferences)
							   , DataItem<T, COUNT>( name
												   , initialValuePointer
												   , rxTxType
												   , updateStoreType
												   , rate
												   , serialPortMessageManager )
							   
		{
			CreatePreferencesTimer();
		}
		
		DataItemWithPreferences( const String name
							   , const T initialValue
							   , const RxTxType_t rxTxType
							   , const UpdateStoreType_t updateStoreType
							   , const uint16_t rate
							   , Preferences *preferences
							   , SerialPortMessageManager &serialPortMessageManager )
							   : m_Preferences(preferences)
							   , DataItem<T, COUNT>( name
												   , initialValue
												   , rxTxType
												   , updateStoreType
												   , rate
												   , serialPortMessageManager )
							   
		{
			CreatePreferencesTimer();
		}
		
		virtual void Setup() override
		{
			DataItem<T, COUNT>::Setup();
			InitializeNVM();
		}
		virtual ~DataItemWithPreferences(){}
		static void Static_Update_Preference(void *arg);
		void Update_Preference(const String &UpdateType);
	private:
		Preferences *m_Preferences = nullptr;
		esp_timer_handle_t m_PreferenceTimer;
		uint64_t m_Preferences_Last_Update = millis();
		bool m_PreferenceTimerActive = false;
		void InitializeNVM();
		void HandleLoaded(const T& initialValue);
		void HandleUpdated();
		void CreatePreferencesTimer();
		virtual bool DataItem_TX_Now() override
		{
			bool result = DataItem<T, COUNT>::DataItem_TX_Now();
			if(result) Update_Preference("Update");
			return result;
		}
		virtual bool NewRXValueReceived(void* Object) override
		{
			bool result = DataItem<T, COUNT>::NewRXValueReceived(Object);
			if(result) Update_Preference("Update");
			return result;
		}
};

#endif