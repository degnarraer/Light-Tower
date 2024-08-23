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
#include "Mock_Callbacks.h"
#include "Mock_SetupCallInterface.h"
#include "Mock_ValidValueChecker.h"
#include "Mock_SerialPortMessageManager.h"

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
        MockSetupCallerInterface *mp_MockSetupCaller;
        MockSerialPortMessageManager *mp_MockSerialPortMessageManager;
        DataItem<int32_t, 1> *mp_DataItem;
        DataItemFunctionCallTests()
            : mp_MockSetupCaller(nullptr)
            , mp_MockSerialPortMessageManager(nullptr)
            , mp_DataItem(nullptr)
        {}
        void SetUp() override
        {
            mp_MockSetupCaller = new MockSetupCallerInterface();
            mp_MockSerialPortMessageManager = new MockSerialPortMessageManager();
            ON_CALL(*mp_MockSerialPortMessageManager, QueueMessageFromData(_,_,_,_)).WillByDefault(Return(true));
            ON_CALL(*mp_MockSerialPortMessageManager, GetName()).WillByDefault(Return(spmm));
        }
        void CreateDataItem(RxTxType_t rxTxType, UpdateStoreType_t updateStoreType, uint16_t rate)
        {
            EXPECT_CALL(*mp_MockSetupCaller, RegisterForSetupCall(NotNull())).Times(1);
            mp_DataItem = new DataItem<int32_t, 1>( name 
                                                  , initialValue
                                                  , rxTxType
                                                  , updateStoreType
                                                  , rate
                                                  , mp_MockSerialPortMessageManager
                                                  , nullptr
                                                  , mp_MockSetupCaller );
            ::testing::Mock::VerifyAndClearExpectations(&mp_MockSetupCaller);
            EXPECT_CALL(*mp_MockSerialPortMessageManager, RegisterForNewValueNotification(mp_DataItem)).Times(1);
            if( rxTxType == RxTxType_Tx_Periodic || rxTxType == RxTxType_Tx_On_Change || rxTxType == RxTxType_Tx_On_Change_With_Heartbeat )
            {
                EXPECT_CALL(*mp_MockSerialPortMessageManager, QueueMessageFromData(name,_,_,_))
                    .Times(1)
                    .WillOnce(Return(true));
            }
            mp_DataItem->Setup();
            ::testing::Mock::VerifyAndClearExpectations(&mp_MockSerialPortMessageManager);
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
                EXPECT_CALL(*mp_MockSetupCaller, DeRegisterForSetupCall(mp_DataItem)).Times(1);
                EXPECT_CALL(*mp_MockSerialPortMessageManager, DeRegisterForNewValueNotification(mp_DataItem)).Times(1);
                delete mp_DataItem;
                mp_DataItem = nullptr;
                ::testing::Mock::VerifyAndClearExpectations(&mp_MockSetupCaller);
                ::testing::Mock::VerifyAndClearExpectations(&mp_MockSerialPortMessageManager);
            }
        }
        void TestSetupCallRegistration(RxTxType_t rxtxtype)
        {
            CreateDataItem(rxtxtype, UpdateStoreType_On_Tx, 1000);
            DestroyDataItem();
        }
        void TestNewValueNotificationRegistration(RxTxType_t rxtxtype)
        {
            CreateDataItem(rxtxtype, UpdateStoreType_On_Tx, 1000);
            DestroyDataItem();
        }
};

TEST_F(DataItemFunctionCallTests, Registration_With_Setup_Caller)
{
    TestSetupCallRegistration(RxTxType_Tx_Periodic);
    TestSetupCallRegistration(RxTxType_Tx_On_Change_With_Heartbeat);
    TestSetupCallRegistration(RxTxType_Tx_On_Change);
    TestSetupCallRegistration(RxTxType_Rx_Only);
    TestSetupCallRegistration(RxTxType_Rx_Echo_Value);
}

TEST_F(DataItemFunctionCallTests, Registration_For_New_Value_Notification)
{
    TestNewValueNotificationRegistration(RxTxType_Tx_Periodic);
    TestNewValueNotificationRegistration(RxTxType_Tx_On_Change_With_Heartbeat);
    TestNewValueNotificationRegistration(RxTxType_Tx_On_Change);
    TestNewValueNotificationRegistration(RxTxType_Rx_Only);
    TestNewValueNotificationRegistration(RxTxType_Rx_Echo_Value);
}


