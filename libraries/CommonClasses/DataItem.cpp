#include "DataItem.h"

#define TIMER_TIME 10000UL
#define TIMER_BUFFER 1000UL

template class DataItem<float, 1>;
template class DataItem<bool, 1>;
template class DataItem<char, 50>;
template class DataItem<ConnectionStatus_t, 1>;
template class DataItem<BT_Info_With_LastUpdateTime_t, 1>;
template class DataItemWithPreferences<float, 1>;
template class DataItemWithPreferences<bool, 1>;
template class DataItemWithPreferences<char, 50>;

template <typename T, int COUNT>
DataItem<T, COUNT>::DataItem( const String name
							, const T &initialValue
							, const RxTxType_t rxTxType
							, const UpdateStoreType_t updateStoreType
							, const uint16_t rate
							, SerialPortMessageManager &serialPortMessageManager )
							: m_Name(name)
							, m_InitialValue(initialValue)
							, m_RxTxType(rxTxType)
							, m_UpdateStoreType(updateStoreType)
							, m_Rate(rate)
							, m_SerialPortMessageManager(serialPortMessageManager)
							, NewRxTxVoidObjectCalleeInterface(COUNT)
{
	CreateTxTimer();
	m_SerialPortMessageManager.RegisterForSetupCall(this);
}

template <typename T, int COUNT>
DataItem<T, COUNT>::~DataItem()
{
	heap_caps_free(mp_Value);
	heap_caps_free(mp_RxValue);
	heap_caps_free(mp_TxValue);
	heap_caps_free(mp_InitialValue);
	esp_timer_stop(m_TxTimer);
	esp_timer_delete(m_TxTimer);
	m_SerialPortMessageManager.DeRegisterForSetupCall(this);
}

