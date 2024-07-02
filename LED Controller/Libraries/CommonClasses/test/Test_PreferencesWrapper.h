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


TEST(PreferencesWrapperInstantiation, Instantion_Destruction)
{
    const String key = "key1";
    const String testValue = "Key 1 Value";
    MockPreferences *mockPreferences = new MockPreferences();
    PreferenceManager *preferenceManager = new PreferenceManager(mockPreferences, key, testValue, nullptr, nullptr);
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
        const String initialValue = "Initial Value";
        void SetUp() override
        {
            mockPreferences = new MockPreferences();
            preferenceManager = new PreferenceManager(mockPreferences, key, initialValue, nullptr, nullptr);
        }
        void TearDown() override 
        {
            delete mockPreferences;
            delete preferenceManager;
        }
};

TEST_F(PreferenceManagerTests, Initialization_of_New_Key_Not_Immediately_Saved)
{
    EXPECT_CALL(*mockPreferences, isKey( StrEq(key.c_str()) )).WillOnce(Return(false));
    EXPECT_CALL(*mockPreferences, putString( StrEq(key.c_str()), A<String>() )).Times(0);
    EXPECT_CALL(*mockPreferences, getString( StrEq(key.c_str()), A<String>() )).Times(0);
    preferenceManager->InitializeAndLoadPreference(); 
}

TEST_F(PreferenceManagerTests, Initialize_New_Key_Saved_After_TIMER_TIME_ms)
{
    EXPECT_CALL(*mockPreferences, isKey( StrEq(key.c_str()) )).WillOnce(Return(false));
    EXPECT_CALL(*mockPreferences, putString( StrEq(key.c_str()), A<String>() )).WillOnce(Return(strlen(key.c_str())));
    EXPECT_CALL(*mockPreferences, getString( StrEq(key.c_str()), A<String>() )).Times(1);
    preferenceManager->InitializeAndLoadPreference();
    std::this_thread::sleep_for(std::chrono::milliseconds(TIMER_TIME + 50));
}

TEST_F(PreferenceManagerTests, Initialize_Existing_Key_Loaded)
{
    EXPECT_CALL(*mockPreferences, isKey( StrEq(key.c_str() ))).WillOnce(Return(true));
    EXPECT_CALL(*mockPreferences, getString( StrEq(key.c_str()), A<String>() )).Times(1); 
    preferenceManager->InitializeAndLoadPreference(); 
}