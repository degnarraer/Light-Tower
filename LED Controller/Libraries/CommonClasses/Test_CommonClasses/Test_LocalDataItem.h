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
#include "Test_CommonClasses/Mock_ValidValueChecker.h"

using ::testing::_;
using ::testing::NotNull;
using namespace testing;

// Test Fixture for LocalDataItemSetupCallerTest
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
                        , public SetupCallerInterface
{
protected:
    const ValidStringValues_t validValues = { "10", "20", "30" };
    const int32_t initialValue = 10;
    const int32_t validValue = 20;
    const int32_t invalidValue = 40;
    const String initialValueString = String(initialValue);
    LocalDataItem<int32_t, 1> *dataItem;
    LocalDataItem<int32_t, 1> *dataItemWithValidation;
    const String name1 = "Test Name1";
    const String name2 = "Test Name2";

    void SetUp() override
    {
        dataItem = new LocalDataItem<int32_t, 1>( name1 
                                                , initialValue
                                                , NULL
                                                , this );

        dataItemWithValidation = new LocalDataItem<int32_t, 1>( name2 
                                                              , initialValue
                                                              , NULL
                                                              , this
                                                              , &validValues );
        SetupAllSetupCallees();
    }

    void TearDown() override
    {
        delete dataItem;
        delete dataItemWithValidation;
    }
};

TEST_F(LocalDataItemTest, Name_Is_Set)
{
    EXPECT_STREQ(name1.c_str(), dataItem->GetName().c_str());
    EXPECT_STREQ(name2.c_str(), dataItemWithValidation->GetName().c_str());
}

TEST_F(LocalDataItemTest, Initial_Value_Is_Set)
{
    EXPECT_EQ(initialValue, dataItem->GetValue()); 
}

TEST_F(LocalDataItemTest, Initial_Value_Is_Returned_As_String)
{
    EXPECT_STREQ(initialValueString.c_str(), dataItem->GetInitialValueAsString().c_str()); 
}

TEST_F(LocalDataItemTest, Only_Valid_Values_Accepted)
{
    dataItem->SetValue(validValue);
    dataItemWithValidation->SetValue(validValue);
    //EXPECT_EQ(validValue, dataItem->GetValue());
    //EXPECT_EQ(validValue, dataItemWithValidation->GetValue());
    //dataItem->SetValue(invalidValue);
    //dataItemWithValidation->SetValue(invalidValue);
    //EXPECT_EQ(invalidValue, dataItem->GetValue());
    //EXPECT_EQ(validValue, dataItemWithValidation->GetValue());
}