#pragma once
#include "DataTypes.h"

#define TIMER_TIME 10000UL
#define TIMER_BUFFER 1000UL

template <typename T, size_t COUNT>
class PreferencesWrapper: public DataTypeFunctions
{
	
	public:
		enum PreferenceUpdateType
		{
			Initialize,
			Load,
			Save,
			Timer
		};
		struct PreferencesWrapperTimerArgs;
		PreferencesWrapper( Preferences *Preferences )
						  : mp_Preferences(Preferences){}
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
			if(aPreferenceWrapper)
			{
				aPreferenceWrapper->Update_Preference(PreferenceUpdateType::Timer, timerArgsPtr->Name, timerArgsPtr->Value, timerArgsPtr->InitialValue);
			}
		}
		void Update_Preference(const PreferenceUpdateType &UpdateType, const String& Name, String value, const String initialValue)
		{
			if(nullptr == mp_Preferences) return;

			unsigned long currentMillis = millis();
			unsigned long elapsedTime = currentMillis - m_Preferences_Last_Update;
			if (elapsedTime <= TIMER_TIME && UpdateType == PreferenceUpdateType::Save)
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
			if ( PreferenceUpdateType::Timer == UpdateType )
			{
				ESP_LOGD("SetDataLinkEnabled: Update_Preference", "\"%s\": Delayed Save", Name.c_str());
				HandleSave( Name, value );
				m_PreferenceTimerActive = false;
				m_Preferences_Last_Update = currentMillis;
			}
			else if ( PreferenceUpdateType::Initialize == UpdateType )
			{
				ESP_LOGD("SetDataLinkEnabled: Update_Preference", "\"%s\": Initializing Preference", Name.c_str());
				HandleSave( Name, initialValue );
			}
			else if ( PreferenceUpdateType::Load == UpdateType )
			{
				ESP_LOGD("SetDataLinkEnabled: Update_Preference", "\"%s\": Loading Preference", Name.c_str());
				HandleLoad( Name, value, initialValue );
			} 
			else if ( PreferenceUpdateType::Save == UpdateType )
			{
				ESP_LOGD("SetDataLinkEnabled: Update_Preference", "\"%s\": Updating Preference", Name.c_str());
				HandleSave( Name, value );
				m_Preferences_Last_Update = currentMillis;
			}
			else
			{
				ESP_LOGE("SetDataLinkEnabled: Update_Preference", "\"%s\": Unsupported Update Type", Name.c_str());
			}
		}
		struct PreferencesWrapperTimerArgs 
		{
			PreferencesWrapperTimerArgs(PreferencesWrapper<T, COUNT>* preferenceWrapper, const String& name, String value, const String initialValue)
				: PreferenceWrapper(preferenceWrapper), Name(name), Value(value), InitialValue(initialValue) {}

			PreferencesWrapper<T, COUNT>* PreferenceWrapper;
			String Name;
			String Value;
			String InitialValue;
		};
	protected:
		void CreatePreferencesTimer(const String& key, String value, const String initialValue)
		{
			mp_TimerArgs = new PreferencesWrapperTimerArgs(this, key, value, initialValue);
			esp_timer_create_args_t timerArgs;
			timerArgs.callback = &Static_Update_Preference;
			timerArgs.arg = mp_TimerArgs;
			timerArgs.name = "Preferences_Timer";

			// Create the timer
			esp_timer_create(&timerArgs, &m_PreferenceTimer);
		}

		void InitializeNVM(const String& key, String value, const String initialValue)
		{
			if(mp_Preferences)
			{
				if (mp_Preferences->isKey(key.c_str()))
				{
					ESP_LOGI("InitializeNVM", "Preference Found: \"%s\"", key.c_str());
					Update_Preference(PreferenceUpdateType::Load, key, value, initialValue);
				}
				else
				{
					ESP_LOGE("InitializeNVM", "Preference Not Found: \"%s\"", key.c_str());
					Update_Preference(PreferenceUpdateType::Initialize, key, value, initialValue);
				}
			}
			else
			{
				ESP_LOGE("PreferencesWrapper: InitializeNVM", "Null Preferences Pointer!");
			}
		}
		void HandleLoad(const String& key, String value, const String initialValue)
		{
			value = mp_Preferences->getString( key.c_str(), initialValue.c_str() );
			ESP_LOGI("PreferencesWrapper: HandleLoaded", "Loaded Key: \"%s\" Value: \"%s\"", key.c_str(), value.c_str());	
		}
		void HandleSave(const String& key, String value)
		{		
			mp_Preferences->putString( key.c_str(), value );
			ESP_LOGI("PreferencesWrapper: Handle Updated", "Saved Key: \"%s\" Value: \"%s\"", key.c_str(), value.c_str());
		}
	private:
		Preferences *mp_Preferences = nullptr;
		esp_timer_handle_t m_PreferenceTimer;
		PreferencesWrapperTimerArgs* mp_TimerArgs;
		uint64_t m_Preferences_Last_Update = millis();
		bool m_PreferenceTimerActive = false;
};