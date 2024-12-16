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
#include "DataItem/StringDataItem.h"
#include "Mock_SetupCallInterface.h"
#include "Mock_ValidValueChecker.h"
#include "Mock_SerialPortMessageManager.h"
#include "Mock_Callbacks.h"

using ::testing::_;
using ::testing::NotNull;
using ::testing::InvokeWithoutArgs;
using namespace testing;

// Test Fixture for LocalDataItemSetupCallerTest
class StringDataItemFunctionCallTests : public Test
{
protected:
    MockSetupCallerInterface *mp_MockSetupCaller;
    MockSerialPortMessageManager *mp_MockSerialPortMessageManager;
    StringDataItem *mp_DataItem;
    const std::string spmm = "Serial Port Message Manager";
    const std::string name = "name";
    const std::string initialValue = "Initial Value";

    void SetUp() override
    {
        mp_MockSetupCaller = new NiceMock<MockSetupCallerInterface>();
        mp_MockSerialPortMessageManager = new NiceMock<MockSerialPortMessageManager>();
        ON_CALL(*mp_MockSerialPortMessageManager, QueueMessageFromDataType(_,_,_,_,_)).WillByDefault(Return(true));
        ON_CALL(*mp_MockSerialPortMessageManager, GetName()).WillByDefault(Return(spmm));
    }
    void CreateDataItem(RxTxType_t rxTxType,  uint16_t rate)
    {
        EXPECT_CALL(*mp_MockSetupCaller, RegisterForSetupCall(NotNull())).Times(1);
        mp_DataItem = new StringDataItem( name 
                                        , initialValue
                                        , rxTxType
                                        , rate
                                        , mp_MockSerialPortMessageManager
                                        , NULL
                                        , mp_MockSetupCaller );                               
        if( rxTxType == RxTxType_Tx_Periodic || rxTxType == RxTxType_Tx_On_Change || rxTxType == RxTxType_Tx_On_Change_With_Heartbeat )
        {
            EXPECT_CALL(*mp_MockSerialPortMessageManager, QueueMessageFromDataType(name,_,_,_,_))
                .Times(1)
                .WillOnce(Return(true));
        }
        mp_DataItem->Setup();
        ::testing::Mock::VerifyAndClearExpectations(&mp_MockSetupCaller);
        ::testing::Mock::VerifyAndClearExpectations(&mp_MockSerialPortMessageManager);
    }

    void DestroyDataItem()
    {
        if(mp_DataItem)
        {
            EXPECT_CALL(*mp_MockSetupCaller, DeRegisterForSetupCall(NotNull())).Times(1);
            delete mp_DataItem;
            mp_DataItem = nullptr;
            ::testing::Mock::VerifyAndClearExpectations(&mp_MockSetupCaller);
        }
    }
    void TearDown() override
    {
        DestroyDataItem();
        delete mp_MockSerialPortMessageManager;
        delete mp_MockSetupCaller;
    }
    void TestCallRegistrationDeregistration(RxTxType_t rxtxtype)
    {
        CreateDataItem(rxtxtype, 1000);
        DestroyDataItem();
    }
};

TEST_F(StringDataItemFunctionCallTests, Registration_DeRegistration)
{
    TestCallRegistrationDeregistration(RxTxType_Tx_Periodic);
    TestCallRegistrationDeregistration(RxTxType_Tx_On_Change_With_Heartbeat);
    TestCallRegistrationDeregistration(RxTxType_Tx_On_Change);
    TestCallRegistrationDeregistration(RxTxType_Rx_Only);
    TestCallRegistrationDeregistration(RxTxType_Rx_Echo_Value);
}


