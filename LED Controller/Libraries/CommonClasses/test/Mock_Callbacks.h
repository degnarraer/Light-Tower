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
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "DataTypes.h"

class MockPreferenceCallback
{
    public:
        MOCK_METHOD(bool, LoadValueCallbackFunction, (const String&, void* object));
};
static MockPreferenceCallback mockPreferenceCallback;
class PreferenceCallback
{
    protected:
        static bool LoadValueCallbackFunction(const String& value, void* object)
        {
            return mockPreferenceCallback.LoadValueCallbackFunction(value, object);  
        }
};

class MockNamedCallback_Callback
{
    public:
        MOCK_METHOD(void, NewValueCallbackFunction, (const String& name, void* callback, void* arg));
};
static MockNamedCallback_Callback mockNamedCallback_Callback;
class MockNamedCallback: public NamedCallback_t
{
    protected:
        static void NewValueCallbackFunction(const String& name, void* callback, void* arg)
        {
            mockNamedCallback_Callback.NewValueCallbackFunction(name, callback, arg);  
        }
    public:
        MockNamedCallback(): NamedCallback_t("", NewValueCallbackFunction, nullptr)
        {
            ESP_LOGD("MockNamedCallback", "MockNamedCallback Default Constructor called");
        }
        MockNamedCallback(const String& name, void* arg): NamedCallback_t(name, NewValueCallbackFunction, arg)
        {
            ESP_LOGD("MockNamedCallback", "MockNamedCallback Constructor1 called. Name: \"%s\"", name);
        }
        virtual ~MockNamedCallback()
        {
            ESP_LOGD("MockNamedCallback", "~MockNamedCallback Name: \"%s\"", Name.c_str()); 
        }
};