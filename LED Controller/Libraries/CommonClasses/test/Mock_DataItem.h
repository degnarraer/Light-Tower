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
#include "Streaming.h"
#include "DataItem/DataItem.h"
#include "Mock_SetupCallInterface.h"
#include "Mock_ValidValueChecker.h"
#include "Mock_SerialPortMessageManager.h"

template <typename T, size_t COUNT>
class MockDataItem : public DataItem<T, COUNT> {
    public:
        MockDataItem(const String name
				    , const T* initialValue
				    , const RxTxType_t rxTxType
				    , const UpdateStoreType_t updateStoreType
				    , const uint16_t rate )
                    : DataItem<T, COUNT>(name, initialValue, rxTxType, updateStoreType, rate){}
        MockDataItem(const String name
				    , const T& initialValue
				    , const RxTxType_t rxTxType
				    , const UpdateStoreType_t updateStoreType
				    , const uint16_t rate )
                    : DataItem<T, COUNT>(name, initialValue, rxTxType, updateStoreType, rate){}
    protected:
        MOCK_METHOD(void, Setup, (), (override));
        MOCK_METHOD(bool, SetValueFromString, (const String& stringValue), (override));
        MOCK_METHOD(bool, NewRxValueReceived, (const Named_Object_Callee_Interface* sender, const void* values, size_t count), (override));
        MOCK_METHOD(bool, SetValue, (const T* values, size_t count));
        MOCK_METHOD(bool, SetValue, (T value));
};