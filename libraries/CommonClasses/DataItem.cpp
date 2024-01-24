#include "DataItem.h"

template class DataItem<float, 1>;
template class DataItem<bool, 1>;
template class DataItem<char, 50>;
template class DataItem<ConnectionStatus_t, 1>;
template class DataItem<CompatibleDevice_t, 1>;
template class DataItem<BT_Device_Info_With_LastUpdateTime_t, 1>;
template class DataItem<SoundInputSource_t, 1>;
template class DataItem<SoundOutputSource_t, 1>;

template class PreferencesWrapper<float, 1>;
template class PreferencesWrapper<bool, 1>;
template class PreferencesWrapper<char, 50>;
template class PreferencesWrapper<SoundInputSource_t, 1>;
template class PreferencesWrapper<SoundOutputSource_t, 1>;

template class DataItemWithPreferences<float, 1>;
template class DataItemWithPreferences<bool, 1>;
template class DataItemWithPreferences<char, 50>;
template class DataItemWithPreferences<SoundInputSource_t, 1>;
template class DataItemWithPreferences<SoundOutputSource_t, 1>;


template <typename T, size_t COUNT>
void PreferencesWrapper<T, COUNT>::CreatePreferencesTimer(const String& Name, T* Value, T* InitialValue)
{
	mp_TimerArgs = new PreferencesWrapperTimerArgs(this, Name, Value, InitialValue);
	esp_timer_create_args_t timerArgs;
	timerArgs.callback = &Static_Update_Preference;
	timerArgs.arg = mp_TimerArgs;
	timerArgs.name = "Preferences_Timer";

	// Create the timer
	esp_timer_create(&timerArgs, &m_PreferenceTimer);
}

template <typename T, size_t COUNT>
void PreferencesWrapper<T, COUNT>::InitializeNVM(const String& Name, T* ValuePtr, T* InitialValuePtr)
{
	if(m_Preferences)
	{
		if (true == m_Preferences->isKey(Name.c_str()))
		{
			Update_Preference("Loaded", Name, ValuePtr, InitialValuePtr);
		}
		else
		{
			Update_Preference("Initialized", Name, ValuePtr, InitialValuePtr);
		}
	}
}

template <typename T, size_t COUNT>
void PreferencesWrapper<T, COUNT>::Static_Update_Preference(void *arg)
{
	PreferencesWrapperTimerArgs* timerArgsPtr = static_cast<PreferencesWrapperTimerArgs*>(arg);
	PreferencesWrapper<T, COUNT>* aPreferenceWrapper = const_cast<PreferencesWrapper<T, COUNT>*>(timerArgsPtr->PreferenceWrapper);
	const String& name = timerArgsPtr->Name;
	T* valuePtr = const_cast<T*>(timerArgsPtr->Value);
	T* initialValuePtr = const_cast<T*>(timerArgsPtr->InitialValue);
	if(aPreferenceWrapper)
	{
		aPreferenceWrapper->Update_Preference("Timer", name, valuePtr, initialValuePtr);
	}
}

