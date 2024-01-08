#include "DataItem.h"

#define TIMER_TIME 10000UL
#define TIMER_BUFFER 1000UL

template class DataItem<float, 1>;
template class DataItem<bool, 1>;
template class DataItem<String, 1>;
template class DataItem<ConnectionStatus_t, 1>;
template class DataItem<BT_Info_With_LastUpdateTime_t, 1>;
template class DataItemWithPreferences<float, 1>;
template class DataItemWithPreferences<bool, 1>;

template <typename T, int COUNT>
DataItem<T, COUNT>::DataItem( const String name
							, const T initialValue
							, const RxTxType_t rxTxType
							, const UpdateStoreType_t updateStoreType
							, const uint16_t rate
							, SerialPortMessageManager &serialPortMessageManager )
							: m_Name(name)
							, m_RxTxType(rxTxType)
							, m_UpdateStoreType(updateStoreType)
							, m_Rate(rate)
							, m_SerialPortMessageManager(serialPortMessageManager)
{
	mp_Value = new T[COUNT];
	mp_RxValue = new T[COUNT];
	mp_TxValue = new T[COUNT];
	mp_InitialValue =  new T[COUNT];
	for (int i = 0; i < COUNT; ++i)
	{
		mp_Value[i] = T(initialValue);
		mp_RxValue[i] = T(initialValue);
		mp_TxValue[i] = T(initialValue);
		mp_InitialValue[i] = T(initialValue);
	}
	CreateTxTimer();
	SetDataLinkEnabled(true);
	m_SerialPortMessageManager.RegisterForSetupCall(this);
}

template <typename T, int COUNT>
DataItem<T, COUNT>::~DataItem()
{
	delete[] mp_Value;
	delete[] mp_RxValue;
	delete[] mp_TxValue;
	esp_timer_stop(m_TxTimer);
	esp_timer_delete(m_TxTimer);
	m_SerialPortMessageManager.DeRegisterForSetupCall(this);
}

template <typename T, int COUNT>				 
void DataItem<T, COUNT>::Setup()
{
}

template <typename T, int COUNT>
void DataItem<T, COUNT>::CreateTxTimer()
{
	esp_timer_create_args_t timerArgs;
	timerArgs.callback = &StaticDataItem_Periodic_TX;
	timerArgs.arg = this;
	timerArgs.name = "Tx_Timer";

	// Create the timer
	esp_timer_create(&timerArgs, &m_TxTimer);
}

template <typename T, int COUNT>
String DataItem<T, COUNT>::GetName()
{
	return m_Name;
}

template <typename T, int COUNT>
T* DataItem<T, COUNT>::GetValuePointer()
{
	return mp_Value;
}

template <typename T, int COUNT>
T DataItem<T, COUNT>::GetValue()
{
	static_assert(COUNT == 1, "Count should be 1 to do this");
	return mp_Value[0];
}

template <typename T, int COUNT>
String DataItem<T, COUNT>::GetValueAsString()
{
	return GetValueAsStringForDataType(mp_Value, GetDataTypeFromTemplateType<T>(), COUNT);
}

template <typename T, int COUNT>
void DataItem<T, COUNT>::SetNewTxValue(T* Value)
{
	ESP_LOGD("DataItem: SetNewTxValue", "\"%s\" SetNewTxValue to: \"%s\"", m_Name.c_str(), GetValueAsStringForDataType(Value, GetDataTypeFromTemplateType<T>(), COUNT));
	SetValue(Value);
}

template <typename T, int COUNT>
void DataItem<T, COUNT>::SetValue(T Value)
{
	assert(mp_TxValue != nullptr && "mp_Value must not be null");
	static_assert(COUNT == 1, "Count should be 1 to do this");
	ESP_LOGD( "DataItem: SetValue"
			, "\"%s\" Set Value: \"%s\""
			, m_Name.c_str()
			, GetValueAsStringForDataType(&Value, GetDataTypeFromTemplateType<T>(), COUNT));
	bool ValueChanged = (*mp_TxValue != Value);
	*mp_TxValue = Value;
	if(ValueChanged)
	{
		DataItem_Try_TX_On_Change();
	}
}

template <typename T, int COUNT>
void DataItem<T, COUNT>::SetValue(T *Value)
{
	assert(Value != nullptr && "Value must not be null");
	assert(mp_Value != nullptr && "mp_Value must not be null");
	assert(COUNT > 0 && "COUNT must be a valid index range for mp_Value");
	ESP_LOGI( "DataItem: SetValue"
			, "\"%s\" Set Value: \"%s\""
			, m_Name.c_str()
			, GetValueAsStringForDataType(Value, GetDataTypeFromTemplateType<T>(), COUNT));
	bool ValueChanged = (memcmp(mp_TxValue, Value, sizeof(T) * COUNT) != 0);
	memcpy(mp_TxValue, Value, sizeof(T) * COUNT);
	if(ValueChanged)
	{
		DataItem_Try_TX_On_Change();
	}
}

