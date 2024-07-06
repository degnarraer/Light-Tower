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
#include <thread>
#include <chrono>
#include "DataItem/PreferencesWrapper.h"
#include "Mock_Preferences.h"
#include "Mock_Callbacks.h"

#define TIMEOUT_TIME 100UL

TEST(PreferencesWrapperInstantiation, Instantion_Destruction)
{
    const String key = "key1";
    const String testValue = "Key 1 Value";
    MockPreferences *mockPreferences = new MockPreferences();
    PreferenceManager *preferenceManager = new PreferenceManager(mockPreferences, key, testValue, TIMEOUT_TIME, nullptr, nullptr);
    delete mockPreferences;
    delete preferenceManager;
}

using ::testing::_;
using ::testing::NotNull;
using ::testing::NiceMock;
using ::testing::Return;
using namespace testing;

// Test Fixture for PreferenceManager
class PreferenceManagerTests : public Test
                             , PreferenceCallback
{
    protected:
        MockPreferences *mockPreferences;
        PreferenceManager *preferenceManagerWithCallback;
        PreferenceManager *preferenceManagerNoCallback;
    protected:
        const String key1 = "key1";
        const String key2 = "key2";
        const String testValue = "Key 1 Value";
        const String invalidTestValue = "Key 1 Vslue";
        const size_t validTestValueLength = testValue.length();
        const size_t invalidTestValueLength = testValue.length()+1;
        const String initialValue = "Initial Value";
        const String invalidInitialValue = "Initibl Value";
        const size_t validInitialValueLength = initialValue.length();
        const size_t invalidInitialValueLength = initialValue.length()+1;
        void SetUp() override
        {
            mockPreferences = new MockPreferences();
            preferenceManagerWithCallback = new PreferenceManager(mockPreferences, key1, initialValue, TIMEOUT_TIME, LoadValueCallbackFunction, this);
            preferenceManagerNoCallback = new PreferenceManager(mockPreferences, key2, initialValue, TIMEOUT_TIME, nullptr, nullptr);
            ON_CALL(mockPreferenceCallback, LoadValueCallbackFunction(_,_)).WillByDefault(Return(true));
        }
        void TearDown() override 
        {
            if(mockPreferences)
            {
                delete mockPreferences;
                mockPreferences = nullptr;
            }
            if(preferenceManagerWithCallback)
            {
                delete preferenceManagerWithCallback;
                preferenceManagerWithCallback = nullptr;
            }
            if(preferenceManagerNoCallback)
            {
                delete preferenceManagerNoCallback;
                preferenceManagerNoCallback = nullptr;
            }
        }
};

TEST_F(PreferenceManagerTests, Initialization_of_New_Preference_Not_Immediately_Saved)
{
    EXPECT_CALL(*mockPreferences, isKey( StrEq(key1.c_str()) )).Times(1).WillOnce(Return(false));
    EXPECT_CALL(*mockPreferences, putString( StrEq(key1.c_str()), A<String>() )).Times(0);
    EXPECT_CALL(*mockPreferences, getString( StrEq(key1.c_str()), A<String>() )).Times(0);
    EXPECT_CALL(mockPreferenceCallback, LoadValueCallbackFunction(_,_)).Times(0);
    EXPECT_EQ(true, preferenceManagerWithCallback->InitializeAndLoadPreference()); 
}

TEST_F(PreferenceManagerTests, Initialization_Of_New_Preference_Saved_After_TIMEOUT_TIME_ms)
{
    EXPECT_CALL(*mockPreferences, isKey( StrEq(key1.c_str()) )).Times(1).WillOnce(Return(false));
    EXPECT_CALL(*mockPreferences, putString( StrEq(key1.c_str()), initialValue )).Times(1).WillOnce(Return(validInitialValueLength));
    EXPECT_CALL(*mockPreferences, getString( StrEq(key1.c_str()), initialValue )).Times(1).WillOnce(Return(initialValue));
    EXPECT_CALL(mockPreferenceCallback, LoadValueCallbackFunction(_,_)).Times(1).WillOnce(Return(true));
    EXPECT_EQ(true, preferenceManagerWithCallback->InitializeAndLoadPreference()); 
    std::this_thread::sleep_for(std::chrono::milliseconds(TIMEOUT_TIME + 50));
}

TEST_F(PreferenceManagerTests, Initialization_Of_Existing_Preference_Triggers_Loading_Of_Preference)
{
    EXPECT_CALL(*mockPreferences, isKey( StrEq(key1.c_str() ))).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*mockPreferences, getString( StrEq(key1.c_str()), initialValue )).Times(1).WillOnce(Return(initialValue));
    EXPECT_CALL(mockPreferenceCallback, LoadValueCallbackFunction(_,_)).Times(1);
    EXPECT_EQ(true, preferenceManagerWithCallback->InitializeAndLoadPreference());
}