template <typename T, size_t COUNT>
void PreferencesWrapper<T, COUNT>::Update_Preference(const String &UpdateType, const String& Name, T* ValuePtr, T* InitialValuePtr)
{
    if(nullptr == m_Preferences) return;
    assert((UpdateType == "Initialized" || UpdateType == "Loaded" || UpdateType == "Updated" || UpdateType == "Timer") && "Misuse of function");

    unsigned long currentMillis = millis();
    unsigned long elapsedTime = currentMillis - m_Preferences_Last_Update;
	if (elapsedTime <= TIMER_TIME && UpdateType.equals("Updated"))
	{
		ESP_LOGI("SetDataLinkEnabled: Update_Preference", "\"%s\": To early to save preference", Name.c_str());
		if(false == m_PreferenceTimerActive)
		{
			ESP_LOGI("SetDataLinkEnabled: Update_Preference", "\"%s\": Started NVM Update Timer", Name.c_str());
			esp_timer_start_once(m_PreferenceTimer, ((TIMER_TIME - elapsedTime) + TIMER_BUFFER) * 1000);
			m_PreferenceTimerActive = true;
		}
		return;
	}

    if ( UpdateType.equals("Loaded") )
	{
        ESP_LOGI("SetDataLinkEnabled: Update_Preference", "\"%s\": Loading Preference", Name.c_str());
        HandleLoaded(Name, ValuePtr, InitialValuePtr);
    } 
	else if ( UpdateType.equals("Initialized") )
	{
        ESP_LOGI("SetDataLinkEnabled: Update_Preference", "\"%s\": Initializing Preference", Name.c_str());
		HandleUpdated( Name, InitialValuePtr );
	}
	else if ( UpdateType.equals("Updated") )
	{
        ESP_LOGI("SetDataLinkEnabled: Update_Preference", "\"%s\": Updating Preference", Name.c_str());
        HandleUpdated( Name, ValuePtr );
		m_Preferences_Last_Update = currentMillis;
	}		
	else if ( UpdateType.equals("Timer") )
	{
        ESP_LOGI("SetDataLinkEnabled: Update_Preference", "\"%s\": Updating Preference", Name.c_str());
        HandleUpdated( Name, ValuePtr );
		m_PreferenceTimerActive = false;
		m_Preferences_Last_Update = currentMillis;
    }
	else
	{
        ESP_LOGE("SetDataLinkEnabled: Update_Preference", "\"%s\": Unsupported Update Type", Name.c_str());
	}
}

template <typename T, size_t COUNT>
void PreferencesWrapper<T, COUNT>::HandleLoaded(const String& Name, T* ValuePtr, T* InitialValuePtr)
{
    if(!ValuePtr || !InitialValuePtr) return;
	if (std::is_same<T, bool>::value)
	{
		assert(COUNT == 1 && "Count should be 1 to do this");
		bool InitialValue;
		memcpy(&InitialValue , InitialValuePtr, sizeof(bool));
		bool Result = m_Preferences->getBool(Name.c_str(), InitialValue);
        memcpy(ValuePtr, &Result, sizeof(bool));
		ESP_LOGI("DataItem: HandleLoaded", "Name: \"%s\": Loaded Bool: %i", Name.c_str(), Result);	
    }
	else if (std::is_same<T, int32_t>::value)
	{
		assert(COUNT == 1 && "Count should be 1 to do this");
		int32_t InitialValue;
		memcpy(&InitialValue , InitialValuePtr, sizeof(int32_t));
		int32_t Result = m_Preferences->getInt(Name.c_str(), InitialValue);
        memcpy(ValuePtr, &Result, sizeof(int32_t));
		ESP_LOGI("DataItem: HandleLoaded", "Name: \"%s\": Loaded int32_t: %i", Name.c_str(), Result);	
    }
	else if (std::is_same<T, int16_t>::value)
	{
		assert(COUNT == 1 && "Count should be 1 to do this");
		int16_t InitialValue;
		memcpy(&InitialValue , InitialValuePtr, sizeof(int16_t));
		int16_t Result = m_Preferences->getInt(Name.c_str(), InitialValue);
        memcpy(ValuePtr, &Result, sizeof(int16_t));
		ESP_LOGI("DataItem: HandleLoaded", "Data Item: \"%s\": Loaded int16_t: %i", Name.c_str(), Result);	
    }
	else if (std::is_same<T, int8_t>::value)
	{
		assert(COUNT == 1 && "Count should be 1 to do this");
		int8_t InitialValue;
		memcpy(&InitialValue , InitialValuePtr, sizeof(int8_t));
		int8_t Result = m_Preferences->getInt(Name.c_str(), InitialValue);
        memcpy(ValuePtr, &Result, sizeof(int8_t));
		ESP_LOGI("DataItem: HandleLoaded", "Name: \"%s\": Loaded int8_t: %i", Name.c_str(), Result);	
    }
	else if (std::is_same<T, SoundInputSource_t>::value)
	{
		assert(COUNT == 1 && "Count should be 1 to do this");
		SoundInputSource_t InitialValue;
		memcpy(&InitialValue , InitialValuePtr, sizeof(SoundInputSource_t));
		SoundInputSource_t Result = static_cast<SoundInputSource_t>(m_Preferences->getInt(Name.c_str(), static_cast<int32_t>(InitialValue)));
        memcpy(ValuePtr, &Result, sizeof(SoundInputSource_t));
		ESP_LOGI("DataItem: HandleLoaded", "Name: \"%s\": Loaded int8_t: %i", Name.c_str(), Result);	
    }
	else if (std::is_same<T, float>::value)
	{
		assert(COUNT == 1 && "Count should be 1 to do this");
		float InitialValue;
		memcpy(&InitialValue , InitialValuePtr, sizeof(float));
		float Result = m_Preferences->getFloat(Name.c_str(), InitialValue);
        memcpy(ValuePtr, &Result, sizeof(float));
		ESP_LOGI("DataItem: HandleLoaded", "Data Item: \"%s\": Loaded float: %f", Name.c_str(), Result);	
    }
	else if (std::is_same<T, double>::value)
	{
		assert(COUNT == 1 && "Count should be 1 to do this");
		double InitialValue;
		memcpy(&InitialValue , InitialValuePtr, sizeof(double));
		double Result = m_Preferences->getDouble(Name.c_str(), InitialValue);
        memcpy(ValuePtr, &Result, sizeof(double));
		ESP_LOGI("DataItem: HandleLoaded", "Name: \"%s\": Loaded double: %d", Name.c_str(), Result);	
    }
	else if (std::is_same<T, char>::value)
	{
		char value[COUNT];
		char zeroChar = '\0';
		for (size_t i = 0; i < COUNT - 1; ++i)
		{
			memcpy(value+i, InitialValuePtr+i, sizeof(char));
			memcpy(ValuePtr+i, &zeroChar, sizeof(char));
		}
		value[COUNT - 1] = '\0';
		
		String Result = m_Preferences->getString(Name.c_str(), value);
		assert(Result.length() <= COUNT);
		
		size_t i = 0;
		while (i < COUNT - 1 && i < Result.length())
		{
			memcpy(ValuePtr+i, Result.c_str()+i, sizeof(char));
			++i;
		}
		ESP_LOGI("DataItem: HandleLoaded", "Name: \"%s\": Loaded String: \"%s\"", Name.c_str(), Result.c_str());
    }
	else
	{
        ESP_LOGE("DataItem: HandleLoaded", "Name: \"%s\": Unsupported Data Type", Name.c_str());
    }
}

