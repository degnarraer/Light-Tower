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
    const std::string StringValue;
} ValidValueComparator_t;

typedef const std::vector<std::string> ValidStringValues_t;
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
    bool IsConfigured() const
    {
        return m_IsConfigured;
    }
    virtual bool IsValidStringValue(const std::string &stringValue) const
    {
        if (mp_ValidStrings)
        {
            for (const std::string& validValue : *mp_ValidStrings)
            {
                ESP_LOGD("ValidValueChecker:IsValidStringValue", 
                         "IsValidStringValue Match Check between: \"%s\" and \"%s\"", 
                         stringValue.c_str(), validValue.c_str());
                if (stringValue == validValue)
                {
                    ESP_LOGD("ValidValueChecker:IsValidStringValue", 
                             "\"%s\" IsValidStringValue VALID VALUE: \"%s\"", 
                             stringValue.c_str(), validValue.c_str());
                    return true;
                }
            }
            ESP_LOGW("ValidValueChecker:IsValidStringValue", "WARNING! IsValidStringValue INVALID VALUE");
        } 
        else if (mp_ValidValueComparators)
        {
            float numericValue = std::stof(stringValue);  // Convert the string to a float
            for (const ValidValueComparator_t& comparator : *mp_ValidValueComparators)
            {
                float comparatorValue = std::stof(comparator.StringValue);  // Convert comparator value to float
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
                    ESP_LOGD("ValidValueChecker:IsValidStringValue", 
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