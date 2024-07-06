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
#include "DataItem/LocalStringDataItem.h"
#include "Mock_SetupCallInterface.h"
#include "Mock_ValidValueChecker.h"
#include "Mock_Callbacks.h"

using ::testing::_;
using ::testing::NotNull;
using namespace testing;

// Test Fixture for LocalDataItemSetupCallerTest
class LocalStringDataItemSetupCallerTest : public Test
{
protected:
    NiceMock<MockSetupCallerInterface> *mp_MockSetupCaller;
    LocalStringDataItem *mp_DataItem;
    const String name = "name";
    const String initialValue = "Initial Value";

    void SetUp() override
    {
        mp_MockSetupCaller = new NiceMock<MockSetupCallerInterface>();
    }

    void TearDown() override
    {
        delete mp_MockSetupCaller;
    }
};

TEST_F(LocalStringDataItemSetupCallerTest, Registered_With_Setup_Caller)
{
    EXPECT_CALL(*mp_MockSetupCaller, RegisterForSetupCall(NotNull())).Times(1);
    mp_DataItem = new LocalStringDataItem( name 
                                         , initialValue
                                         , nullptr
                                         , mp_MockSetupCaller );
    EXPECT_CALL(*mp_MockSetupCaller, DeRegisterForSetupCall(mp_DataItem)).Times(1);
    delete mp_DataItem;
}

// Test Fixture for LocalDataItemTest
class LocalStringDataItemTest: public Test
                             , public SetupCallerInterface
{
protected:
    const String initialValue = "Initial Value";
    const String value1 = "Value 1";
    const String value2 = "Value 2";
    MockNamedCallback *mp_mockNamedCallback;
    LocalStringDataItem *mp_DataItem;
    const String name = "Name";

    void SetUp() override
    {
        mp_mockNamedCallback = new MockNamedCallback(name, nullptr);
        mp_DataItem = new LocalStringDataItem( name 
                                             , initialValue
                                             , mp_mockNamedCallback
                                             , this );

        EXPECT_CALL(mockNamedCallback_Callback, NewValueCallbackFunction(_,_,_)).Times(1);
        SetupAllSetupCallees();
        ::testing::Mock::VerifyAndClearExpectations(&mockNamedCallback_Callback);
    }

    void TearDown() override
    {
        if(mp_DataItem)
        {
            delete mp_DataItem;
        }
    }
};

TEST_F(LocalStringDataItemTest, dataItem_Name_Is_Set)
{
    EXPECT_STREQ(name.c_str(), mp_DataItem->GetName().c_str());
}

TEST_F(LocalStringDataItemTest, dataItem_Initial_Value_Is_Returned_As_String)
{
    EXPECT_STREQ(initialValue.c_str(), mp_DataItem->GetInitialValueAsString().c_str());
    EXPECT_STREQ(initialValue.c_str(), mp_DataItem->GetValueAsString().c_str());
}

TEST_F(LocalStringDataItemTest, dataItem_Set_Value_From_Char_Pointer_Converts_To_String)
{
    EXPECT_CALL(mockNamedCallback_Callback, NewValueCallbackFunction(_,_,_)).Times(1);
    mp_DataItem->SetValue(value1.c_str(), value1.length());
    ::testing::Mock::VerifyAndClearExpectations(&mockNamedCallback_Callback);
    EXPECT_STREQ(value1.c_str(), mp_DataItem->GetValueAsString().c_str());
}

TEST_F(LocalStringDataItemTest, Change_Count_Changes_Properly)
{
    EXPECT_EQ(0, mp_DataItem->GetChangeCount());
    mp_DataItem->SetValue(value1.c_str(), value1.length());
    EXPECT_EQ(1, mp_DataItem->GetChangeCount());
    mp_DataItem->SetValue(value1.c_str(), value1.length());
    EXPECT_EQ(1, mp_DataItem->GetChangeCount());
    mp_DataItem->SetValue(value2.c_str(), value1.length());
    EXPECT_EQ(2, mp_DataItem->GetChangeCount());
    mp_DataItem->SetValue(value2.c_str(), value1.length());
    EXPECT_EQ(2, mp_DataItem->GetChangeCount());
}

TEST_F(LocalStringDataItemTest, Callback_Only_Called_For_New_Values)
{
    EXPECT_CALL(mockNamedCallback_Callback, NewValueCallbackFunction(_,_,_)).Times(1);
    mp_DataItem->SetValue(value1.c_str(), value1.length());
    ::testing::Mock::VerifyAndClearExpectations(&mockNamedCallback_Callback);

    EXPECT_CALL(mockNamedCallback_Callback, NewValueCallbackFunction(_,_,_)).Times(0);
    mp_DataItem->SetValue(value1.c_str(), value1.length());
    ::testing::Mock::VerifyAndClearExpectations(&mockNamedCallback_Callback);

    EXPECT_CALL(mockNamedCallback_Callback, NewValueCallbackFunction(_,_,_)).Times(1);
    mp_DataItem->SetValue(value2.c_str(), value1.length());
    ::testing::Mock::VerifyAndClearExpectations(&mockNamedCallback_Callback);

    EXPECT_CALL(mockNamedCallback_Callback, NewValueCallbackFunction(_,_,_)).Times(0);
    mp_DataItem->SetValue(value2.c_str(), value1.length());
    ::testing::Mock::VerifyAndClearExpectations(&mockNamedCallback_Callback);
}

TEST_F(LocalStringDataItemTest, Count_Reflects_DataItem_Count)
{
    EXPECT_EQ(50, mp_DataItem->GetCount());
}