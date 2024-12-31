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
#include <iostream>
#include <sstream>

enum RxTxType_t
{
	RxTxType_Tx_Periodic,
	RxTxType_Tx_On_Change,
	RxTxType_Tx_On_Change_With_Heartbeat,
	RxTxType_Rx_Only,
	RxTxType_Rx_Echo_Value,
	RxTxType_Count
};

template <typename T, size_t COUNT>
class SerialMessageInterface: public Rx_Value_Caller_Interface<T>
			  				, public Named_Object_Callee_Interface
{
	public:
		SerialMessageInterface( SerialPortMessageManager *serialPortMessageManager = nullptr )
							  : Named_Object_Callee_Interface(COUNT)
							  , mp_SerialPortMessageManager(serialPortMessageManager)
							  {
								ESP_LOGD("SerialMessageInterface", "Constructor1 SerialMessageInterface");
								m_ValueSemaphore = xSemaphoreCreateMutex();
								if (m_ValueSemaphore == nullptr)
								{
									ESP_LOGE("AudioBuffer", "ERROR! Failed to create semaphore.");
								}
							  }
		SerialMessageInterface( RxTxType_t rxTxType
							  , uint16_t rate
							  , SerialPortMessageManager *serialPortMessageManager = nullptr )
							  : Named_Object_Callee_Interface(COUNT)
							  , m_RxTxType(rxTxType)
							  , m_Rate(rate)
							  , mp_SerialPortMessageManager(serialPortMessageManager)
							  {
								ESP_LOGD("SerialMessageInterface", "Constructor2 SerialMessageInterface");
								m_ValueSemaphore = xSemaphoreCreateMutex();
								if (m_ValueSemaphore == nullptr)
								{
									ESP_LOGE("AudioBuffer", "ERROR! Failed to create semaphore.");
								}
							  }

		virtual ~SerialMessageInterface()
		{
			if(xSemaphoreTakeRecursive(m_ValueSemaphore, pdMS_TO_TICKS(0)) == pdTRUE)
			{
				ESP_LOGD("~DataItem", "Deleting SerialMessageInterface");
				DestroyTimer(m_TxTimer);
				if(mp_SerialPortMessageManager)
				{
					mp_SerialPortMessageManager->DeRegisterForNewRxValueNotification(this);
				}
				if(mp_RxValue)
				{
					ESP_LOGD("~SerialMessageInterface", "freeing mp_RxValue Memory");
					free(mp_RxValue);
				}
				if(mp_TxValue)
				{
					ESP_LOGD("~SerialMessageInterface", "freeing mp_TxValue Memory");
					free(mp_TxValue);	
				}
				xSemaphoreGiveRecursive(m_ValueSemaphore);
			}
			else
			{
				ESP_LOGW("Semaphore Take Failure", "WARNING! Failed to take Semaphore");
			}
			if(m_ValueSemaphore)
			{
				vSemaphoreDelete(m_ValueSemaphore);
				m_ValueSemaphore = nullptr;
			}
		}
		virtual T* GetValuePointer() const = 0;
		virtual UpdateStatus_t UpdateStore(T *oldValues, const T *newValues, const size_t changeCount) = 0;
		virtual bool EqualsValue(T *Object, size_t Count) const = 0;
		virtual std::string GetName() const = 0;
		virtual size_t GetChangeCount() const = 0;
		virtual bool ConfirmValueValidity(const T* values, size_t count) const = 0;
		virtual std::string GetValueAsString() const = 0;
		virtual DataType_t GetDataType() = 0;
		virtual std::string ConvertValueToString(const T *pvalue, size_t count) const = 0;
		virtual size_t ParseStringValueIntoValues(const std::string& stringValue, T* values) = 0;

		bool IsChangeCountGreater(size_t changeCount)
		{
			size_t currentChangeCount = GetChangeCount();
			return (changeCount > currentChangeCount) || (currentChangeCount - changeCount > (SIZE_MAX / 2));
		}
		
		UpdateStatus_t Try_Echo_Value(const T* receivedValues)
		{
			UpdateStatus_t storeUpdated;
			if(xSemaphoreTakeRecursive(m_ValueSemaphore, pdMS_TO_TICKS(0)) == pdTRUE)
			{
				if(RxTxType_Rx_Echo_Value == m_RxTxType)
				{
					ESP_LOGD( "NewRxValueReceived"
							, "Rx Echo for: \"%s\" with Value: \"%s\""
							, GetName().c_str()
							, ConvertValueToString(receivedValues, GetCount()).c_str() );
					if(UpdateStore(mp_TxValue, mp_RxValue, GetChangeCount()).UpdateSuccessful)
					{
						storeUpdated |= Tx_Now(GetChangeCount());
					}
				}
				xSemaphoreGiveRecursive(m_ValueSemaphore);
			}
			else
			{
				ESP_LOGW("Semaphore Take Failure", "WARNING! Failed to take Semaphore");
			}
			return storeUpdated;
		}
		void Configure( RxTxType_t rxTxType, uint16_t rate )
		{
			m_RxTxType = rxTxType;
			m_Rate = rate;
		}

		//Named_Object_Callee_Interface
		virtual UpdateStatus_t New_Object_From_Sender(const Named_Object_Caller_Interface* sender, const void* values, const size_t changeCount) override
		{
			UpdateStatus_t storeUpdated;
			if(xSemaphoreTakeRecursive(m_ValueSemaphore, pdMS_TO_TICKS(0)) == pdTRUE)
			{
				const T* receivedValues = static_cast<const T*>(values);
				ESP_LOGD( "NewRxValueReceived"
						, "\"%s\" Rx: \"%s\" Value: \"%s\" Change Count: \"%i\""
						, mp_SerialPortMessageManager->GetName().c_str()
						, GetName().c_str()
						, ConvertValueToString(receivedValues, GetCount()).c_str()
						, GetChangeCount());
				if(UpdateStore(mp_RxValue, receivedValues, changeCount).UpdateSuccessful)
				{
					storeUpdated |= UpdateStore(GetValuePointer(), mp_RxValue, changeCount);
					this->Notify_NewRxValue_Callees(mp_RxValue, COUNT, changeCount);
				}
				storeUpdated |= Try_Echo_Value(receivedValues);
				xSemaphoreGiveRecursive(m_ValueSemaphore);
			}
			else
			{
				ESP_LOGW("Semaphore Take Failure", "WARNING! Failed to take Semaphore");
			}
			return storeUpdated;
		}

		void Setup()
		{
			if(xSemaphoreTakeRecursive(m_ValueSemaphore, pdMS_TO_TICKS(0)) == pdTRUE)
			{
				if(!mp_RxValue) mp_RxValue = (T*)malloc(sizeof(T)*COUNT);
				if(!mp_TxValue) mp_TxValue = (T*)malloc(sizeof(T)*COUNT);
				if(GetValuePointer())
				{
					if (mp_RxValue && mp_TxValue)
					{
						ESP_LOGD("DataItem<T, COUNT>::Setup()", "Setting Initial Tx/Rx Values to: %s", GetValueAsString().c_str());
						UpdateStore(mp_RxValue, GetValuePointer(), GetChangeCount());
						UpdateStore(mp_TxValue, GetValuePointer(), GetChangeCount());
						SetDataLinkEnabled(true);
					}
					else
					{
						ESP_LOGE("DataItem<T, COUNT>::Setup()", "ERROR! Failed to allocate memory on SPI RAM.");
					}
				}
				else
				{
					ESP_LOGE("DataItem<T, COUNT>::Setup()", "ERROR! Null Pointer.");
				}
				xSemaphoreGiveRecursive(m_ValueSemaphore);
			}
			else
			{
				ESP_LOGW("Semaphore Take Failure", "WARNING! Failed to take Semaphore");
			}
		}

		UpdateStatus_t Set_Tx_Value(const T* newTxValues, size_t count)
		{
			UpdateStatus_t updateStatus;
			if(xSemaphoreTakeRecursive(m_ValueSemaphore, pdMS_TO_TICKS(0)) == pdTRUE)
			{
				ESP_LOGD( "Set_Tx_Value"
						, "\"%s\" Set Tx Value: \"%s\""
						, GetName().c_str()
						, ConvertValueToString(newTxValues, count).c_str());
				assert(newTxValues != nullptr);
				assert(mp_TxValue != nullptr);
				assert(COUNT > 0);
				assert(count <= COUNT);
				updateStatus.ValueChanged = (0 != memcmp(mp_TxValue, newTxValues, sizeof(T)*count));
				updateStatus.ValidValue = ConfirmValueValidity(newTxValues, COUNT);
				updateStatus.UpdateAllowed = (updateStatus.ValueChanged && updateStatus.ValidValue);
				ESP_LOGD( "Set_Tx_Value", "\"%s\": UpdateAllowed: \"%i\" Current Value: \"%s\" New Value: \"%s\""
						, GetName().c_str()
						, updateStatus.UpdateAllowed
						, ConvertValueToString(mp_TxValue, count).c_str()
						, ConvertValueToString(newTxValues, count).c_str() );
				if(updateStatus.UpdateAllowed)
				{
					ESP_LOGD( "Set_Tx_Value", "\"%s\" Set Tx Value for: \"%s\": Value changed."
								, mp_SerialPortMessageManager->GetName().c_str()
								, GetName().c_str() );
					updateStatus |= Update_Tx_Store_And_Try_Tx_On_Change(newTxValues);
				}
				else
				{
					ESP_LOGD( "LocalDataItem: SetValue", "Set Value Failed");
				}
				xSemaphoreGiveRecursive(m_ValueSemaphore);
			}
			else
			{
				ESP_LOGW("Semaphore Take Failure", "WARNING! Failed to take Semaphore");
			}
			return updateStatus;
		}

		UpdateStatus_t Tx_Now(size_t changeCount)
		{
			UpdateStatus_t updateStatus;
			if(xSemaphoreTakeRecursive(m_ValueSemaphore, pdMS_TO_TICKS(0)) == pdTRUE)
			{
				ESP_LOGD( "Tx_Now", "\"%s\" Tx: \"%s\" Value: \"%s\" Change Count: \"%i\"", mp_SerialPortMessageManager->GetName().c_str(), GetName().c_str(), ConvertValueToString(mp_TxValue, COUNT), GetChangeCount() );
				if(mp_SerialPortMessageManager)
				{
					updateStatus |= UpdateStore(GetValuePointer(), mp_TxValue, changeCount);
					ESP_LOGD( "Tx_Now", "\"%s\": Tx Message Change Count \"%i\"", GetName(), GetChangeCount());
					if(!mp_SerialPortMessageManager->QueueMessageFromDataType(GetName(), GetDataType(), mp_TxValue, GetCount(), GetChangeCount()))
					{
						ESP_LOGE("Tx_Now", "ERROR! Data Item: \"%s\": Unable to Tx Message.", GetName().c_str());
					}
				}
				else
				{
					ESP_LOGE("Tx_Now", "ERROR! Null Pointer!");
				}
				xSemaphoreGiveRecursive(m_ValueSemaphore);
			}
			else
			{
				ESP_LOGW("Semaphore Take Failure", "WARNING! Failed to take Semaphore");
			}
			return updateStatus;
		}		
	protected:
		bool m_DataLinkEnabled = false;
		RxTxType_t m_RxTxType;
		uint16_t m_Rate;
		SerialPortMessageManager *mp_SerialPortMessageManager = nullptr;
		T *mp_RxValue = nullptr;
		T *mp_TxValue = nullptr;
		SemaphoreHandle_t m_ValueSemaphore;
	private:
		esp_timer_handle_t m_TxTimer = nullptr;
		esp_timer_create_args_t m_TxTimerArgs;
		
		void SetDataLinkEnabled(bool enable)
		{
			if(mp_SerialPortMessageManager)
			{
				if(enable)
				{
					ESP_LOGD( "SetDataLinkEnabled", "\"%s\" Set Datalink Enabled for: \"%s\""
							, mp_SerialPortMessageManager->GetName().c_str()
							, GetName().c_str() );
					bool enablePeriodicTx = false;
					bool enableRx = true;
					switch(m_RxTxType)
					{
						case RxTxType_Tx_Periodic:
						case RxTxType_Tx_On_Change_With_Heartbeat:
							enablePeriodicTx = true;
							Tx_Now(GetChangeCount());
							break;
						case RxTxType_Tx_On_Change:
							Tx_Now(GetChangeCount());
							break;
						case RxTxType_Rx_Only:
						case RxTxType_Rx_Echo_Value:
							break;
						default:
						break;
					}
					EnableRx(enableRx);
					EnablePeriodicTx(enablePeriodicTx);
				}
				else
				{
					ESP_LOGD( "SetDataLinkEnabled", "\"%s\" Set Datalink Disabled for: \"%s\""
							, mp_SerialPortMessageManager->GetName().c_str()
							, GetName().c_str() );
					EnableRx(false);
					EnablePeriodicTx(false);
				}
				m_DataLinkEnabled = enable;
			}
			else
			{
				ESP_LOGE( "SetDataLinkEnabled", "ERROR! Null Pointer." );
			}
		}

		static void Static_Periodic_Tx(void *arg)
		{
			SerialMessageInterface *aSerialMessageInterface = static_cast<SerialMessageInterface*>(arg);
			if(aSerialMessageInterface)
			{
				ESP_LOGD( "EnablePeriodicTx", "\"%s\": Periodic Tx"
						, aSerialMessageInterface->GetName().c_str() );
				aSerialMessageInterface->Tx_Now(aSerialMessageInterface->GetChangeCount());
			}
			else
			{
				ESP_LOGE( "SetDataLinkEnabled", "ERROR! Null Pointer." );
			}
		}

		UpdateStatus_t Update_Tx_Store_And_Try_Tx_On_Change(const T *newValues)
		{
			UpdateStatus_t storeUpdated;
			if(UpdateStore(mp_TxValue, newValues, GetChangeCount()).UpdateSuccessful)
			{
				storeUpdated |= Try_TX_On_Change(newValues, GetChangeCount()+1);
			}
			else
			{
				storeUpdated |= Try_TX_On_Change(newValues, GetChangeCount());
			}
			return storeUpdated;
		}

		UpdateStatus_t Try_TX_On_Change(const T *newValues, size_t changeCount)
		{
			ESP_LOGD("Try_TX_On_Change", "\"%s\": Try TX On Change", GetName().c_str());
			UpdateStatus_t storeUpdated;
			if(m_RxTxType == RxTxType_Tx_On_Change || m_RxTxType == RxTxType_Tx_On_Change_With_Heartbeat)
			{
				storeUpdated = Tx_Now(changeCount);
			}
			return storeUpdated;
		}

		void EnablePeriodicTx(bool enablePeriodicTx)
		{
			if(enablePeriodicTx)
			{
				ESP_LOGD( "EnablePeriodicTx", "\"%s\" Enable Tx for: \"%s\""
					, mp_SerialPortMessageManager->GetName().c_str()
					, GetName().c_str() );
				StartTxTimer();
			}
			else
			{
				ESP_LOGD( "EnablePeriodicTx", "\"%s\" Disable Tx for: \"%s\""
					, mp_SerialPortMessageManager->GetName().c_str()
					, GetName().c_str() );
				StopTimer(m_TxTimer);
			}
		}

		void EnableRx(bool enableRX)
		{
			if(enableRX)
			{
				if(mp_SerialPortMessageManager)
				{
					ESP_LOGD( "EnableRx", "\"%s\" Enable Rx for: \"%s\""
							, mp_SerialPortMessageManager->GetName().c_str()
							, GetName().c_str() );
					mp_SerialPortMessageManager->RegisterForNewRxValueNotification(this);
				}
				else
				{
					ESP_LOGE("SetDataLinkEnabled", "ERROR!  Null Pointer.");
				}
			}
			else
			{
				if(mp_SerialPortMessageManager)
				{
					ESP_LOGD( "EnableRx", "\"%s\" Disable Rx for: \"%s\""
							, mp_SerialPortMessageManager->GetName().c_str()
							, GetName().c_str() );
					mp_SerialPortMessageManager->DeRegisterForNewRxValueNotification(this);
				}
				else
				{
					ESP_LOGE("SetDataLinkEnabled", "ERROR! Null Pointer.");
				}
			}
		}

		bool CreateTxTimer()
		{
			ESP_LOGD("CreateTxTimer", "Creating Timer");
			m_TxTimerArgs.callback = &Static_Periodic_Tx;
			m_TxTimerArgs.arg = this;
			m_TxTimerArgs.name = "Tx_Timer";
			m_TxTimerArgs.dispatch_method = ESP_TIMER_TASK;
			bool success = false;
			if (!m_TxTimer)
			{
				esp_err_t error = esp_timer_create(&m_TxTimerArgs, &m_TxTimer);
				switch(error)
				{
					case ESP_OK:
						ESP_LOGD("CreateTxTimer", "Timer Created");
						success = true;
					break;
					case ESP_ERR_NO_MEM:
						ESP_LOGE("CreateTxTimer", "ERROR! Unable to create timer, no memory!");
					break;
					case ESP_ERR_INVALID_ARG:
						ESP_LOGE("CreateTxTimer", "ERROR! Unable to create timer, Invalid Argument!");
					break;
					default:
						ESP_LOGE("CreateTxTimer", "ERROR! Unable to create timer!");
					break;
				}
			}
			else
			{
				ESP_LOGD("CreateTxTimer", "Timer already exists.");
			}
			return success;
		}

		bool DestroyTimer(esp_timer_handle_t& timer)
		{
			ESP_LOGD("DestroyTimer", "Destroying Timer");
			if (timer)
			{
				if (StopTimer(timer))
				{
					if (ESP_OK == esp_timer_delete(timer))
					{
						ESP_LOGD("DestroyTimer", "Timer deleted");
						timer = nullptr;
						return true;
					}
					else
					{
						ESP_LOGE("DestroyTimer", "ERROR! Unable to delete Timer.");
					}
				}
				else
				{
					ESP_LOGW("DestroyTimer", "WARNING! Unable to stop Timer.");
				}
			}
			else
			{
				ESP_LOGD("DestroyTimer", "Timer does not exist");
			}
			return false;
		}

		bool StartTxTimer()
		{
			if (m_TxTimer || CreateTxTimer())
			{
				ESP_LOGD("StartTxTimer", "Starting Timer");
				if (ESP_OK == esp_timer_start_periodic(m_TxTimer, m_Rate * 1000))
				{
					ESP_LOGD("StartTxTimer", "Timer Started");
					return true;
				}
				else
				{
					ESP_LOGE("StartTxTimer", "ERROR! Unable to start timer.");
				}
			}
			else
			{
				ESP_LOGE("StartTxTimer", "ERROR! Null.");
			}
			return false;
		}

		bool StopTimer(esp_timer_handle_t& timer)
		{
			bool result = false;
			if (timer)
			{
				if (ESP_OK == esp_timer_stop(timer))
				{
					ESP_LOGD("StopTimer", "Timer Stopped!");
					result = true;
				}
				else
				{
					ESP_LOGE("StopTimer", "ERROR! Unable to stop timer.");
				}
			}
			else
			{
				ESP_LOGD("StopTimer", "Timer does not exist");
				result = true;
			}
			return result;
		}

		void ZeroOutMemory(T* object)
		{
			memset((void*)object, 0, sizeof(T) * COUNT);
		}
};