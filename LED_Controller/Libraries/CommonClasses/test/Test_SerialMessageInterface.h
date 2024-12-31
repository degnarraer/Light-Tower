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
#include "DataTypes.h"
#include "DataItem/SerialMessageInterface.h"
#include "Mock_SerialPortMessageManager.h"
#include "Mock_SetupCallInterface.h"
#include "Mock_ValidValueChecker.h"
#include <gtest/gtest.h>
#include <gmock/gmock.h>

using ::testing::_;
using ::testing::NotNull;
using ::testing::NiceMock;
using namespace testing;

template <typename T, size_t COUNT>
class MockSerialMessageInterface
{
    public:
        MOCK_METHOD(T*, GetValuePointer, (), (const));
        MOCK_METHOD(UpdateStatus_t, UpdateStore, (T* oldValues, const T *newValues, size_t count), ());
        MOCK_METHOD(bool, EqualsValue, (T *object, size_t count), (const));
        MOCK_METHOD(std::string, GetName, (), (const));
        MOCK_METHOD(size_t, GetChangeCount, (), (const));
        MOCK_METHOD(bool, ConfirmValueValidity, (const T* values, size_t count), (const));
        MOCK_METHOD(std::string, GetValueAsString, (), (const));
        MOCK_METHOD(DataType_t, GetDataType, (), (const));
        MOCK_METHOD(std::string, ConvertValueToString, (const T *object, size_t count), (const));
		MOCK_METHOD(bool, ParseStringValueIntoValues, (const std::string& stringValue, T* values), ());
};

template <typename T, size_t COUNT>
class SerialMessageInterfaceTester: public SerialMessageInterface<T, COUNT>
{
    public:
        SerialMessageInterfaceTester( MockSerialPortMessageManager* mockSerialPortMessageManager )
                                    : SerialMessageInterface<T, COUNT>( mockSerialPortMessageManager )
        {
            ESP_LOGD("SerialMessageInterfaceTester", "Construct SerialMessageInterfaceTester");
        }
        virtual ~SerialMessageInterfaceTester() override
        {
        ESP_LOGD("~SerialMessageInterfaceTester", "Deleting SerialMessageInterfaceTester");
        }
        MockSerialMessageInterface<T, COUNT>& GetMock()
        {
            return m_MockSerialMessageInterface;
        }
        virtual T* GetValuePointer() const override
        {
            return m_MockSerialMessageInterface.GetValuePointer();
        }
		virtual UpdateStatus_t UpdateStore(T* oldValues, const T *newValues, const size_t changeCount) override
        {
            return m_MockSerialMessageInterface.UpdateStore(oldValues, newValues, changeCount);
        }
		virtual bool EqualsValue(T *object, size_t count) const override
        {
            return m_MockSerialMessageInterface.EqualsValue(object, count);
        }
		virtual std::string GetName() const override
        {
            return m_MockSerialMessageInterface.GetName();
        }
        virtual size_t GetChangeCount()const override
        {
            return m_MockSerialMessageInterface.GetChangeCount();
        }
        virtual bool ConfirmValueValidity(const T* values, size_t count) const override
        {
            return m_MockSerialMessageInterface.ConfirmValueValidity(values, count);
        }
		virtual std::string GetValueAsString() const override
        {
            return m_MockSerialMessageInterface.GetValueAsString();
        }
		virtual DataType_t GetDataType() override
        {
            return m_MockSerialMessageInterface.GetDataType();
        }
        virtual std::string ConvertValueToString(const T *pvalue, size_t count) const override
        {
            return m_MockSerialMessageInterface.ConvertValueToString(pvalue, count);
        }
		virtual size_t ParseStringValueIntoValues(const std::string& stringValue, T* values)
        {
            return m_MockSerialMessageInterface.ParseStringValueIntoValues(stringValue, values);
        }

        void Configure( RxTxType_t rxTxType, uint16_t rate )
        {
            SerialMessageInterface<T, COUNT>::Configure(rxTxType, rate);
        }

        void Setup()
        {
            SerialMessageInterface<T, COUNT>::Setup();
        }
    private:
        T m_value[COUNT];
        NiceMock<MockSerialMessageInterface<T, COUNT>> m_MockSerialMessageInterface;
};

