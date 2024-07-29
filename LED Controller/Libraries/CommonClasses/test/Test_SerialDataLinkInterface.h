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
        MOCK_METHOD(T*, GetValuePointer, (), ());
        MOCK_METHOD(bool, SetValue, (const T *value, size_t count), ());
        MOCK_METHOD(bool, EqualsValue, (T *object, size_t count), ());
        MOCK_METHOD(String, GetName, (), ());
        MOCK_METHOD(String, GetValueAsString, (), ());
        MOCK_METHOD(DataType_t, GetDataType, (), ());
};

template <typename T, size_t COUNT>
class SerialDataLinkInterfaceTester: public SerialDataLinkInterface<T, COUNT>
{
    public:
        SerialDataLinkInterfaceTester( RxTxType_t rxTxType
                                     , UpdateStoreType_t updateStoreType
                                     , uint16_t rate
                                     , MockSerialPortMessageManager* mockSerialPortMessageManager )
                                     : SerialDataLinkInterface<T, COUNT>( rxTxType, 
                                                                          updateStoreType, 
                                                                          rate, 
                                                                          mockSerialPortMessageManager ){}
		
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
    private:
        MockSerialDataLinkInterface<T, COUNT> m_MockSerialDataLinkInterface;
};

template <typename T, size_t COUNT>
class SerialDataLinkInterfaceTests : public Test
{
protected:
    const String m_Name = "SPMM";
    MockHardwareSerial m_MockHardwareSerial;
    MockDataSerializer m_MockDataSerializer;
    MockSerialPortMessageManager *mp_MockSerialPortMessageManager = nullptr;
    SerialDataLinkInterfaceTester<T, COUNT>* mp_SerialDataLinkInterfaceTester = nullptr;
    MockSerialDataLinkInterface<T, COUNT>* mp_MockSerialDataLinkInterface = nullptr;
    void SetUp() override
    {
        mp_MockSerialPortMessageManager = new MockSerialPortMessageManager(m_Name, m_MockHardwareSerial, m_MockDataSerializer, 0);
    }

    void TearDown() override
    {
        delete mp_MockSerialPortMessageManager;
    }
};

using SerialDataLinkInterfaceTests_int32_1 = SerialDataLinkInterfaceTests<int32_t, 1>;
using SerialDataLinkInterfaceTests_uint32_1 = SerialDataLinkInterfaceTests<uint32_t, 1>;
TEST_F(SerialDataLinkInterfaceTests_int32_1, TEST1)
{
    EXPECT_EQ(1, 1);
}
TEST_F(SerialDataLinkInterfaceTests_uint32_1, TEST2)
{
    EXPECT_EQ(1, 1);
}