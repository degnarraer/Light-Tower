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

#pragma once
#include <Arduino.h>
#include <Helpers.h>

template <typename T, size_t COUNT>
class DataItemInterface
{
    public:
        DataItemInterface(){}
        virtual String GetName() const = 0;
		virtual size_t GetCount() const = 0;
		virtual size_t GetChangeCount() const = 0;
		virtual DataType_t GetDataType() = 0;
        virtual void GetValue(void* object, size_t count) const = 0;
        virtual T* GetValuePointer() const = 0;
        virtual T GetValue() const = 0;
        virtual bool GetInitialValueAsString(String &stringValue) const = 0;
        virtual String GetInitialValueAsString() const = 0;
        virtual bool GetValueAsString(String &stringValue) const = 0;
        virtual String GetValueAsString() const = 0;
        virtual bool SetValueFromString(const String& stringValue) = 0;
        virtual bool SetValue(const T *value, size_t count) = 0;
        virtual bool SetValue(const T value) = 0;
        virtual bool EqualsValue(T *object, size_t count) const = 0;
};