// Test Fixture for DataItem Rx Tx Tests
class DataItemRxTxTests : public Test
                        , public SetupCallerInterface
{
    protected:
        const int32_t initialValue = 10;
        const String name = "Test Name1";
        const String spmm = "Serial Port Message Manager";
        MockSerialPortMessageManager *mp_MockSerialPortMessageManager;
        DataItem<int32_t, 1> *mp_DataItem;
        DataItemRxTxTests()
            : mp_MockSerialPortMessageManager(nullptr)
            , mp_DataItem(nullptr)
        {}

        void SetUp() override
        {
            mp_MockSerialPortMessageManager = new MockSerialPortMessageManager();
            ON_CALL(*mp_MockSerialPortMessageManager, GetName()).WillByDefault(Return(spmm));
        }

        void CreateDataItem(RxTxType_t rxTxType, UpdateStoreType_t updateStoreType, uint16_t rate)
        {
            mp_DataItem = new DataItem<int32_t, 1>( name 
                                                , initialValue
                                                , rxTxType
                                                , updateStoreType
                                                , rate
                                                , mp_MockSerialPortMessageManager
                                                , nullptr
                                                , this );
            EXPECT_CALL(*mp_MockSerialPortMessageManager, RegisterForNewValueNotification(mp_DataItem)).Times(1);
            if( rxTxType == RxTxType_Tx_Periodic || rxTxType == RxTxType_Tx_On_Change || rxTxType == RxTxType_Tx_On_Change_With_Heartbeat )
            {
                EXPECT_CALL(*mp_MockSerialPortMessageManager, QueueMessageFromData(name,_,_,_))
                    .Times(1)
                    .WillOnce(Return(true));
            }
            SetupAllSetupCallees();
            ::testing::Mock::VerifyAndClearExpectations(&mp_MockSerialPortMessageManager);
        }

        void DestroyDataItem()
        {
            if(mp_DataItem)
            {
                EXPECT_CALL(*mp_MockSerialPortMessageManager, DeRegisterForNewValueNotification(mp_DataItem)).Times(1);
                delete mp_DataItem;
                mp_DataItem = nullptr;
                ::testing::Mock::VerifyAndClearExpectations(&mp_MockSerialPortMessageManager);
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
    CreateDataItem(RxTxType_Tx_Periodic, UpdateStoreType_On_Tx, 100);
    EXPECT_CALL(*mp_MockSerialPortMessageManager, QueueMessageFromData(name,_,_,_)).Times(10)
        .WillRepeatedly(Return(true));
    std::this_thread::sleep_for(std::chrono::milliseconds(1050));
    ::testing::Mock::VerifyAndClearExpectations(&mp_MockSerialPortMessageManager);
}

// Test Fixture for DataItemGetAndSetValueTests
template <typename T, size_t COUNT>
class DataItemGetAndSetValueTests : public Test
                                  , public SetupCallerInterface
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
    MockSerialPortMessageManager *mp_MockSerialPortMessageManager;
    DataItem<T, COUNT> *mp_DataItem;
    MockNamedCallback *mp_mockNamedCallback;
    const String spmm = "Serial Port Message Manager";
    const String name1 = "Test Name1";
    const String name2 = "Test Name2";
    const String name3 = "Test Name3";
    const String name4 = "Test Name4";

    void SetUp() override
    {
        mp_MockSerialPortMessageManager = new MockSerialPortMessageManager();
        EXPECT_CALL(*mp_MockSerialPortMessageManager, GetName()).WillRepeatedly(Return(spmm));
    }

    void CreateDataItem( const String name, int32_t initialValue, RxTxType_t rxTxType, UpdateStoreType_t updateStoreType, uint16_t rate, ValidStringValues_t *validStringValues )
    {
        mp_mockNamedCallback = new MockNamedCallback(name, nullptr);
        mp_DataItem = new DataItem<T, COUNT>( name, initialValue, rxTxType, updateStoreType, rate, mp_MockSerialPortMessageManager, mp_mockNamedCallback, this, validStringValues);
        EXPECT_CALL(mockNamedCallback_Callback, NewValueCallbackFunction(name,_,_)).Times(1);
        EXPECT_CALL(*mp_MockSerialPortMessageManager, RegisterForNewValueNotification(mp_DataItem)).Times(1);
        if( rxTxType == RxTxType_Tx_Periodic || rxTxType == RxTxType_Tx_On_Change || rxTxType == RxTxType_Tx_On_Change_With_Heartbeat )
        {
            EXPECT_CALL(*mp_MockSerialPortMessageManager, QueueMessageFromData(name,_,_,_))
                .Times(1)
                .WillOnce(Return(true));
        }
        SetupAllSetupCallees();
        ::testing::Mock::VerifyAndClearExpectations(&mockNamedCallback_Callback);
        ::testing::Mock::VerifyAndClearExpectations(&mp_MockSerialPortMessageManager);
    }

    void DestroyDataItem()
    {
        if(mp_DataItem)
        {
            EXPECT_CALL(*mp_MockSerialPortMessageManager, DeRegisterForNewValueNotification(mp_DataItem)).Times(1);
            delete mp_DataItem;
            mp_DataItem = nullptr;
            ::testing::Mock::VerifyAndClearExpectations(&mp_MockSerialPortMessageManager);
        }
    }

    void TearDown() override
    {
        DestroyDataItem();
        if(mp_MockSerialPortMessageManager)
        {
            delete mp_MockSerialPortMessageManager;
            mp_MockSerialPortMessageManager = nullptr;
        }
    }

    void TestNameIsSet( const String name, int32_t initialValue, RxTxType_t rxTxType, UpdateStoreType_t updateStoreType, uint16_t rate, ValidStringValues_t *validStringValues )
    {
        CreateDataItem(name, initialValue, rxTxType, updateStoreType, rate, validStringValues);
        EXPECT_STREQ(name.c_str(), mp_DataItem->GetName().c_str());
        DestroyDataItem();
    }

    void TestInitialValueIsSet( const String name, int32_t initialValue, RxTxType_t rxTxType, UpdateStoreType_t updateStoreType, uint16_t rate, ValidStringValues_t *validStringValues )
    {
        CreateDataItem(name, initialValue, rxTxType, updateStoreType, rate, validStringValues);
        for(size_t i = 0; i < mp_DataItem->GetCount(); ++i)
        {
            EXPECT_EQ(initialValue, mp_DataItem->GetValuePointer()[i]);
        }
        DestroyDataItem();
    }

    void TestInitialValueReturnedAsString( const String name, int32_t initialValue, const String resultString, RxTxType_t rxTxType, UpdateStoreType_t updateStoreType, uint16_t rate, ValidStringValues_t *validStringValues )
    {
        CreateDataItem(name, initialValue, rxTxType, updateStoreType, rate, validStringValues);
        EXPECT_STREQ(resultString.c_str(), mp_DataItem->GetInitialValueAsString().c_str());
        DestroyDataItem();
    }

    void SetRxTxCallExpectations( const String name, RxTxType_t rxTxType, UpdateStoreType_t updateStoreType, bool expectValueAccepted )
    {
        bool loggedType = false;
        switch(rxTxType)
        {
            case RxTxType_Tx_Periodic:
            {
                ESP_LOGD( "SetRxTxCallExpectations", "RxTxType_Tx_Periodic");
                if(expectValueAccepted)
                {
                    EXPECT_CALL(*mp_MockSerialPortMessageManager, QueueMessageFromData(_,_,_,_)).Times(2).WillRepeatedly(Return(true));
                    ESP_LOGD( "SetRxTxCallExpectations", "EXPECT_CALL: QueueMessageFromData x 1");
                }
                else
                {
                    EXPECT_CALL(*mp_MockSerialPortMessageManager, QueueMessageFromData(_,_,_,_)).Times(1);
                    ESP_LOGD( "SetRxTxCallExpectations", "EXPECT_CALL: QueueMessageFromData x 0");
                }
                if(	UpdateStoreType_On_Tx == updateStoreType && expectValueAccepted)
                {
                    EXPECT_CALL(mockNamedCallback_Callback, NewValueCallbackFunction(name,_,_)).Times(1);
                    ESP_LOGD( "SetRxTxCallExpectations", "EXPECT_CALL: NewValueCallbackFunction x 1");
                }
                else
                {
                    EXPECT_CALL(mockNamedCallback_Callback, NewValueCallbackFunction(name,_,_)).Times(0);
                    ESP_LOGD( "SetRxTxCallExpectations", "EXPECT_CALL: NewValueCallbackFunction x 0");
                }
                break;
            }
	        case RxTxType_Tx_On_Change:
            {
                ESP_LOGD( "SetRxTxCallExpectations", "RxTxType_Tx_On_Change");
                if(expectValueAccepted)
                {
                    EXPECT_CALL(*mp_MockSerialPortMessageManager, QueueMessageFromData(_,_,_,_)).Times(1).WillRepeatedly(Return(true));
                    ESP_LOGD( "SetRxTxCallExpectations", "EXPECT_CALL: QueueMessageFromData x 1");
                }
                else
                {
                    EXPECT_CALL(*mp_MockSerialPortMessageManager, QueueMessageFromData(_,_,_,_)).Times(0);
                    ESP_LOGD( "SetRxTxCallExpectations", "EXPECT_CALL: QueueMessageFromData x 0");
                }
                if(	UpdateStoreType_On_Tx == updateStoreType && expectValueAccepted)
                {
                    EXPECT_CALL(mockNamedCallback_Callback, NewValueCallbackFunction(name,_,_)).Times(1);
                    ESP_LOGD( "SetRxTxCallExpectations", "EXPECT_CALL: NewValueCallbackFunction x 1");
                }
                else
                {
                    EXPECT_CALL(mockNamedCallback_Callback, NewValueCallbackFunction(name,_,_)).Times(0);
                    ESP_LOGD( "SetRxTxCallExpectations", "EXPECT_CALL: NewValueCallbackFunction x 0");
                }
                break;
            }
	        case RxTxType_Tx_On_Change_With_Heartbeat:
            {
                ESP_LOGD( "SetRxTxCallExpectations", "RxTxType_Tx_On_Change_With_Heartbeat");
                if(expectValueAccepted)
                {
                    EXPECT_CALL(*mp_MockSerialPortMessageManager, QueueMessageFromData(_,_,_,_)).Times(2).WillRepeatedly(Return(true));
                    ESP_LOGD( "SetRxTxCallExpectations", "EXPECT_CALL: QueueMessageFromData x 1");
                }
                else
                {
                    EXPECT_CALL(*mp_MockSerialPortMessageManager, QueueMessageFromData(_,_,_,_)).Times(1);
                    ESP_LOGD( "SetRxTxCallExpectations", "EXPECT_CALL: QueueMessageFromData x 0");
                }
                if(	UpdateStoreType_On_Tx == updateStoreType && expectValueAccepted)
                {
                    EXPECT_CALL(mockNamedCallback_Callback, NewValueCallbackFunction(name,_,_)).Times(1);
                    ESP_LOGD( "SetRxTxCallExpectations", "EXPECT_CALL: NewValueCallbackFunction x 1");
                }
                else
                {
                    EXPECT_CALL(mockNamedCallback_Callback, NewValueCallbackFunction(name,_,_)).Times(0);
                    ESP_LOGD( "SetRxTxCallExpectations", "EXPECT_CALL: NewValueCallbackFunction x 0");
                }
                break;
            }
            case RxTxType_Rx_Only:
            {
                ESP_LOGD( "SetRxTxCallExpectations", "RxTxType_Rx_Only");
                EXPECT_CALL(*mp_MockSerialPortMessageManager, QueueMessageFromData(_,_,_,_)).Times(0);
                ESP_LOGD( "SetRxTxCallExpectations", "EXPECT_CALL: QueueMessageFromData x 0");
                if(	UpdateStoreType_On_Rx == updateStoreType && expectValueAccepted)
                {
                    EXPECT_CALL(mockNamedCallback_Callback, NewValueCallbackFunction(name,_,_)).Times(1);
                    ESP_LOGD( "SetRxTxCallExpectations", "EXPECT_CALL: NewValueCallbackFunction x 1");
                }
                else
                {
                    EXPECT_CALL(mockNamedCallback_Callback, NewValueCallbackFunction(name,_,_)).Times(0);
                    ESP_LOGD( "SetRxTxCallExpectations", "EXPECT_CALL: NewValueCallbackFunction x 0");
                }
                break;
            }
	        case RxTxType_Rx_Echo_Value:
            {
                ESP_LOGD( "SetRxTxCallExpectations", "RxTxType_Rx_Echo_Value");
                EXPECT_CALL(*mp_MockSerialPortMessageManager, QueueMessageFromData(_,_,_,_)).Times(0);
                ESP_LOGD( "SetRxTxCallExpectations", "EXPECT_CALL: QueueMessageFromData x 0");
                if(	UpdateStoreType_On_Rx == updateStoreType && expectValueAccepted)
                {
                    EXPECT_CALL(mockNamedCallback_Callback, NewValueCallbackFunction(name,_,_)).Times(1);
                    ESP_LOGD( "SetRxTxCallExpectations", "EXPECT_CALL: NewValueCallbackFunction x 1");
                }
                else
                {
                    EXPECT_CALL(mockNamedCallback_Callback, NewValueCallbackFunction(name,_,_)).Times(0);
                    ESP_LOGD( "SetRxTxCallExpectations", "EXPECT_CALL: NewValueCallbackFunction x 0");
                }
                break;
            }
            default:
                break;
        }
    }

    void TestSetValueFromValueConvertsToString( const String name, const int32_t* testValue, const String resultString, int32_t initialValue, RxTxType_t rxTxType, UpdateStoreType_t updateStoreType, uint16_t rate, ValidStringValues_t *validStringValues )
    {
        CreateDataItem(name, initialValue, rxTxType, updateStoreType, rate, validStringValues);
        SetRxTxCallExpectations(name, rxTxType, updateStoreType, true);
        mp_DataItem->SetValue(testValue, COUNT);
        ::testing::Mock::VerifyAndClearExpectations(&mockNamedCallback_Callback);
        ::testing::Mock::VerifyAndClearExpectations(&mp_MockSerialPortMessageManager);
        EXPECT_STREQ(resultString.c_str(), mp_DataItem->GetValueAsString().c_str());
        DestroyDataItem();
    }
    void TestSetValueFromStringConvertsToValue( const String name, const String testString, const int32_t* resultValue, int32_t initialValue, RxTxType_t rxTxType, UpdateStoreType_t updateStoreType, uint16_t rate, ValidStringValues_t *validStringValues )
    {
        CreateDataItem(name, initialValue, rxTxType, updateStoreType, rate, validStringValues);
        SetRxTxCallExpectations(name, rxTxType, updateStoreType, true);
        mp_DataItem->SetValueFromString(testString);
        ::testing::Mock::VerifyAndClearExpectations(&mockNamedCallback_Callback);
        ::testing::Mock::VerifyAndClearExpectations(&mp_MockSerialPortMessageManager);
        for(size_t i = 0; i < COUNT; ++i)
        {
            EXPECT_EQ(resultValue[i], mp_DataItem->GetValuePointer()[i]);
        }
        DestroyDataItem();
    }

    void TestSettingValue( const String name, const int32_t initialValue, const int32_t* testValue, const String testValueString, bool expectValueAccepted)
    {
        CreateDataItem(name, initialValue, RxTxType_Tx_On_Change, UpdateStoreType_On_Tx, 0, &validValues);
        SetRxTxCallExpectations(name, RxTxType_Tx_On_Change, UpdateStoreType_On_Tx, expectValueAccepted);
        mp_DataItem->SetValue(testValue, COUNT);
        ::testing::Mock::VerifyAndClearExpectations(&mockNamedCallback_Callback);
        ::testing::Mock::VerifyAndClearExpectations(&mp_MockSerialPortMessageManager);
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
        DestroyDataItem();
    }

    void TestSettingStringValue( const String name, const int32_t initialValue, const int32_t* testValue, const String testValueString, bool expectValueAccepted)
    {
        CreateDataItem(name, initialValue, RxTxType_Tx_On_Change, UpdateStoreType_On_Tx, 0, &validValues);
        SetRxTxCallExpectations(name, RxTxType_Tx_On_Change, UpdateStoreType_On_Tx, expectValueAccepted);
        mp_DataItem->SetValueFromString(testValueString);
        ::testing::Mock::VerifyAndClearExpectations(&mockNamedCallback_Callback);
        ::testing::Mock::VerifyAndClearExpectations(&mp_MockSerialPortMessageManager);
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
        DestroyDataItem();
    }
};

using DataItemGetAndSetValueTestsInt1 = DataItemGetAndSetValueTests<int32_t, 1>;
using DataItemGetAndSetValueTestsInt10 = DataItemGetAndSetValueTests<int32_t, 10>;

// ************ Name is set ******************
TEST_F(DataItemGetAndSetValueTestsInt1, dataItem_Name_Is_Set)
{
    TestNameIsSet(name1, validValue10, RxTxType_Tx_Periodic, UpdateStoreType_On_Rx, 1000, nullptr);
    TestNameIsSet(name1, validValue10, RxTxType_Tx_On_Change, UpdateStoreType_On_Rx, 0, nullptr);
    TestNameIsSet(name1, validValue10, RxTxType_Tx_On_Change_With_Heartbeat, UpdateStoreType_On_Rx, 1000, nullptr);
    TestNameIsSet(name1, validValue10, RxTxType_Rx_Only, UpdateStoreType_On_Rx, 0, nullptr);
    TestNameIsSet(name1, validValue10, RxTxType_Rx_Echo_Value, UpdateStoreType_On_Rx, 0, nullptr);
    
    TestNameIsSet(name1, validValue10, RxTxType_Tx_Periodic, UpdateStoreType_On_Tx, 1000, nullptr);
    TestNameIsSet(name1, validValue10, RxTxType_Tx_On_Change, UpdateStoreType_On_Tx, 0, nullptr);
    TestNameIsSet(name1, validValue10, RxTxType_Tx_On_Change_With_Heartbeat, UpdateStoreType_On_Tx, 1000, nullptr);
    TestNameIsSet(name1, validValue10, RxTxType_Rx_Only, UpdateStoreType_On_Tx, 0, nullptr);
    TestNameIsSet(name1, validValue10, RxTxType_Rx_Echo_Value, UpdateStoreType_On_Tx, 0, nullptr);
}
TEST_F(DataItemGetAndSetValueTestsInt10, dataItemArray_Name_Is_Set)
{
    TestNameIsSet(name1, validValue10, RxTxType_Tx_Periodic, UpdateStoreType_On_Rx, 1000, nullptr);
    TestNameIsSet(name1, validValue10, RxTxType_Tx_On_Change, UpdateStoreType_On_Rx, 0, nullptr);
    TestNameIsSet(name1, validValue10, RxTxType_Tx_On_Change_With_Heartbeat, UpdateStoreType_On_Rx, 1000, nullptr);
    TestNameIsSet(name1, validValue10, RxTxType_Rx_Only, UpdateStoreType_On_Rx, 0, nullptr);
    TestNameIsSet(name1, validValue10, RxTxType_Rx_Echo_Value, UpdateStoreType_On_Rx, 0, nullptr);
    
    TestNameIsSet(name1, validValue10, RxTxType_Tx_Periodic, UpdateStoreType_On_Tx, 1000, nullptr);
    TestNameIsSet(name1, validValue10, RxTxType_Tx_On_Change, UpdateStoreType_On_Tx, 0, nullptr);
    TestNameIsSet(name1, validValue10, RxTxType_Tx_On_Change_With_Heartbeat, UpdateStoreType_On_Tx, 1000, nullptr);
    TestNameIsSet(name1, validValue10, RxTxType_Rx_Only, UpdateStoreType_On_Tx, 0, nullptr);
    TestNameIsSet(name1, validValue10, RxTxType_Rx_Echo_Value, UpdateStoreType_On_Tx, 0, nullptr);
}
TEST_F(DataItemGetAndSetValueTestsInt1, dataItemWithValidation_Name_Is_Set)
{
    TestNameIsSet(name1, validValue10, RxTxType_Tx_Periodic, UpdateStoreType_On_Rx, 1000, nullptr);
    TestNameIsSet(name1, validValue10, RxTxType_Tx_On_Change, UpdateStoreType_On_Rx, 0, nullptr);
    TestNameIsSet(name1, validValue10, RxTxType_Tx_On_Change_With_Heartbeat, UpdateStoreType_On_Rx, 1000, nullptr);
    TestNameIsSet(name1, validValue10, RxTxType_Rx_Only, UpdateStoreType_On_Rx, 0, nullptr);
    TestNameIsSet(name1, validValue10, RxTxType_Rx_Echo_Value, UpdateStoreType_On_Rx, 0, nullptr);
    
    TestNameIsSet(name1, validValue10, RxTxType_Tx_Periodic, UpdateStoreType_On_Tx, 1000, nullptr);
    TestNameIsSet(name1, validValue10, RxTxType_Tx_On_Change, UpdateStoreType_On_Tx, 0, nullptr);
    TestNameIsSet(name1, validValue10, RxTxType_Tx_On_Change_With_Heartbeat, UpdateStoreType_On_Tx, 1000, nullptr);
    TestNameIsSet(name1, validValue10, RxTxType_Rx_Only, UpdateStoreType_On_Tx, 0, nullptr);
    TestNameIsSet(name1, validValue10, RxTxType_Rx_Echo_Value, UpdateStoreType_On_Tx, 0, nullptr);
}

TEST_F(DataItemGetAndSetValueTestsInt10, dataItemArrayWithValidation_Name_Is_Set)
{
    TestNameIsSet(name1, validValue10, RxTxType_Tx_Periodic, UpdateStoreType_On_Rx, 1000, nullptr);
    TestNameIsSet(name1, validValue10, RxTxType_Tx_On_Change, UpdateStoreType_On_Rx, 0, nullptr);
    TestNameIsSet(name1, validValue10, RxTxType_Tx_On_Change_With_Heartbeat, UpdateStoreType_On_Rx, 1000, nullptr);
    TestNameIsSet(name1, validValue10, RxTxType_Rx_Only, UpdateStoreType_On_Rx, 0, nullptr);
    TestNameIsSet(name1, validValue10, RxTxType_Rx_Echo_Value, UpdateStoreType_On_Rx, 0, nullptr);
    
    TestNameIsSet(name1, validValue10, RxTxType_Tx_Periodic, UpdateStoreType_On_Tx, 1000, nullptr);
    TestNameIsSet(name1, validValue10, RxTxType_Tx_On_Change, UpdateStoreType_On_Tx, 0, nullptr);
    TestNameIsSet(name1, validValue10, RxTxType_Tx_On_Change_With_Heartbeat, UpdateStoreType_On_Tx, 1000, nullptr);
    TestNameIsSet(name1, validValue10, RxTxType_Rx_Only, UpdateStoreType_On_Tx, 0, nullptr);
    TestNameIsSet(name1, validValue10, RxTxType_Rx_Echo_Value, UpdateStoreType_On_Tx, 0, nullptr);
}


// ************ Initial Value is set ******************
TEST_F(DataItemGetAndSetValueTestsInt1, dataItem_Initial_Value_Is_Set)
{
    TestInitialValueIsSet(name1, validValue10, RxTxType_Tx_Periodic, UpdateStoreType_On_Rx, 1000, nullptr);
    TestInitialValueIsSet(name1, validValue10, RxTxType_Tx_On_Change, UpdateStoreType_On_Rx, 0, nullptr);
    TestInitialValueIsSet(name1, validValue10, RxTxType_Tx_On_Change_With_Heartbeat, UpdateStoreType_On_Rx, 1000, nullptr);
    TestInitialValueIsSet(name1, validValue10, RxTxType_Rx_Only, UpdateStoreType_On_Rx, 0, nullptr);
    TestInitialValueIsSet(name1, validValue10, RxTxType_Rx_Echo_Value, UpdateStoreType_On_Rx, 0, nullptr);
    
    TestInitialValueIsSet(name1, validValue10, RxTxType_Tx_Periodic, UpdateStoreType_On_Tx, 1000, nullptr);
    TestInitialValueIsSet(name1, validValue10, RxTxType_Tx_On_Change, UpdateStoreType_On_Tx, 0, nullptr);
    TestInitialValueIsSet(name1, validValue10, RxTxType_Tx_On_Change_With_Heartbeat, UpdateStoreType_On_Tx, 1000, nullptr);
    TestInitialValueIsSet(name1, validValue10, RxTxType_Rx_Only, UpdateStoreType_On_Tx, 0, nullptr);
    TestInitialValueIsSet(name1, validValue10, RxTxType_Rx_Echo_Value, UpdateStoreType_On_Tx, 0, nullptr);
}
TEST_F(DataItemGetAndSetValueTestsInt10, dataItemArray_Initial_Value_Is_Set)
{
    TestInitialValueIsSet(name1, validValue10, RxTxType_Tx_Periodic, UpdateStoreType_On_Rx, 1000, nullptr);
    TestInitialValueIsSet(name1, validValue10, RxTxType_Tx_On_Change, UpdateStoreType_On_Rx, 0, nullptr);
    TestInitialValueIsSet(name1, validValue10, RxTxType_Tx_On_Change_With_Heartbeat, UpdateStoreType_On_Rx, 1000, nullptr);
    TestInitialValueIsSet(name1, validValue10, RxTxType_Rx_Only, UpdateStoreType_On_Rx, 0, nullptr);
    TestInitialValueIsSet(name1, validValue10, RxTxType_Rx_Echo_Value, UpdateStoreType_On_Rx, 0, nullptr);
    
    TestInitialValueIsSet(name1, validValue10, RxTxType_Tx_Periodic, UpdateStoreType_On_Tx, 1000, nullptr);
    TestInitialValueIsSet(name1, validValue10, RxTxType_Tx_On_Change, UpdateStoreType_On_Tx, 0, nullptr);
    TestInitialValueIsSet(name1, validValue10, RxTxType_Tx_On_Change_With_Heartbeat, UpdateStoreType_On_Tx, 1000, nullptr);
    TestInitialValueIsSet(name1, validValue10, RxTxType_Rx_Only, UpdateStoreType_On_Tx, 0, nullptr);
    TestInitialValueIsSet(name1, validValue10, RxTxType_Rx_Echo_Value, UpdateStoreType_On_Tx, 0, nullptr);
}
TEST_F(DataItemGetAndSetValueTestsInt1, dataItemWithValidation_Initial_Value_Is_Set)
{
    TestInitialValueIsSet(name1, validValue10, RxTxType_Tx_Periodic, UpdateStoreType_On_Rx, 1000, nullptr);
    TestInitialValueIsSet(name1, validValue10, RxTxType_Tx_On_Change, UpdateStoreType_On_Rx, 0, nullptr);
    TestInitialValueIsSet(name1, validValue10, RxTxType_Tx_On_Change_With_Heartbeat, UpdateStoreType_On_Rx, 1000, nullptr);
    TestInitialValueIsSet(name1, validValue10, RxTxType_Rx_Only, UpdateStoreType_On_Rx, 0, nullptr);
    TestInitialValueIsSet(name1, validValue10, RxTxType_Rx_Echo_Value, UpdateStoreType_On_Rx, 0, nullptr);
    
    TestInitialValueIsSet(name1, validValue10, RxTxType_Tx_Periodic, UpdateStoreType_On_Tx, 1000, nullptr);
    TestInitialValueIsSet(name1, validValue10, RxTxType_Tx_On_Change, UpdateStoreType_On_Tx, 0, nullptr);
    TestInitialValueIsSet(name1, validValue10, RxTxType_Tx_On_Change_With_Heartbeat, UpdateStoreType_On_Tx, 1000, nullptr);
    TestInitialValueIsSet(name1, validValue10, RxTxType_Rx_Only, UpdateStoreType_On_Tx, 0, nullptr);
    TestInitialValueIsSet(name1, validValue10, RxTxType_Rx_Echo_Value, UpdateStoreType_On_Tx, 0, nullptr);
}
TEST_F(DataItemGetAndSetValueTestsInt10, dataItemArrayWithValidation_Initial_Value_Is_Set)
{
    TestInitialValueIsSet(name1, validValue10, RxTxType_Tx_Periodic, UpdateStoreType_On_Rx, 1000, nullptr);
    TestInitialValueIsSet(name1, validValue10, RxTxType_Tx_On_Change, UpdateStoreType_On_Rx, 0, nullptr);
    TestInitialValueIsSet(name1, validValue10, RxTxType_Tx_On_Change_With_Heartbeat, UpdateStoreType_On_Rx, 1000, nullptr);
    TestInitialValueIsSet(name1, validValue10, RxTxType_Rx_Only, UpdateStoreType_On_Rx, 0, nullptr);
    TestInitialValueIsSet(name1, validValue10, RxTxType_Rx_Echo_Value, UpdateStoreType_On_Rx, 0, nullptr);
    
    TestInitialValueIsSet(name1, validValue10, RxTxType_Tx_Periodic, UpdateStoreType_On_Tx, 1000, nullptr);
    TestInitialValueIsSet(name1, validValue10, RxTxType_Tx_On_Change, UpdateStoreType_On_Tx, 0, nullptr);
    TestInitialValueIsSet(name1, validValue10, RxTxType_Tx_On_Change_With_Heartbeat, UpdateStoreType_On_Tx, 1000, nullptr);
    TestInitialValueIsSet(name1, validValue10, RxTxType_Rx_Only, UpdateStoreType_On_Tx, 0, nullptr);
    TestInitialValueIsSet(name1, validValue10, RxTxType_Rx_Echo_Value, UpdateStoreType_On_Tx, 0, nullptr);
}

// ************ Initial Value Returned as String ******************
TEST_F(DataItemGetAndSetValueTestsInt1, dataItem_Initial_Value_Is_Returned_As_String)
{
    TestInitialValueReturnedAsString(name1, validValue10, validValue10String, RxTxType_Tx_Periodic, UpdateStoreType_On_Rx, 1000, nullptr);
    TestInitialValueReturnedAsString(name1, validValue10, validValue10String, RxTxType_Tx_On_Change, UpdateStoreType_On_Rx, 0, nullptr);
    TestInitialValueReturnedAsString(name1, validValue10, validValue10String, RxTxType_Tx_On_Change_With_Heartbeat, UpdateStoreType_On_Rx, 1000, nullptr);
    TestInitialValueReturnedAsString(name1, validValue10, validValue10String, RxTxType_Rx_Only, UpdateStoreType_On_Rx, 0, nullptr);
    TestInitialValueReturnedAsString(name1, validValue10, validValue10String, RxTxType_Rx_Echo_Value, UpdateStoreType_On_Rx, 0, nullptr);
    
    TestInitialValueReturnedAsString(name1, validValue10, validValue10String, RxTxType_Tx_Periodic, UpdateStoreType_On_Tx, 1000, nullptr);
    TestInitialValueReturnedAsString(name1, validValue10, validValue10String, RxTxType_Tx_On_Change, UpdateStoreType_On_Tx, 0, nullptr);
    TestInitialValueReturnedAsString(name1, validValue10, validValue10String, RxTxType_Tx_On_Change_With_Heartbeat, UpdateStoreType_On_Tx, 1000, nullptr);
    TestInitialValueReturnedAsString(name1, validValue10, validValue10String, RxTxType_Rx_Only, UpdateStoreType_On_Tx, 0, nullptr);
    TestInitialValueReturnedAsString(name1, validValue10, validValue10String, RxTxType_Rx_Echo_Value, UpdateStoreType_On_Tx, 0, nullptr);
}
TEST_F(DataItemGetAndSetValueTestsInt10, dataItemArray_Initial_Value_Is_Returned_As_String)
{
    TestInitialValueReturnedAsString(name1, validValue10, validValue10ArrayString, RxTxType_Tx_Periodic, UpdateStoreType_On_Rx, 1000, nullptr);
    TestInitialValueReturnedAsString(name1, validValue10, validValue10ArrayString, RxTxType_Tx_On_Change, UpdateStoreType_On_Rx, 0, nullptr);
    TestInitialValueReturnedAsString(name1, validValue10, validValue10ArrayString, RxTxType_Tx_On_Change_With_Heartbeat, UpdateStoreType_On_Rx, 1000, nullptr);
    TestInitialValueReturnedAsString(name1, validValue10, validValue10ArrayString, RxTxType_Rx_Only, UpdateStoreType_On_Rx, 0, nullptr);
    TestInitialValueReturnedAsString(name1, validValue10, validValue10ArrayString, RxTxType_Rx_Echo_Value, UpdateStoreType_On_Rx, 0, nullptr);
    
    TestInitialValueReturnedAsString(name1, validValue10, validValue10ArrayString, RxTxType_Tx_Periodic, UpdateStoreType_On_Tx, 1000, nullptr);
    TestInitialValueReturnedAsString(name1, validValue10, validValue10ArrayString, RxTxType_Tx_On_Change, UpdateStoreType_On_Tx, 0, nullptr);
    TestInitialValueReturnedAsString(name1, validValue10, validValue10ArrayString, RxTxType_Tx_On_Change_With_Heartbeat, UpdateStoreType_On_Tx, 1000, nullptr);
    TestInitialValueReturnedAsString(name1, validValue10, validValue10ArrayString, RxTxType_Rx_Only, UpdateStoreType_On_Tx, 0, nullptr);
    TestInitialValueReturnedAsString(name1, validValue10, validValue10ArrayString, RxTxType_Rx_Echo_Value, UpdateStoreType_On_Tx, 0, nullptr);
}
TEST_F(DataItemGetAndSetValueTestsInt1, dataItemWithValidation_Initial_Value_Is_Returned_As_String)
{
    TestInitialValueReturnedAsString(name1, validValue10, validValue10String, RxTxType_Tx_Periodic, UpdateStoreType_On_Rx, 1000, &validValues);
    TestInitialValueReturnedAsString(name1, validValue10, validValue10String, RxTxType_Tx_On_Change, UpdateStoreType_On_Rx, 0, &validValues);
    TestInitialValueReturnedAsString(name1, validValue10, validValue10String, RxTxType_Tx_On_Change_With_Heartbeat, UpdateStoreType_On_Rx, 1000, &validValues);
    TestInitialValueReturnedAsString(name1, validValue10, validValue10String, RxTxType_Rx_Only, UpdateStoreType_On_Rx, 0, &validValues);
    TestInitialValueReturnedAsString(name1, validValue10, validValue10String, RxTxType_Rx_Echo_Value, UpdateStoreType_On_Rx, 0, &validValues);
    
    TestInitialValueReturnedAsString(name1, validValue10, validValue10String, RxTxType_Tx_Periodic, UpdateStoreType_On_Tx, 1000, &validValues);
    TestInitialValueReturnedAsString(name1, validValue10, validValue10String, RxTxType_Tx_On_Change, UpdateStoreType_On_Tx, 0, &validValues);
    TestInitialValueReturnedAsString(name1, validValue10, validValue10String, RxTxType_Tx_On_Change_With_Heartbeat, UpdateStoreType_On_Tx, 1000, &validValues);
    TestInitialValueReturnedAsString(name1, validValue10, validValue10String, RxTxType_Rx_Only, UpdateStoreType_On_Tx, 0, &validValues);
    TestInitialValueReturnedAsString(name1, validValue10, validValue10String, RxTxType_Rx_Echo_Value, UpdateStoreType_On_Tx, 0, &validValues);
}
TEST_F(DataItemGetAndSetValueTestsInt10, dataItemArrayWithValidation_Initial_Value_Is_Returned_As_String)
{
    TestInitialValueReturnedAsString(name1, validValue10, validValue10ArrayString, RxTxType_Tx_Periodic, UpdateStoreType_On_Rx, 1000, &validValues);
    TestInitialValueReturnedAsString(name1, validValue10, validValue10ArrayString, RxTxType_Tx_On_Change, UpdateStoreType_On_Rx, 0, &validValues);
    TestInitialValueReturnedAsString(name1, validValue10, validValue10ArrayString, RxTxType_Tx_On_Change_With_Heartbeat, UpdateStoreType_On_Rx, 1000, &validValues);
    TestInitialValueReturnedAsString(name1, validValue10, validValue10ArrayString, RxTxType_Rx_Only, UpdateStoreType_On_Rx, 0, &validValues);
    TestInitialValueReturnedAsString(name1, validValue10, validValue10ArrayString, RxTxType_Rx_Echo_Value, UpdateStoreType_On_Rx, 0, &validValues);
    
    TestInitialValueReturnedAsString(name1, validValue10, validValue10ArrayString, RxTxType_Tx_Periodic, UpdateStoreType_On_Tx, 1000, &validValues);
    TestInitialValueReturnedAsString(name1, validValue10, validValue10ArrayString, RxTxType_Tx_On_Change, UpdateStoreType_On_Tx, 0, &validValues);
    TestInitialValueReturnedAsString(name1, validValue10, validValue10ArrayString, RxTxType_Tx_On_Change_With_Heartbeat, UpdateStoreType_On_Tx, 1000, &validValues);
    TestInitialValueReturnedAsString(name1, validValue10, validValue10ArrayString, RxTxType_Rx_Only, UpdateStoreType_On_Tx, 0, &validValues);
    TestInitialValueReturnedAsString(name1, validValue10, validValue10ArrayString, RxTxType_Rx_Echo_Value, UpdateStoreType_On_Tx, 0, &validValues);
}

// ************ Set Value From Value Converts To String ******************
TEST_F(DataItemGetAndSetValueTestsInt1, dataItem_Set_Value_From_Value_Converts_To_String)
{
    TestSetValueFromValueConvertsToString(name1, &validValue20, validValue20String, validValue10, RxTxType_Tx_On_Change, UpdateStoreType_On_Tx, 0, nullptr);
}
TEST_F(DataItemGetAndSetValueTestsInt1, dataItemWithValidation_Set_Value_From_Value_Converts_To_String)
{
    TestSetValueFromValueConvertsToString(name1, &validValue20, validValue20String, validValue10, RxTxType_Tx_On_Change, UpdateStoreType_On_Tx, 0, &validValues);
}
TEST_F(DataItemGetAndSetValueTestsInt10, dataItemArray_Set_Value_From_Value_Converts_To_String)
{
    TestSetValueFromValueConvertsToString(name1, validValue20Array, validValue20ArrayString, validValue10, RxTxType_Tx_On_Change, UpdateStoreType_On_Tx, 0, nullptr);
}
TEST_F(DataItemGetAndSetValueTestsInt10, dataItemArrayWithValidation_Set_Value_From_Value_Converts_To_String)
{
    TestSetValueFromValueConvertsToString(name1, validValue20Array, validValue20ArrayString, validValue10, RxTxType_Tx_On_Change, UpdateStoreType_On_Tx, 0, &validValues);
}


// ************ Set Value From String Converts To Value ******************
TEST_F(DataItemGetAndSetValueTestsInt1, dataItem_Set_Value_From_String_Converts_To_Value)
{
    TestSetValueFromStringConvertsToValue(name1, validValue20String, &validValue20, validValue10, RxTxType_Tx_On_Change, UpdateStoreType_On_Tx, 0, nullptr);
}
TEST_F(DataItemGetAndSetValueTestsInt1, dataItemWithValidation_Set_Value_From_String_Converts_To_Value)
{
    TestSetValueFromStringConvertsToValue(name1, validValue20String, &validValue20, validValue10, RxTxType_Tx_On_Change, UpdateStoreType_On_Tx, 0, &validValues);
}
TEST_F(DataItemGetAndSetValueTestsInt10, dataItemArray_Set_Value_From_String_Converts_To_Value)
{
    TestSetValueFromStringConvertsToValue(name1, validValue20ArrayString, validValue20Array, validValue10, RxTxType_Tx_On_Change, UpdateStoreType_On_Tx, 0, nullptr);
}
TEST_F(DataItemGetAndSetValueTestsInt10, dataItemArrayWithValidation_Set_Value_From_String_Converts_To_Value)
{
    TestSetValueFromStringConvertsToValue(name1, validValue20ArrayString, validValue20Array, validValue10, RxTxType_Tx_On_Change, UpdateStoreType_On_Tx, 0, &validValues);
}

TEST_F(DataItemGetAndSetValueTestsInt1, dataItem_Set_Valid_Values_When_Validation_Is_Used)
{
    TestSettingValue(name1, validValue10, &validValue20, validValue20String, true);
}
TEST_F(DataItemGetAndSetValueTestsInt1, dataItem_Reject_Invalid_Value_When_Validation_Is_Used)
{
    TestSettingValue(name1, validValue10, &invalidValue, invalidValueString, false);
}
TEST_F(DataItemGetAndSetValueTestsInt10, dataItemArray_Set_Valid_Array_Values_When_Validation_Is_Used)
{
    TestSettingValue(name1, validValue10, validValue20Array, validValue20ArrayString, true);
}
TEST_F(DataItemGetAndSetValueTestsInt10, dataItemArray_Reject_Invalid_Array_Values_When_Validation_Is_Used)
{
    TestSettingValue(name1, validValue10, invalidValueArray, invalidValueArrayString, false);
}

TEST_F(DataItemGetAndSetValueTestsInt1, dataItem_Set_Valid_String_Values_When_Validation_Is_Used)
{
    TestSettingStringValue(name1, validValue10, &validValue20, validValue20String, true);
}
TEST_F(DataItemGetAndSetValueTestsInt1, dataItem_Reject_Invalid_String_Value_When_Validation_Is_Used)
{
    TestSettingStringValue(name1, validValue10, &invalidValue, invalidValueString, false);
}
TEST_F(DataItemGetAndSetValueTestsInt10, dataItemArray_Set_Valid_String_Array_Values_When_Validation_Is_Used)
{
    TestSettingStringValue(name1, validValue10, validValue20Array, validValue20ArrayString, true);
}
TEST_F(DataItemGetAndSetValueTestsInt10, dataItemArray_Reject_Invalid_String_Array_Values_When_Validation_Is_Used)
{
    TestSettingStringValue(name1, validValue10, invalidValueArray, invalidValueArrayString, false);
}

TEST_F(DataItemGetAndSetValueTestsInt1, Callbacks_Called_And_Change_Count_Changes_Properly_When_Validation_Used)
{
    CreateDataItem(name1, validValue10, RxTxType_Tx_On_Change, UpdateStoreType_On_Tx, 0, &validValues);
    EXPECT_EQ(0, mp_DataItem->GetChangeCount());

    SetRxTxCallExpectations(name1, RxTxType_Tx_On_Change, UpdateStoreType_On_Tx, true);
    mp_DataItem->SetValue(validValue20);
    EXPECT_EQ(1, mp_DataItem->GetChangeCount());
    ::testing::Mock::VerifyAndClearExpectations(&mp_MockSerialPortMessageManager);
    ::testing::Mock::VerifyAndClearExpectations(&mockNamedCallback_Callback);
    
    SetRxTxCallExpectations(name1, RxTxType_Tx_On_Change, UpdateStoreType_On_Tx, false);
    mp_DataItem->SetValue(validValue20);
    EXPECT_EQ(1, mp_DataItem->GetChangeCount());
    ::testing::Mock::VerifyAndClearExpectations(&mp_MockSerialPortMessageManager);
    ::testing::Mock::VerifyAndClearExpectations(&mockNamedCallback_Callback);
    
    SetRxTxCallExpectations(name1, RxTxType_Tx_On_Change, UpdateStoreType_On_Tx, false);
    mp_DataItem->SetValue(invalidValue);
    EXPECT_EQ(1, mp_DataItem->GetChangeCount());
    ::testing::Mock::VerifyAndClearExpectations(&mp_MockSerialPortMessageManager);
    ::testing::Mock::VerifyAndClearExpectations(&mockNamedCallback_Callback);
    
    SetRxTxCallExpectations(name1, RxTxType_Tx_On_Change, UpdateStoreType_On_Tx, true);
    mp_DataItem->SetValue(validValue30);
    EXPECT_EQ(2, mp_DataItem->GetChangeCount());
    ::testing::Mock::VerifyAndClearExpectations(&mp_MockSerialPortMessageManager);
    ::testing::Mock::VerifyAndClearExpectations(&mockNamedCallback_Callback);
}

TEST_F(DataItemGetAndSetValueTestsInt1, Callbacks_Called_And_Change_Count_Changes_Properly_When_Validation_Not_Used)
{
    CreateDataItem(name1, validValue10, RxTxType_Tx_On_Change, UpdateStoreType_On_Tx, 0, nullptr);
    EXPECT_EQ(0, mp_DataItem->GetChangeCount());
    
    SetRxTxCallExpectations(name1, RxTxType_Tx_On_Change, UpdateStoreType_On_Tx, true);
    mp_DataItem->SetValue(validValue20);
    EXPECT_EQ(1, mp_DataItem->GetChangeCount());
    ::testing::Mock::VerifyAndClearExpectations(&mp_MockSerialPortMessageManager);
    ::testing::Mock::VerifyAndClearExpectations(&mockNamedCallback_Callback);
    
    SetRxTxCallExpectations(name1, RxTxType_Tx_On_Change, UpdateStoreType_On_Tx, false);
    mp_DataItem->SetValue(validValue20);
    EXPECT_EQ(1, mp_DataItem->GetChangeCount());
    ::testing::Mock::VerifyAndClearExpectations(&mp_MockSerialPortMessageManager);
    ::testing::Mock::VerifyAndClearExpectations(&mockNamedCallback_Callback);
    
    SetRxTxCallExpectations(name1, RxTxType_Tx_On_Change, UpdateStoreType_On_Tx, true);
    mp_DataItem->SetValue(invalidValue);
    EXPECT_EQ(2, mp_DataItem->GetChangeCount());
    ::testing::Mock::VerifyAndClearExpectations(&mp_MockSerialPortMessageManager);
    ::testing::Mock::VerifyAndClearExpectations(&mockNamedCallback_Callback);
    
    SetRxTxCallExpectations(name1, RxTxType_Tx_On_Change, UpdateStoreType_On_Tx, true);
    mp_DataItem->SetValue(validValue30);
    EXPECT_EQ(3
    , mp_DataItem->GetChangeCount());
    ::testing::Mock::VerifyAndClearExpectations(&mp_MockSerialPortMessageManager);
    ::testing::Mock::VerifyAndClearExpectations(&mockNamedCallback_Callback);
}