template <typename T, int COUNT>
size_t DataItem<T, COUNT>::GetCount()
{
	return COUNT;
}

template <typename T, int COUNT>
void DataItem<T, COUNT>::SetDataLinkEnabled(bool enable)
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
			ESP_LOGI("DataItem: SetDataLinkEnabled", "Data Item: \"%s\": Enabled Periodic TX", m_Name.c_str());
		}
		else
		{
			esp_timer_stop(m_TxTimer);
			ESP_LOGI("DataItem: SetDataLinkEnabled", "Data Item: \"%s\": Disabled Periodic TX", m_Name.c_str());
		}
		if(enablePeriodicRX)
		{
			m_SerialPortMessageManager.RegisterForNewValueNotification(this);
			ESP_LOGI("DataItem: SetDataLinkEnabled", "Data Item: \"%s\": Enabled Periodic RX", m_Name.c_str());
		}
		else
		{
			m_SerialPortMessageManager.DeRegisterForNewValueNotification(this);
			ESP_LOGI("DataItem: SetDataLinkEnabled", "Data Item: \"%s\": Disabled Periodic RX", m_Name.c_str());
		}
	}
	else
	{
		esp_timer_stop(m_TxTimer);
		m_SerialPortMessageManager.DeRegisterForNewValueNotification(this);
		ESP_LOGD("SetDataLinkEnabled", "Data Item: \"%s\": Disabled Datalink", m_Name.c_str());
	}
}

template <typename T, int COUNT>
void DataItem<T, COUNT>::DataItem_Try_TX_On_Change()
{
	ESP_LOGI("DataItem& DataItem_Try_TX_On_Change", "Data Item: \"%s\": Try TX On Change", m_Name.c_str());
	if(m_RxTxType == RxTxType_Tx_On_Change || m_RxTxType == RxTxType_Tx_On_Change_With_Heartbeat)
	{
		DataItem_TX_Now();
	}
}

template <typename T, int COUNT>
bool DataItem<T, COUNT>::DataItem_TX_Now()
{
	bool ValueUpdated = false;
	if(m_SerialPortMessageManager.QueueMessageFromData(m_Name, GetDataTypeFromTemplateType<T>(), mp_TxValue, COUNT))
	{
		bool TxValueChanged = (memcmp(mp_Value, mp_TxValue, sizeof(T) * COUNT) != 0);
		if(m_UpdateStoreType == UpdateStoreType_On_Tx)
		{
			if(TxValueChanged)
			{
				memcpy(mp_Value, mp_TxValue, sizeof(T) * COUNT);
				ValueUpdated = true;
			}
		}
		ESP_LOGD("DataItem: DataItem_TX_Now", "Data Item: \"%s\": TX Now: \"%s\"", m_Name.c_str(), GetValueAsStringForDataType(mp_TxValue, GetDataTypeFromTemplateType<T>(), COUNT).c_str());
	}
	else
	{
		ESP_LOGE("DataItem: DataItem_TX_Now", "Data Item: \"%s\": Unable to Tx Message", m_Name.c_str());
	}
	return ValueUpdated;
}

template <typename T, int COUNT>
void DataItem<T, COUNT>::StaticDataItem_Periodic_TX(void *arg)
{
	DataItem *aDataItem = static_cast<DataItem*>(arg);
	if(aDataItem)
	{
		aDataItem->DataItem_Periodic_TX();
	}
}

template <typename T, int COUNT>
void DataItem<T, COUNT>::DataItem_Periodic_TX()
{
	if(m_SerialPortMessageManager.QueueMessageFromData(m_Name, GetDataTypeFromTemplateType<T>(), mp_Value, COUNT))
	{
		ESP_LOGI("DataItem: DataItem_TX_Now", "Data Item: \"%s\": Periodic TX: \"%s\"", m_Name.c_str(), GetValueAsStringForDataType(mp_Value, GetDataTypeFromTemplateType<T>(), COUNT).c_str());
	}
}

