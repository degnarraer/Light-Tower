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
#include "Test_CommonClasses/Mock_SetupCallInterface.h"
#include "Test_CommonClasses/Mock_ValidValueChecker.h"
#include "Test_CommonClasses/Mock_SerialMessageManager.h"

using ::testing::_;
using ::testing::NotNull;
using namespace testing;

// Test Fixture for DataItemFunctionCallTests
class DataItemFunctionCallTests : public Test
{
protected:
    const int32_t initialValue = 10;
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
        free(mp_MockSerialPortMessageManager);
        free(mp_MockSetupCaller);
        DestroyDataItem();
    }
    void DestroyDataItem()
    {
        if(mp_DataItem)
        {
            free(mp_DataItem);
            mp_DataItem = nullptr;
        }
    }
};

TEST_F(DataItemFunctionCallTests, Registered_With_Setup_Caller)
{
    EXPECT_CALL(*mp_MockSetupCaller, RegisterForSetupCall(NotNull())).Times(1);
    CreateDataItem(RxTxType_Rx_Only, UpdateStoreType_On_Rx, 0);
}

TEST_F(DataItemFunctionCallTests, DeRegistered_With_Setup_Caller_On_Deletion)
{
    CreateDataItem(RxTxType_Rx_Only, UpdateStoreType_On_Rx, 0);
    EXPECT_CALL(*mp_MockSetupCaller, DeRegisterForSetupCall(NotNull())).Times(1);
    DestroyDataItem();
}

TEST_F(DataItemFunctionCallTests, Registers_For_New_Value_Notification_For_Rx_Only)
{
    EXPECT_CALL(*mp_MockSerialPortMessageManager, RegisterForNewValueNotification(NotNull())).Times(1);
    CreateDataItem(RxTxType_Rx_Only, UpdateStoreType_On_Rx, 0);
}
TEST_F(DataItemFunctionCallTests, Registers_For_New_Value_Notification_For_Rx_Echo_Value)
{
    EXPECT_CALL(*mp_MockSerialPortMessageManager, RegisterForNewValueNotification(NotNull())).Times(1);
    CreateDataItem(RxTxType_Rx_Echo_Value, UpdateStoreType_On_Rx, 0);
}

TEST_F(DataItemFunctionCallTests, DeRegisters_For_New_Value_Notification_For_Rx_At_Deletion)
{
    CreateDataItem(RxTxType_Rx_Only, UpdateStoreType_On_Rx, 0);
    EXPECT_CALL(*mp_MockSerialPortMessageManager, DeRegisterForNewValueNotification(NotNull())).Times(1);
    DestroyDataItem();
    CreateDataItem(RxTxType_Rx_Echo_Value, UpdateStoreType_On_Rx, 0);
    EXPECT_CALL(*mp_MockSerialPortMessageManager, DeRegisterForNewValueNotification(NotNull())).Times(1);
    DestroyDataItem();
}

TEST_F(DataItemFunctionCallTests, Does_Not_Register_Or_Deregister_For_New_Value_Notification_For_Tx)
{
    EXPECT_CALL(*mp_MockSerialPortMessageManager, RegisterForNewValueNotification(NotNull())).Times(0);
    EXPECT_CALL(*mp_MockSerialPortMessageManager, DeRegisterForNewValueNotification(NotNull())).Times(0);
    CreateDataItem(RxTxType_Tx_Periodic, UpdateStoreType_On_Rx, 0);
    DestroyDataItem();
    CreateDataItem(RxTxType_Tx_On_Change, UpdateStoreType_On_Rx, 0);
    DestroyDataItem();
    CreateDataItem(RxTxType_Tx_On_Change_With_Heartbeat, UpdateStoreType_On_Rx, 0);
    DestroyDataItem();
}

