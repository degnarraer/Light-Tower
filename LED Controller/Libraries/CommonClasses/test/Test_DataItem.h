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
#include "DataItem/DataItem.h"
#include "Mock_SetupCallInterface.h"
#include "Mock_ValidValueChecker.h"
#include "Mock_SerialMessageManager.h"

using ::testing::_;
using ::testing::NotNull;
using ::testing::NiceMock;
using namespace testing;

// Test Fixture for DataItemFunctionCallTests
class DataItemFunctionCallTests : public Test
{
    protected:
        const int32_t initialValue = 10;
        const String spmm = "Serial Port Message Manager";
        const String name = "Test Name";
        NiceMock<MockSetupCallerInterface> *mp_MockSetupCaller;
        NiceMock<MockSerialPortMessageManager> *mp_MockSerialPortMessageManager;
        MockHardwareSerial m_MockHardwareSerial;
        MockDataSerializer m_MockDataSerializer;
        DataItem<int32_t, 1> *mp_DataItem;
        DataItemFunctionCallTests()
            : mp_MockSetupCaller(nullptr)
            , mp_MockSerialPortMessageManager(nullptr)
            , mp_DataItem(nullptr)
        {}
        void SetUp() override
        {
            mp_MockSetupCaller = new NiceMock<MockSetupCallerInterface>();
            mp_MockSerialPortMessageManager = new NiceMock<MockSerialPortMessageManager>( name, m_MockHardwareSerial, m_MockDataSerializer, 0 );
            ON_CALL(*mp_MockSerialPortMessageManager, QueueMessageFromData(_,_,_,_)).WillByDefault(Return(true));
            ON_CALL(*mp_MockSerialPortMessageManager, GetName()).WillByDefault(Return(spmm));
        }
        void CreateDataItem(RxTxType_t rxTxType, UpdateStoreType_t updateStoreType, uint16_t rate)
        {
            mp_DataItem = new DataItem<int32_t, 1>( name 
                                                , initialValue
                                                , rxTxType
                                                , updateStoreType
                                                , rate
                                                , *mp_MockSerialPortMessageManager
                                                , nullptr
                                                , mp_MockSetupCaller );
            mp_DataItem->Setup();
        }
        void TearDown() override
        {
            DestroyDataItem();
            if (mp_MockSerialPortMessageManager)
            {
                delete mp_MockSerialPortMessageManager;
                mp_MockSerialPortMessageManager = nullptr;
            }
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
                delete mp_DataItem;
                mp_DataItem = nullptr;
            }
        }
        void TestSetupCallRegistration(RxTxType_t rxtxtype, size_t callTimes)
        {
            EXPECT_CALL(*mp_MockSetupCaller, RegisterForSetupCall(NotNull())).Times(callTimes);
            CreateDataItem(rxtxtype, UpdateStoreType_On_Rx, 1000);
            EXPECT_CALL(*mp_MockSetupCaller, DeRegisterForSetupCall(NotNull())).Times(callTimes);
            DestroyDataItem();
        }
        void TestNewValueNotificationRegistration(RxTxType_t rxtxtype, size_t callTimes)
        {
            EXPECT_CALL(*mp_MockSerialPortMessageManager, RegisterForNewValueNotification(NotNull())).Times(callTimes);
            CreateDataItem(rxtxtype, UpdateStoreType_On_Rx, 1000);
            EXPECT_CALL(*mp_MockSerialPortMessageManager, DeRegisterForNewValueNotification(NotNull())).Times(callTimes);
            DestroyDataItem();
        }
};

TEST_F(DataItemFunctionCallTests, Registration_With_Setup_Caller)
{
    TestSetupCallRegistration(RxTxType_Tx_Periodic, 1);
    TestSetupCallRegistration(RxTxType_Tx_On_Change_With_Heartbeat, 1);
    TestSetupCallRegistration(RxTxType_Tx_On_Change, 1);
    TestSetupCallRegistration(RxTxType_Rx_Only, 1);
    TestSetupCallRegistration(RxTxType_Rx_Echo_Value, 1);
}