template <typename T, int COUNT>				 
void DataItem<T, COUNT>::Setup()
{
	ESP_LOGI("DataItem<T, COUNT>::Setup()", "\"%s\": Allocating Memory", m_Name.c_str());
	mp_Value = (T*)heap_caps_malloc(sizeof(T)*COUNT, MALLOC_CAP_SPIRAM);
	mp_RxValue = (T*)heap_caps_malloc(sizeof(T)*COUNT, MALLOC_CAP_SPIRAM);
	mp_TxValue = (T*)heap_caps_malloc(sizeof(T)*COUNT, MALLOC_CAP_SPIRAM);
	mp_InitialValue = (T*)heap_caps_malloc(sizeof(T)*COUNT, MALLOC_CAP_SPIRAM);
	if (mp_Value && mp_RxValue && mp_TxValue && mp_InitialValue)
	{
		if (std::is_same<T, char>::value)
		{
			ESP_LOGI("DataItem<T, COUNT>::Setup()", "\"%s\": Allocating Char", m_Name.c_str());
			bool eolFound = false;
			for (int i = 0; i < COUNT; ++i)
			{
				char value;
				memcpy(&value, &m_InitialValue+i, sizeof(char));
				if (eolFound || memcmp(&value, "\0", sizeof(char)) == 0 || i == COUNT-1)
				{
					eolFound = true;
					memcpy(&value, "\0", sizeof(char));
				}
				memcpy(mp_Value+i, &value, sizeof(char));
				memcpy(mp_RxValue+i, &value, sizeof(char));
				memcpy(mp_TxValue+i, &value, sizeof(char));
				memcpy(mp_InitialValue+i, &value, sizeof(char));
			}
		}
		else
		{
			ESP_LOGI("DataItem<T, COUNT>::Setup()", "\"%s\": Allocating Other", m_Name.c_str());
			for (int i = 0; i < COUNT; ++i)
			{
				memcpy(mp_Value+i, &m_InitialValue, sizeof(T));
				memcpy(mp_RxValue+i, &m_InitialValue, sizeof(T));
				memcpy(mp_TxValue+i, &m_InitialValue, sizeof(T));
				memcpy(mp_InitialValue+i, &m_InitialValue, sizeof(T));
			}
		}
		SetDataLinkEnabled(true);
	}
	else
	{
		ESP_LOGE("DataItem<T, COUNT>::Setup()", "Failed to allocate memory on SPI RAM");
    }
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
void DataItem<T, COUNT>::GetValue(void* Object, size_t Count)
{
	assert(Count == COUNT && "Counts must be equal");
	if(mp_Value)
	{
		memcpy(Object, mp_Value, sizeof(T)*Count);
	}
	else
	{
		*reinterpret_cast<T**>(Object) = nullptr;
	}
}

template <typename T, int COUNT>
String DataItem<T, COUNT>::GetValueAsString()
{
	return GetValueAsStringForDataType(mp_Value, GetDataTypeFromTemplateType<T>(), COUNT);
}

template <typename T, int COUNT>
void DataItem<T, COUNT>::SetNewTxValue(const T* Value, const size_t Count)
{
	ESP_LOGD("DataItem: SetNewTxValue", "\"%s\" SetNewTxValue to: \"%s\"", m_Name.c_str(), GetValueAsStringForDataType(Value, GetDataTypeFromTemplateType<T>(), COUNT));
	SetValue(Value, Count);
}

/*
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
*/

template <typename T, int COUNT>
void DataItem<T, COUNT>::SetValue(const T *Value, size_t Count)
{
	assert(Value != nullptr && "Value must not be null");
	assert(mp_Value != nullptr && "mp_Value must not be null");
	assert(COUNT > 0 && "COUNT must be a valid index range for mp_Value");
	assert(COUNT == Count && "Counts must match");
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
bool DataItem<T, COUNT>::NewRXValueReceived(void* Object, size_t Count)
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
														  , const T &initialValue
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
	if(result) Update_Preference("Updated");
	return result;
}

template <typename T, int COUNT>
bool DataItemWithPreferences<T, COUNT>::NewRXValueReceived(void* Object, size_t Count)
{
	bool result = DataItem<T, COUNT>::NewRXValueReceived(Object, Count);
	if(result) Update_Preference("Updated");
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
		if (true == m_Preferences->isKey(this->m_Name.c_str()))
		{
			Update_Preference("Loaded");
		}
		else
		{
			Update_Preference("Initialized");
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

    if ( UpdateType.equals("Loaded") )
	{
        HandleLoaded( this->mp_InitialValue[0] );
    } 
	else if ( UpdateType.equals("Initialized") )
	{
		HandleUpdated( this->mp_InitialValue[0] );
	}
	else if ( UpdateType.equals("Updated") )
	{
        HandleUpdated( this->mp_Value[0] );
	}		
	else if ( UpdateType.equals("Timer") )
	{
        HandleUpdated( this->mp_Value[0] );
		m_PreferenceTimerActive = false;
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
		assert(COUNT == 1 && "Count should be 1 to do this");
		bool value;
		memcpy(&value , &initialValue, sizeof(bool));
		bool result = m_Preferences->getBool(this->m_Name.c_str(), value);
        memcpy(this->mp_Value, &result, sizeof(bool));
        memcpy(this->mp_TxValue, &result, sizeof(bool));
		ESP_LOGI("DataItem: HandleLoaded", "Data Item: \"%s\": Loaded Bool", this->m_Name.c_str());	
    }
	else if (std::is_same<T, int32_t>::value)
	{
		assert(COUNT == 1 && "Count should be 1 to do this");
		int32_t value;
		memcpy(&value , &initialValue, sizeof(int32_t));
		int32_t result = m_Preferences->getInt(this->m_Name.c_str(), value);
        memcpy(this->mp_Value, &result, sizeof(int32_t));
        memcpy(this->mp_TxValue, &result, sizeof(int32_t));
		ESP_LOGI("DataItem: HandleLoaded", "Data Item: \"%s\": Loaded int32_t", this->m_Name.c_str());	
    }
	else if (std::is_same<T, int16_t>::value)
	{
		assert(COUNT == 1 && "Count should be 1 to do this");
		int16_t value;
		memcpy(&value , &initialValue, sizeof(int16_t));
		int16_t result = m_Preferences->getInt(this->m_Name.c_str(), value);
        memcpy(this->mp_Value, &result, sizeof(int16_t));
        memcpy(this->mp_TxValue, &result, sizeof(int16_t));
		ESP_LOGI("DataItem: HandleLoaded", "Data Item: \"%s\": Loaded int16_t", this->m_Name.c_str());	
    }
	else if (std::is_same<T, int8_t>::value)
	{
		assert(COUNT == 1 && "Count should be 1 to do this");
		int8_t value;
		memcpy(&value , &initialValue, sizeof(int8_t));
		int8_t result = m_Preferences->getInt(this->m_Name.c_str(), value);
        memcpy(this->mp_Value, &result, sizeof(int8_t));
        memcpy(this->mp_TxValue, &result, sizeof(int8_t));
		ESP_LOGI("DataItem: HandleLoaded", "Data Item: \"%s\": Loaded int8_t", this->m_Name.c_str());	
    }
	else if (std::is_same<T, float>::value)
	{
		assert(COUNT == 1 && "Count should be 1 to do this");
		float value;
		memcpy(&value , &initialValue, sizeof(float));
		float result = m_Preferences->getFloat(this->m_Name.c_str(), value);
        memcpy(this->mp_Value, &result, sizeof(float));
        memcpy(this->mp_TxValue, &result, sizeof(float));
		ESP_LOGI("DataItem: HandleLoaded", "Data Item: \"%s\": Loaded float", this->m_Name.c_str());	
    }
	else if (std::is_same<T, double>::value)
	{
		assert(COUNT == 1 && "Count should be 1 to do this");
		double value;
		memcpy(&value , &initialValue, sizeof(double));
		double result = m_Preferences->getDouble(this->m_Name.c_str(), value);
        memcpy(this->mp_Value, &result, sizeof(double));
        memcpy(this->mp_TxValue, &result, sizeof(double));
		ESP_LOGI("DataItem: HandleLoaded", "Data Item: \"%s\": Loaded double", this->m_Name.c_str());	
    }
	else if (std::is_same<T, char>::value)
	{
		char value[COUNT];
		for (size_t i = 0; i < COUNT - 1; ++i)
		{
			value[i] = ((char*)(&initialValue))[i];
			this->mp_Value[i] = '\0';
			this->mp_TxValue[i] = '\0';
		}
		value[COUNT - 1] = '\0';
		
		String result = m_Preferences->getString(this->m_Name.c_str(), value);
		assert(result.length() <= COUNT);
		
		size_t i = 0;
		while (i < COUNT - 1 && i < result.length())
		{
			this->mp_Value[i] = result[i];
			this->mp_TxValue[i] = result[i];
			++i;
		}
		ESP_LOGI("DataItem: HandleLoaded", "Data Item: \"%s\": Loaded char array", this->m_Name.c_str());
    }
	else
	{
        ESP_LOGE("SetDataLinkEnabled", "Data Item: \"%s\": Unsupported Data Type", this->m_Name.c_str());
    }
	DataItem<T, COUNT>::DataItem_Try_TX_On_Change();
}

template <typename T, int COUNT>
void DataItemWithPreferences<T, COUNT>::HandleUpdated(const T& value)
{	
    if (std::is_same<T, bool>::value)
	{
        m_Preferences->putBool(this->m_Name.c_str(), value);
		ESP_LOGI("DataItem: HandleLoaded", "Data Item: \"%s\": Saving bool", this->m_Name.c_str());	
    } 
	else if (std::is_same<T, char>::value)
	{
		char charValues[COUNT];
		// Copy characters from initialValue to value, ensuring null-termination.
		for (size_t i = 0; i < COUNT - 1; ++i)
		{
			charValues[i] = ((char*)(&value))[i];
		}
		charValues[COUNT - 1] = '\0';
        m_Preferences->putString(this->m_Name.c_str(), charValues);
		ESP_LOGI("DataItem: HandleLoaded", "Data Item: \"%s\": Saving String: %s", this->m_Name.c_str(), String(charValues).c_str() );	
    } 
	else if (std::is_integral<T>::value)
	{
        m_Preferences->putInt(this->m_Name.c_str(), value);
		ESP_LOGI("DataItem: HandleLoaded", "Data Item: \"%s\": Saving integer", this->m_Name.c_str());	
    } 
	else if (std::is_floating_point<T>::value)
	{
        m_Preferences->putFloat(this->m_Name.c_str(), value);
		ESP_LOGI("DataItem: HandleLoaded", "Data Item: \"%s\": Saving float", this->m_Name.c_str());	
    }
	else if (std::is_same<T, double>::value)
	{
        m_Preferences->putDouble(this->m_Name.c_str(), value);
		ESP_LOGI("DataItem: HandleLoaded", "Data Item: \"%s\": Saving double", this->m_Name.c_str());	
    } 
	else 
	{
        ESP_LOGE("SetDataLinkEnabled", "Data Item: \"%s\": Unsupported Data Type", this->m_Name.c_str());
    }
}