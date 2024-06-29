/*
    Light Tower by Rob Shockency
    Copyright (C) 2021 Rob Shockency degnarraer@yahoo.com

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version of the License, or
    (at your option) any later version. 3

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once;
#include "gmock/gmock.h"
#include <Arduino.h>

/*

class MockPreferences : public Preferences {
public:
    MOCK_METHOD(bool, begin, (const char* name, bool readOnly, const char* partition_label), (override));
    MOCK_METHOD(void, end, (), (override));

    MOCK_METHOD(bool, clear, (), (override));
    MOCK_METHOD(bool, remove, (const char* key), (override));

    MOCK_METHOD(size_t, putChar, (const char* key, int8_t value), (override));
    MOCK_METHOD(size_t, putUChar, (const char* key, uint8_t value), (override));
    MOCK_METHOD(size_t, putShort, (const char* key, int16_t value), (override));
    MOCK_METHOD(size_t, putUShort, (const char* key, uint16_t value), (override));
    MOCK_METHOD(size_t, putInt, (const char* key, int32_t value), (override));
    MOCK_METHOD(size_t, putUInt, (const char* key, uint32_t value), (override));
    MOCK_METHOD(size_t, putLong, (const char* key, int32_t value), (override));
    MOCK_METHOD(size_t, putULong, (const char* key, uint32_t value), (override));
    MOCK_METHOD(size_t, putLong64, (const char* key, int64_t value), (override));
    MOCK_METHOD(size_t, putULong64, (const char* key, uint64_t value), (override));
    MOCK_METHOD(size_t, putFloat, (const char* key, float_t value), (override));
    MOCK_METHOD(size_t, putDouble, (const char* key, double_t value), (override));
    MOCK_METHOD(size_t, putBool, (const char* key, bool value), (override));
    MOCK_METHOD(size_t, putString, (const char* key, const char* value), (override));
    MOCK_METHOD(size_t, putString, (const char* key, String value), (override));
    MOCK_METHOD(size_t, putBytes, (const char* key, const void* value, size_t len), (override));

    MOCK_METHOD(bool, isKey, (const char* key), (override));
    MOCK_METHOD(PreferenceType, getType, (const char* key), (override));
    MOCK_METHOD(int8_t, getChar, (const char* key, int8_t defaultValue), (override));
    MOCK_METHOD(uint8_t, getUChar, (const char* key, uint8_t defaultValue), (override));
    MOCK_METHOD(int16_t, getShort, (const char* key, int16_t defaultValue), (override));
    MOCK_METHOD(uint16_t, getUShort, (const char* key, uint16_t defaultValue), (override));
    MOCK_METHOD(int32_t, getInt, (const char* key, int32_t defaultValue), (override));
    MOCK_METHOD(uint32_t, getUInt, (const char* key, uint32_t defaultValue), (override));
    MOCK_METHOD(int32_t, getLong, (const char* key, int32_t defaultValue), (override));
    MOCK_METHOD(uint32_t, getULong, (const char* key, uint32_t defaultValue), (override));
    MOCK_METHOD(int64_t, getLong64, (const char* key, int64_t defaultValue), (override));
    MOCK_METHOD(uint64_t, getULong64, (const char* key, uint64_t defaultValue), (override));
    MOCK_METHOD(float_t, getFloat, (const char* key, float_t defaultValue), (override));
    MOCK_METHOD(double_t, getDouble, (const char* key, double_t defaultValue), (override));
    MOCK_METHOD(bool, getBool, (const char* key, bool defaultValue), (override));
    MOCK_METHOD(size_t, getString, (const char* key, char* value, size_t maxLen), (override));
    MOCK_METHOD(String, getString, (const char* key, String defaultValue), (override));
    MOCK_METHOD(size_t, getBytesLength, (const char* key), (override));
    MOCK_METHOD(size_t, getBytes, (const char* key, void* buf, size_t maxLen), (override));
    MOCK_METHOD(size_t, freeEntries, (), (override));
};

*/