TEST_F(DataItemFunctionCallTests, Registration_For_New_Value_Notification)
{
    TestNewValueNotificationRegistration(RxTxType_Tx_Periodic, 1);
    TestNewValueNotificationRegistration(RxTxType_Tx_On_Change_With_Heartbeat, 1);
    TestNewValueNotificationRegistration(RxTxType_Tx_On_Change, 1);
    TestNewValueNotificationRegistration(RxTxType_Rx_Only, 1);
    TestNewValueNotificationRegistration(RxTxType_Rx_Echo_Value, 1);
}


// Test Fixture for DataItem Rx Tx Tests
class DataItemRxTxTests : public Test
                        , public SetupCallerInterface
{
    protected:
        const int32_t initialValue = 10;
        const String name = "Test Name1";
        const String spmm = "Serial Port Message Manager";
        MockHardwareSerial m_MockHardwareSerial;
        MockDataSerializer m_MockDataSerializer;
        NiceMock<MockSerialPortMessageManager> *mp_MockSerialPortMessageManager;
        DataItem<int32_t, 1> *mp_DataItem;
        DataItemRxTxTests()
            : mp_MockSerialPortMessageManager(nullptr)
            , mp_DataItem(nullptr)
        {}

        void SetUp() override
        {
            mp_MockSerialPortMessageManager = new NiceMock<MockSerialPortMessageManager>( name, m_MockHardwareSerial, m_MockDataSerializer, 0 );
            ON_CALL(*mp_MockSerialPortMessageManager, QueueMessageFromData(_,_,_,_)).WillByDefault(Return(true));
            ON_CALL(*mp_MockSerialPortMessageManager, GetName()).WillByDefault(Return(spmm));
        }

        void CreateDataItem(RxTxType_t rxTxType, UpdateStoreType_t updateStoreType, uint16_t rate)
        {
            mp_DataItem = new DataItem<int32_t, 1>( name 
                                                , initialValue
                                                , rxTxType
                                                , updateStoreType
                                                , rate
                                                , *mp_MockSerialPortMessageManager
                                                , NULL
                                                , this );
            SetupAllSetupCallees();
        }

        void DestroyDataItem()
        {
            if(mp_DataItem)
            {
                EXPECT_CALL(*mp_MockSerialPortMessageManager, DeRegisterForNewValueNotification(NotNull()));
                delete mp_DataItem;
                mp_DataItem = nullptr;
            }
        }

        void TearDown() override
        {
            DestroyDataItem();
            if (mp_MockSerialPortMessageManager)
            {
                delete mp_MockSerialPortMessageManager;
                mp_MockSerialPortMessageManager = nullptr;
            }
        }
};

TEST_F(DataItemRxTxTests, Tx_Called_Periodically)
{
    EXPECT_CALL(*mp_MockSerialPortMessageManager, QueueMessageFromData(_,_,_,_)).Times(10)
        .WillRepeatedly(Return(true));
    CreateDataItem(RxTxType_Tx_Periodic, UpdateStoreType_On_Rx, 100);
    std::this_thread::sleep_for(std::chrono::milliseconds(1050));
}

