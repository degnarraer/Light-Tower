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
#include "DataItem/LocalDataItem.h"
#include "Mock_SetupCallInterface.h"
#include "Mock_ValidValueChecker.h"
#include "Mock_SerialPortMessageManager.h"
#include "Mock_Callbacks.h"

using ::testing::_;
using ::testing::NotNull;
using ::testing::NiceMock;
using namespace testing;

// Test Fixture for DataItemFunctionCallTests
class LocalDataItemFunctionCallTests : public Test
{
    protected:
        const int32_t initialValue = 10;
        const String spmm = "Serial Port Message Manager";
        const String name = "Test Name";
        MockSetupCallerInterface *mp_MockSetupCaller;
        LocalDataItem<int32_t, 1> *mp_DataItem;
        LocalDataItemFunctionCallTests(): mp_MockSetupCaller(nullptr), mp_DataItem(nullptr)
        {}
        void SetUp() override
        {
            mp_MockSetupCaller = new MockSetupCallerInterface();
        }
        void CreateDataItem()
        {
            EXPECT_CALL(*mp_MockSetupCaller, RegisterForSetupCall(NotNull())).Times(1);
            mp_DataItem = new LocalDataItem<int32_t, 1>( name 
                                                       , initialValue
                                                       , nullptr
                                                       , mp_MockSetupCaller );
            mp_DataItem->Setup();
            ::testing::Mock::VerifyAndClear(&mp_MockSetupCaller);
        }
        void TearDown() override
        {
            DestroyDataItem();
            if (mp_MockSetupCaller)
            {
                delete mp_MockSetupCaller;
                mp_MockSetupCaller = nullptr;
            }
        }
        void DestroyDataItem()
        {
            if(mp_DataItem)
            {
                EXPECT_CALL(*mp_MockSetupCaller, DeRegisterForSetupCall(mp_DataItem)).Times(1);
                delete mp_DataItem;
                mp_DataItem = nullptr;
                ::testing::Mock::VerifyAndClearExpectations(&mp_MockSetupCaller);
            }
        }
};

TEST_F(LocalDataItemFunctionCallTests, Registration_With_Setup_Caller)
{
    CreateDataItem();
    DestroyDataItem();
}

// Test Fixture for DataItemGetAndSetValueTests
template <typename T, size_t COUNT>
class LocalDataItemGetAndSetValueTests : public Test, public SetupCallerInterface
{
protected:
    const ValidStringValues_t validValues = { "10", "20", "30" };
    const int32_t validValue10 = 10;
    const int32_t validValue20 = 20;
    const int32_t validValue30 = 30;
    const int32_t invalidValue = 40;
    const String validValue10String = String(validValue10);
    const String validValue20String = String(validValue20);
    const String validValue30String = String(validValue30);
    const String invalidValueString = String(invalidValue);
    const int32_t validValue10Array[10] = {10,10,10,10,10,10,10,10,10,10};
    const int32_t validValue20Array[10] = {20,20,20,20,20,20,20,20,20,20};
    const int32_t validValue30Array[10] = {30,30,30,30,30,30,30,30,30,30};
    const int32_t invalidValueArray[10] = {40,40,40,40,40,40,40,40,40,40};
    const String validValue10ArrayString = "10|10|10|10|10|10|10|10|10|10";
    const String validValue20ArrayString = "20|20|20|20|20|20|20|20|20|20";
    const String validValue30ArrayString = "30|30|30|30|30|30|30|30|30|30";
    const String invalidValueArrayString = "40|40|40|40|40|40|40|40|40|40";
    MockNamedCallback *mp_mockNamedCallback;
    LocalDataItem<T, COUNT> *mp_DataItem;
    const String name1 = "Test Name1";
    const String name2 = "Test Name2";
    const String name3 = "Test Name3";
    const String name4 = "Test Name4";

    void SetUp() override
    {
    }

