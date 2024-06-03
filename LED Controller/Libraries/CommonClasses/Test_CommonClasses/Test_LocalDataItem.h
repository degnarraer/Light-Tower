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
#include "DataItem/LocalDataItem.h"
#include "Test_CommonClasses/Mock_SetupCallInterface.h"

using ::testing::_;
using ::testing::NotNull;
using namespace testing;

// Test Fixture for LocalDataItemTest
class LocalDataItemSetupCallerTest : public Test
{
protected:
    MockSetupCallerInterface* mockSetupCaller;
    const int32_t initialValue = 10;

    void SetUp() override
    {
        mockSetupCaller = new MockSetupCallerInterface();
    }

    void TearDown() override
    {
        delete mockSetupCaller;
    }
};

TEST_F(LocalDataItemSetupCallerTest, Registered_With_Setup_Caller)
{
    EXPECT_CALL(*mockSetupCaller, RegisterForSetupCall(NotNull())).Times(1);
    LocalDataItem<int32_t, 1> *dataItem = new LocalDataItem<int32_t, 1>( "Test Name" 
                                                                       , initialValue
                                                                       , NULL
                                                                       , mockSetupCaller );
    delete dataItem;
}

TEST_F(LocalDataItemSetupCallerTest, DeRegistered_With_Setup_Caller_On_Deletion)
{
    EXPECT_CALL(*mockSetupCaller, DeRegisterForSetupCall(NotNull())).Times(1);
    LocalDataItem<int32_t, 1> *dataItem = new LocalDataItem<int32_t, 1>( "Test Name" 
                                                                       , initialValue
                                                                       , NULL
                                                                       , mockSetupCaller );
    delete dataItem;
}

// Test Fixture for LocalDataItemTest
class LocalDataItemTest : public Test
{
protected:
    MockSetupCallerInterface* mockSetupCaller;
    const int32_t initialValue = 10;
    LocalDataItem<int32_t, 1> *dataItem;
    const String name = "Test Name";

    void SetUp() override
    {
        mockSetupCaller = new MockSetupCallerInterface();
        LocalDataItem<int32_t, 1> *dataItem = new LocalDataItem<int32_t, 1>( name 
                                                                           , initialValue
                                                                           , NULL
                                                                           , mockSetupCaller );
    }

    void TearDown() override
    {
        delete mockSetupCaller;
        delete dataItem;
    }
};

TEST_F(LocalDataItemTest, Name_Is_Set)
{
    EXPECT_EQ(name, dataItem->GetName()); 
}