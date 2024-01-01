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
				, Preferences *preferences
				, SerialPortMessageManager &serialPortMessageManager )
			    : m_Name(name)
				, m_RxTxType(rxTxType)
				, m_UpdateStoreType(updateStoreType)
				, m_Rate(rate)
				, m_Preferences(preferences)
				, m_SerialPortMessageManager(serialPortMessageManager)
		{
			mp_Value =  new T[COUNT];
			for (int i = 0; i < COUNT; ++i)
			{
				mp_Value[i] = initialValuePointer[i];
				mp_RxValue[i] = initialValuePointer[i];
				mp_TxValue[i] = initialValuePointer[i];
			}
			CreateTxTimer();
			SetDataLinkEnabled(true);
		}
		
		DataItem( const String name
				, const T initialValue
				, const RxTxType_t rxTxType
				, const UpdateStoreType_t updateStoreType
				, const uint16_t rate
				, Preferences *preferences
				, SerialPortMessageManager &serialPortMessageManager )
			    : m_Name(name)
				, m_RxTxType(rxTxType)
				, m_UpdateStoreType(updateStoreType)
				, m_Rate(rate)
				, m_Preferences(preferences)
				, m_SerialPortMessageManager(serialPortMessageManager)
		{
			mp_Value = new T[COUNT];
			mp_RxValue = new T[COUNT];
			mp_TxValue = new T[COUNT];
			for (int i = 0; i < COUNT; ++i)
			{
				mp_Value[i] = T(initialValue);
				mp_RxValue[i] = T(initialValue);
				mp_TxValue[i] = T(initialValue);
			}
			CreateTxTimer();
			SetDataLinkEnabled(true);
			m_SerialPortMessageManager.RegisterForSetupCall(this);
		}
		
		
		virtual ~DataItem()
		{
			delete[] mp_Value;
			delete[] mp_RxValue;
			delete[] mp_TxValue;
			esp_timer_stop(m_TxTimer);
			esp_timer_delete(m_TxTimer);
			m_SerialPortMessageManager.DeRegisterForSetupCall(this);
		}
		void Setup();
		void InitializeNVM();
		void LoadFromNVM();
		void CreateTxTimer();
		void CreatePreferencesTimer();
		void ResetPreferencesTimer();
		
		String GetName();
		T* GetValuePointer();
		T GetValue();
		String GetValueAsString();
		void SetNewTxValue(T* Value);
		void SetValue(T Value);
		void SetValue(T *Value);
		size_t GetCount();
		void SetDataLinkEnabled(bool enable);
	private:
		Preferences *m_Preferences = nullptr;
		const String m_Name;
		const RxTxType_t m_RxTxType;
		const UpdateStoreType_t m_UpdateStoreType;
		const uint16_t m_Rate;
		T *mp_Value;
		T *mp_RxValue;
		T *mp_TxValue;
		bool m_DataLinkEnabled = true;
		SerialPortMessageManager &m_SerialPortMessageManager;
		esp_timer_handle_t m_TxTimer;
		esp_timer_handle_t m_PreferenceTimer;
		
		void DataItem_Try_TX_On_Change();
		static void Static_Update_Preference(void *arg);
		void Update_Preference(const String UpdateType);
		static void StaticDataItem_TX_Now(void *arg);
		void DataItem_TX_Now();
		void NewRXValueReceived(void* Object);
};

#endif