    void CreateDataItem( const String name, int32_t initialValue, ValidStringValues_t *validStringValues )
    {
        mp_mockNamedCallback = new MockNamedCallback(name, nullptr);
        mp_DataItem = new LocalDataItem<T, COUNT>( name, initialValue, mp_mockNamedCallback, this, validStringValues);
        EXPECT_CALL(mockNamedCallback_Callback, NewValueCallbackFunction(_,_,_)).Times(1);
        SetupAllSetupCallees();
        ::testing::Mock::VerifyAndClearExpectations(&mockNamedCallback_Callback);
    }

    void DestroyDataItem()
    {
        if(mp_DataItem)
        {
            delete mp_DataItem;
            mp_DataItem = nullptr;
        }
    }

    void TearDown() override
    {
        DestroyDataItem();
    }

    void TestNameIsSet( const String name, int32_t initialValue, ValidStringValues_t *validStringValues )
    {
        CreateDataItem(name1, initialValue, validStringValues);
        EXPECT_STREQ(name1.c_str(), mp_DataItem->GetName().c_str());
    }

    void TestInitialValueIsSet( const String name, int32_t initialValue, ValidStringValues_t *validStringValues )
    {
        CreateDataItem(name1, initialValue, validStringValues);
        for(size_t i = 0; i < mp_DataItem->GetCount(); ++i)
        {
            EXPECT_EQ(initialValue, mp_DataItem->GetValuePointer()[i]);
        }
    }

    void TestInitialValueReturnedAsString( const String name, int32_t initialValue, String resultString, ValidStringValues_t *validStringValues )
    {
        CreateDataItem(name1, validValue10, validStringValues);
        EXPECT_STREQ(resultString.c_str(), mp_DataItem->GetInitialValueAsString().c_str());
    }

    void TestSetValueFromValueConvertsToString( const int32_t* testValue, const String resultString, const String name, int32_t initialValue, ValidStringValues_t *validStringValues )
    {
        CreateDataItem(name1, initialValue, validStringValues);
        EXPECT_CALL(mockNamedCallback_Callback, NewValueCallbackFunction(_,_,_)).Times(1);
        mp_DataItem->SetValue(testValue, COUNT);
        EXPECT_STREQ(resultString.c_str(), mp_DataItem->GetValueAsString().c_str());
    }

    void TestSetValueFromStringConvertsToValue( const String testString, const int32_t* resultValue, const String name, int32_t initialValue, ValidStringValues_t *validStringValues )
    {
        CreateDataItem(name1, initialValue, validStringValues);
        EXPECT_CALL(mockNamedCallback_Callback, NewValueCallbackFunction(_,_,_)).Times(1);
        mp_DataItem->SetValueFromString(testString);
        for(size_t i = 0; i < COUNT; ++i)
        {
            EXPECT_EQ(resultValue[i], mp_DataItem->GetValuePointer()[i]);
        }
    }

    void TestSettingValue(const int32_t initialValue, const int32_t* testValue, bool expectValueAccepted)
    {
        CreateDataItem(name1, initialValue, &validValues);
        if(expectValueAccepted)
        {
            EXPECT_CALL(mockNamedCallback_Callback, NewValueCallbackFunction(_,_,_)).Times(1);
        }
        else
        {
            EXPECT_CALL(mockNamedCallback_Callback, NewValueCallbackFunction(_,_,_)).Times(0);
        }
        mp_DataItem->SetValue(testValue, COUNT);
        for(size_t i = 0; i < COUNT; ++i)
        {
            if(expectValueAccepted)
            {
                EXPECT_EQ(testValue[i], mp_DataItem->GetValuePointer()[i]);
            }
            else
            {
                EXPECT_NE(testValue[i], mp_DataItem->GetValuePointer()[i]);
                EXPECT_EQ(initialValue, mp_DataItem->GetValuePointer()[i]);
            }
        }
        ::testing::Mock::VerifyAndClearExpectations(&mockNamedCallback_Callback);
    }
    