// Test Fixture for DataItemGetAndSetValueTests
template <typename T, size_t COUNT>
class DataItemGetAndSetValueTests : public Test, public SetupCallerInterface
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
    DataItem<T, COUNT> *mp_DataItem;
    MockHardwareSerial m_MockHardwareSerial;
    MockDataSerializer m_MockDataSerializer;
    NiceMock<MockSerialPortMessageManager> *mp_MockSerialPortMessageManager;
    const String spmm = "Serial Port Message Manager";
    const String name1 = "Test Name1";
    const String name2 = "Test Name2";
    const String name3 = "Test Name3";
    const String name4 = "Test Name4";

    void SetUp() override
    {
        ESP_LOGD("SetUp", "Test SetUp!");
        mp_MockSerialPortMessageManager = new NiceMock<MockSerialPortMessageManager>( spmm, m_MockHardwareSerial, m_MockDataSerializer, 0 );
        EXPECT_CALL(*mp_MockSerialPortMessageManager, GetName()).WillRepeatedly(Return(spmm));
    }

    void CreateDataItem( const String name, int32_t initialValue, RxTxType_t rxTxType, UpdateStoreType_t updateStoreType, uint16_t rate, ValidStringValues_t *validStringValues )
    {
        mp_DataItem = new DataItem<T, COUNT>( name, initialValue, rxTxType, updateStoreType, rate, *mp_MockSerialPortMessageManager, nullptr, this, validStringValues);
        SetupAllSetupCallees();
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
        ESP_LOGD("TearDown", "Test TearDown!");
        DestroyDataItem();
        delete mp_MockSerialPortMessageManager;
    }

    void TestNameIsSet( const String name, int32_t initialValue, RxTxType_t rxTxType, UpdateStoreType_t updateStoreType, uint16_t rate, ValidStringValues_t *validStringValues )
    {
        CreateDataItem(name1, initialValue, rxTxType, updateStoreType, rate, validStringValues);
        EXPECT_STREQ(name1.c_str(), mp_DataItem->GetName().c_str());
    }

    void TestInitialValueIsSet( const String name, int32_t initialValue, RxTxType_t rxTxType, UpdateStoreType_t updateStoreType, uint16_t rate, ValidStringValues_t *validStringValues )
    {
        CreateDataItem(name1, initialValue, rxTxType, updateStoreType, rate, validStringValues);
        for(size_t i = 0; i < mp_DataItem->GetCount(); ++i)
        {
            EXPECT_EQ(initialValue, mp_DataItem->GetValuePointer()[i]);
        }
    }

    void TestInitialValueReturnedAsString( const String name, int32_t initialValue, RxTxType_t rxTxType, UpdateStoreType_t updateStoreType, uint16_t rate, ValidStringValues_t *validStringValues )
    {
        CreateDataItem(name1, validValue10, rxTxType, updateStoreType, rate, validStringValues);
        EXPECT_STREQ(validValue10String.c_str(), mp_DataItem->GetInitialValueAsString().c_str());
    }

    void TestSetValueFromValueConvertsToString( const int32_t* testValue, const String resultString, const String name, int32_t initialValue, RxTxType_t rxTxType, UpdateStoreType_t updateStoreType, uint16_t rate, ValidStringValues_t *validStringValues )
    {
        CreateDataItem(name1, initialValue, rxTxType, updateStoreType, rate, validStringValues);
        mp_DataItem->SetValue(testValue, COUNT);
        EXPECT_STREQ(resultString.c_str(), mp_DataItem->GetValueAsString().c_str());
    }

    void TestSetValueFromStringConvertsToValue( const String testString, const int32_t* resultValue, const String name, int32_t initialValue, RxTxType_t rxTxType, UpdateStoreType_t updateStoreType, uint16_t rate, ValidStringValues_t *validStringValues )
    {
        CreateDataItem(name1, initialValue, rxTxType, updateStoreType, rate, validStringValues);
        mp_DataItem->SetValueFromString(testString);
        for(size_t i = 0; i < COUNT; ++i)
        {
            EXPECT_EQ(resultValue[i], mp_DataItem->GetValuePointer()[i]);
        }
    }

    void TestSettingValue(const int32_t initialValue, const int32_t* testValue, const String testValueString, bool expectEqual)
    {
        CreateDataItem(name1, initialValue, RxTxType_Rx_Only, UpdateStoreType_On_Rx, 0, &validValues);
        mp_DataItem->SetValue(testValue, COUNT);
        for(size_t i = 0; i < COUNT; ++i)
        {
            if(expectEqual)
            {
                EXPECT_EQ(testValue[i], mp_DataItem->GetValuePointer()[i]);
            }
            else
            {
                EXPECT_NE(testValue[i], mp_DataItem->GetValuePointer()[i]);
                EXPECT_EQ(initialValue, mp_DataItem->GetValuePointer()[i]);
            }
        }

        mp_DataItem->SetValueFromString(testValueString);
        for(size_t i = 0; i < COUNT; ++i)
        {
            if(expectEqual)
            {
                EXPECT_EQ(testValue[i], mp_DataItem->GetValuePointer()[i]);
            }
            else
            {
                EXPECT_NE(testValue[i], mp_DataItem->GetValuePointer()[i]);
                EXPECT_EQ(initialValue, mp_DataItem->GetValuePointer()[i]);
            }
        }
    }
};

