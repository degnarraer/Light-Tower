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
#include "Mock_SerialMessageManager.h"

using ::testing::_;
using ::testing::NotNull;
using ::testing::InvokeWithoutArgs;
using namespace testing;

// Test Fixture for LocalDataItemSetupCallerTest
class StringDataItemFunctionCallTests : public Test
{
protected:
    MockSetupCallerInterface *mp_MockSetupCaller;
    MockHardwareSerial m_MockHardwareSerial;
    MockDataSerializer m_MockDataSerializer;
    MockSerialPortMessageManager *mp_MockSerialPortMessageManager;
    StringDataItem *mp_DataItem;
    const String spmm = "Serial Port Message Manager";
    const String name = "name";
    const String initialValue = "Initial Value";

    void SetUp() override
    {
        mp_MockSetupCaller = new MockSetupCallerInterface();
        mp_MockSerialPortMessageManager = new MockSerialPortMessageManager( name
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
        mp_DataItem = new StringDataItem( name 
                                        , initialValue
                                        , rxTxType
                                        , updateStoreType
                                        , rate
                                        , *mp_MockSerialPortMessageManager
                                        , NULL
                                        , mp_MockSetupCaller );
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
        delete mp_MockSerialPortMessageManager;
        delete mp_MockSetupCaller;
        DestroyDataItem();
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

TEST_F(StringDataItemFunctionCallTests, Registration_With_Setup_Caller)
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

TEST_F(StringDataItemFunctionCallTests, Registration_For_New_Value_Notification)
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

/*
// Test Fixture for LocalDataItemTest
class StringDataItemTest: public Test
                        , public SetupCallerInterface
{
protected:
    MockHardwareSerial m_MockHardwareSerial;
    MockDataSerializer m_MockDataSerializer;
    MockSerialPortMessageManager *mp_MockSerialPortMessageManager;
    const String spmm = "Serial Port Message Manager";
    const String initialValue = "Initial Value";
    const String value1 = "Value 1";
    const String value2 = "Value 2";
    StringDataItem *mp_DataItem;
    const String name = "Name";

    void SetUp() override
    {
        mp_MockSerialPortMessageManager = new MockSerialPortMessageManager( name
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
        mp_DataItem = new StringDataItem( name 
                                        , initialValue
                                        , rxTxType
                                        , updateStoreType
                                        , rate
                                        , *mp_MockSerialPortMessageManager
                                        , NULL
                                        , this );
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
        if(mp_MockSerialPortMessageManager)
        {
            delete mp_MockSerialPortMessageManager;
        }
        DestroyDataItem();
    }
};

TEST_F(LocalStringDataItemTest, dataItem_Name_Is_Set)
{
    EXPECT_STREQ(name.c_str(), mp_DataItem->GetName().c_str());
}
/*
TEST_F(LocalStringDataItemTest, dataItem_Initial_Value_Is_Returned_As_String)
{
    EXPECT_STREQ(initialValue.c_str(), mp_DataItem->GetInitialValueAsString().c_str());
    EXPECT_STREQ(initialValue.c_str(), mp_DataItem->GetValueAsString().c_str());
}

TEST_F(LocalStringDataItemTest, dataItem_Set_Value_From_Char_Pointer_Converts_To_String)
{
    mp_DataItem->SetValue(value1.c_str(), value1.length());
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

TEST_F(LocalStringDataItemTest, Count_Reflects_DataItem_Count)
{
    EXPECT_EQ(50, mp_DataItem->GetCount());
}
*/