template <typename T, size_t COUNT>
void PreferencesWrapper<T, COUNT>::HandleUpdated(const String& Name, T* ValuePtr)
{	
    if(!ValuePtr) return;
    if (std::is_same<T, bool>::value)
	{
        m_Preferences->putBool(Name.c_str(), *ValuePtr);
		ESP_LOGI("DataItem: HandleLoaded", "Data Item: \"%s\": Saving bool: %i", Name.c_str(), *ValuePtr);	
    } 
	else if (std::is_same<T, char>::value)
	{
		char charValues[COUNT];
		// Copy characters from initialValue to value, ensuring null-termination.
		for (size_t i = 0; i < COUNT - 1; ++i)
		{
			memcpy(charValues+i, ValuePtr+i, sizeof(char));
		}
		charValues[COUNT - 1] = '\0';
        m_Preferences->putString(Name.c_str(), charValues);
		ESP_LOGI("DataItem: HandleLoaded", "Data Item: \"%s\": Saving String: %s", Name.c_str(), String(charValues).c_str() );	
    }
	else if (std::is_same<T, float>::value)
	{
        m_Preferences->putFloat(Name.c_str(), *ValuePtr);
		ESP_LOGI("DataItem: HandleLoaded", "Data Item: \"%s\": Saving float: %f", Name.c_str(), *ValuePtr);	
    }
	else if (std::is_same<T, double>::value)
	{
        m_Preferences->putDouble(Name.c_str(), *ValuePtr);
		ESP_LOGI("DataItem: HandleLoaded", "Data Item: \"%s\": Saving double: %d", Name.c_str(), *ValuePtr);	
    }
	else if ( std::is_integral<T>::value ||
			  std::is_convertible<T, int32_t>::value )
	{
        m_Preferences->putInt(Name.c_str(), *ValuePtr);
		ESP_LOGI("DataItem: HandleLoaded", "Data Item: \"%s\": Saving integer: %i", Name.c_str(), *ValuePtr);	
    }  
	else 
	{
        ESP_LOGE("SetDataLinkEnabled", "Data Item: \"%s\": Unsupported Data Type", Name.c_str());
    }
}



