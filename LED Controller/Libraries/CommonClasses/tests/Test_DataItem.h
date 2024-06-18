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
#include "tests/Mock_SetupCallInterface.h"
#include "tests/Mock_ValidValueChecker.h"
#include "tests/Mock_SerialMessageManager.h"

using ::testing::_;
using ::testing::NotNull;
using ::testing::InvokeWithoutArgs;
using namespace testing;


// Test Fixture for DataItemFunctionCallTests
class DataItemFunctionCallTests : public Test
{
protected:
    const int32_t initialValue = 10;
    const String spmm = "Serial Port Message Manager";
    const String name1 = "Test Name1";
    MockSetupCallerInterface *mp_MockSetupCaller;
    MockHardwareSerial m_MockHardwareSerial;
    MockDataSerializer m_MockDataSerializer;
    MockSerialPortMessageManager *mp_MockSerialPortMessageManager;
    DataItem<int32_t, 1> *mp_DataItem;
    void SetUp() override
    {
        mp_MockSetupCaller = new MockSetupCallerInterface();
        mp_MockSerialPortMessageManager = new MockSerialPortMessageManager( name1
                                                                          , m_MockHardwareSerial
                                                                          , m_MockDataSerializer
                                                                          , 0 );
        ON_CALL(*mp_MockSetupCaller, RegisterForSetupCall(NotNull())).WillByDefault(InvokeWithoutArgs([]{}));
        ON_CALL(*mp_MockSetupCaller, DeRegisterForSetupCall(NotNull())).WillByDefault(InvokeWithoutArgs([]{}));
        ON_CALL(*mp_MockSerialPortMessageManager, RegisterForNewValueNotification(NotNull())).WillByDefault(InvokeWithoutArgs([]{}));
        ON_CALL(*mp_MockSerialPortMessageManager, DeRegisterForNewValueNotification(NotNull())).WillByDefault(InvokeWithoutArgs([]{}));
        ON_CALL(*mp_MockSerialPortMessageManager, QueueMessageFromData(_,_,_,_)).WillByDefault(Return(true));
        ON_CALL(*mp_MockSerialPortMessageManager, GetName()).WillByDefault(Return(spmm));
    }
    void CreateDataItem(RxTxType_t rxTxType, UpdateStoreType_t updateStoreType, uint16_t rate)
    {
        mp_DataItem = new DataItem<int32_t, 1>( name1 
                                              , initialValue
                                              , rxTxType
                                              , updateStoreType
                                              , rate
                                              , *mp_MockSerialPortMessageManager
                                              , NULL
                                              , mp_MockSetupCaller );
    }
    void TearDown() override
    {
        delete mp_MockSerialPortMessageManager;
        delete mp_MockSetupCaller;
        DestroyDataItem();
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
    }
    void TestSetupCallDeregistration(RxTxType_t rxtxtype, size_t callTimes)
    {    
        EXPECT_CALL(*mp_MockSerialPortMessageManager, DeRegisterForNewValueNotification(NotNull()));
        EXPECT_CALL(*mp_MockSetupCaller, DeRegisterForSetupCall(NotNull())).Times(callTimes);
        DestroyDataItem();
    }
    void TestNewValueNotificationRegistration(RxTxType_t rxtxtype, size_t callTimes)
    {
        EXPECT_CALL(*mp_MockSetupCaller, RegisterForSetupCall(NotNull()));
        EXPECT_CALL(*mp_MockSerialPortMessageManager, RegisterForNewValueNotification(NotNull())).Times(callTimes);
        CreateDataItem(rxtxtype, UpdateStoreType_On_Rx, 1000);
        mp_DataItem->Setup();
    }
    void TestNewValueNotificationDeregistration(RxTxType_t rxtxtype, size_t callTimes)
    {
        EXPECT_CALL(*mp_MockSetupCaller, DeRegisterForSetupCall(NotNull()));
        EXPECT_CALL(*mp_MockSerialPortMessageManager, DeRegisterForNewValueNotification(NotNull())).Times(callTimes);
        DestroyDataItem();
    }

};

