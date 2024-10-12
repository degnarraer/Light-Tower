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
#include "SerialMessageManager.h"

template <typename T>
class MockRx_Value_Callee_Interface : public Rx_Value_Callee_Interface<T> {
public:
    MOCK_METHOD(bool, NewRxValueReceived, (const T* values, size_t count, size_t changeCount), (override));
    MOCK_METHOD(String, GetName, (), (override));
    MOCK_METHOD(size_t, GetCount, (), (override));
};

// Mock class for Named_Callback_Caller_Interface
template <typename T>
class MockNamed_Callback_Caller_Interface : public Named_Callback_Caller_Interface<T>
{
public:
    MOCK_METHOD(void, RegisterNamedCallback, (NamedCallback_t *namedCallback), (override));
    MOCK_METHOD(void, DeRegisterNamedCallback, (NamedCallback_t *namedCallback), (override));
    MOCK_METHOD(void, CallNamedCallback, (const String& name, T* object), (override));
};

template <typename T>
class MockRx_Value_Caller_Interface : public Rx_Value_Caller_Interface<T> {
public:
    MOCK_METHOD(void, RegisterForNewRxValueNotification, (Rx_Value_Callee_Interface<T>* newCallee), (override));
    MOCK_METHOD(void, DeRegisterForNewRxValueNotification, (Rx_Value_Callee_Interface<T>* callee), (override));
    MOCK_METHOD(void, NotifyCallee, (const String& name, T* object), (override));
};

class MockNamed_Object_Callee_Interface : public Named_Object_Callee_Interface {
public:
    MockNamed_Object_Callee_Interface(size_t count) : Named_Object_Callee_Interface(count) {}
    MOCK_METHOD(bool, New_Object_From_Sender, (const Named_Object_Caller_Interface* sender, const void* object, size_t changeCount), (override));
    MOCK_METHOD(String, GetName, (), ());
    MOCK_METHOD(size_t, GetCount, (), ());
};

class MockNamed_Object_Caller_Interface : public Named_Object_Caller_Interface {
public:
    MOCK_METHOD(void, RegisterForNewRxValueNotification, (Named_Object_Callee_Interface* newCallee), (override));
    MOCK_METHOD(void, DeRegisterForNewRxValueNotification, (Named_Object_Callee_Interface* callee), (override));
    MOCK_METHOD(void, Call_Named_Object_Callback, (const String& name, void* object, const size_t changeCount), (override));
};

class MockHardwareSerial : public HardwareSerial {
public:
    MockHardwareSerial() : HardwareSerial(1) {} // Call base class constructor with dummy value
    virtual ~MockHardwareSerial(){}

    size_t write(uint8_t data) override {
        buffer += (char)data;
        return 1;
    }

    int available() override {
        return buffer.length();
    }

    int read() override {
        if (buffer.length() == 0) {
            return -1;
        }
        char c = buffer.charAt(0);
        buffer.remove(0, 1);
        return c;
    }

    int peek() override {
        if (buffer.length() == 0) {
            return -1;
        }
        return buffer.charAt(0);
    }

    void flush() override {
        buffer = "";
    }

    void print(const String& data) {
        for (char c : data) {
            write(c);
        }
    }

    void println(const String& data) {
        print(data);
        write('\n');
    }

private:
    String buffer;
};

// Mock class for DataSerializer
class MockDataSerializer : public DataSerializer {
public:
    MOCK_METHOD(void, SetDataSerializerDataItems, (DataItem_t& DataItem, size_t DataItemCount), (override));
    MOCK_METHOD(String, SerializeDataItemToJson, (String Name, DataType_t DataType, void* Object, size_t Count, size_t changeCount), (override));
    MOCK_METHOD(bool, DeSerializeJsonToNamedObject, (String json, NamedObject_t &NamedObject), (override));
    MOCK_METHOD(void, FailPercentage, (), (override));
    MOCK_METHOD(bool, AllTagsExist, (JSONVar &jsonObject), (override));
};

// Mock for SerialPortMessageManager, if needed, inheriting the mock caller interface
class MockSerialPortMessageManager : public SerialPortMessageManager {
public:
    MockSerialPortMessageManager()
                                : SerialPortMessageManager()
    {
        ESP_LOGD("MockSerialPortMessageManager", "Constructing MockSerialPortMessageManager");
    }
    virtual ~MockSerialPortMessageManager() override 
    {
        ESP_LOGD("MockSerialPortMessageManager", "Deleting MockSerialPortMessageManager");
    }
    
    MOCK_METHOD(void, SetupSerialPortMessageManager, (), (override));
    MOCK_METHOD(bool, QueueMessageFromDataType, (const String& Name, DataType_t DataType, void* Object, size_t Count, size_t changeCount), (override));
    MOCK_METHOD(bool, QueueMessage, (const String& message), (override));
    MOCK_METHOD(String, GetName, (), ());
    MOCK_METHOD(void, SerialPortMessageManager_RxTask, (), (override));
    MOCK_METHOD(void, SerialPortMessageManager_TxTask, (), (override));

    //Named_Object_Caller_Interface
    MOCK_METHOD(void, RegisterForNewRxValueNotification, (Named_Object_Callee_Interface* NewCallee), (override));	
    MOCK_METHOD(void, DeRegisterForNewRxValueNotification, (Named_Object_Callee_Interface* Callee), (override));
    MOCK_METHOD(void, Call_Named_Object_Callback, (const String& name, void* object, const size_t changeCount), (override));
};