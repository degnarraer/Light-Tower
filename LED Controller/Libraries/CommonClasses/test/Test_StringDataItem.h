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
    MockHardwareSerial m_MockHardwareSerial;
    MockDataSerializer m_MockDataSerializer;
    NiceMock<MockSetupCallerInterface> *mp_MockSetupCaller;
    NiceMock<MockSerialPortMessageManager> *mp_MockSerialPortMessageManager;
    StringDataItem *mp_DataItem;
    const String spmm = "Serial Port Message Manager";
    const String name = "name";
    const String initialValue = "Initial Value";

    void SetUp() override
    {
        mp_MockSetupCaller = new NiceMock<MockSetupCallerInterface>();
        mp_MockSerialPortMessageManager = new NiceMock<MockSerialPortMessageManager>( name, m_MockHardwareSerial, m_MockDataSerializer, 0 );
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
                                        , mp_MockSerialPortMessageManager
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
    void TestSetupCallRegistration(RxTxType_t rxtxtype)
    {
        EXPECT_CALL(*mp_MockSetupCaller, RegisterForSetupCall(NotNull())).Times(1);
        CreateDataItem(rxtxtype, UpdateStoreType_On_Rx, 1000);
    }
    void TestSetupCallDeregistration(RxTxType_t rxtxtype)
    {
        EXPECT_CALL(*mp_MockSetupCaller, DeRegisterForSetupCall(NotNull())).Times(1);
        DestroyDataItem();
    }
    void TestNewValueNotificationRegistration(RxTxType_t rxtxtype)
    {
        EXPECT_CALL(*mp_MockSerialPortMessageManager, RegisterForNewValueNotification(NotNull())).Times(1);
        CreateDataItem(rxtxtype, UpdateStoreType_On_Rx, 1000);
        mp_DataItem->Setup();
    }
    void TestNewValueNotificationDeregistration(RxTxType_t rxtxtype)
    {
        EXPECT_CALL(*mp_MockSerialPortMessageManager, DeRegisterForNewValueNotification(NotNull())).Times(1);
        DestroyDataItem();
    }
};

TEST_F(StringDataItemFunctionCallTests, Registration_With_Setup_Caller)
{
    TestSetupCallRegistration(RxTxType_Tx_Periodic);
    TestSetupCallDeregistration(RxTxType_Tx_Periodic);

    TestSetupCallRegistration(RxTxType_Tx_On_Change_With_Heartbeat);
    TestSetupCallDeregistration(RxTxType_Tx_On_Change_With_Heartbeat);

    TestSetupCallRegistration(RxTxType_Tx_On_Change);
    TestSetupCallDeregistration(RxTxType_Tx_On_Change);

    TestSetupCallRegistration(RxTxType_Rx_Only);
    TestSetupCallDeregistration(RxTxType_Rx_Only);

    TestSetupCallRegistration(RxTxType_Rx_Echo_Value);
    TestSetupCallDeregistration(RxTxType_Rx_Echo_Value);
}

TEST_F(StringDataItemFunctionCallTests, Registration_For_New_Value_Notification)
{
    TestNewValueNotificationRegistration(RxTxType_Tx_Periodic);
    TestNewValueNotificationDeregistration(RxTxType_Tx_Periodic);

    TestNewValueNotificationRegistration(RxTxType_Tx_On_Change_With_Heartbeat);
    TestNewValueNotificationDeregistration(RxTxType_Tx_On_Change_With_Heartbeat);

    TestNewValueNotificationRegistration(RxTxType_Tx_On_Change);
    TestNewValueNotificationDeregistration(RxTxType_Tx_On_Change);

    TestNewValueNotificationRegistration(RxTxType_Rx_Only);
    TestNewValueNotificationDeregistration(RxTxType_Rx_Only);

    TestNewValueNotificationRegistration(RxTxType_Rx_Echo_Value);    
    TestNewValueNotificationDeregistration(RxTxType_Rx_Echo_Value);
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
    NiceMock<MockSerialPortMessageManager> *mp_MockSerialPortMessageManager;
    StringDataItem *mp_DataItem;

    void SetUp() override
    {
        mp_MockSerialPortMessageManager = new NiceMock<MockSerialPortMessageManager>( name, m_MockHardwareSerial, m_MockDataSerializer, 0 );
        ON_CALL(*mp_MockSerialPortMessageManager, GetName()).WillByDefault(Return(spmm));
    }
    void CreateDataItem(RxTxType_t rxTxType, UpdateStoreType_t updateStoreType, uint16_t rate)
    {
        mp_DataItem = new StringDataItem( name 
                                        , initialValue
                                        , rxTxType
                                        , updateStoreType
                                        , rate
                                        , mp_MockSerialPortMessageManager
                                        , NULL
                                        , this );
        EXPECT_CALL(*mp_MockSerialPortMessageManager, RegisterForNewValueNotification(mp_DataItem)).Times(1);
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
            EXPECT_CALL(*mp_MockSerialPortMessageManager, DeRegisterForNewValueNotification(mp_DataItem)).Times(1);
            delete mp_DataItem;
            mp_DataItem = nullptr;
        }
    }
};

TEST_F(StringDataItemRxTxTests, Tx_Called_Periodically)
{
    EXPECT_CALL(*mp_MockSerialPortMessageManager, QueueMessageFromData(_,_,_,_)).Times(10)
        .WillRepeatedly(Return(true));
    CreateDataItem(RxTxType_Tx_Periodic, UpdateStoreType_On_Rx, 100);
    std::this_thread::sleep_for(std::chrono::milliseconds(1050));
}