template <typename T, size_t COUNT>
DataItem<T, COUNT>::DataItem( const String name
							, const T* initialValue
							, const RxTxType_t rxTxType
							, const UpdateStoreType_t updateStoreType
							, const uint16_t rate
							, SerialPortMessageManager &serialPortMessageManager )
							: m_Name(name)
							, mp_InitialValuePtr(initialValue)
							, m_RxTxType(rxTxType)
							, m_UpdateStoreType(updateStoreType)
							, m_Rate(rate)
							, m_SerialPortMessageManager(serialPortMessageManager)
							, NewRxTxVoidObjectCalleeInterface(COUNT)
{
	CreateTxTimer();
	m_SerialPortMessageManager.RegisterForSetupCall(this);
}

template <typename T, size_t COUNT>
DataItem<T, COUNT>::DataItem( const String name
							, const T& initialValue
							, const RxTxType_t rxTxType
							, const UpdateStoreType_t updateStoreType
							, const uint16_t rate
							, SerialPortMessageManager &serialPortMessageManager )
							: m_Name(name)
							, mp_InitialValuePtr(&initialValue)
							, m_RxTxType(rxTxType)
							, m_UpdateStoreType(updateStoreType)
							, m_Rate(rate)
							, m_SerialPortMessageManager(serialPortMessageManager)
							, NewRxTxVoidObjectCalleeInterface(COUNT)
{
	CreateTxTimer();
	m_SerialPortMessageManager.RegisterForSetupCall(this);
}

template <typename T, size_t COUNT>
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

template <typename T, size_t COUNT>				 
void DataItem<T, COUNT>::Setup()
{
	ESP_LOGI("DataItem<T, COUNT>::Setup()", "\"%s\": Allocating Memory", m_Name.c_str());
	mp_Value = (T*)heap_caps_malloc(sizeof(T)*COUNT, MALLOC_CAP_SPIRAM);
	mp_RxValue = (T*)heap_caps_malloc(sizeof(T)*COUNT, MALLOC_CAP_SPIRAM);
	mp_TxValue = (T*)heap_caps_malloc(sizeof(T)*COUNT, MALLOC_CAP_SPIRAM);
	mp_InitialValue = (T*)heap_caps_malloc(sizeof(T)*COUNT, MALLOC_CAP_SPIRAM);
	if (mp_Value && mp_RxValue && mp_TxValue && mp_InitialValue && mp_InitialValuePtr)
	{
		if (std::is_same<T, char>::value)
		{
			String InitialValue = String((char*)mp_InitialValuePtr);
			ESP_LOGI( "DataItem<T, COUNT>::Setup()", "\"%s\": Setting initial value: \"%s\""
					, m_Name.c_str()
					, InitialValue.c_str());
			bool eolFound = false;
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
				memcpy(mp_RxValue+i, &value, sizeof(char));
				memcpy(mp_TxValue+i, &value, sizeof(char));
				memcpy(mp_InitialValue+i, &value, sizeof(char));
			}
		}
		else
		{
			ESP_LOGI( "DataItem<T, COUNT>::Setup()", "\"%s\": Setting initial value: \"%s\""
					, m_Name.c_str()
					, GetValueAsStringForDataType(mp_InitialValuePtr, GetDataTypeFromTemplateType<T>(), COUNT, "").c_str());
			for (size_t i = 0; i < COUNT; ++i)
			{
				memcpy(mp_Value+i, mp_InitialValuePtr, sizeof(T));
				this->CallCallbacks(m_Name.c_str(), mp_Value);
				memcpy(mp_RxValue+i, mp_InitialValuePtr, sizeof(T));
				memcpy(mp_TxValue+i, mp_InitialValuePtr, sizeof(T));
				memcpy(mp_InitialValue+i, mp_InitialValuePtr, sizeof(T));
			}
		}
		SetDataLinkEnabled(true);
	}
	else
	{
		ESP_LOGE("DataItem<T, COUNT>::Setup()", "Failed to allocate memory on SPI RAM");
    }
}