    void TestSettingStringValue(const int32_t initialValue, const int32_t* testValue, const String testValueString, bool expectValueAccepted)
    {
        CreateDataItem(name1, initialValue, &validValues);
        if(expectValueAccepted)
        {
            EXPECT_CALL(mockNamedCallback_Callback, NewValueCallbackFunction(_,_,_)).Times(1);
        }
        else
        {
            EXPECT_CALL(mockNamedCallback_Callback, NewValueCallbackFunction(_,_,_)).Times(0);
        }
        mp_DataItem->SetValueFromString(testValueString);
        for(size_t i = 0; i < COUNT; ++i)
        {
            if(expectValueAccepted)
            {
                EXPECT_EQ(testValue[i], mp_DataItem->GetValuePointer()[i]);
            }
            else
            {
                EXPECT_NE(testValue[i], mp_DataItem->GetValuePointer()[i]);
                EXPECT_EQ(initialValue, mp_DataItem->GetValuePointer()[i]);
            }
        }
        ::testing::Mock::VerifyAndClearExpectations(&mockNamedCallback_Callback);
    }
};

using LocalDataItemGetAndSetValueTestsInt1 = LocalDataItemGetAndSetValueTests<int32_t, 1>;
using LocalDataItemGetAndSetValueTestsInt10 = LocalDataItemGetAndSetValueTests<int32_t, 10>;

// ************ Name is set ******************
TEST_F(LocalDataItemGetAndSetValueTestsInt1, dataItem_Name_Is_Set)
{
    TestNameIsSet(name1, validValue10, nullptr);
}
TEST_F(LocalDataItemGetAndSetValueTestsInt10, dataItemArray_Name_Is_Set)
{
    TestNameIsSet(name1, validValue10, nullptr);
}
TEST_F(LocalDataItemGetAndSetValueTestsInt1, dataItemWithValidation_Name_Is_Set)
{
    TestNameIsSet(name1, validValue10, &validValues);
}

TEST_F(LocalDataItemGetAndSetValueTestsInt10, dataItemArrayWithValidation_Name_Is_Set)
{
    TestNameIsSet(name1, validValue10, &validValues);
}

// ************ Initial Value is set ******************
TEST_F(LocalDataItemGetAndSetValueTestsInt1, dataItem_Initial_Value_Is_Set)
{
    TestInitialValueIsSet(name1, validValue10, nullptr);
}
TEST_F(LocalDataItemGetAndSetValueTestsInt10, dataItemArray_Initial_Value_Is_Set)
{
    TestInitialValueIsSet(name1, validValue10, nullptr);
}
TEST_F(LocalDataItemGetAndSetValueTestsInt1, dataItemWithValidation_Initial_Value_Is_Set)
{
    TestInitialValueIsSet(name1, validValue10, &validValues);
}
TEST_F(LocalDataItemGetAndSetValueTestsInt10, dataItemArrayWithValidation_Initial_Value_Is_Set)
{
    TestInitialValueIsSet(name1, validValue10, &validValues);
}

// ************ Initial Value Returned as String ******************
TEST_F(LocalDataItemGetAndSetValueTestsInt1, dataItem_Initial_Value_Is_Returned_As_String)
{
    TestInitialValueReturnedAsString(name1, validValue10, validValue10String, nullptr);
}
TEST_F(LocalDataItemGetAndSetValueTestsInt10, dataItemArray_Initial_Value_Is_Returned_As_String)
{
    TestInitialValueReturnedAsString(name1, validValue10, validValue10ArrayString, nullptr);
}
TEST_F(LocalDataItemGetAndSetValueTestsInt1, dataItemWithValidation_Initial_Value_Is_Returned_As_String)
{
    TestInitialValueReturnedAsString(name1, validValue10, validValue10String, &validValues);
}
TEST_F(LocalDataItemGetAndSetValueTestsInt10, dataItemArrayWithValidation_Initial_Value_Is_Returned_As_String)
{
    TestInitialValueReturnedAsString(name1, validValue10, validValue10ArrayString, &validValues);
}

