#pragma once
#include "DataTypes.h"

#define TIMER_TIME 10000UL
#define TIMER_BUFFER 1000UL

template <typename T, size_t COUNT>
class PreferencesWrapper: public DataTypeFunctions
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
			assert((UpdateType == "Initialize" || UpdateType == "Load" || UpdateType == "Save" || UpdateType == "Timer") && "Misuse of function");

			unsigned long currentMillis = millis();
			unsigned long elapsedTime = currentMillis - m_Preferences_Last_Update;
			if (elapsedTime <= TIMER_TIME && UpdateType.equals("Save"))
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
				ESP_LOGD("SetDataLinkEnabled: Update_Preference", "\"%s\": Delayed Save", Name.c_str());
				HandleSave( Name, ValuePtr );
				m_PreferenceTimerActive = false;
				m_Preferences_Last_Update = currentMillis;
			}
			else if ( UpdateType.equals("Initialize") )
			{
				ESP_LOGD("SetDataLinkEnabled: Update_Preference", "\"%s\": Initializing Preference", Name.c_str());
				HandleSave( Name, InitialValuePtr );
			}
			else if ( UpdateType.equals("Load") )
			{
				ESP_LOGD("SetDataLinkEnabled: Update_Preference", "\"%s\": Loading Preference", Name.c_str());
				HandleLoad( Name, ValuePtr, InitialValuePtr );
			} 
			else if ( UpdateType.equals("Save") )
			{
				ESP_LOGD("SetDataLinkEnabled: Update_Preference", "\"%s\": Updating Preference", Name.c_str());
				HandleSave( Name, ValuePtr );
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
		void CreatePreferencesTimer(const String& key, T* value, T* initialValue)
		{
			mp_TimerArgs = new PreferencesWrapperTimerArgs(this, key, value, initialValue);
			esp_timer_create_args_t timerArgs;
			timerArgs.callback = &Static_Update_Preference;
			timerArgs.arg = mp_TimerArgs;
			timerArgs.name = "Preferences_Timer";

			// Create the timer
			esp_timer_create(&timerArgs, &m_PreferenceTimer);
		}

		void InitializeNVM(const String& key, T* valuePtr, T* initialValuePtr)
		{
			if(m_Preferences)
			{
				if (m_Preferences->isKey(key.c_str()))
				{
					ESP_LOGI("InitializeNVM", "Preference Found: \"%s\"", key.c_str());
					Update_Preference("Load", key, valuePtr, initialValuePtr);
				}
				else
				{
					ESP_LOGE("InitializeNVM", "Preference Not Found: \"%s\"", key.c_str());
					Update_Preference("Initialize", key, valuePtr, initialValuePtr);
				}
			}
			else
			{
				ESP_LOGE("PreferencesWrapper: InitializeNVM", "Key: \"%s\": Null Value Pointer!", key.c_str());
			}
		}
		void HandleLoad(const String& key, T* valuePtr, T* initialValuePtr)
		{
			if(valuePtr && initialValuePtr)
			{ 
				String value = m_Preferences->getString( key.c_str()
														, GetValueAsStringForDataType( initialValuePtr
																					 , GetDataTypeFromTemplateType<T>()
																					 , COUNT
														  							 , "|") );
				SetValueFromStringForDataType( valuePtr
										 	 , value
										 	 , GetDataTypeFromTemplateType<T>() );
				ESP_LOGI("PreferencesWrapper: HandleLoaded", "Loaded Key: \"%s\" Value: \"%s\"", key.c_str(), value.c_str());
			}
			else
			{
				ESP_LOGE("PreferencesWrapper: HandleLoaded", "Key: \"%s\": Null Value Pointer!", key.c_str());
			}
		}
		void HandleSave(const String& key, T* valuePtr)
		{	
			if(valuePtr)
			{
				String value = GetValueAsStringForDataType( valuePtr 
														  , GetDataTypeFromTemplateType<T>()
														  , COUNT
														  , "|" );
				m_Preferences->putString( key.c_str()
										, value );
				ESP_LOGI("PreferencesWrapper: Handle Updated", "Saved Key: \"%s\" Value: \"%s\"", key.c_str(), value.c_str());
			}
			else
			{
				ESP_LOGE("PreferencesWrapper: Handle Updated", "Key: \"%s\": Null Value Pointer!", key.c_str());
			}
		}
	private:
		Preferences *m_Preferences = nullptr;
		esp_timer_handle_t m_PreferenceTimer;
		PreferencesWrapperTimerArgs* mp_TimerArgs;
		uint64_t m_Preferences_Last_Update = millis();
		bool m_PreferenceTimerActive = false;
};