template <typename T, int COUNT>
bool DataItem<T, COUNT>::NewRXValueReceived(void* Object)
{	
	bool ValueUpdated = false;
	T* receivedValue = static_cast<T*>(Object);
	bool ValueChanged = (memcmp(mp_RxValue, receivedValue, sizeof(T) * COUNT) != 0);
	if(ValueChanged)
	{
		memcpy(mp_RxValue, receivedValue, sizeof(T) * COUNT);
		ESP_LOGI( "DataItem: NewRXValueReceived"
				, "\"%s\" New RX Value Received: \"%s\""
				, m_Name.c_str()
				, GetValueAsStringForDataType(mp_RxValue, GetDataTypeFromTemplateType<T>(), COUNT));
		
		bool RxValueChanged = (memcmp(mp_Value, mp_RxValue, sizeof(T) * COUNT) != 0);
		if( UpdateStoreType_On_Rx == m_UpdateStoreType )
		{
			if(RxValueChanged)
			{
				memcpy(mp_Value, mp_RxValue, sizeof(T) * COUNT);
				ValueUpdated = true;
			}
		}
		if(RxTxType_Rx_Echo_Value == m_RxTxType)
		{
			memcpy(mp_TxValue, mp_RxValue, sizeof(T) * COUNT);
			DataItem_TX_Now();
		}
	}
	return ValueUpdated;
}

