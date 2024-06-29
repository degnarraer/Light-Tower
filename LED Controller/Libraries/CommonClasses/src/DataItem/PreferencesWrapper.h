#pragma once
#include "DataTypes.h"
#include "Preferences.h"

#define TIMER_TIME 10000UL
#define TIMER_BUFFER 1000UL

class IPreferences
{
    public:
        virtual ~IPreferences() {}

        virtual bool begin(const char* name, bool readOnly = false, const char* partition_label = nullptr) = 0;
        virtual void end() = 0;
        virtual bool clear() = 0;
        virtual bool remove(const char* key) = 0;
        virtual size_t putChar(const char* key, int8_t value) = 0;
        virtual size_t putUChar(const char* key, uint8_t value) = 0;
        virtual size_t putShort(const char* key, int16_t value) = 0;
        virtual size_t putUShort(const char* key, uint16_t value) = 0;
        virtual size_t putInt(const char* key, int32_t value) = 0;
        virtual size_t putUInt(const char* key, uint32_t value) = 0;
        virtual size_t putLong(const char* key, int32_t value) = 0;
        virtual size_t putULong(const char* key, uint32_t value) = 0;
        virtual size_t putLong64(const char* key, int64_t value) = 0;
        virtual size_t putULong64(const char* key, uint64_t value) = 0;
        virtual size_t putFloat(const char* key, float_t value) = 0;
        virtual size_t putDouble(const char* key, double_t value) = 0;
        virtual size_t putBool(const char* key, bool value) = 0;
        virtual size_t putString(const char* key, const char* value) = 0;
        virtual size_t putString(const char* key, String value) = 0;
        virtual size_t putBytes(const char* key, const void* value, size_t len) = 0;
        virtual bool isKey(const char* key) = 0;
        virtual PreferenceType getType(const char* key) = 0;
        virtual int8_t getChar(const char* key, int8_t defaultValue = 0) = 0;
        virtual uint8_t getUChar(const char* key, uint8_t defaultValue = 0) = 0;
        virtual int16_t getShort(const char* key, int16_t defaultValue = 0) = 0;
        virtual uint16_t getUShort(const char* key, uint16_t defaultValue = 0) = 0;
        virtual int32_t getInt(const char* key, int32_t defaultValue = 0) = 0;
        virtual uint32_t getUInt(const char* key, uint32_t defaultValue = 0) = 0;
        virtual int32_t getLong(const char* key, int32_t defaultValue = 0) = 0;
        virtual uint32_t getULong(const char* key, uint32_t defaultValue = 0) = 0;
        virtual int64_t getLong64(const char* key, int64_t defaultValue = 0) = 0;
        virtual uint64_t getULong64(const char* key, uint64_t defaultValue = 0) = 0;
        virtual float_t getFloat(const char* key, float_t defaultValue = NAN) = 0;
        virtual double_t getDouble(const char* key, double_t defaultValue = NAN) = 0;
        virtual bool getBool(const char* key, bool defaultValue = false) = 0;
        virtual size_t getString(const char* key, char* value, size_t maxLen) = 0;
        virtual String getString(const char* key, String defaultValue = String()) = 0;
        virtual size_t getBytesLength(const char* key) = 0;
        virtual size_t getBytes(const char* key, void* buf, size_t maxLen) = 0;
        virtual size_t freeEntries() = 0;
};

