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
#include "SerialMessageManager.h"

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
class SerialDataLinkIntertface: public NewRxTxValueCallerInterface<T>
			  				  , public NewRxTxVoidObjectCalleeInterface
{
	public:
		SerialDataLinkIntertface( const RxTxType_t rxTxType
								, const UpdateStoreType_t updateStoreType
								, const uint16_t rate
								, SerialPortMessageManager *serialPortMessageManager = nullptr )
								: NewRxTxVoidObjectCalleeInterface(COUNT)
								, m_RxTxType(rxTxType)
								, m_UpdateStoreType(updateStoreType)
								, m_Rate(rate)
								, mp_SerialPortMessageManager(serialPortMessageManager)
        {

        }

		virtual void SetNewTxValue(const T* object, const size_t count) = 0;
		virtual bool NewRxValueReceived(void* object, size_t Count) = 0;
		virtual bool DataItem_TX_Now() = 0;
	protected:
		const RxTxType_t m_RxTxType;
		const UpdateStoreType_t m_UpdateStoreType;
		const uint16_t m_Rate;
		SerialPortMessageManager *mp_SerialPortMessageManager = nullptr;
		T *mp_RxValue = nullptr;
		T *mp_TxValue = nullptr;
		void DataItem_Try_TX_On_Change()
		{
			ESP_LOGI("DataItem& DataItem_Try_TX_On_Change", "Data Item: \"%s\": Try TX On Change", LocalDataItem<T,COUNT>::GetName().c_str());
			if(this->m_RxTxType == RxTxType_Tx_On_Change || this->m_RxTxType == RxTxType_Tx_On_Change_With_Heartbeat)
			{
				DataItem_TX_Now();
			}
		}
};