template <typename T, size_t COUNT>
void DataItem<T, COUNT>::CreateTxTimer()
{
	esp_timer_create_args_t timerArgs;
	timerArgs.callback = &StaticDataItem_Periodic_TX;
	timerArgs.arg = this;
	timerArgs.name = "Tx_Timer";

	// Create the timer
	esp_timer_create(&timerArgs, &m_TxTimer);
}

template <typename T, size_t COUNT>
String DataItem<T, COUNT>::GetName()
{
	return m_Name;
}

template <typename T, size_t COUNT>
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

template <typename T, size_t COUNT>
String DataItem<T, COUNT>::GetValueAsString(const String &Divider)
{
	return GetValueAsStringForDataType(mp_Value, GetDataTypeFromTemplateType<T>(), COUNT, Divider);
}

template <typename T, size_t COUNT>
void DataItem<T, COUNT>::SetNewTxValue(const T* Value, const size_t Count)
{
	ESP_LOGD("DataItem: SetNewTxValue", "\"%s\" SetNewTxValue to: \"%s\"", m_Name.c_str(), GetValueAsStringForDataType(Value, GetDataTypeFromTemplateType<T>(), COUNT, ""));
	SetValue(Value, Count);
}

template <typename T, size_t COUNT>
void DataItem<T, COUNT>::SetValue(const T *Value, size_t Count)
{
	assert(Value != nullptr && "Value must not be null");
	assert(mp_Value != nullptr && "mp_Value must not be null");
	assert(COUNT > 0 && "COUNT must be a valid index range for mp_Value");
	assert(COUNT == Count && "Counts must match");
	ESP_LOGD( "DataItem: SetValue"
			, "\"%s\" Set Value: \"%s\""
			, m_Name.c_str()
			, GetValueAsStringForDataType(Value, GetDataTypeFromTemplateType<T>(), COUNT, "").c_str());
	bool ValueChanged = (memcmp(mp_TxValue, Value, sizeof(T) * COUNT) != 0);
	memcpy(mp_TxValue, Value, sizeof(T) * COUNT);
	if(ValueChanged)
	{
		DataItem_Try_TX_On_Change();
	}
}

template <typename T, size_t COUNT>
size_t DataItem<T, COUNT>::GetCount()
{
	return m_Count;
}

template <typename T, size_t COUNT>
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

template <typename T, size_t COUNT>
void DataItem<T, COUNT>::DataItem_Try_TX_On_Change()
{
	ESP_LOGD("DataItem& DataItem_Try_TX_On_Change", "Data Item: \"%s\": Try TX On Change", m_Name.c_str());
	if(m_RxTxType == RxTxType_Tx_On_Change || m_RxTxType == RxTxType_Tx_On_Change_With_Heartbeat)
	{
		DataItem_TX_Now();
	}
}

template <typename T, size_t COUNT>
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
				this->CallCallbacks(m_Name.c_str(), mp_Value);
				ValueUpdated = true;
			}
		}
		ESP_LOGD("DataItem: DataItem_TX_Now", "Data Item: \"%s\": TX Now: \"%s\"", m_Name.c_str(), GetValueAsStringForDataType(mp_TxValue, GetDataTypeFromTemplateType<T>(), COUNT, "").c_str());
	}
	else
	{
		ESP_LOGE("DataItem: DataItem_TX_Now", "Data Item: \"%s\": Unable to Tx Message", m_Name.c_str());
	}
	return ValueUpdated;
}

template <typename T, size_t COUNT>
void DataItem<T, COUNT>::StaticDataItem_Periodic_TX(void *arg)
{
	DataItem *aDataItem = static_cast<DataItem*>(arg);
	if(aDataItem)
	{
		aDataItem->DataItem_Periodic_TX();
	}
}

template <typename T, size_t COUNT>
void DataItem<T, COUNT>::DataItem_Periodic_TX()
{
	if(m_SerialPortMessageManager.QueueMessageFromData(m_Name, GetDataTypeFromTemplateType<T>(), mp_Value, COUNT))
	{
		ESP_LOGD("DataItem: DataItem_TX_Now", "Data Item: \"%s\": Periodic TX: \"%s\"", m_Name.c_str(), GetValueAsStringForDataType(mp_Value, GetDataTypeFromTemplateType<T>(), COUNT, "").c_str());
	}
}