TEST_F(PreferenceManagerTests, Active_Timer_Canceled_When_Preference_Manager_Deleted)
{
    EXPECT_CALL(*mockPreferences, isKey( StrEq(key1.c_str()) )).Times(1).WillOnce(Return(false));
    EXPECT_CALL(*mockPreferences, putString( StrEq(key1.c_str()), initialValue )).Times(0);
    EXPECT_CALL(*mockPreferences, getString( StrEq(key1.c_str()), initialValue )).Times(0);
    EXPECT_CALL(mockPreferenceCallback, LoadValueCallbackFunction( _, _ )).Times(0);
    EXPECT_EQ(true, preferenceManagerWithCallback->InitializeAndLoadPreference());
    delete preferenceManagerWithCallback;
    preferenceManagerWithCallback = nullptr;
    std::this_thread::sleep_for(std::chrono::milliseconds(TIMEOUT_TIME + 50));
}

TEST_F(PreferenceManagerTests, Loading_Preference_Calls_Callback_When_Provided)
{
    EXPECT_CALL(*mockPreferences, getString( StrEq(key1.c_str()), initialValue )).Times(1).WillOnce(Return(initialValue));
    EXPECT_CALL(mockPreferenceCallback, LoadValueCallbackFunction(_,_)).Times(1);
    EXPECT_EQ(true, preferenceManagerWithCallback->Update_Preference(PreferenceManager::PreferenceUpdateType::Load, ""));

    EXPECT_CALL(*mockPreferences, getString( StrEq(key2.c_str()), initialValue )).Times(1).WillOnce(Return(testValue));
    EXPECT_CALL(mockPreferenceCallback, LoadValueCallbackFunction(_,_)).Times(0);
    EXPECT_EQ(true, preferenceManagerNoCallback->Update_Preference(PreferenceManager::PreferenceUpdateType::Load, ""));
}

TEST_F(PreferenceManagerTests, Save_Preference_Saves_Value)
{
    EXPECT_CALL(*mockPreferences, isKey( StrEq(key1.c_str()) )).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*mockPreferences, getString( StrEq(key1.c_str()), initialValue )).Times(1).WillOnce(Return(initialValue));
    EXPECT_CALL(mockPreferenceCallback, LoadValueCallbackFunction(_,_)).Times(1);
    EXPECT_EQ(true, preferenceManagerWithCallback->InitializeAndLoadPreference());
    std::this_thread::sleep_for(std::chrono::milliseconds(TIMEOUT_TIME + 50));
    EXPECT_CALL(*mockPreferences, putString( StrEq(key1.c_str()), testValue )).Times(1).WillOnce(Return(validTestValueLength));
    EXPECT_CALL(*mockPreferences, getString( StrEq(key1.c_str()), initialValue )).Times(1).WillOnce(Return(testValue));
    EXPECT_EQ(true, preferenceManagerWithCallback->Update_Preference(PreferenceManager::PreferenceUpdateType::Save, testValue));
}

TEST_F(PreferenceManagerTests, Save_Preference_Uses_Timeout_Timer_To_Limit_Frequent_Saves)
{
    EXPECT_CALL(*mockPreferences, isKey( StrEq(key1.c_str()) )).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*mockPreferences, getString( StrEq(key1.c_str()), initialValue )).Times(1).WillOnce(Return(initialValue));
    EXPECT_CALL(mockPreferenceCallback, LoadValueCallbackFunction(_,_)).Times(1);
    EXPECT_EQ(true, preferenceManagerWithCallback->InitializeAndLoadPreference());
    std::this_thread::sleep_for(std::chrono::milliseconds(TIMEOUT_TIME + 50));
    EXPECT_CALL(*mockPreferences, putString( StrEq(key1.c_str()), testValue )).Times(1).WillOnce(Return(validTestValueLength));
    EXPECT_CALL(*mockPreferences, getString( StrEq(key1.c_str()), initialValue )).Times(1).WillOnce(Return(testValue));
    EXPECT_EQ(true, preferenceManagerWithCallback->Update_Preference(PreferenceManager::PreferenceUpdateType::Save, testValue));
    EXPECT_EQ(true, preferenceManagerWithCallback->Update_Preference(PreferenceManager::PreferenceUpdateType::Save, testValue));
}

TEST_F(PreferenceManagerTests, Save_Preference_Failure_Due_to_Save_Length_Error)
{
    EXPECT_CALL(*mockPreferences, putString( StrEq(key1.c_str()), testValue )).Times(1).WillOnce(Return(invalidTestValueLength));
    EXPECT_CALL(*mockPreferences, getString( StrEq(key1.c_str()), initialValue )).Times(0);
    EXPECT_EQ(false, preferenceManagerWithCallback->Update_Preference(PreferenceManager::PreferenceUpdateType::Save, testValue));
}

TEST_F(PreferenceManagerTests, Save_Preference_Failure_Due_to_Value_Mismatch)
{
    EXPECT_CALL(*mockPreferences, putString( StrEq(key1.c_str()), testValue )).Times(1).WillOnce(Return(validTestValueLength));
    EXPECT_CALL(*mockPreferences, getString( StrEq(key1.c_str()), initialValue )).Times(1).WillOnce(Return(invalidTestValue));
    EXPECT_EQ(false, preferenceManagerWithCallback->Update_Preference(PreferenceManager::PreferenceUpdateType::Save, testValue));
}