TEST_F(DataItemFunctionCallTests, Registration_With_Setup_Caller)
{
    TestSetupCallRegistration(RxTxType_Tx_Periodic, 1);
    TestSetupCallDeregistration(RxTxType_Tx_Periodic, 1);

    TestSetupCallRegistration(RxTxType_Tx_On_Change_With_Heartbeat, 1);
    TestSetupCallDeregistration(RxTxType_Tx_On_Change_With_Heartbeat, 1);

    TestSetupCallRegistration(RxTxType_Tx_On_Change, 1);
    TestSetupCallDeregistration(RxTxType_Tx_On_Change, 1);

    TestSetupCallRegistration(RxTxType_Rx_Only, 1);
    TestSetupCallDeregistration(RxTxType_Rx_Only, 1);

    TestSetupCallRegistration(RxTxType_Rx_Echo_Value, 1);
    TestSetupCallDeregistration(RxTxType_Rx_Echo_Value, 1);
}

TEST_F(DataItemFunctionCallTests, Registration_For_New_Value_Notification)
{
    TestNewValueNotificationRegistration(RxTxType_Tx_Periodic, 1);
    TestNewValueNotificationDeregistration(RxTxType_Tx_Periodic, 1);

    TestNewValueNotificationRegistration(RxTxType_Tx_On_Change_With_Heartbeat, 1);
    TestNewValueNotificationDeregistration(RxTxType_Tx_On_Change_With_Heartbeat, 1);

    TestNewValueNotificationRegistration(RxTxType_Tx_On_Change, 1);
    TestNewValueNotificationDeregistration(RxTxType_Tx_On_Change, 1);

    TestNewValueNotificationRegistration(RxTxType_Rx_Only, 1);
    TestNewValueNotificationDeregistration(RxTxType_Rx_Only, 1);

    TestNewValueNotificationRegistration(RxTxType_Rx_Echo_Value, 1);    
    TestNewValueNotificationDeregistration(RxTxType_Rx_Echo_Value, 1);
}


// Test Fixture for DataItem Rx Tx Tests
class DataItemRxTxTests : public Test
                        , public SetupCallerInterface
{
protected:
    const int32_t initialValue = 10;
    const String name1 = "Test Name1";
    const String spmm = "Serial Port Message Manager";
    MockHardwareSerial m_MockHardwareSerial;
    MockDataSerializer m_MockDataSerializer;
    MockSerialPortMessageManager *mp_MockSerialPortMessageManager;
    DataItem<int32_t, 1> *mp_DataItem;

    void SetUp() override
    {
        mp_MockSerialPortMessageManager = new MockSerialPortMessageManager( name1
                                                                          , m_MockHardwareSerial
                                                                          , m_MockDataSerializer
                                                                          , 0 );
        ON_CALL(*mp_MockSerialPortMessageManager, RegisterForNewValueNotification(NotNull())).WillByDefault(InvokeWithoutArgs([]{}));
        ON_CALL(*mp_MockSerialPortMessageManager, DeRegisterForNewValueNotification(NotNull())).WillByDefault(InvokeWithoutArgs([]{}));
        ON_CALL(*mp_MockSerialPortMessageManager, QueueMessageFromData(_,_,_,_)).WillByDefault(Return(true));
        ON_CALL(*mp_MockSerialPortMessageManager, GetName()).WillByDefault(Return(spmm));
    }
    void CreateDataItem(RxTxType_t rxTxType, UpdateStoreType_t updateStoreType, uint16_t rate)
    {
        mp_DataItem = new DataItem<int32_t, 1>( name1 
                                              , initialValue
                                              , rxTxType
                                              , updateStoreType
                                              , rate
                                              , *mp_MockSerialPortMessageManager
                                              , NULL
                                              , this );
        SetupAllSetupCallees();
    }

    void TearDown() override
    {
        
        if(mp_MockSerialPortMessageManager)
        {
            delete mp_MockSerialPortMessageManager;
        }
        DestroyDataItem();
    }
    void DestroyDataItem()
    {
        if(mp_DataItem)
        {
            delete mp_DataItem;
            mp_DataItem = nullptr;
        }
    }
};

