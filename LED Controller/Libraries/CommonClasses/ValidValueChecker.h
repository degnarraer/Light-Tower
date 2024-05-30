#pragma once

#include <Arduino.h>
#include <vector>
#include <sstream>

typedef enum ComparatorType_t
{
    Greater,
    GreaterOrEqual,
    Equal,
    LessOrEqual,
    Less,
};
typedef struct ValidValueComparator_t
{
    const ComparatorType_t Type;
    const String Value;
};
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

    bool IsValidStringValue(String value) {
        if (mp_ValidStrings)
        {
            for (const String& validValue : *mp_ValidStrings)
            {
                ESP_LOGE("TEST", "IsValidStringValue Match Check between: \"%s\" and \"%s\"", value.c_str(), validValue.c_str() );
                if (value.equals(validValue))
                {
                    ESP_LOGE("TEST", "\"%s\" IsValidStringValue VALID VALUE: \"%s\"", value.c_str(), validValue.c_str() );
                    return true;
                }
            }
            ESP_LOGE("TEST", "IsValidStringValue INVALID VALUE!" );
            return false;
        } 
        else
        {
            ESP_LOGE("TEST", "IsValidStringValue NULL POINTER!" );
            return true;
        }
    }

private:
    ValidStringValues_t* mp_ValidStrings;
    ValidValueComparators_t* mp_ValidValueComparators;
};