class PreferencesWrapper : public IPreferences
{
    public:
        PreferencesWrapper(Preferences* preferences){}
    private:
        Preferences* mp_preferences;
    public:
        bool begin(const char* name, bool readOnly = false, const char* partition_label = nullptr) override
        {
            assert(mp_preferences);
            return mp_preferences->begin(name, readOnly, partition_label);
        }
        void end() override
        {
            assert(mp_preferences);
            mp_preferences->end();
        }
        bool clear() override
        {
            assert(mp_preferences);
            return mp_preferences->clear();
        }
        bool remove(const char* key) override
        {
            assert(mp_preferences);
            return mp_preferences->remove(key);
        }
        size_t putChar(const char* key, int8_t value) override
        {
            assert(mp_preferences);
            return mp_preferences->putChar(key, value);
        }
        size_t putUChar(const char* key, uint8_t value) override
        {
            assert(mp_preferences);
            return mp_preferences->putUChar(key, value);
        }
        size_t putShort(const char* key, int16_t value) override
        {
            assert(mp_preferences);
            return mp_preferences->putShort(key, value);
        }
        size_t putUShort(const char* key, uint16_t value) override
        {
            assert(mp_preferences);
            return mp_preferences->putUShort(key, value);
        }
        size_t putInt(const char* key, int32_t value) override
        {
            assert(mp_preferences);
            return mp_preferences->putInt(key, value);
        }
        size_t putUInt(const char* key, uint32_t value) override
        {
            assert(mp_preferences);
            return mp_preferences->putUInt(key, value);
        }
        size_t putLong(const char* key, int32_t value) override
        {
            assert(mp_preferences);
            return mp_preferences->putLong(key, value);
        }
        size_t putULong(const char* key, uint32_t value) override
        {
            assert(mp_preferences);
            return mp_preferences->putULong(key, value);
        }
        size_t putLong64(const char* key, int64_t value) override
        {
            assert(mp_preferences);
            return mp_preferences->putLong64(key, value);
        }
        size_t putULong64(const char* key, uint64_t value) override
        {
            assert(mp_preferences);
            return mp_preferences->putULong64(key, value);
        }
        size_t putFloat(const char* key, float_t value) override
        {
            assert(mp_preferences);
            return mp_preferences->putFloat(key, value);
        }
        size_t putDouble(const char* key, double_t value) override
        {
            assert(mp_preferences);
            return mp_preferences->putDouble(key, value);
        }
        size_t putBool(const char* key, bool value) override
        {
            assert(mp_preferences);
            return mp_preferences->putBool(key, value);
        }
        size_t putString(const char* key, const char* value) override
        {
            assert(mp_preferences);
            return mp_preferences->putString(key, value);
        }
        size_t putString(const char* key, String value) override
        {
            assert(mp_preferences);
            return mp_preferences->putString(key, value);
        }
        size_t putBytes(const char* key, const void* value, size_t len) override
        {
            assert(mp_preferences);
            return mp_preferences->putBytes(key, value, len);
        }
        bool isKey(const char* key) override
        {
            assert(mp_preferences);
            return mp_preferences->isKey(key);
        }
        PreferenceType getType(const char* key) override
        {
            assert(mp_preferences);
            return mp_preferences->getType(key);
        }
        int8_t getChar(const char* key, int8_t defaultValue = 0) override
        {
            assert(mp_preferences);
            return mp_preferences->getChar(key, defaultValue);
        }
        uint8_t getUChar(const char* key, uint8_t defaultValue = 0) override
        {
            assert(mp_preferences);
            return mp_preferences->getUChar(key, defaultValue);
        }
        int16_t getShort(const char* key, int16_t defaultValue = 0) override
        {
            assert(mp_preferences);
            return mp_preferences->getShort(key, defaultValue);
        }
        uint16_t getUShort(const char* key, uint16_t defaultValue = 0) override
        {
            assert(mp_preferences);
            return mp_preferences->getUShort(key, defaultValue);
        }
        int32_t getInt(const char* key, int32_t defaultValue = 0) override
        {
            assert(mp_preferences);
            return mp_preferences->getInt(key, defaultValue);
        }
        uint32_t getUInt(const char* key, uint32_t defaultValue = 0) override
        {
            assert(mp_preferences);
            return mp_preferences->getUInt(key, defaultValue);
        }
        int32_t getLong(const char* key, int32_t defaultValue = 0) override
        {
            assert(mp_preferences);
            return mp_preferences->getLong(key, defaultValue);
        }
        uint32_t getULong(const char* key, uint32_t defaultValue = 0) override
        {
            assert(mp_preferences);
            return mp_preferences->getULong(key, defaultValue);
        }
        int64_t getLong64(const char* key, int64_t defaultValue = 0) override
        {
            assert(mp_preferences);
            return mp_preferences->getLong64(key, defaultValue);
        }
        uint64_t getULong64(const char* key, uint64_t defaultValue = 0) override
        {
            assert(mp_preferences);
            return mp_preferences->getULong64(key, defaultValue);
        }
        float_t getFloat(const char* key, float_t defaultValue = NAN) override
        {
            assert(mp_preferences);
            return mp_preferences->getFloat(key, defaultValue);
        }
        double_t getDouble(const char* key, double_t defaultValue = NAN) override
        {
            assert(mp_preferences);
            return mp_preferences->getDouble(key, defaultValue);
        }
        bool getBool(const char* key, bool defaultValue = false) override
        {
            assert(mp_preferences);
            return mp_preferences->getBool(key, defaultValue);
        }
        size_t getString(const char* key, char* value, size_t maxLen) override
        {
            assert(mp_preferences);
            return mp_preferences->getString(key, value, maxLen);
        }
        String getString(const char* key, String defaultValue = String()) override
        {
            assert(mp_preferences);
            return mp_preferences->getString(key, defaultValue);
        }
        size_t getBytesLength(const char* key) override
        {
            assert(mp_preferences);
            return mp_preferences->getBytesLength(key);
        }
        size_t getBytes(const char* key, void* buf, size_t maxLen) override
        {
            assert(mp_preferences);
            return mp_preferences->getBytes(key, buf, maxLen);
        }
        size_t freeEntries() override
        {
            assert(mp_preferences);
            return mp_preferences->freeEntries();
        }
};