TEST_F(DataItemRxTxTests, Tx_Called_Periodically)
{
    EXPECT_CALL(*mp_MockSerialPortMessageManager, QueueMessageFromData(_,_,_,_)).Times(10)
        .WillRepeatedly(Return(true));
    EXPECT_CALL(*mp_MockSerialPortMessageManager, RegisterForNewValueNotification(NotNull())).Times(1);
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
    const int32_t initialValueArray[10] = {10,10,10,10,10,10,10,10,10,10};
    const int32_t validValue1Array[10] = {20,20,20,20,20,20,20,20,20,20};
    const int32_t validValue2Array[10] = {30,30,30,30,30,30,30,30,30,30};
    const int32_t invalidValueArray[10] = {40,40,40,40,40,40,40,40,40,40};
    const String initialValueArrayString = "10|10|10|10|10|10|10|10|10|10";
    const String validValue1ArrayString = "20|20|20|20|20|20|20|20|20|20";
    const String validValue2ArrayString = "30|30|30|30|30|30|30|30|30|30";
    const String invalidValueArrayString = "40|40|40|40|40|40|40|40|40|40";
    DataItem<T, COUNT> *mp_DataItem;
    MockHardwareSerial m_MockHardwareSerial;
    MockDataSerializer m_MockDataSerializer;
    MockSerialPortMessageManager *mp_MockSerialPortMessageManager;
    const String spmm = "Serial Port Message Manager";
    const String name1 = "Test Name1";
    const String name2 = "Test Name2";
    const String name3 = "Test Name3";
    const String name4 = "Test Name4";

    void SetUp() override
    {
        ESP_LOGD("SetUp", "Test SetUp!");
        mp_MockSerialPortMessageManager = new MockSerialPortMessageManager(spmm, m_MockHardwareSerial, m_MockDataSerializer, 0);
        EXPECT_CALL(*mp_MockSerialPortMessageManager, GetName()).WillRepeatedly(Return(spmm));
    }

    void CreateDataItem( const String name
                       , int32_t initialValue
                       , RxTxType_t rxTxType
                       , UpdateStoreType_t updateStoreType
                       , uint16_t rate
                       , ValidStringValues_t *validStringValues )
    {
        EXPECT_CALL(*mp_MockSerialPortMessageManager, RegisterForNewValueNotification(NotNull())).Times(1);
        mp_DataItem = new DataItem<T, COUNT>( name
                                            , initialValue
                                            , rxTxType
                                            , updateStoreType
                                            , rate
                                            , *mp_MockSerialPortMessageManager
                                            , nullptr
                                            , this
                                            , validStringValues);
        SetupAllSetupCallees();
    }

    void DestroyDataItem()
    {
        if(mp_DataItem)
        {
            delete mp_DataItem;
        }
    }

    void TearDown() override
    {
        ESP_LOGD("TearDown", "Test TearDown!");
        if(mp_DataItem)
        {
            delete mp_DataItem;
            mp_DataItem = nullptr;
        }
        if(mp_MockSerialPortMessageManager)
        {
            delete mp_MockSerialPortMessageManager;
            mp_MockSerialPortMessageManager = nullptr;
        }
    }

    void TestNameIsSet( const String name
                      , int32_t initialValue
                      , RxTxType_t rxTxType
                      , UpdateStoreType_t updateStoreType
                      , uint16_t rate
                      , ValidStringValues_t *validStringValues)
    {
        CreateDataItem(name1, initialValue, rxTxType, updateStoreType, rate, validStringValues);
        EXPECT_STREQ(name1.c_str(), mp_DataItem->GetName().c_str());
    }

    void TestInitialValueIsSet( const String name
                              , int32_t initialValue
                              , RxTxType_t rxTxType
                              , UpdateStoreType_t updateStoreType
                              , uint16_t rate
                              , ValidStringValues_t *validStringValues)
    {
        CreateDataItem(name1, initialValue, rxTxType, updateStoreType, rate, validStringValues);
        for(size_t i = 0; i < mp_DataItem->GetCount(); ++i)
        {
            EXPECT_EQ(initialValue, mp_DataItem->GetValuePointer()[i]);
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


/*
// ************ Initial Value Returned as String ******************
TEST_F(DataItemGetAndSetValueTests, dataItem_Initial_Value_Is_Returned_As_String)
{
    EXPECT_STREQ(initialValueString.c_str(), mp_DataItem->GetInitialValueAsString().c_str());
}
TEST_F(DataItemGetAndSetValueTests, dataItemArray_Initial_Value_Is_Returned_As_String)
{
    EXPECT_STREQ(initialValueString.c_str(), mp_DataItemArray->GetInitialValueAsString().c_str());
}
TEST_F(DataItemGetAndSetValueTests, dataItemWithValidation_Initial_Value_Is_Returned_As_String)
{
    EXPECT_STREQ(initialValueString.c_str(), mp_DataItemWithValidation->GetInitialValueAsString().c_str());
}
TEST_F(DataItemGetAndSetValueTests, dataItemArrayWithValidation_Initial_Value_Is_Returned_As_String)
{
    EXPECT_STREQ(initialValueString.c_str(), mp_DataItemArrayWithValidation->GetInitialValueAsString().c_str());
}


TEST_F(DataItemGetAndSetValueTests, dataItem_Value_Is_Returned_As_String)
{
    EXPECT_STREQ(initialValueString.c_str(), mp_DataItem->GetValueAsString().c_str());
}
TEST_F(DataItemGetAndSetValueTests, dataItemArray_Value_Is_Returned_As_String)
{
    EXPECT_STREQ(initialValueArrayString.c_str(), mp_DataItemArray->GetValueAsString().c_str());
}
TEST_F(DataItemGetAndSetValueTests, dataItemWithValidation_Value_Is_Returned_As_String)
{
    EXPECT_STREQ(initialValueString.c_str(), mp_DataItemWithValidation->GetValueAsString().c_str());
}

TEST_F(DataItemGetAndSetValueTests, dataItem_Set_Value_From_Value_Converts_To_String)
{
    mp_DataItem->SetValue(validValue1);
    EXPECT_STREQ(validValue1String.c_str(), mp_DataItem->GetValueAsString().c_str());
}
TEST_F(DataItemGetAndSetValueTests, dataItemArray_Set_Value_From_Value_Converts_To_String)
{
    mp_DataItemArray->SetValue(validValue1Array, sizeof(validValue1Array)/sizeof(validValue1Array[0]));
    EXPECT_STREQ(validValue1ArrayString.c_str(), mp_DataItemArray->GetValueAsString().c_str());
}
TEST_F(DataItemGetAndSetValueTests, dataItemWithValidation_Set_Value_From_Value_Converts_To_String)
{
    mp_DataItemWithValidation->SetValue(validValue1);
    EXPECT_STREQ(validValue1String.c_str(), mp_DataItemWithValidation->GetValueAsString().c_str());
}

TEST_F(DataItemGetAndSetValueTests, dataItem_Set_Value_From_String_Converts_To_Value)
{
    mp_DataItem->SetValueFromString(validValue1String);
    EXPECT_EQ(validValue1, mp_DataItem->GetValue());
}
TEST_F(DataItemGetAndSetValueTests, dataItemArray_Set_Value_From_String_Converts_To_Value)
{
    mp_DataItemArray->SetValueFromString(validValue1ArrayString);
    for(size_t i = 0; i < mp_DataItemArray->GetCount(); ++i)
    {
        EXPECT_EQ(validValue1Array[i], mp_DataItemArray->GetValuePointer()[i]);
    }
}
TEST_F(DataItemGetAndSetValueTests, dataItemWithValidation_Set_Value_From_String_Converts_To_Value)
{
    mp_DataItemWithValidation->SetValueFromString(validValue1String);
    EXPECT_EQ(validValue1, mp_DataItemWithValidation->GetValue());
}

TEST_F(DataItemGetAndSetValueTests, dataItem_Valid_Values_Accepted_When_Validation_Is_Used)
{
    mp_DataItem->SetValue(validValue1);
    EXPECT_EQ(validValue1, mp_DataItem->GetValue());

    mp_DataItem->SetValueFromString(validValue2String);
    EXPECT_EQ(validValue2, mp_DataItem->GetValue());
}
TEST_F(DataItemGetAndSetValueTests, dataItemWithValidation_Valid_Values_Accepted_When_Validation_Is_Used)
{
    mp_DataItemWithValidation->SetValue(validValue1);
    EXPECT_EQ(validValue1, mp_DataItemWithValidation->GetValue());

    mp_DataItemWithValidation->SetValueFromString(validValue2String);
    EXPECT_EQ(validValue2, mp_DataItemWithValidation->GetValue());
}
TEST_F(DataItemGetAndSetValueTests, dataItemArrayWithValidation_Valid_Values_Accepted_When_Validation_Is_Used)
{
    mp_DataItemArrayWithValidation->SetValue(validValue1Array, sizeof(validValue1Array)/sizeof(validValue1Array[0]));
    for(size_t i = 0; i < mp_DataItemArrayWithValidation->GetCount(); ++i)
    {
        EXPECT_EQ(validValue1Array[i], mp_DataItemArrayWithValidation->GetValuePointer()[i]);
    }

    mp_DataItemArrayWithValidation->SetValue(validValue2Array, sizeof(validValue2Array)/sizeof(validValue2Array[0]));
    for(size_t i = 0; i < mp_DataItemArrayWithValidation->GetCount(); ++i)
    {
        EXPECT_EQ(validValue2Array[i], mp_DataItemArrayWithValidation->GetValuePointer()[i]);
    }
}

TEST_F(DataItemGetAndSetValueTests, dataItem_Invalid_Values_Rejected_When_Validation_Is_Used)
{
    mp_DataItem->SetValue(invalidValue);
    EXPECT_EQ(invalidValue, mp_DataItem->GetValue());
    
    mp_DataItem->SetValue(initialValue);
    EXPECT_EQ(initialValue, mp_DataItem->GetValue());
    
    mp_DataItem->SetValueFromString(invalidValueString);
    EXPECT_EQ(invalidValue, mp_DataItem->GetValue());
}
TEST_F(DataItemGetAndSetValueTests, dataItemWithValidation_Invalid_Values_Rejected_When_Validation_Is_Used)
{
    mp_DataItemWithValidation->SetValue(invalidValue);
    EXPECT_NE(invalidValue, mp_DataItemWithValidation->GetValue());

    mp_DataItemWithValidation->SetValue(initialValue);
    EXPECT_EQ(initialValue, mp_DataItemWithValidation->GetValue());

    mp_DataItemWithValidation->SetValueFromString(invalidValueString);
    EXPECT_NE(invalidValue, mp_DataItemWithValidation->GetValue());
}
TEST_F(DataItemGetAndSetValueTests, dataItemArrayWithValidation_Invalid_Values_Rejected_When_Validation_Is_Used)
{
    mp_DataItemArrayWithValidation->SetValue(invalidValueArray, sizeof(invalidValueArray)/sizeof(invalidValueArray[0]));
    for(size_t i = 0; i < mp_DataItemArrayWithValidation->GetCount(); ++i)
    {
        EXPECT_NE(invalidValueArray[i], mp_DataItemArrayWithValidation->GetValuePointer()[i]);
    }
    
    mp_DataItemArrayWithValidation->SetValue(initialValueArray, sizeof(initialValueArray)/sizeof(initialValueArray[0]));
    for(size_t i = 0; i < mp_DataItemArrayWithValidation->GetCount(); ++i)
    {
        EXPECT_EQ(initialValueArray[i], mp_DataItemArrayWithValidation->GetValuePointer()[i]);
    }
    
    mp_DataItemArrayWithValidation->SetValue(invalidValueArray, sizeof(invalidValueArray)/sizeof(invalidValueArray[0]));
    for(size_t i = 0; i < mp_DataItemArrayWithValidation->GetCount(); ++i)
    {
        EXPECT_NE(invalidValueArray[i], mp_DataItemArrayWithValidation->GetValuePointer()[i]);
    }
}

TEST_F(DataItemGetAndSetValueTests, Previous_Value_Retained_When_Value_Rejected)
{
    mp_DataItemWithValidation->SetValue(invalidValue);
    EXPECT_EQ(initialValue, mp_DataItemWithValidation->GetValue());
}

TEST_F(DataItemGetAndSetValueTests, Change_Count_Changes_Properly)
{
    EXPECT_EQ(0, mp_DataItemWithValidation->GetChangeCount());
    mp_DataItemWithValidation->SetValue(validValue1);
    EXPECT_EQ(1, mp_DataItemWithValidation->GetChangeCount());
    mp_DataItemWithValidation->SetValue(validValue1);
    EXPECT_EQ(1, mp_DataItemWithValidation->GetChangeCount());
    mp_DataItemWithValidation->SetValue(validValue2);
    EXPECT_EQ(2, mp_DataItemWithValidation->GetChangeCount());
    mp_DataItemWithValidation->SetValue(validValue2);
    EXPECT_EQ(2, mp_DataItemWithValidation->GetChangeCount());
    mp_DataItemWithValidation->SetValue(validValue1);
    EXPECT_EQ(3, mp_DataItemWithValidation->GetChangeCount());
    mp_DataItemWithValidation->SetValue(invalidValue);
    EXPECT_EQ(3, mp_DataItemWithValidation->GetChangeCount());
}

TEST_F(DataItemGetAndSetValueTests, Count_Reflects_DataItem_Count)
{
    EXPECT_EQ(1, mp_DataItem->GetCount());
    EXPECT_EQ(10, mp_DataItemArray->GetCount());
    EXPECT_EQ(1, mp_DataItemWithValidation->GetCount());
}

TEST_F(DataItemGetAndSetValueTests, New_Value_Triggers_Tx)
{

}
*/