using DataItemGetAndSetValueTestsInt1 = DataItemGetAndSetValueTests<int32_t, 1>;
using DataItemGetAndSetValueTestsInt10 = DataItemGetAndSetValueTests<int32_t, 10>;

// ************ Name is set ******************
TEST_F(DataItemGetAndSetValueTestsInt1, dataItem_Name_Is_Set)
{
    TestNameIsSet(name1, validValue10, RxTxType_Rx_Only, UpdateStoreType_On_Rx, 0, nullptr);
}
TEST_F(DataItemGetAndSetValueTestsInt10, dataItemArray_Name_Is_Set)
{
    TestNameIsSet(name1, validValue10, RxTxType_Rx_Only, UpdateStoreType_On_Rx, 0, nullptr);
}
TEST_F(DataItemGetAndSetValueTestsInt1, dataItemWithValidation_Name_Is_Set)
{
    TestNameIsSet(name1, validValue10, RxTxType_Rx_Only, UpdateStoreType_On_Rx, 0, &validValues);
}

TEST_F(DataItemGetAndSetValueTestsInt10, dataItemArrayWithValidation_Name_Is_Set)
{
    TestNameIsSet(name1, validValue10, RxTxType_Rx_Only, UpdateStoreType_On_Rx, 0, &validValues);
}


// ************ Initial Value is set ******************
TEST_F(DataItemGetAndSetValueTestsInt1, dataItem_Initial_Value_Is_Set)
{
    TestInitialValueIsSet(name1, validValue10, RxTxType_Rx_Only, UpdateStoreType_On_Rx, 0, nullptr);
}
TEST_F(DataItemGetAndSetValueTestsInt10, dataItemArray_Initial_Value_Is_Set)
{
    TestInitialValueIsSet(name1, validValue10, RxTxType_Rx_Only, UpdateStoreType_On_Rx, 0, nullptr);
}
TEST_F(DataItemGetAndSetValueTestsInt1, dataItemWithValidation_Initial_Value_Is_Set)
{
    TestInitialValueIsSet(name1, validValue10, RxTxType_Rx_Only, UpdateStoreType_On_Rx, 0, &validValues);
}
TEST_F(DataItemGetAndSetValueTestsInt10, dataItemArrayWithValidation_Initial_Value_Is_Set)
{
    TestInitialValueIsSet(name1, validValue10, RxTxType_Rx_Only, UpdateStoreType_On_Rx, 0, &validValues);
}

// ************ Initial Value Returned as String ******************
TEST_F(DataItemGetAndSetValueTestsInt1, dataItem_Initial_Value_Is_Returned_As_String)
{
    TestInitialValueReturnedAsString(name1, validValue10, RxTxType_Rx_Only, UpdateStoreType_On_Rx, 0, nullptr);
}
TEST_F(DataItemGetAndSetValueTestsInt10, dataItemArray_Initial_Value_Is_Returned_As_String)
{
    TestInitialValueReturnedAsString(name1, validValue10, RxTxType_Rx_Only, UpdateStoreType_On_Rx, 0, nullptr);
}
TEST_F(DataItemGetAndSetValueTestsInt1, dataItemWithValidation_Initial_Value_Is_Returned_As_String)
{
    TestInitialValueReturnedAsString(name1, validValue10, RxTxType_Rx_Only, UpdateStoreType_On_Rx, 0, &validValues);
}
TEST_F(DataItemGetAndSetValueTestsInt10, dataItemArrayWithValidation_Initial_Value_Is_Returned_As_String)
{
    TestInitialValueReturnedAsString(name1, validValue10, RxTxType_Rx_Only, UpdateStoreType_On_Rx, 0, &validValues);
}