// ************ Set Value From Value Converts To String ******************
TEST_F(LocalDataItemGetAndSetValueTestsInt1, dataItem_Set_Value_From_Value_Converts_To_String)
{
    TestSetValueFromValueConvertsToString(&validValue20, validValue20String, name1, validValue10, nullptr);
}
TEST_F(LocalDataItemGetAndSetValueTestsInt1, dataItemWithValidation_Set_Value_From_Value_Converts_To_String)
{
    TestSetValueFromValueConvertsToString(&validValue20, validValue20String, name1, validValue10, &validValues);
}
TEST_F(LocalDataItemGetAndSetValueTestsInt10, dataItemArray_Set_Value_From_Value_Converts_To_String)
{
    TestSetValueFromValueConvertsToString(validValue20Array, validValue20ArrayString, name1, validValue10, nullptr);
}
TEST_F(LocalDataItemGetAndSetValueTestsInt10, dataItemArrayWithValidation_Set_Value_From_Value_Converts_To_String)
{
    TestSetValueFromValueConvertsToString(validValue20Array, validValue20ArrayString, name1, validValue10, &validValues);
}

// ************ Set Value From String Converts To Value ******************
TEST_F(LocalDataItemGetAndSetValueTestsInt1, dataItem_Set_Value_From_String_Converts_To_Value)
{
    TestSetValueFromStringConvertsToValue(validValue20String, &validValue20, name1, validValue10, nullptr);
}
TEST_F(LocalDataItemGetAndSetValueTestsInt1, dataItemWithValidation_Set_Value_From_String_Converts_To_Value)
{
    TestSetValueFromStringConvertsToValue(validValue20String, &validValue20, name1, validValue10, &validValues);
}
TEST_F(LocalDataItemGetAndSetValueTestsInt10, dataItemArray_Set_Value_From_String_Converts_To_Value)
{
    TestSetValueFromStringConvertsToValue(validValue20ArrayString, validValue20Array, name1, validValue10, nullptr);
}
TEST_F(LocalDataItemGetAndSetValueTestsInt10, dataItemArrayWithValidation_Set_Value_From_String_Converts_To_Value)
{
    TestSetValueFromStringConvertsToValue(validValue20ArrayString, validValue20Array, name1, validValue10, &validValues);
}

TEST_F(LocalDataItemGetAndSetValueTestsInt1, dataItem_Set_Valid_Values_When_Validation_Is_Used)
{
    TestSettingValue(validValue10, &validValue20, true);
}
TEST_F(LocalDataItemGetAndSetValueTestsInt1, dataItem_Reject_Invalid_Value_When_Validation_Is_Used)
{
    TestSettingValue(validValue10, &invalidValue, false);
}
TEST_F(LocalDataItemGetAndSetValueTestsInt10, dataItemArray_Set_Valid_Array_Values_When_Validation_Is_Used)
{
    TestSettingValue(validValue10, validValue20Array, true);
}
TEST_F(LocalDataItemGetAndSetValueTestsInt10, dataItemArray_Reject_Invalid_Array_Values_When_Validation_Is_Used)
{
    TestSettingValue(validValue10, invalidValueArray, false);
}

TEST_F(LocalDataItemGetAndSetValueTestsInt1, dataItem_Set_Valid_String_Value_When_Validation_Is_Used)
{
    TestSettingStringValue(validValue10, &validValue20, validValue20String, true);
}
TEST_F(LocalDataItemGetAndSetValueTestsInt1, dataItem_Reject_Invalid_String_Value_When_Validation_Is_Used)
{
    TestSettingStringValue(validValue10, &invalidValue, invalidValueString, false);
}
TEST_F(LocalDataItemGetAndSetValueTestsInt10, dataItemArray_Set_Valid_Array_String_Value_When_Validation_Is_Used)
{
    TestSettingStringValue(validValue10, validValue20Array, validValue20ArrayString, true);
}
TEST_F(LocalDataItemGetAndSetValueTestsInt10, dataItemArray_Reject_Invalid_Array_String_Value_When_Validation_Is_Used)
{
    TestSettingStringValue(validValue10, invalidValueArray, invalidValueArrayString, false);
}

