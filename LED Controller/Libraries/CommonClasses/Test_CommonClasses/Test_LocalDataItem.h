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
    const int32_t validValue1 = 20;
    const int32_t validValue2 = 30;
    const int32_t invalidValue = 40;
    const String initialValueString = String(initialValue);
    const String validValue1String = String(validValue1);
    const String validValue2String = String(validValue2);
    const String invalidValueString = String(invalidValue);
    const int32_t initialValueArray[10] = {10,10,10,10,10,10,10,10,10,10};
    const int32_t validValue1Array[10] = {20,20,20,20,20,20,20,20,20,20};
    const int32_t validValue2Array[10] = {30,30,30,30,30,30,30,30,30,30};
    const int32_t invalidValueArray[10] = {40,40,40,40,40,40,40,40,40,40};
    const String initialValueArrayString = "10|10|10|10|10|10|10|10|10|10";
    const String validValue1ArrayString = "20|20|20|20|20|20|20|20|20|20";
    const String validValue2ArrayString = "30|30|30|30|30|30|30|30|30|30";
    const String invalidValueArrayString = "40|40|40|40|40|40|40|40|40|40";
    LocalDataItem<int32_t, 1> *dataItem;
    LocalDataItem<int32_t, 10> *dataItemArray;
    LocalDataItem<int32_t, 1> *dataItemWithValidation;
    LocalDataItem<int32_t, 10> *dataItemArrayWithValidation;
    const String name1 = "Test Name1";
    const String name2 = "Test Name2";
    const String name3 = "Test Name3";
    const String name4 = "Test Name4";

    void SetUp() override
    {
        dataItem = new LocalDataItem<int32_t, 1>( name1 
                                                , initialValue
                                                , nullptr
                                                , this );

        dataItemArray = new LocalDataItem<int32_t, 10>( name2 
                                                      , initialValue
                                                      , nullptr
                                                      , this );

        dataItemWithValidation = new LocalDataItem<int32_t, 1>( name3 
                                                              , initialValue
                                                              , nullptr
                                                              , this
                                                              , &validValues );

        dataItemArrayWithValidation = new LocalDataItem<int32_t, 10>( name4 
                                                                    , initialValue
                                                                    , nullptr
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

TEST_F(LocalDataItemTest, dataItem_Name_Is_Set)
{
    EXPECT_STREQ(name1.c_str(), dataItem->GetName().c_str());
}
TEST_F(LocalDataItemTest, dataItemArray_Name_Is_Set)
{
    EXPECT_STREQ(name2.c_str(), dataItemArray->GetName().c_str());
}
TEST_F(LocalDataItemTest, dataItemWithValidation_Name_Is_Set)
{
    EXPECT_STREQ(name3.c_str(), dataItemWithValidation->GetName().c_str());
}

TEST_F(LocalDataItemTest, dataItem_Initial_Value_Is_Set)
{
    EXPECT_EQ(initialValue, dataItem->GetValue());
}
TEST_F(LocalDataItemTest, dataItemArray_Initial_Value_Is_Set)
{
    EXPECT_EQ(initialValue, dataItemWithValidation->GetValue());
}
TEST_F(LocalDataItemTest, dataItemWithValidation_Initial_Value_Is_Set)
{
    EXPECT_EQ(initialValue, dataItemWithValidation->GetValue());
}

TEST_F(LocalDataItemTest, dataItem_Initial_Value_Is_Returned_As_String)
{
    EXPECT_STREQ(initialValueString.c_str(), dataItem->GetInitialValueAsString().c_str());
}
TEST_F(LocalDataItemTest, dataItemArray_Initial_Value_Is_Returned_As_String)
{
    EXPECT_STREQ(initialValueString.c_str(), dataItemArray->GetInitialValueAsString().c_str());
}
TEST_F(LocalDataItemTest, dataItemWithValidation_Initial_Value_Is_Returned_As_String)
{
    EXPECT_STREQ(initialValueString.c_str(), dataItemWithValidation->GetInitialValueAsString().c_str());
}

TEST_F(LocalDataItemTest, dataItem_Value_Is_Returned_As_String)
{
    EXPECT_STREQ(initialValueString.c_str(), dataItem->GetValueAsString().c_str());
}
TEST_F(LocalDataItemTest, dataItemArray_Value_Is_Returned_As_String)
{
    EXPECT_STREQ(initialValueArrayString.c_str(), dataItemArray->GetValueAsString().c_str());
}
TEST_F(LocalDataItemTest, dataItemWithValidation_Value_Is_Returned_As_String)
{
    EXPECT_STREQ(initialValueString.c_str(), dataItemWithValidation->GetValueAsString().c_str());
}

TEST_F(LocalDataItemTest, dataItem_Set_Value_From_Value_Converts_To_String)
{
    dataItem->SetValue(validValue1);
    EXPECT_STREQ(validValue1String.c_str(), dataItem->GetValueAsString().c_str());
}
TEST_F(LocalDataItemTest, dataItemArray_Set_Value_From_Value_Converts_To_String)
{
    dataItemArray->SetValue(validValue1Array, sizeof(validValue1Array)/sizeof(validValue1Array[0]));
    EXPECT_STREQ(validValue1ArrayString.c_str(), dataItemArray->GetValueAsString().c_str());
}
TEST_F(LocalDataItemTest, dataItemWithValidation_Set_Value_From_Value_Converts_To_String)
{
    dataItemWithValidation->SetValue(validValue1);
    EXPECT_STREQ(validValue1String.c_str(), dataItemWithValidation->GetValueAsString().c_str()); 
}

TEST_F(LocalDataItemTest, dataItem_Set_Value_From_String_Converts_To_Value)
{
    dataItem->SetValueFromString(validValue1String);
    EXPECT_EQ(validValue1, dataItem->GetValue());
}
TEST_F(LocalDataItemTest, dataItemArray_Set_Value_From_String_Converts_To_Value)
{
    dataItemArray->SetValueFromString(validValue1ArrayString);
    for(size_t i = 0; i < dataItemArray->GetCount(); ++i)
    {
        EXPECT_EQ(validValue1Array[i], dataItemArray->GetValuePointer()[i]);
    }
}
TEST_F(LocalDataItemTest, dataItemWithValidation_Set_Value_From_String_Converts_To_Value)
{
    dataItemWithValidation->SetValueFromString(validValue1String);
    EXPECT_EQ(validValue1, dataItemWithValidation->GetValue()); 
}

TEST_F(LocalDataItemTest, dataItem_Valid_Values_Accepted_When_Validation_Is_Used)
{
    dataItem->SetValue(validValue1);
    EXPECT_EQ(validValue1, dataItem->GetValue());

    dataItem->SetValueFromString(validValue2String);
    EXPECT_EQ(validValue2, dataItem->GetValue());
}
TEST_F(LocalDataItemTest, dataItemWithValidation_Valid_Values_Accepted_When_Validation_Is_Used)
{
    dataItemWithValidation->SetValue(validValue1);
    EXPECT_EQ(validValue1, dataItemWithValidation->GetValue());

    dataItemWithValidation->SetValueFromString(validValue2String);
    EXPECT_EQ(validValue2, dataItemWithValidation->GetValue());
}
TEST_F(LocalDataItemTest, dataItemArrayWithValidation_Valid_Values_Accepted_When_Validation_Is_Used)
{
    dataItemArrayWithValidation->SetValue(validValue1Array, sizeof(validValue1Array)/sizeof(validValue1Array[0]));
    for(size_t i = 0; i < dataItemArrayWithValidation->GetCount(); ++i)
    {
        EXPECT_EQ(validValue1Array[i], dataItemArrayWithValidation->GetValuePointer()[i]);
    }

    dataItemArrayWithValidation->SetValue(validValue2Array, sizeof(validValue2Array)/sizeof(validValue2Array[0]));
    for(size_t i = 0; i < dataItemArrayWithValidation->GetCount(); ++i)
    {
        EXPECT_EQ(validValue2Array[i], dataItemArrayWithValidation->GetValuePointer()[i]);
    }
}

TEST_F(LocalDataItemTest, dataItem_Invalid_Values_Rejected_When_Validation_Is_Used)
{
    dataItem->SetValue(invalidValue);
    EXPECT_EQ(invalidValue, dataItem->GetValue());
    
    dataItem->SetValue(initialValue);
    EXPECT_EQ(initialValue, dataItem->GetValue());
    
    dataItem->SetValueFromString(invalidValueString);
    EXPECT_EQ(invalidValue, dataItem->GetValue());
}
TEST_F(LocalDataItemTest, dataItemWithValidation_Invalid_Values_Rejected_When_Validation_Is_Used)
{
    dataItemWithValidation->SetValue(invalidValue);
    EXPECT_NE(invalidValue, dataItemWithValidation->GetValue());

    dataItemWithValidation->SetValue(initialValue);
    EXPECT_EQ(initialValue, dataItemWithValidation->GetValue());

    dataItemWithValidation->SetValueFromString(invalidValueString);
    EXPECT_NE(invalidValue, dataItemWithValidation->GetValue());
}
TEST_F(LocalDataItemTest, dataItemArrayWithValidation_Invalid_Values_Rejected_When_Validation_Is_Used)
{
    dataItemArrayWithValidation->SetValue(invalidValueArray, sizeof(invalidValueArray)/sizeof(invalidValueArray[0]));
    for(size_t i = 0; i < dataItemArrayWithValidation->GetCount(); ++i)
    {
        EXPECT_NE(invalidValueArray[i], dataItemArrayWithValidation->GetValuePointer()[i]);
    }
    
    dataItemArrayWithValidation->SetValue(initialValueArray, sizeof(initialValueArray)/sizeof(initialValueArray[0]));
    for(size_t i = 0; i < dataItemArrayWithValidation->GetCount(); ++i)
    {
        EXPECT_EQ(initialValueArray[i], dataItemArrayWithValidation->GetValuePointer()[i]);
    }
    
    dataItemArrayWithValidation->SetValue(invalidValueArray, sizeof(invalidValueArray)/sizeof(invalidValueArray[0]));
    for(size_t i = 0; i < dataItemArrayWithValidation->GetCount(); ++i)
    {
        EXPECT_NE(invalidValueArray[i], dataItemArrayWithValidation->GetValuePointer()[i]);
    }
}

TEST_F(LocalDataItemTest, Previous_Value_Retained_When_Value_Rejected)
{
    dataItemWithValidation->SetValue(invalidValue);
    EXPECT_EQ(initialValue, dataItemWithValidation->GetValue());
}

TEST_F(LocalDataItemTest, Change_Count_Changes_Properly)
{
    EXPECT_EQ(0, dataItemWithValidation->GetChangeCount());
    dataItemWithValidation->SetValue(validValue1);
    EXPECT_EQ(1, dataItemWithValidation->GetChangeCount());
    dataItemWithValidation->SetValue(validValue1);
    EXPECT_EQ(1, dataItemWithValidation->GetChangeCount());
    dataItemWithValidation->SetValue(validValue2);
    EXPECT_EQ(2, dataItemWithValidation->GetChangeCount());
    dataItemWithValidation->SetValue(validValue2);
    EXPECT_EQ(2, dataItemWithValidation->GetChangeCount());
    dataItemWithValidation->SetValue(validValue1);
    EXPECT_EQ(3, dataItemWithValidation->GetChangeCount());
    dataItemWithValidation->SetValue(invalidValue);
    EXPECT_EQ(3, dataItemWithValidation->GetChangeCount());
}

TEST_F(LocalDataItemTest, Count_Reflects_DataItem_Count)
{
    EXPECT_EQ(1, dataItem->GetCount());
    EXPECT_EQ(10, dataItemArray->GetCount());
    EXPECT_EQ(1, dataItemWithValidation->GetCount());
}