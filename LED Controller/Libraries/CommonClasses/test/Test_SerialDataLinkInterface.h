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
#include "DataItem/SerialDataLinkInterface.h"
#include "Mock_SerialMessageManager.h"
#include "Mock_SetupCallInterface.h"
#include "Mock_ValidValueChecker.h"
#include "Mock_SerialMessageManager.h"
#include <gtest/gtest.h>
#include <gmock/gmock.h>

using ::testing::_;
using ::testing::NotNull;
using ::testing::NiceMock;
using namespace testing;

template <typename T, size_t COUNT>
class MockSerialDataLinkInterface
{
    public:
        MOCK_METHOD(T*, GetValuePointer, (), (const));
        MOCK_METHOD(bool, SetValue, (const T *value, size_t count), ());
        MOCK_METHOD(bool, EqualsValue, (T *object, size_t count), (const));
        MOCK_METHOD(String, GetName, (), (const));
        MOCK_METHOD(String, GetValueAsString, (), (const));
        MOCK_METHOD(DataType_t, GetDataType, (), (const));
};

template <typename T, size_t COUNT>
class SerialDataLinkInterfaceTester: public SerialDataLinkInterface<T, COUNT>
{
    public:
        SerialDataLinkInterfaceTester( MockSerialPortMessageManager* mockSerialPortMessageManager )
                                     : SerialDataLinkInterface<T, COUNT>( mockSerialPortMessageManager )
                                     {
                                     }
        SerialDataLinkInterfaceTester( RxTxType_t rxTxType
                                     , UpdateStoreType_t updateStoreType
                                     , uint16_t rate
                                     , MockSerialPortMessageManager* mockSerialPortMessageManager )
                                     : SerialDataLinkInterface<T, COUNT>( mockSerialPortMessageManager )
                                     {
                                     }
        MockSerialDataLinkInterface<T, COUNT>& GetMock()
        {
            return m_MockSerialDataLinkInterface;
        }
        virtual T* GetValuePointer() const override
        {
            return m_MockSerialDataLinkInterface.GetValuePointer();
        }
        virtual bool SetValue(const T *value, size_t count) override
        {
            return m_MockSerialDataLinkInterface.SetValue(value, count);
        }
		virtual bool EqualsValue(T *object, size_t count) const override
        {
            return m_MockSerialDataLinkInterface.EqualsValue(object, count);
        }
		virtual String GetName() const override
        {
            return m_MockSerialDataLinkInterface.GetName();
        }
		virtual String GetValueAsString() const override
        {
            return m_MockSerialDataLinkInterface.GetValueAsString();
        }
		virtual DataType_t GetDataType() override
        {
            return m_MockSerialDataLinkInterface.GetDataType();
        }

        void Configure( RxTxType_t rxTxType
                      , UpdateStoreType_t updateStoreType
                      , uint16_t rate )
        {
            SerialDataLinkInterface<T, COUNT>::Configure(rxTxType, updateStoreType, rate);
        }

        void Setup()
        {
            SerialDataLinkInterface<T, COUNT>::Setup();
        }
    private:
        T m_value[COUNT];
        MockSerialDataLinkInterface<T, COUNT> m_MockSerialDataLinkInterface;
};

