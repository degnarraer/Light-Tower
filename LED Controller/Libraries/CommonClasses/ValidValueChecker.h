#pragma once

#include <Arduino.h>
#include <vector>
#include <sstream>

typedef const std::vector<String> ValidStringValues_t;

class ValidValueChecker {
public:
    ValidValueChecker() : mp_ValidStrings(nullptr) {}
    ValidValueChecker(ValidStringValues_t* validStrings) 
        : mp_ValidStrings(validStrings) {}

    virtual ~ValidValueChecker() {}

    bool IsValidValue(String value) {
        if (mp_ValidStrings) {
            for (const String& validValue : *mp_ValidStrings) {
                if (value.equals(validValue)) {
                    return true;
                }
            }
        } else {
            return true;
        }
    }

private:
    ValidStringValues_t* mp_ValidStrings;
};