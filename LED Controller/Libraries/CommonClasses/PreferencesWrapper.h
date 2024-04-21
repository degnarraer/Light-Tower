#pragma once

#define TIMER_TIME 10000UL
#define TIMER_BUFFER 1000UL

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
		static void Static_Update_Preference(void *arg)
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
		void Update_Preference(const String &UpdateType, const String& Name, T* ValuePtr, T* InitialValuePtr)
		{
			if(nullptr == m_Preferences) return;
			assert((UpdateType == "Initialized" || UpdateType == "Loaded" || UpdateType == "Updated" || UpdateType == "Timer") && "Misuse of function");

			unsigned long currentMillis = millis();
			unsigned long elapsedTime = currentMillis - m_Preferences_Last_Update;
			if (elapsedTime <= TIMER_TIME && UpdateType.equals("Updated"))
			{
				ESP_LOGD("SetDataLinkEnabled: Update_Preference", "\"%s\": To early to save preference", Name.c_str());
				if(false == m_PreferenceTimerActive)
				{
					ESP_LOGD("SetDataLinkEnabled: Update_Preference", "\"%s\": Started NVM Update Timer", Name.c_str());
					esp_timer_start_once(m_PreferenceTimer, ((TIMER_TIME - elapsedTime) + TIMER_BUFFER) * 1000);
					m_PreferenceTimerActive = true;
				}
				return;
			}	
			if ( UpdateType.equals("Timer") )
			{
				ESP_LOGD("SetDataLinkEnabled: Update_Preference", "\"%s\": Delayed Preference Update", Name.c_str());
				HandleUpdated( Name, ValuePtr );
				m_PreferenceTimerActive = false;
				m_Preferences_Last_Update = currentMillis;
			}
			else if ( UpdateType.equals("Loaded") )
			{
				ESP_LOGD("SetDataLinkEnabled: Update_Preference", "\"%s\": Loading Preference", Name.c_str());
				HandleLoaded(Name, ValuePtr, InitialValuePtr);
			} 
			else if ( UpdateType.equals("Initialized") )
			{
				ESP_LOGD("SetDataLinkEnabled: Update_Preference", "\"%s\": Initializing Preference", Name.c_str());
				HandleUpdated( Name, InitialValuePtr );
			}
			else if ( UpdateType.equals("Updated") )
			{
				ESP_LOGD("SetDataLinkEnabled: Update_Preference", "\"%s\": Updating Preference", Name.c_str());
				HandleUpdated( Name, ValuePtr );
				m_Preferences_Last_Update = currentMillis;
			}
			else
			{
				ESP_LOGE("SetDataLinkEnabled: Update_Preference", "\"%s\": Unsupported Update Type", Name.c_str());
			}
		}
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
		void CreatePreferencesTimer(const String& Name, T* Value, T* InitialValue)
		{
			mp_TimerArgs = new PreferencesWrapperTimerArgs(this, Name, Value, InitialValue);
			esp_timer_create_args_t timerArgs;
			timerArgs.callback = &Static_Update_Preference;
			timerArgs.arg = mp_TimerArgs;
			timerArgs.name = "Preferences_Timer";

			// Create the timer
			esp_timer_create(&timerArgs, &m_PreferenceTimer);
		}
		void InitializeNVM(const String& Name, T* ValuePtr, T* InitialValuePtr)
		{
			if(m_Preferences)
			{
				if (m_Preferences->isKey(Name.c_str()))
				{
					ESP_LOGD("InitializeNVM", "Preference Found: \"%s\"", Name.c_str());
					Update_Preference("Loaded", Name, ValuePtr, InitialValuePtr);
				}
				else
				{
					ESP_LOGE("InitializeNVM", "Preference Not Found: \"%s\"", Name.c_str());
					Update_Preference("Initialized", Name, ValuePtr, InitialValuePtr);
				}
			}
		}
		void HandleLoaded(const String& Name, T* ValuePtr, T* InitialValuePtr)
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
				ESP_LOGI("DataItem: HandleLoaded", "Name: \"%s\": Loaded SoundInputSource_t: %i", Name.c_str(), Result);	
			}
			else if (std::is_same<T, SoundOutputSource_t>::value)
			{
				assert(COUNT == 1 && "Count should be 1 to do this");
				SoundOutputSource_t InitialValue;
				memcpy(&InitialValue , InitialValuePtr, sizeof(SoundOutputSource_t));
				SoundOutputSource_t Result = static_cast<SoundOutputSource_t>(m_Preferences->getInt(Name.c_str(), static_cast<uint32_t>(InitialValue)));
				memcpy(ValuePtr, &Result, sizeof(SoundOutputSource_t));
				ESP_LOGI("DataItem: HandleLoaded", "Name: \"%s\": Loaded SoundOutputSource_t: %i", Name.c_str(), Result);	
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
		void HandleUpdated(const String& Name, T* ValuePtr)
		{	
			if(ValuePtr)
			{
				if (std::is_same<T, bool>::value)
				{
					ESP_LOGI("DataItem: HandleUpdated", "Data Item: \"%s\": Saving bool: %i", Name.c_str(), *ValuePtr);
					m_Preferences->putBool(Name.c_str(), *ValuePtr);
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
					ESP_LOGI("DataItem: HandleUpdated", "Data Item: \"%s\": Saving String: %s", Name.c_str(), String(charValues).c_str() );
					m_Preferences->putString(Name.c_str(), charValues);
				}
				else if (std::is_same<T, float>::value)
				{
					ESP_LOGI("DataItem: HandleUpdated", "Data Item: \"%s\": Saving float: %f", Name.c_str(), *ValuePtr);
					m_Preferences->putFloat(Name.c_str(), *ValuePtr);	
				}
				else if (std::is_same<T, double>::value)
				{
					ESP_LOGI("DataItem: HandleUpdated", "Data Item: \"%s\": Saving double: %d", Name.c_str(), *ValuePtr);
					m_Preferences->putDouble(Name.c_str(), *ValuePtr);
				}
				else if ( std::is_integral<T>::value ||
						std::is_convertible<T, int32_t>::value )
				{
					ESP_LOGI("DataItem: HandleUpdated", "Data Item: \"%s\": Saving integer: %i", Name.c_str(), *ValuePtr);
					m_Preferences->putInt(Name.c_str(), *ValuePtr);
				}  
				else 
				{
					ESP_LOGE("SetDataLinkEnabled", "Data Item: \"%s\": Unsupported Data Type", Name.c_str());
				}
			}
			else
			{
				ESP_LOGE("SetDataLinkEnabled", "Data Item: \"%s\": Null Value Pointer!", Name.c_str());
			}
		}
	private:
		Preferences *m_Preferences = nullptr;
		esp_timer_handle_t m_PreferenceTimer;
		PreferencesWrapperTimerArgs* mp_TimerArgs;
		uint64_t m_Preferences_Last_Update = millis();
		bool m_PreferenceTimerActive = false;
};