template <typename T, size_t COUNT>
bool DataItem<T, COUNT>::NewRXValueReceived(void* Object, size_t Count)
{	
	bool ValueUpdated = false;
	T* receivedValue = static_cast<T*>(Object);
	bool ValueChanged = (memcmp(mp_RxValue, receivedValue, sizeof(T) * COUNT) != 0);
	if(ValueChanged)
	{
		memcpy(mp_RxValue, receivedValue, sizeof(T) * COUNT);
		ESP_LOGD( "DataItem: NewRXValueReceived"
				, "\"%s\" New RX Value Received: \"%s\""
				, m_Name.c_str()
				, GetValueAsStringForDataType(mp_RxValue, GetDataTypeFromTemplateType<T>(), COUNT, "").c_str());
		
		bool RxValueChanged = (memcmp(mp_Value, mp_RxValue, sizeof(T) * COUNT) != 0);
		if( UpdateStoreType_On_Rx == m_UpdateStoreType )
		{
			if(RxValueChanged)
			{
				memcpy(mp_Value, mp_RxValue, sizeof(T) * COUNT);
				this->CallCallbacks(m_Name.c_str(), mp_Value);
				ValueUpdated = true;
			}
		}
	}
	if(RxTxType_Rx_Echo_Value == m_RxTxType)
	{
		memcpy(mp_TxValue, mp_RxValue, sizeof(T) * COUNT);
		DataItem_TX_Now();
	}
	return ValueUpdated;
}

template <typename T, size_t COUNT>
DataItemWithPreferences<T, COUNT>::DataItemWithPreferences( const String name
														  , const T* initialValue
														  , const RxTxType_t rxTxType
														  , const UpdateStoreType_t updateStoreType
														  , const uint16_t rate
														  , Preferences *preferences
														  , SerialPortMessageManager &serialPortMessageManager )
														  : PreferencesWrapper<T, COUNT>(preferences)
														  , DataItem<T, COUNT>( name
																			  , initialValue
																			  , rxTxType
																			  , updateStoreType
																			  , rate
																			  , serialPortMessageManager )
							   
{
	this->CreatePreferencesTimer(this->GetName().c_str(), this->mp_Value, this->mp_InitialValue);
}

template <typename T, size_t COUNT>
DataItemWithPreferences<T, COUNT>::DataItemWithPreferences( const String name
														  , const T& initialValue
														  , const RxTxType_t rxTxType
														  , const UpdateStoreType_t updateStoreType
														  , const uint16_t rate
														  , Preferences *preferences
														  , SerialPortMessageManager &serialPortMessageManager )
														  : PreferencesWrapper<T, COUNT>(preferences)
														  , DataItem<T, COUNT>( name
																			  , initialValue
																			  , rxTxType
																			  , updateStoreType
																			  , rate
																			  , serialPortMessageManager )
							   
{
	this->CreatePreferencesTimer(this->GetName().c_str(), this->mp_Value, this->mp_InitialValue);
}

template <typename T, size_t COUNT>
void DataItemWithPreferences<T, COUNT>::Setup()
{
	DataItem<T, COUNT>::Setup();
	this->InitializeNVM(this->GetName().c_str(), this->mp_Value, this->mp_InitialValue);
}

template <typename T, size_t COUNT>
bool DataItemWithPreferences<T, COUNT>::DataItem_TX_Now()
{
	bool result = DataItem<T, COUNT>::DataItem_TX_Now();
	if(result) this->Update_Preference("Updated", this->GetName().c_str(), this->mp_Value, this->mp_InitialValue);
	return result;
}

template <typename T, size_t COUNT>
bool DataItemWithPreferences<T, COUNT>::NewRXValueReceived(void* Object, size_t Count)
{
	bool result = DataItem<T, COUNT>::NewRXValueReceived(Object, Count);
	if(result) this->Update_Preference("Updated", this->GetName().c_str(), this->mp_RxValue, this->mp_InitialValue);
	return result;
}
