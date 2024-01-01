#include "DataItem.h"

template class DataItem<float, 1>;
template class DataItem<bool, 1>;
template class DataItem<ConnectionStatus_t, 1>;


template <typename T, int COUNT>
DataItem<T, COUNT>::DataItem( const String name
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

template <typename T, int COUNT>
DataItem<T, COUNT>::DataItem( const String name
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
	InitializeNVM();
	LoadFromNVM();
}

template <typename T, int COUNT>
void DataItem<T, COUNT>::InitializeNVM()
{
	if(m_Preferences)
	{
		if(1 == COUNT)
		{
			if (false == m_Preferences->isKey(m_Name.c_str()))
			{
				Update_Preference("Initialized");
			}
			else
			{
				ESP_LOGI("Dataitem: InitializeNVM", "\"%s\" NVM Found", m_Name.c_str());
			}
		}
		else
		{
			ESP_LOGE("Dataitem: InitializeNVM", "Cannot use preferences for items with non 1 COUNT size");
		}
	}
}

template <typename T, int COUNT>
void DataItem<T, COUNT>::LoadFromNVM()
{
	
}

template <typename T, int COUNT>
void DataItem<T, COUNT>::CreateTxTimer()
{
	esp_timer_create_args_t timerArgs;
	timerArgs.callback = &StaticDataItem_TX_Now;
	timerArgs.arg = this;
	timerArgs.name = "Tx_Timer";

	// Create the timer
	esp_timer_create(&timerArgs, &m_TxTimer);
}

template <typename T, int COUNT>
void DataItem<T, COUNT>::CreatePreferencesTimer()
{
	esp_timer_create_args_t timerArgs;
	timerArgs.callback = &Static_Update_Preference;
	timerArgs.arg = this;
	timerArgs.name = "Preferences_Timer";

	// Create the timer
	esp_timer_create(&timerArgs, &m_PreferenceTimer);
}

template <typename T, int COUNT>
void DataItem<T, COUNT>::ResetPreferencesTimer()
{
	esp_timer_stop(m_PreferenceTimer);
	esp_timer_start_periodic(m_PreferenceTimer, 300000 * 1000);
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
	return *mp_Value;
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
void DataItem<T, COUNT>::Static_Update_Preference(void *arg)
{
	DataItem *aDataItem = static_cast<DataItem*>(arg);
	if(aDataItem)
	{
		aDataItem->Update_Preference("Updated");
	}
}

template <typename T, int COUNT>
void DataItem<T, COUNT>::Update_Preference(const String UpdateType)
{
	/*
	if (std::is_same<T, String>::value)
	{
		m_Preferences->putString(m_Name.c_str(), mp_Value[0]);
		ESP_LOGI("Dataitem: Update_Preference", "\"%s\" %s STRING Preference", m_Name.c_str(), UpdateType.c_str());
	} 
	else*/ if (std::is_same<T, bool>::value)
	{
		m_Preferences->putBool(m_Name.c_str(), mp_Value[0]); // Assuming bool is stored as an integer
		ESP_LOGI("Dataitem: InitializeNVM", "\"%s\" $s BOOL Preference", m_Name.c_str(), UpdateType.c_str());
	}
	else if (std::is_integral<T>::value)
	{
		m_Preferences->putInt(m_Name.c_str(), mp_Value[0]);
		ESP_LOGI("Dataitem: InitializeNVM", "\"%s\" $s INTEGRAL Preference", m_Name.c_str(), UpdateType.c_str());
	}
	else if (std::is_floating_point<T>::value)
	{
		m_Preferences->putFloat(m_Name.c_str(), mp_Value[0]);
		ESP_LOGI("Dataitem: InitializeNVM", "\"%s\" $s FLOAT Preference", m_Name.c_str(), UpdateType.c_str());
	}
	else if (std::is_same<T, double>::value)
	{
		m_Preferences->putDouble(m_Name.c_str(), mp_Value[0]);
		ESP_LOGI("Dataitem: InitializeNVM", "\"%s\" $s DOUBLE Preference", m_Name.c_str(), UpdateType.c_str());
	}
	else
	{
		ESP_LOGE("Dataitem: InitializeNVM", "\"%s\" data type is not supported for Preference", m_Name.c_str());
	}
	ResetPreferencesTimer();
}

template <typename T, int COUNT>
void DataItem<T, COUNT>::StaticDataItem_TX_Now(void *arg)
{
	DataItem *aDataItem = static_cast<DataItem*>(arg);
	if(aDataItem)
	{
		aDataItem->DataItem_TX_Now();
	}
}

template <typename T, int COUNT>
void DataItem<T, COUNT>::DataItem_TX_Now()
{
	if(m_SerialPortMessageManager.QueueMessageFromData(m_Name, GetDataTypeFromTemplateType<T>(), mp_TxValue, COUNT))
	{
		bool TxValueChanged = (memcmp(mp_Value, mp_TxValue, sizeof(T) * COUNT) != 0);
		if(m_UpdateStoreType == UpdateStoreType_On_Tx)
		{
			if(TxValueChanged) memcpy(mp_Value, mp_TxValue, sizeof(T) * COUNT);
			Update_Preference("Updated");
		}
		ESP_LOGD("DataItem: DataItem_TX_Now", "Data Item: \"%s\": TX Now: \"%s\"", m_Name.c_str(), GetValueAsStringForDataType(mp_Value, GetDataTypeFromTemplateType<T>(), COUNT).c_str());
	}
	else
	{
		ESP_LOGE("DataItem: DataItem_TX_Now", "Data Item: \"%s\": Unable to Tx Message", m_Name.c_str());
	}
	
}

template <typename T, int COUNT>
void DataItem<T, COUNT>::NewRXValueReceived(void* Object)
{	
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
			if(RxValueChanged) memcpy(mp_Value, mp_RxValue, sizeof(T) * COUNT);
			Update_Preference("Updated");
		}
		if(RxTxType_Rx_Echo_Value == m_RxTxType)
		{
			memcpy(mp_TxValue, mp_RxValue, sizeof(T) * COUNT);
			DataItem_TX_Now();
		}
	}
}