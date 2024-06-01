#pragma once

#include <Arduino.h>
#include <vector>
#include <sstream>

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
    const String Value;
} ValidValueComparator_t;

typedef const std::vector<String> ValidStringValues_t;
typedef const std::vector<ValidValueComparator_t> ValidValueComparators_t;

class ValidValueChecker {
public:
    ValidValueChecker()
        : mp_ValidStrings(nullptr)
        , mp_ValidValueComparators(nullptr) {}

    ValidValueChecker(ValidStringValues_t* validStrings)
        : mp_ValidStrings(validStrings)
        , mp_ValidValueComparators(nullptr) {}

    ValidValueChecker(ValidValueComparators_t* validValueComparators)
        : mp_ValidStrings(nullptr)
        , mp_ValidValueComparators(validValueComparators) {}

    virtual ~ValidValueChecker() {}

    bool IsConfigured() const
    {
        return (mp_ValidStrings != nullptr || mp_ValidValueComparators != nullptr);
    }

    bool IsValidStringValue(String value)
    {
        if (mp_ValidStrings)
        {
            for (const String& validValue : *mp_ValidStrings)
            {
                ESP_LOGD("ValidValueChecker:IsValidStringValue", 
                         "IsValidStringValue Match Check between: \"%s\" and \"%s\"", 
                         value.c_str(), validValue.c_str());
                if (value.equals(validValue))
                {
                    ESP_LOGI("ValidValueChecker:IsValidStringValue", 
                             "\"%s\" IsValidStringValue VALID VALUE: \"%s\"", 
                             value.c_str(), validValue.c_str());
                    return true;
                }
            }
            ESP_LOGE("ValidValueChecker:IsValidStringValue", "IsValidStringValue INVALID VALUE!");
        } 
        else if (mp_ValidValueComparators)
        {
            float numericValue = value.toFloat();  // Convert the string to a float
            for (const ValidValueComparator_t& comparator : *mp_ValidValueComparators)
            {
                float comparatorValue = comparator.Value.toFloat();  // Convert comparator value to float
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
                             value.c_str(), comparator.Value.c_str(), comparator.ComparatorType);
                    return true;
                }
            }
            ESP_LOGE("ValidValueChecker:IsValidStringValue", "IsValidStringValue INVALID VALUE!");
        } 
        else 
        {
            ESP_LOGE("ValidValueChecker:IsValidStringValue", "NULL Pointer!");
        }
        return false;
    }

private:
    const ValidStringValues_t* mp_ValidStrings;
    const ValidValueComparators_t* mp_ValidValueComparators;
};