template <typename T, size_t COUNT>
class SerialDataLinkInterfaceTests : public Test
                                   , public DataTypeFunctions
{
    public:
        SerialDataLinkInterfaceTests()
            : mp_MockSerialPortMessageManager(nullptr)
            , mp_SerialDataLinkInterfaceTester(nullptr)
        {
        }
        SerialDataLinkInterfaceTests( RxTxType_t rxTxType
                                    , UpdateStoreType_t updateStoreType
                                    , uint16_t rate )                     
        {

        }
    protected:
        T m_value[COUNT];
        const String m_SerialPortMessageManagerName = "SPMM";
        MockHardwareSerial m_MockHardwareSerial;
        MockDataSerializer m_MockDataSerializer;
        MockSerialPortMessageManager *mp_MockSerialPortMessageManager = nullptr;
        SerialDataLinkInterfaceTester<T, COUNT>* mp_SerialDataLinkInterfaceTester = nullptr;
        const String m_SerialPortInterfaceName = "SPI";
        void SetUp() override
        {
            mp_MockSerialPortMessageManager = new MockSerialPortMessageManager( m_SerialPortMessageManagerName, m_MockHardwareSerial, m_MockDataSerializer, 0 );
            mp_SerialDataLinkInterfaceTester = new SerialDataLinkInterfaceTester<T, COUNT>( mp_MockSerialPortMessageManager );
            ON_CALL(mp_SerialDataLinkInterfaceTester->GetMock(), GetValuePointer()).WillByDefault(Return(m_value));
            ON_CALL(mp_SerialDataLinkInterfaceTester->GetMock(), GetName()).WillByDefault(Return(m_SerialPortInterfaceName));
            ON_CALL(mp_SerialDataLinkInterfaceTester->GetMock(), GetDataType()).WillByDefault(Return(GetDataTypeFromTemplateType<T>()));
            ON_CALL(*mp_MockSerialPortMessageManager, QueueMessageFromData(_,_,_,_)).WillByDefault(Return(true));
            ON_CALL(*mp_MockSerialPortMessageManager, GetName()).WillByDefault(Return(m_SerialPortMessageManagerName));
        }

        void TearDown() override
        {
            delete mp_SerialDataLinkInterfaceTester;
            delete mp_MockSerialPortMessageManager;
        }

        void SetupInterface()
        {
            EXPECT_CALL(mp_SerialDataLinkInterfaceTester->GetMock(), GetName()).WillRepeatedly(Return(m_SerialPortInterfaceName));
            EXPECT_CALL(*mp_MockSerialPortMessageManager, GetName()).WillRepeatedly(Return(m_SerialPortInterfaceName));
            EXPECT_CALL(mp_SerialDataLinkInterfaceTester->GetMock(), GetValuePointer()).Times(3).WillRepeatedly(Return(m_value));
            EXPECT_CALL(*mp_MockSerialPortMessageManager, RegisterForNewValueNotification(mp_SerialDataLinkInterfaceTester)).Times(1);
            mp_SerialDataLinkInterfaceTester->Setup();
            ::testing::Mock::VerifyAndClearExpectations(&(mp_SerialDataLinkInterfaceTester->GetMock()));
            ::testing::Mock::VerifyAndClearExpectations(mp_MockSerialPortMessageManager);
        }

        void Configure( RxTxType_t rxTxType
                      , UpdateStoreType_t updateStoreType
                      , uint16_t rate )
        {
            mp_SerialDataLinkInterfaceTester->Configure(rxTxType, updateStoreType, rate);
        }
};

using SerialDataLinkInterfaceTests_int32_t_1 = SerialDataLinkInterfaceTests<int32_t, 1>;
using SerialDataLinkInterfaceTests_uint32_t_1 = SerialDataLinkInterfaceTests<uint32_t, 1>;

TEST_F(SerialDataLinkInterfaceTests_int32_t_1, TEST1)
{
    EXPECT_CALL(*mp_MockSerialPortMessageManager, QueueMessageFromData(_,_,_,_)).Times(10);
    Configure(RxTxType_Tx_Periodic, UpdateStoreType_On_Tx, 100);
    SetupInterface();
    std::this_thread::sleep_for(std::chrono::milliseconds(1050));
    ::testing::Mock::VerifyAndClearExpectations(mp_MockSerialPortMessageManager);
}

TEST_F(SerialDataLinkInterfaceTests_uint32_t_1, TEST2)
{
    Configure(RxTxType_Tx_Periodic, UpdateStoreType_On_Tx, 1000);
    EXPECT_EQ(1, 1);
}