class PreferencesManager : public DataTypeFunctions 
{
public:
	typedef bool (*LoadedValueCallback_t)(const String&, void* object);
    enum class PreferenceUpdateType
    {
        Initialize,
        Load,
        Save,
        Timer
    };

    struct PreferencesManagerTimerArgs
    {
        PreferencesManager* PreferencesManagerInstance;
        String Name;
        String Value;
        String InitialValue;
        LoadedValueCallback_t Callback;
        void* Object;

        PreferencesManagerTimerArgs( PreferencesManager* preferencesManager
                                   , const String& name
                                   , String value
                                   , const String& initialValue )
                                   : PreferencesManagerInstance(preferencesManager)
                                   , Name(name)
                                   , Value(value)
                                   , InitialValue(initialValue) {}
    };

    PreferencesManager(IPreferences* preferencesInterface)
        : mp_PreferencesInterface(preferencesInterface)
		, mp_TimerArgs(nullptr)
		, m_PreferenceTimer(nullptr)
		, m_Preferences_Last_Update(0)
		, m_PreferenceTimerActive(false) {}

    virtual ~PreferencesManager()
    {
        if (mp_TimerArgs)
        {
            delete mp_TimerArgs;
        }
    }

    static void Static_Update_Preference(void* arg)
    {
        PreferencesManagerTimerArgs* timerArgsPtr = static_cast<PreferencesManagerTimerArgs*>(arg);
        PreferencesManager* aPreferencesManager = timerArgsPtr->PreferencesManagerInstance;
        if (aPreferencesManager)
        {
            aPreferencesManager->Update_Preference( PreferenceUpdateType::Timer
                                                  , timerArgsPtr->Name
                                                  , timerArgsPtr->Value
                                                  , timerArgsPtr->InitialValue
                                                  , timerArgsPtr->Callback
                                                  , timerArgsPtr->Object );
        }
    }

    void Update_Preference( const PreferenceUpdateType& updateType
						  , const String& name
						  , const String& saveValue
						  , const String& initialValue
						  , LoadedValueCallback_t callback
                          , void* object )
    {
        if (!mp_PreferencesInterface) return;

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
            HandleLoad(name, initialValue, callback, object);
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
        mp_TimerArgs = new PreferencesManagerTimerArgs(this, key, value, initialValue);
        esp_timer_create_args_t timerArgs;
        timerArgs.callback = &Static_Update_Preference;
        timerArgs.arg = mp_TimerArgs;
        timerArgs.name = "Preferences_Timer";
        esp_timer_create(&timerArgs, &m_PreferenceTimer);
    }