// Test Fixture for DataItem Rx Tx Tests
class StringDataItemRxTxTests : public Test
                              , public SetupCallerInterface
{
protected:
    const std::string initialValue = "Initial Value";
    const std::string name = "Name";
    const std::string spmm = "Serial Port Message Manager";
    NiceMock<MockSerialPortMessageManager> *mp_MockSerialPortMessageManager;
    StringDataItem *mp_DataItem;

    void SetUp() override
    {
        mp_MockSerialPortMessageManager = new NiceMock<MockSerialPortMessageManager>();
        ON_CALL(*mp_MockSerialPortMessageManager, QueueMessageFromDataType(_,_,_,_,_)).WillByDefault(Return(true));
        ON_CALL(*mp_MockSerialPortMessageManager, GetName()).WillByDefault(Return(spmm));
    }
    void CreateDataItem(RxTxType_t rxTxType,  uint16_t rate)
    {
        mp_DataItem = new StringDataItem( name 
                                        , initialValue
                                        , rxTxType
                                        , rate
                                        , mp_MockSerialPortMessageManager
                                        , nullptr
                                        , this );
        SetupAllSetupCallees();
    }

    void TearDown() override
    {
        DestroyDataItem();
        if(mp_MockSerialPortMessageManager)
        {
            delete mp_MockSerialPortMessageManager;
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
};

TEST_F(StringDataItemRxTxTests, Tx_Called_Periodically)
{
    CreateDataItem(RxTxType_Tx_Periodic, 100);
    EXPECT_CALL(*mp_MockSerialPortMessageManager, QueueMessageFromDataType(_,_,_,_,_)).Times(10)
        .WillRepeatedly(Return(true));
    std::this_thread::sleep_for(std::chrono::milliseconds(1050));
    ::testing::Mock::VerifyAndClearExpectations(&mp_MockSerialPortMessageManager);
}

// Test Fixture for StringDataItemTest
class StringDataItemTest: public Test
                        , public SetupCallerInterface
{
protected:
    const std::string initialValue = "Initial Value";
    const std::string value1 = "Value 1";
    const std::string value2 = "Value 2";
    const std::string name = "Name";
    const std::string spmm = "Serial Port Message Manager";
    NiceMock<MockSerialPortMessageManager> *mp_MockSerialPortMessageManager;
    MockNamedCallback *mp_mockNamedCallback;
    StringDataItem *mp_DataItem;

    void SetUp() override
    {
        mp_MockSerialPortMessageManager = new NiceMock<MockSerialPortMessageManager>();
        mp_mockNamedCallback = new MockNamedCallback(name, nullptr);
        ON_CALL(*mp_MockSerialPortMessageManager, QueueMessageFromDataType(_,_,_,_,_)).WillByDefault(Return(true));
        ON_CALL(*mp_MockSerialPortMessageManager, GetName()).WillByDefault(Return(spmm));
    }

    void CreateDataItem(RxTxType_t rxTxType,  uint16_t rate)
    {
        mp_DataItem = new StringDataItem( name 
                                        , initialValue
                                        , rxTxType
					                    , rate
					                    , mp_MockSerialPortMessageManager
                                        , mp_mockNamedCallback
                                        , this );
        EXPECT_CALL(mockNamedCallback_Callback, NewValueCallbackFunction(name,_,_)).Times(1);
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
        if(mp_mockNamedCallback)
        {
            delete mp_mockNamedCallback;
            mp_mockNamedCallback = nullptr;
        }
        if (mp_MockSerialPortMessageManager)
        {
            delete mp_MockSerialPortMessageManager;
            mp_MockSerialPortMessageManager = nullptr;
        }
    }
    void SetRxTxCallExpectations( const std::string name, RxTxType_t rxTxType,  bool expectValueAccepted )
    {
        bool loggedType = false;
        switch(rxTxType)
        {
            case RxTxType_Tx_Periodic:
            {
                ESP_LOGD( "SetRxTxCallExpectations", "RxTxType_Tx_Periodic");
                if(expectValueAccepted)
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
                if(expectValueAccepted)
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
                if(expectValueAccepted)
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
};

TEST_F(StringDataItemTest, StringDataItem_Name_Is_Set)
{
    CreateDataItem(RxTxType_Tx_On_Change, 0);
    EXPECT_STREQ(name.c_str(), mp_DataItem->GetName().c_str());
    DestroyDataItem();
}

TEST_F(StringDataItemTest, StringDataItem_Initial_Value_Is_Returned_As_String)
{
    CreateDataItem(RxTxType_Tx_On_Change, 0);
    EXPECT_STREQ(initialValue.c_str(), mp_DataItem->GetInitialValueAsString().c_str());
    EXPECT_STREQ(initialValue.c_str(), mp_DataItem->GetValueAsString().c_str());
    DestroyDataItem();
}

TEST_F(StringDataItemTest, StringDataItem_Set_Value_From_Char_Pointer_Converts_To_String)
{
    CreateDataItem(RxTxType_Tx_On_Change, 0);
    SetRxTxCallExpectations(name, RxTxType_Tx_On_Change, true);
    mp_DataItem->SetValue(value1.c_str(), value1.length());
    ::testing::Mock::VerifyAndClearExpectations(&mockNamedCallback_Callback);
    EXPECT_STREQ(value1.c_str(), mp_DataItem->GetValueAsString().c_str());
    DestroyDataItem();
}

TEST_F(StringDataItemTest, Change_Count_Changes_Properly)
{
    CreateDataItem(RxTxType_Tx_On_Change, 0);
    EXPECT_EQ(0, mp_DataItem->GetChangeCount());

    SetRxTxCallExpectations(name, RxTxType_Tx_On_Change, true);
    mp_DataItem->SetValue(value1.c_str(), value1.length());
    EXPECT_EQ(1, mp_DataItem->GetChangeCount());
    ::testing::Mock::VerifyAndClearExpectations(&mockNamedCallback_Callback);

    SetRxTxCallExpectations(name, RxTxType_Tx_On_Change, false);
    mp_DataItem->SetValue(value1.c_str(), value1.length());
    EXPECT_EQ(1, mp_DataItem->GetChangeCount());
    ::testing::Mock::VerifyAndClearExpectations(&mockNamedCallback_Callback);

    SetRxTxCallExpectations(name, RxTxType_Tx_On_Change, true);
    mp_DataItem->SetValue(value2.c_str(), value1.length());
    EXPECT_EQ(2, mp_DataItem->GetChangeCount());
    ::testing::Mock::VerifyAndClearExpectations(&mockNamedCallback_Callback);

    SetRxTxCallExpectations(name, RxTxType_Tx_On_Change, false);
    mp_DataItem->SetValue(value2.c_str(), value1.length());
    EXPECT_EQ(2, mp_DataItem->GetChangeCount());
    ::testing::Mock::VerifyAndClearExpectations(&mockNamedCallback_Callback);
    DestroyDataItem();
}

TEST_F(StringDataItemTest, Callback_Only_Called_For_New_Values)
{
    CreateDataItem(RxTxType_Tx_On_Change, 0);
    
    SetRxTxCallExpectations(name, RxTxType_Tx_On_Change, true);
    mp_DataItem->SetValue(value1.c_str(), value1.length());
    ::testing::Mock::VerifyAndClearExpectations(&mockNamedCallback_Callback);

    SetRxTxCallExpectations(name, RxTxType_Tx_On_Change, false);
    mp_DataItem->SetValue(value1.c_str(), value1.length());
    ::testing::Mock::VerifyAndClearExpectations(&mockNamedCallback_Callback);

    SetRxTxCallExpectations(name, RxTxType_Tx_On_Change, true);
    mp_DataItem->SetValue(value2.c_str(), value1.length());
    ::testing::Mock::VerifyAndClearExpectations(&mockNamedCallback_Callback);

    SetRxTxCallExpectations(name, RxTxType_Tx_On_Change, false);
    mp_DataItem->SetValue(value2.c_str(), value1.length());
    ::testing::Mock::VerifyAndClearExpectations(&mockNamedCallback_Callback);
    DestroyDataItem();
}

TEST_F(StringDataItemTest, Count_Reflects_DataItem_Count)
{
    CreateDataItem(RxTxType_Tx_On_Change, 0);
    EXPECT_EQ(50, mp_DataItem->GetCount());
    DestroyDataItem();
}