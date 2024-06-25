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
        DestroyDataItem();
        delete mp_MockSerialPortMessageManager;
        delete mp_MockSetupCaller;
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


// Test Fixture for DataItem Rx Tx Tests
class StringDataItemRxTxTests : public Test
                              , public SetupCallerInterface
{
protected:
    const String initialValue = "Initial Value";
    const String name = "Name";
    const String spmm = "Serial Port Message Manager";
    MockHardwareSerial m_MockHardwareSerial;
    MockDataSerializer m_MockDataSerializer;
    MockSerialPortMessageManager *mp_MockSerialPortMessageManager;
    StringDataItem *mp_DataItem;

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
            EXPECT_CALL(*mp_MockSerialPortMessageManager, DeRegisterForNewValueNotification(NotNull())).Times(1);
            delete mp_DataItem;
            mp_DataItem = nullptr;
        }
    }
};

TEST_F(StringDataItemRxTxTests, Tx_Called_Periodically)
{
    EXPECT_CALL(*mp_MockSerialPortMessageManager, QueueMessageFromData(_,_,_,_)).Times(10)
        .WillRepeatedly(Return(true));
    EXPECT_CALL(*mp_MockSerialPortMessageManager, RegisterForNewValueNotification(NotNull())).Times(1);
    CreateDataItem(RxTxType_Tx_Periodic, UpdateStoreType_On_Rx, 100);
    std::this_thread::sleep_for(std::chrono::milliseconds(1050));
}