    void InitializeAndLoadPreference(const String& key, const String& initialValue, LoadedValueCallback_t callback, void* object)
    {
        if (mp_PreferencesInterface)
        {
            if (mp_PreferencesInterface->isKey(key.c_str()))
            {
                ESP_LOGI("InitializeAndLoadPreference", "Preference Found: \"%s\"", key.c_str());
                Update_Preference(PreferenceUpdateType::Load, key, "", initialValue, callback, object);
            }
            else
            {
                ESP_LOGI("InitializeAndLoadPreference", "Preference Not Found: \"%s\"", key.c_str());
                Update_Preference(PreferenceUpdateType::Initialize, key, "", initialValue, callback, object);
            }
        }
        else
        {
            ESP_LOGE("PreferencesManager: InitializeAndLoadPreference", "Null Preferences Pointer!");
        }
    }

    void HandleLoad(const String& key, const String& initialValue, LoadedValueCallback_t callback, void* object)
    {
        
        if(mp_PreferencesInterface)
        {
            ESP_LOGD("PreferencesManager: HandleLoad", "Loading Key: \"%s\"", key.c_str());
            String loadedValue = mp_PreferencesInterface->getString(key.c_str(), initialValue);
            if (callback && object)
            {
                if(callback(loadedValue, object))
                {
                    ESP_LOGI("PreferencesManager: HandleLoad", "Successfully Loaded Key: \"%s\" Value: \"%s\"", key.c_str(), loadedValue.c_str());
                }
                else
                {
                    ESP_LOGW("PreferencesManager: HandleLoad", "\"%s\" Failed to Load Value. Loading Default Value: \"%s\"", key.c_str(), initialValue.c_str());
                    if(callback(initialValue, object))
                    {
                        ESP_LOGI("PreferencesManager: HandleLoad", "Successfully Loaded Key: \"%s\" Default Value: \"%s\"", key.c_str(), loadedValue.c_str());
                    }
                    else
                    {
                        ESP_LOGE("PreferencesManager: HandleLoad", "\"%s\" Failed to Load Default Value!: \"%s\"", key.c_str(), initialValue.c_str());
                    }
                }
            }
            else
            {
                ESP_LOGE("PreferencesManager: HandleLoad", "\"%s\" Null Callback Pointers!", key.c_str());
            }
        }
        else
        {
            ESP_LOGE("PreferencesManager: HandleLoad", "\"%s\" Null Pointer!", key.c_str());
        }
    }

    void HandleSave(const String& key, const String& string)
    {
        ESP_LOGE("PreferencesManager: HandleSave", "Saving Key: \"%s\" Value: \"%s\"", key.c_str(), string.c_str());
        if(mp_PreferencesInterface)
        {
            mp_PreferencesInterface->putString(key.c_str(), string);
            String savedString = mp_PreferencesInterface->getString(key.c_str(),"");
            if(!string.equals(savedString))
            {
                ESP_LOGE("PreferencesManager: HandleSave", "Saved Key: \"%s\" Did Not Save Properly! String to save: \"%s\" Saved String: \"%s\"", key.c_str(), string.c_str(), savedString.c_str());   
            }
            else
            {
                ESP_LOGE("PreferencesManager: HandleSave", "Saved Key: \"%s\" String Saved: \"%s\"", key.c_str(), savedString.c_str());
            }
        }
        else
        {
            ESP_LOGE("PreferencesManager: HandleSave", "\"%s\" Null Pointer!", key.c_str());
        }
    }

private:
    IPreferences* mp_PreferencesInterface;
    PreferencesManagerTimerArgs* mp_TimerArgs;
    esp_timer_handle_t m_PreferenceTimer;
    uint64_t m_Preferences_Last_Update;
    bool m_PreferenceTimerActive;
};