template <typename T, int COUNT>
DataItemWithPreferences<T, COUNT>::DataItemWithPreferences( const String name
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

template <typename T, int COUNT>
void DataItemWithPreferences<T, COUNT>::Setup()
{
	DataItem<T, COUNT>::Setup();
	InitializeNVM();
}

template <typename T, int COUNT>
bool DataItemWithPreferences<T, COUNT>::DataItem_TX_Now()
{
	bool result = DataItem<T, COUNT>::DataItem_TX_Now();
	if(result) Update_Preference("Update");
	return result;
}

template <typename T, int COUNT>
bool DataItemWithPreferences<T, COUNT>::NewRXValueReceived(void* Object)
{
	bool result = DataItem<T, COUNT>::NewRXValueReceived(Object);
	if(result) Update_Preference("Update");
	return result;
}

template <typename T, int COUNT>
void DataItemWithPreferences<T, COUNT>::CreatePreferencesTimer()
{
	esp_timer_create_args_t timerArgs;
	timerArgs.callback = &Static_Update_Preference;
	timerArgs.arg = this;
	timerArgs.name = "Preferences_Timer";

	// Create the timer
	esp_timer_create(&timerArgs, &m_PreferenceTimer);
}

template <typename T, int COUNT>
void DataItemWithPreferences<T, COUNT>::InitializeNVM()
{
	if(m_Preferences)
	{
		if(1 == COUNT)
		{
			if (true == m_Preferences->isKey(this->m_Name.c_str()))
			{
				Update_Preference("Loaded");
			}
			else
			{
				Update_Preference("Initialized");
			}
		}
		else
		{
			ESP_LOGE("Dataitem: InitializeNVM", "Cannot use preferences for items with non 1 COUNT size");
		}
	}
}

template <typename T, int COUNT>
void DataItemWithPreferences<T, COUNT>::Static_Update_Preference(void *arg)
{
	DataItemWithPreferences *aDataItem = static_cast<DataItemWithPreferences*>(arg);
	if(aDataItem)
	{
		aDataItem->Update_Preference("Timer");
	}
}

template <typename T, int COUNT>
void DataItemWithPreferences<T, COUNT>::Update_Preference(const String &UpdateType)
{
    static_assert(COUNT == 1, "Count should be 1 to do this");
    if(nullptr == m_Preferences) return;
    assert((UpdateType == "Initialized" || UpdateType == "Loaded" || UpdateType == "Updated" || UpdateType == "Timer") && "Misuse of function");

    unsigned long currentMillis = millis();
    unsigned long elapsedTime = currentMillis - m_Preferences_Last_Update;
	if (elapsedTime <= TIMER_TIME && UpdateType.equals("Updated"))
	{
		ESP_LOGI("SetDataLinkEnabled: Update_Preference", "\"%s\": To early to save preference", this->m_Name.c_str());
		if(false == m_PreferenceTimerActive)
		{
			ESP_LOGI("SetDataLinkEnabled: Update_Preference", "\"%s\": Started NVM Update Timer", this->m_Name.c_str());
			esp_timer_start_once(m_PreferenceTimer, ((TIMER_TIME - elapsedTime) + TIMER_BUFFER) * 1000);
			m_PreferenceTimerActive = true;
		}
		return;
	}

    if (UpdateType.equals("Loaded"))
	{
        HandleLoaded(this->mp_InitialValue[0]);
    } 
	else if ( UpdateType.equals("Initialized") || 
			  UpdateType.equals("Updated") || 
			  UpdateType.equals("Timer") )
	{
        HandleUpdated();
		if(UpdateType.equals("Timer"))
		{
			m_PreferenceTimerActive = false;
		}
		m_Preferences_Last_Update = currentMillis;
    }
	else
	{
        ESP_LOGE("SetDataLinkEnabled: Update_Preference", "\"%s\": Unsupported Update Type", this->m_Name.c_str());
	}
}

template <typename T, int COUNT>
void DataItemWithPreferences<T, COUNT>::HandleLoaded(const T& initialValue)
{
    if (std::is_same<T, bool>::value)
	{
		bool value;
		memcpy(&value , &initialValue, sizeof(bool));
		bool result = m_Preferences->getBool(this->m_Name.c_str(), value);
        memcpy(this->mp_Value, &result, sizeof(bool));
        memcpy(this->mp_TxValue, &result, sizeof(bool));
		ESP_LOGI("DataItem: HandleLoaded", "Data Item: \"%s\": Loaded Bool", this->m_Name.c_str());	
    }
	else if (std::is_same<T, int32_t>::value)
	{
		int32_t value;
		memcpy(&value , &initialValue, sizeof(int32_t));
		int32_t result = m_Preferences->getInt(this->m_Name.c_str(), value);
        memcpy(this->mp_Value, &result, sizeof(int32_t));
        memcpy(this->mp_TxValue, &result, sizeof(int32_t));
		ESP_LOGI("DataItem: HandleLoaded", "Data Item: \"%s\": Loaded int32_t", this->m_Name.c_str());	
    }
	else if (std::is_same<T, int16_t>::value)
	{
		int16_t value;
		memcpy(&value , &initialValue, sizeof(int16_t));
		int16_t result = m_Preferences->getInt(this->m_Name.c_str(), value);
        memcpy(this->mp_Value, &result, sizeof(int16_t));
        memcpy(this->mp_TxValue, &result, sizeof(int16_t));
		ESP_LOGI("DataItem: HandleLoaded", "Data Item: \"%s\": Loaded int16_t", this->m_Name.c_str());	
    }
	else if (std::is_same<T, int8_t>::value)
	{
		int8_t value;
		memcpy(&value , &initialValue, sizeof(int8_t));
		int8_t result = m_Preferences->getInt(this->m_Name.c_str(), value);
        memcpy(this->mp_Value, &result, sizeof(int8_t));
        memcpy(this->mp_TxValue, &result, sizeof(int8_t));
		ESP_LOGI("DataItem: HandleLoaded", "Data Item: \"%s\": Loaded int8_t", this->m_Name.c_str());	
    }
	else if (std::is_same<T, float>::value)
	{
		float value;
		memcpy(&value , &initialValue, sizeof(float));
		float result = m_Preferences->getFloat(this->m_Name.c_str(), value);
        memcpy(this->mp_Value, &result, sizeof(float));
        memcpy(this->mp_TxValue, &result, sizeof(float));
		ESP_LOGI("DataItem: HandleLoaded", "Data Item: \"%s\": Loaded float", this->m_Name.c_str());	
    }
	else if (std::is_same<T, double>::value)
	{
		double value;
		memcpy(&value , &initialValue, sizeof(double));
		double result = m_Preferences->getDouble(this->m_Name.c_str(), value);
        memcpy(this->mp_Value, &result, sizeof(double));
        memcpy(this->mp_TxValue, &result, sizeof(double));
		ESP_LOGI("DataItem: HandleLoaded", "Data Item: \"%s\": Loaded double", this->m_Name.c_str());	
    }
	else
	{
        ESP_LOGE("SetDataLinkEnabled", "Data Item: \"%s\": Unsupported Data Type", this->m_Name.c_str());
    }
	DataItem<T, COUNT>::DataItem_Try_TX_On_Change();
}

template <typename T, int COUNT>
void DataItemWithPreferences<T, COUNT>::HandleUpdated()
{
	
    if (std::is_same<T, bool>::value)
	{
        m_Preferences->putBool(this->m_Name.c_str(), this->mp_Value[0]);
		ESP_LOGI("DataItem: HandleLoaded", "Data Item: \"%s\": Saving bool", this->m_Name.c_str());	
    } 
	else if (std::is_integral<T>::value)
	{
        m_Preferences->putInt(this->m_Name.c_str(), this->mp_Value[0]);
		ESP_LOGI("DataItem: HandleLoaded", "Data Item: \"%s\": Saving integer", this->m_Name.c_str());	
    } 
	else if (std::is_floating_point<T>::value)
	{
        m_Preferences->putFloat(this->m_Name.c_str(), this->mp_Value[0]);
		ESP_LOGI("DataItem: HandleLoaded", "Data Item: \"%s\": Saving float", this->m_Name.c_str());	
    }
	else if (std::is_same<T, double>::value)
	{
        m_Preferences->putDouble(this->m_Name.c_str(), this->mp_Value[0]);
		ESP_LOGI("DataItem: HandleLoaded", "Data Item: \"%s\": Saving double", this->m_Name.c_str());	
    } 
	else 
	{
        ESP_LOGE("SetDataLinkEnabled", "Data Item: \"%s\": Unsupported Data Type", this->m_Name.c_str());
    }
}