TEST_F(LocalDataItemGetAndSetValueTestsInt1, Change_Count_Changes_Properly_When_Validation_Used)
{
    CreateDataItem(name1, validValue10, &validValues);
    EXPECT_EQ(0, mp_DataItem->GetChangeCount());
    mp_DataItem->SetValue(validValue20);
    EXPECT_EQ(1, mp_DataItem->GetChangeCount());
    mp_DataItem->SetValue(validValue20);
    EXPECT_EQ(1, mp_DataItem->GetChangeCount());
    mp_DataItem->SetValue(invalidValue);
    EXPECT_EQ(1, mp_DataItem->GetChangeCount());
    mp_DataItem->SetValue(validValue30);
    EXPECT_EQ(2, mp_DataItem->GetChangeCount());
}

TEST_F(LocalDataItemGetAndSetValueTestsInt1, Change_Count_Changes_Properly_When_Validation_Not_Used)
{
    CreateDataItem(name1, validValue10, nullptr);
    EXPECT_EQ(0, mp_DataItem->GetChangeCount());
    mp_DataItem->SetValue(validValue20);
    EXPECT_EQ(1, mp_DataItem->GetChangeCount());
    mp_DataItem->SetValue(validValue20);
    EXPECT_EQ(1, mp_DataItem->GetChangeCount());
    mp_DataItem->SetValue(invalidValue);
    EXPECT_EQ(2, mp_DataItem->GetChangeCount());
    mp_DataItem->SetValue(validValue30);
    EXPECT_EQ(3, mp_DataItem->GetChangeCount());
}

TEST_F(LocalDataItemGetAndSetValueTestsInt1, Callback_Only_Called_For_New_Valid_Values_When_Validation_Used)
{
    CreateDataItem(name1, validValue10, &validValues);
    
    EXPECT_CALL(mockNamedCallback_Callback, NewValueCallbackFunction(_,_,_)).Times(1);
    mp_DataItem->SetValue(validValue20);
    ::testing::Mock::VerifyAndClearExpectations(&mockNamedCallback_Callback);

    EXPECT_CALL(mockNamedCallback_Callback, NewValueCallbackFunction(_,_,_)).Times(0);
    mp_DataItem->SetValue(validValue20);
    ::testing::Mock::VerifyAndClearExpectations(&mockNamedCallback_Callback);
    
    EXPECT_CALL(mockNamedCallback_Callback, NewValueCallbackFunction(_,_,_)).Times(0);
    mp_DataItem->SetValue(invalidValue);
    ::testing::Mock::VerifyAndClearExpectations(&mockNamedCallback_Callback);
    
    EXPECT_CALL(mockNamedCallback_Callback, NewValueCallbackFunction(_,_,_)).Times(1);
    mp_DataItem->SetValue(validValue30);
    ::testing::Mock::VerifyAndClearExpectations(&mockNamedCallback_Callback);
}

TEST_F(LocalDataItemGetAndSetValueTestsInt1, Callback_Only_Called_For_New_Values_When_Validation_Not_Used)
{
    CreateDataItem(name1, validValue10, nullptr);
    EXPECT_CALL(mockNamedCallback_Callback, NewValueCallbackFunction(_,_,_)).Times(1);
    mp_DataItem->SetValue(validValue20);
    ::testing::Mock::VerifyAndClearExpectations(&mockNamedCallback_Callback);

    EXPECT_CALL(mockNamedCallback_Callback, NewValueCallbackFunction(_,_,_)).Times(0);
    mp_DataItem->SetValue(validValue20);
    ::testing::Mock::VerifyAndClearExpectations(&mockNamedCallback_Callback);

    EXPECT_CALL(mockNamedCallback_Callback, NewValueCallbackFunction(_,_,_)).Times(1);
    mp_DataItem->SetValue(invalidValue);
    ::testing::Mock::VerifyAndClearExpectations(&mockNamedCallback_Callback);

    EXPECT_CALL(mockNamedCallback_Callback, NewValueCallbackFunction(_,_,_)).Times(1);
    mp_DataItem->SetValue(validValue30);
    ::testing::Mock::VerifyAndClearExpectations(&mockNamedCallback_Callback);
}