// ************ Set Value From Value Converts To String ******************
TEST_F(DataItemGetAndSetValueTestsInt1, dataItem_Set_Value_From_Value_Converts_To_String)
{
    TestSetValueFromValueConvertsToString(&validValue20, validValue20String, name1, validValue10, RxTxType_Rx_Only, UpdateStoreType_On_Rx, 0, nullptr);
}
TEST_F(DataItemGetAndSetValueTestsInt1, dataItemWithValidation_Set_Value_From_Value_Converts_To_String)
{
    TestSetValueFromValueConvertsToString(&validValue20, validValue20String, name1, validValue10, RxTxType_Rx_Only, UpdateStoreType_On_Rx, 0, &validValues);
}
TEST_F(DataItemGetAndSetValueTestsInt10, dataItemArray_Set_Value_From_Value_Converts_To_String)
{
    TestSetValueFromValueConvertsToString(validValue20Array, validValue20ArrayString, name1, validValue10, RxTxType_Rx_Only, UpdateStoreType_On_Rx, 0, nullptr);
}
TEST_F(DataItemGetAndSetValueTestsInt10, dataItemArrayWithValidation_Set_Value_From_Value_Converts_To_String)
{
    TestSetValueFromValueConvertsToString(validValue20Array, validValue20ArrayString, name1, validValue10, RxTxType_Rx_Only, UpdateStoreType_On_Rx, 0, &validValues);
}


// ************ Set Value From String Converts To Value ******************
TEST_F(DataItemGetAndSetValueTestsInt1, dataItem_Set_Value_From_String_Converts_To_Value)
{
    TestSetValueFromStringConvertsToValue(validValue20String, &validValue20, name1, validValue10, RxTxType_Rx_Only, UpdateStoreType_On_Rx, 0, nullptr);
}
TEST_F(DataItemGetAndSetValueTestsInt1, dataItemWithValidation_Set_Value_From_String_Converts_To_Value)
{
    TestSetValueFromStringConvertsToValue(validValue20String, &validValue20, name1, validValue10, RxTxType_Rx_Only, UpdateStoreType_On_Rx, 0, &validValues);
}
TEST_F(DataItemGetAndSetValueTestsInt10, dataItemArray_Set_Value_From_String_Converts_To_Value)
{
    TestSetValueFromStringConvertsToValue(validValue20ArrayString, validValue20Array, name1, validValue10, RxTxType_Rx_Only, UpdateStoreType_On_Rx, 0, nullptr);
}
TEST_F(DataItemGetAndSetValueTestsInt10, dataItemArrayWithValidation_Set_Value_From_String_Converts_To_Value)
{
    TestSetValueFromStringConvertsToValue(validValue20ArrayString, validValue20Array, name1, validValue10, RxTxType_Rx_Only, UpdateStoreType_On_Rx, 0, &validValues);
}

TEST_F(DataItemGetAndSetValueTestsInt1, dataItem_Set_Valid_Values_When_Validation_Is_Used)
{
    TestSettingValue(validValue10, &validValue20, validValue20String, true);
}
TEST_F(DataItemGetAndSetValueTestsInt1, dataItem_Reject_Invalid_Value_When_Validation_Is_Used)
{
    TestSettingValue(validValue10, &invalidValue, invalidValueString, false);
}
TEST_F(DataItemGetAndSetValueTestsInt10, dataItemArray_Set_Valid_Array_Values_When_Validation_Is_Used)
{
    TestSettingValue(validValue10, validValue20Array, validValue20ArrayString, true);
}
TEST_F(DataItemGetAndSetValueTestsInt10, dataItemArray_Reject_Invalid_Array_Values_When_Validation_Is_Used)
{
    TestSettingValue(validValue10, invalidValueArray, invalidValueArrayString, false);
}

TEST_F(DataItemGetAndSetValueTestsInt1, Change_Count_Changes_Properly_When_Validation_Used)
{
    CreateDataItem(name1, validValue10, RxTxType_Rx_Only, UpdateStoreType_On_Rx, 0, &validValues);
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

TEST_F(DataItemGetAndSetValueTestsInt1, Change_Count_Changes_Properly_When_Validation_Not_Used)
{
    CreateDataItem(name1, validValue10, RxTxType_Rx_Only, UpdateStoreType_On_Rx, 0, nullptr);
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