// Test Fixture for DataItem Rx Tx Tests
class DataItemRxTxTests : public Test
                        , public SetupCallerInterface
{
protected:
    const int32_t initialValue = 10;
    const String name1 = "Test Name1";
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
        free(mp_MockSerialPortMessageManager);
        DestroyDataItem();
    }
    void DestroyDataItem()
    {
        if(mp_DataItem)
        {
            free(mp_DataItem);
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
class DataItemGetAndSetValueTests : public Test
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
    DataItem<int32_t, 1> *mp_DataItem;
    DataItem<int32_t, 10> *mp_DataItemArray;
    DataItem<int32_t, 1> *mp_DataItemWithValidation;
    DataItem<int32_t, 10> *mp_DataItemArrayWithValidation;
    MockHardwareSerial m_MockHardwareSerial;
    MockDataSerializer m_MockDataSerializer;
    MockSerialPortMessageManager *mp_MockSerialPortMessageManager;
    const String name1 = "Test Name1";
    const String name2 = "Test Name2";
    const String name3 = "Test Name3";
    const String name4 = "Test Name4";

    void SetUp() override
    {
        mp_MockSerialPortMessageManager = new MockSerialPortMessageManager( name1
                                                                          , m_MockHardwareSerial
                                                                          , m_MockDataSerializer
                                                                          , 0 );
                                                                          
        mp_DataItem = new DataItem<int32_t, 1>( name1 
                                              , initialValue
                                              , RxTxType_Rx_Only
                                              , UpdateStoreType_On_Rx
                                              , 0
                                              , *mp_MockSerialPortMessageManager
                                              , nullptr
                                              , this );

        mp_DataItemArray = new DataItem<int32_t, 10>( name2 
                                                    , initialValue
                                                    , RxTxType_Rx_Only
                                                    , UpdateStoreType_On_Rx
                                                    , 0
                                                    , *mp_MockSerialPortMessageManager
                                                    , nullptr
                                                    , this );

        mp_DataItemWithValidation = new DataItem<int32_t, 1>( name3 
                                                            , initialValue
                                                            , RxTxType_Rx_Only
                                                            , UpdateStoreType_On_Rx
                                                            , 0
                                                            , *mp_MockSerialPortMessageManager
                                                            , nullptr
                                                            , this
                                                            , &validValues );

        mp_DataItemArrayWithValidation = new DataItem<int32_t, 10>( name4 
                                                                  , initialValue
                                                                  , RxTxType_Rx_Only
                                                                  , UpdateStoreType_On_Rx
                                                                  , 0
                                                                  , *mp_MockSerialPortMessageManager
                                                                  , nullptr
                                                                  , this
                                                                  , &validValues );
        SetupAllSetupCallees();
    }

    void TearDown() override
    {
        free(mp_DataItem);
        free(mp_DataItemArray);
        free(mp_DataItemWithValidation);
        free(mp_DataItemArrayWithValidation);
        free(mp_MockSerialPortMessageManager);
    }
};

TEST_F(DataItemGetAndSetValueTests, dataItem_Name_Is_Set)
{
    EXPECT_STREQ(name1.c_str(), mp_DataItem->GetName().c_str());
}
TEST_F(DataItemGetAndSetValueTests, dataItemArray_Name_Is_Set)
{
    EXPECT_STREQ(name2.c_str(), mp_DataItemArray->GetName().c_str());
}
TEST_F(DataItemGetAndSetValueTests, dataItemWithValidation_Name_Is_Set)
{
    EXPECT_STREQ(name3.c_str(), mp_DataItemWithValidation->GetName().c_str());
}

TEST_F(DataItemGetAndSetValueTests, dataItem_Initial_Value_Is_Set)
{
    EXPECT_EQ(initialValue, mp_DataItem->GetValue());
}
TEST_F(DataItemGetAndSetValueTests, dataItemArray_Initial_Value_Is_Set)
{
    EXPECT_EQ(initialValue, mp_DataItemWithValidation->GetValue());
}
TEST_F(DataItemGetAndSetValueTests, dataItemWithValidation_Initial_Value_Is_Set)
{
    EXPECT_EQ(initialValue, mp_DataItemWithValidation->GetValue());
}

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