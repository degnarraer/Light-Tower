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
using namespace testing;

// Test Fixture for PreferenceManager
class PreferenceManagerTests : public Test
{
    protected:
        MockPreferences *mockPreferences;
        PreferenceManager *preferenceManager;
    protected:
        const String key = "key1";
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
            preferenceManager = new PreferenceManager(mockPreferences, key, initialValue, TIMEOUT_TIME, nullptr, nullptr);
        }
        void TearDown() override 
        {
            if(mockPreferences)
            {
                delete mockPreferences;
                mockPreferences = nullptr;
            }
            if(preferenceManager)
            {
                delete preferenceManager;
                preferenceManager = nullptr;
            }
        }
};

TEST_F(PreferenceManagerTests, Initialization_of_New_Key_Not_Immediately_Saved)
{
    EXPECT_CALL(*mockPreferences, isKey( StrEq(key.c_str()) )).Times(1).WillOnce(Return(false));
    EXPECT_CALL(*mockPreferences, putString( StrEq(key.c_str()), A<String>() )).Times(0);
    EXPECT_CALL(*mockPreferences, getString( StrEq(key.c_str()), A<String>() )).Times(0);
    EXPECT_EQ(true, preferenceManager->InitializeAndLoadPreference()); 
}

TEST_F(PreferenceManagerTests, Initialize_New_Key_Saved_After_TIMER_TIME_ms)
{
    EXPECT_CALL(*mockPreferences, isKey( StrEq(key.c_str()) )).Times(1).WillOnce(Return(false));
    EXPECT_CALL(*mockPreferences, putString( StrEq(key.c_str()), initialValue )).Times(1).WillOnce(Return(validInitialValueLength));
    EXPECT_CALL(*mockPreferences, getString( StrEq(key.c_str()), initialValue )).Times(1).WillOnce(Return("initialValue"));
    EXPECT_EQ(true, preferenceManager->InitializeAndLoadPreference()); 
    std::this_thread::sleep_for(std::chrono::milliseconds(TIMEOUT_TIME + 50));
}

TEST_F(PreferenceManagerTests, Timer_Not_Called_When_Preference_Manager_Deleted)
{
    EXPECT_CALL(*mockPreferences, isKey( StrEq(key.c_str()) )).Times(1).WillOnce(Return(false));
    EXPECT_CALL(*mockPreferences, putString( StrEq(key.c_str()), initialValue )).Times(0);
    EXPECT_CALL(*mockPreferences, getString( StrEq(key.c_str()), initialValue )).Times(0);
    EXPECT_EQ(true, preferenceManager->InitializeAndLoadPreference());
    delete preferenceManager;
    preferenceManager = nullptr;
    std::this_thread::sleep_for(std::chrono::milliseconds(TIMEOUT_TIME + 50));
}

TEST_F(PreferenceManagerTests, Initialize_Existing_Key_Loaded)
{
    EXPECT_CALL(*mockPreferences, isKey( StrEq(key.c_str() ))).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*mockPreferences, getString( StrEq(key.c_str()), initialValue )).Times(1).WillOnce(Return(initialValue)); 
    preferenceManager->InitializeAndLoadPreference(); 
}

TEST_F(PreferenceManagerTests, Save_Preference_Saved_Correctly)
{
    EXPECT_CALL(*mockPreferences, putString( StrEq(key.c_str()), testValue )).Times(1).WillOnce(Return(validTestValueLength));
    EXPECT_CALL(*mockPreferences, getString( StrEq(key.c_str()), initialValue )).Times(1).WillOnce(Return(testValue));
    EXPECT_EQ(true, preferenceManager->Update_Preference(PreferenceManager::PreferenceUpdateType::Save, testValue));
}

TEST_F(PreferenceManagerTests, Save_Preference_Save_Failure_Due_to_Save_Length_Error)
{
    EXPECT_CALL(*mockPreferences, putString( StrEq(key.c_str()), testValue )).Times(1).WillOnce(Return(invalidTestValueLength));
    EXPECT_CALL(*mockPreferences, getString( StrEq(key.c_str()), initialValue )).Times(0);
    EXPECT_EQ(false, preferenceManager->Update_Preference(PreferenceManager::PreferenceUpdateType::Save, testValue));
}

TEST_F(PreferenceManagerTests, Save_Preference_Save_Failure_Due_to_Failed_String_Compare)
{
    EXPECT_CALL(*mockPreferences, putString( StrEq(key.c_str()), testValue )).Times(1).WillOnce(Return(validTestValueLength));
    EXPECT_CALL(*mockPreferences, getString( StrEq(key.c_str()), initialValue )).Times(1).WillOnce(Return(invalidTestValue));
    EXPECT_EQ(false, preferenceManager->Update_Preference(PreferenceManager::PreferenceUpdateType::Save, testValue));
}

TEST_F(PreferenceManagerTests, Save_Preference_Save_Failure_Due_to_Invalid_Recalled_Value)
{
    EXPECT_CALL(*mockPreferences, putString( StrEq(key.c_str()), testValue )).Times(1).WillOnce(Return(validTestValueLength));
    EXPECT_CALL(*mockPreferences, getString( StrEq(key.c_str()), initialValue )).Times(1).WillOnce(Return(invalidTestValue));
    EXPECT_EQ(false, preferenceManager->Update_Preference(PreferenceManager::PreferenceUpdateType::Save, testValue));
}