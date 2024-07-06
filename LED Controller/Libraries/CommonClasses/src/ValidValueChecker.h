#pragma once

#include <vector>
#include <sstream>
#include "Streaming.h"

typedef enum LogicType_t {
    And,
    Or
} LogicType_t;

typedef enum ComparatorType_t {
    Greater,
    GreaterOrEqual,
    Equal,
    LessOrEqual,
    Less,
} ComparatorType_t;

typedef struct ValidValueComparator_t {
    const ComparatorType_t ComparatorType;
    const String StringValue;
} ValidValueComparator_t;

typedef const std::vector<String> ValidStringValues_t;
typedef const std::vector<ValidValueComparator_t> ValidValueComparators_t;

class ValidValueChecker {
public:
    ValidValueChecker()
        : mp_ValidStrings(nullptr)
        , mp_ValidValueComparators(nullptr) {}

    explicit ValidValueChecker(const ValidStringValues_t* const validStrings)
        : mp_ValidStrings(validStrings)
        , mp_ValidValueComparators(nullptr)
        {
            m_IsConfigured = true;
        }


    explicit ValidValueChecker(const ValidValueComparators_t* const validValueComparators)
        : mp_ValidStrings(nullptr)
        , mp_ValidValueComparators(validValueComparators)
        {
            
            m_IsConfigured = true;
        }

    virtual ~ValidValueChecker() {}

    virtual bool IsValidStringValue(const String &stringValue) const
    {
        if (mp_ValidStrings)
        {
            for (const String& validValue : *mp_ValidStrings)
            {
                ESP_LOGD("ValidValueChecker:IsValidStringValue", 
                         "IsValidStringValue Match Check between: \"%s\" and \"%s\"", 
                         stringValue.c_str(), validValue.c_str());
                if (stringValue.equals(validValue))
                {
                    ESP_LOGD("ValidValueChecker:IsValidStringValue", 
                             "\"%s\" IsValidStringValue VALID VALUE: \"%s\"", 
                             stringValue.c_str(), validValue.c_str());
                    return true;
                }
            }
            ESP_LOGW("ValidValueChecker:IsValidStringValue", "IsValidStringValue INVALID VALUE!");
        } 
        else if (mp_ValidValueComparators)
        {
            float numericValue = stringValue.toFloat();  // Convert the string to a float
            for (const ValidValueComparator_t& comparator : *mp_ValidValueComparators)
            {
                float comparatorValue = comparator.StringValue.toFloat();  // Convert comparator value to float
                bool isValid = false;
                switch (comparator.ComparatorType) 
                {
                    case Greater:
                        isValid = (numericValue > comparatorValue);
                        break;
                    case GreaterOrEqual:
                        isValid = (numericValue >= comparatorValue);
                        break;
                    case Equal:
                        isValid = (numericValue == comparatorValue);
                        break;
                    case LessOrEqual:
                        isValid = (numericValue <= comparatorValue);
                        break;
                    case Less:
                        isValid = (numericValue < comparatorValue);
                        break;
                }
                if (isValid)
                {
                    ESP_LOGI("ValidValueChecker:IsValidStringValue", 
                             "\"%s\" IsValidStringValue VALID VALUE: \"%s\" with comparator %d", 
                             stringValue.c_str(), comparator.StringValue.c_str(), comparator.ComparatorType);
                    return true;
                }
            }
            ESP_LOGE("ValidValueChecker:IsValidStringValue", "IsValidStringValue INVALID VALUE!");
        } 
        else 
        {
            return true;
        }
        return false;
    }

private:
    ValidStringValues_t* const mp_ValidStrings;
    ValidValueComparators_t* const mp_ValidValueComparators;
    bool m_IsConfigured = false;
};