template <typename T, size_t COUNT>
class SerialMessageInterfaceTests : public Test
                                   , public DataTypeFunctions
{
    public:
        SerialMessageInterfaceTests()
            : mp_MockSerialPortMessageManager(nullptr)
            , mp_SerialMessageInterfaceTester(nullptr)
        {
        }
        SerialMessageInterfaceTests( RxTxType_t rxTxType, uint16_t rate )                     
        {

        }
    protected:
        T m_value[COUNT];
        std::string m_SerialPortMessageManagerName = "SPMM";
        MockSerialPortMessageManager *mp_MockSerialPortMessageManager = nullptr;
        SerialMessageInterfaceTester<T, COUNT>* mp_SerialMessageInterfaceTester = nullptr;
        std::string m_SerialPortInterfaceName = "SPI";
        void SetUp() override
        {
            mp_MockSerialPortMessageManager = new NiceMock<MockSerialPortMessageManager>();
            mp_SerialMessageInterfaceTester = new SerialMessageInterfaceTester<T, COUNT>( mp_MockSerialPortMessageManager );
            ON_CALL(mp_SerialMessageInterfaceTester->GetMock(), GetValuePointer()).WillByDefault(Return(m_value));
            ON_CALL(mp_SerialMessageInterfaceTester->GetMock(), GetName()).WillByDefault(Return(m_SerialPortInterfaceName));
            ON_CALL(mp_SerialMessageInterfaceTester->GetMock(), GetDataType()).WillByDefault(Return(GetDataTypeFromTemplateType<T>()));
            ON_CALL(*mp_MockSerialPortMessageManager, QueueMessageFromDataType(_,_,_,_,_)).WillByDefault(Return(true));
            ON_CALL(*mp_MockSerialPortMessageManager, GetName()).WillByDefault(Return(m_SerialPortMessageManagerName));
        }

        void TearDown() override
        {
            delete mp_SerialMessageInterfaceTester;
            delete mp_MockSerialPortMessageManager;
        }

        void SetupInterface(RxTxType_t rxTxType)
        {
            EXPECT_CALL(mp_SerialMessageInterfaceTester->GetMock(), GetName()).WillRepeatedly(Return(m_SerialPortInterfaceName));
            EXPECT_CALL(mp_SerialMessageInterfaceTester->GetMock(), GetValuePointer()).Times(3).WillRepeatedly(Return(m_value));
            EXPECT_CALL(*mp_MockSerialPortMessageManager, GetName()).WillRepeatedly(Return(m_SerialPortMessageManagerName));
            EXPECT_CALL(*mp_MockSerialPortMessageManager, RegisterForNewRxValueNotification(mp_SerialMessageInterfaceTester)).Times(1);
            mp_SerialMessageInterfaceTester->Setup();
            ::testing::Mock::VerifyAndClearExpectations(&(mp_SerialMessageInterfaceTester->GetMock()));
            ::testing::Mock::VerifyAndClearExpectations(mp_MockSerialPortMessageManager);
        }

        void Configure( RxTxType_t rxTxType, uint16_t rate )
        {
            mp_SerialMessageInterfaceTester->Configure(rxTxType, rate);
        }
};

using SerialMessageInterfaceTests_int32_t_1 = SerialMessageInterfaceTests<int32_t, 1>;
using SerialMessageInterfaceTests_uint32_t_1 = SerialMessageInterfaceTests<uint32_t, 1>;

TEST_F(SerialMessageInterfaceTests_int32_t_1, Construct_and_destruct)
{
    Configure(RxTxType_Tx_Periodic, 100);
    EXPECT_CALL(*mp_MockSerialPortMessageManager, QueueMessageFromDataType(_,_,_,_,_)).Times(1);
    SetupInterface(RxTxType_Tx_Periodic);
}

TEST_F(SerialMessageInterfaceTests_uint32_t_1, Construct_and_destruct)
{
    Configure(RxTxType_Tx_Periodic, 1000);
    EXPECT_CALL(*mp_MockSerialPortMessageManager, QueueMessageFromDataType(_,_,_,_,_)).Times(1);
    SetupInterface(RxTxType_Tx_Periodic);
}

TEST_F(SerialMessageInterfaceTests_int32_t_1, Tx_Periodic_Preiodically_Queues_Message)
{
    EXPECT_CALL(*mp_MockSerialPortMessageManager, QueueMessageFromDataType(_,_,_,_,_)).Times(1);
    Configure(RxTxType_Tx_Periodic, 100);
    SetupInterface(RxTxType_Tx_Periodic);
    ::testing::Mock::VerifyAndClearExpectations(mp_MockSerialPortMessageManager);

    EXPECT_CALL(mp_SerialMessageInterfaceTester->GetMock(), GetName()).WillRepeatedly(Return(m_SerialPortInterfaceName));
    EXPECT_CALL(mp_SerialMessageInterfaceTester->GetMock(), UpdateStore(_, _, _)).WillRepeatedly(Return(UpdateStatus_t(true,true,true,true)));
    EXPECT_CALL(mp_SerialMessageInterfaceTester->GetMock(), EqualsValue(_, _)).WillRepeatedly(Return(true));
    EXPECT_CALL(mp_SerialMessageInterfaceTester->GetMock(), GetValueAsString()).WillRepeatedly(Return("10"));
    EXPECT_CALL(mp_SerialMessageInterfaceTester->GetMock(), GetDataType()).WillRepeatedly(Return(GetDataTypeFromTemplateType<int32_t>()));

    EXPECT_CALL(*mp_MockSerialPortMessageManager, GetName()).WillRepeatedly(Return(m_SerialPortMessageManagerName));
    EXPECT_CALL(*mp_MockSerialPortMessageManager, QueueMessageFromDataType(_,_,_,_,_)).Times(10);
    std::this_thread::sleep_for(std::chrono::milliseconds(1050));
    ::testing::Mock::VerifyAndClearExpectations(mp_MockSerialPortMessageManager);
}