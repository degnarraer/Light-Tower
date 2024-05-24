#pragma once
#include "DataTypes.h"

#define TIMER_TIME 10000UL
#define TIMER_BUFFER 1000UL

typedef const std::vector<String> ValidValues_t;

template <size_t COUNT>
class PreferencesWrapper : public DataTypeFunctions
{
public:
	bool (*loadedValueCallback)(const String&);
    enum class PreferenceUpdateType
    {
        Initialize,
        Load,
        Save,
        Timer
    };

    struct PreferencesWrapperTimerArgs
    {
        PreferencesWrapperTimerArgs(PreferencesWrapper<COUNT>* preferenceWrapper, const String& name, String value, const String& initialValue)
            : PreferenceWrapper(preferenceWrapper), Name(name), Value(value), InitialValue(initialValue) {}

        PreferencesWrapper<COUNT>* PreferenceWrapper;
        String Name;
        String Value;
        String InitialValue;
    };

    PreferencesWrapper(Preferences* preferences)
        : mp_Preferences(preferences)
		, mp_TimerArgs(nullptr)
		, m_PreferenceTimer(nullptr)
		, m_Preferences_Last_Update(0)
		, m_PreferenceTimerActive(false) {}

    PreferencesWrapper(Preferences* preferences, ValidValues_t *validValues)
        : mp_Preferences(preferences)
        , mp_ValidValues(validValues)
		, mp_TimerArgs(nullptr)
		, m_PreferenceTimer(nullptr)
		, m_Preferences_Last_Update(0)
		, m_PreferenceTimerActive(false) {}

    virtual ~PreferencesWrapper()
    {
        if (mp_TimerArgs)
        {
            delete mp_TimerArgs;
        }
    }

    static void Static_Update_Preference(void* arg)
    {
        PreferencesWrapperTimerArgs* timerArgsPtr = static_cast<PreferencesWrapperTimerArgs*>(arg);
        PreferencesWrapper<COUNT>* aPreferenceWrapper = timerArgsPtr->PreferenceWrapper;
        if (aPreferenceWrapper)
        {
            aPreferenceWrapper->Update_Preference(PreferenceUpdateType::Timer, timerArgsPtr->Name, timerArgsPtr->Value, timerArgsPtr->InitialValue, nullptr);
        }
    }

    void Update_Preference( const PreferenceUpdateType& updateType
						  , const String& name
						  , const String& saveValue
						  , const String& initialValue
						  , bool (*loadedValueCallback)(const String&))
    {
        if (!mp_Preferences) return;

        unsigned long currentMillis = millis();
        unsigned long elapsedTime = currentMillis - m_Preferences_Last_Update;
        if (elapsedTime <= TIMER_TIME && updateType == PreferenceUpdateType::Save)
        {
            ESP_LOGD("SetDataLinkEnabled: Update_Preference", "\"%s\": Too early to save preference", name.c_str());
            if (!m_PreferenceTimerActive)
            {
                ESP_LOGD("SetDataLinkEnabled: Update_Preference", "\"%s\": Started NVM Update Timer", name.c_str());
                esp_timer_start_once(m_PreferenceTimer, ((TIMER_TIME - elapsedTime) + TIMER_BUFFER) * 1000);
                m_PreferenceTimerActive = true;
            }
            return;
        }

        switch (updateType)
        {
        case PreferenceUpdateType::Timer:
            ESP_LOGD("SetDataLinkEnabled: Update_Preference", "\"%s\": Delayed Save", name.c_str());
            HandleSave(name, saveValue);
            m_PreferenceTimerActive = false;
            m_Preferences_Last_Update = currentMillis;
            break;
        case PreferenceUpdateType::Initialize:
            ESP_LOGD("SetDataLinkEnabled: Update_Preference", "\"%s\": Initializing Preference", name.c_str());
            HandleSave(name, initialValue);
            break;
        case PreferenceUpdateType::Load:
            ESP_LOGD("SetDataLinkEnabled: Update_Preference", "\"%s\": Loading Preference", name.c_str());
            HandleLoad(name, initialValue, loadedValueCallback);
            break;
        case PreferenceUpdateType::Save:
            ESP_LOGD("SetDataLinkEnabled: Update_Preference", "\"%s\": Updating Preference", name.c_str());
            HandleSave(name, saveValue);
            m_Preferences_Last_Update = currentMillis;
            break;
        default:
            ESP_LOGE("SetDataLinkEnabled: Update_Preference", "\"%s\": Unsupported Update Type", name.c_str());
            break;
        }
    }

protected:
    void CreatePreferencesTimer(const String& key, const String& value, const String& initialValue)
    {
        mp_TimerArgs = new PreferencesWrapperTimerArgs(this, key, value, initialValue);
        esp_timer_create_args_t timerArgs;
        timerArgs.callback = &Static_Update_Preference;
        timerArgs.arg = mp_TimerArgs;
        timerArgs.name = "Preferences_Timer";
        esp_timer_create(&timerArgs, &m_PreferenceTimer);
    }

    void InitializeNVM(const String& key, const String& initialValue, bool (*loadedValueCallback)(const String&))
    {
        if (mp_Preferences)
        {
            if (mp_Preferences->isKey(key.c_str()))
            {
                ESP_LOGI("InitializeNVM", "Preference Found: \"%s\"", key.c_str());
                Update_Preference(PreferenceUpdateType::Load, key, initialValue, initialValue, loadedValueCallback);
            }
            else
            {
                ESP_LOGE("InitializeNVM", "Preference Not Found: \"%s\"", key.c_str());
                Update_Preference(PreferenceUpdateType::Initialize, key, initialValue, initialValue, loadedValueCallback);
            }
        }
        else
        {
            ESP_LOGE("PreferencesWrapper: InitializeNVM", "Null Preferences Pointer!");
        }
    }

    void HandleLoad(const String& key, const String& initialValue, bool (*loadedValueCallback)(const String&))
    {
        String loadedValue = mp_Preferences->getString(key.c_str(), initialValue.c_str());
        bool validValue = true;
        if (loadedValueCallback)
        {
            if(mp_ValidValues)
            {
                auto it = std::find(mp_ValidValues->begin(), mp_ValidValues->end(), loadedValue);
                if (mp_ValidValues->end() == it) validValue = false;
            }
            if(validValue)
            {
                if (loadedValueCallback(loadedValue))
                {
                    ESP_LOGI("PreferencesWrapper: HandleLoad", "Loaded Key: \"%s\" Value: \"%s\"", key.c_str(), loadedValue.c_str());
                }
                else
                {
                    ESP_LOGE("HandleLoad", "Failed to Load Value!");
                }
            }
            else
            {
                ESP_LOGE("HandleLoad", "Invalid Value!");
            }
        }
        else
        {
            ESP_LOGE("HandleLoad", "Null Callback Pointer!");
        }
    }

    void HandleSave(const String& key, const String& saveValue)
    {
        assert(saveValue.length() && "Save Value must be provided");
        mp_Preferences->putString(key.c_str(), saveValue);
        ESP_LOGI("PreferencesWrapper: HandleSave", "Saved Key: \"%s\" Value: \"%s\"", key.c_str(), saveValue.c_str());
    }

private:
    Preferences* mp_Preferences;
    ValidValues_t *mp_ValidValues;
    PreferencesWrapperTimerArgs* mp_TimerArgs;
    esp_timer_handle_t m_PreferenceTimer;
    uint64_t m_Preferences_Last_Update;
    bool m_PreferenceTimerActive;
};