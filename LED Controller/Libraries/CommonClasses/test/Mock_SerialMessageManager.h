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
#include "SerialMessageManager.h"

template <typename T>
class MockNewRxTxValueCalleeInterface : public NewRxTxValueCalleeInterface<T> {
public:
    MOCK_METHOD(bool, NewRxValueReceived, (T* object, size_t count), (override));
    MOCK_METHOD(String, GetName, (), (override));
    MOCK_METHOD(size_t, GetCount, (), (override));
};

// Mock class for NamedCallbackInterface
template <typename T>
class MockNamedCallbackInterface : public NamedCallbackInterface<T>
{
public:
    MOCK_METHOD(void, RegisterNamedCallback, (NamedCallback_t *namedCallback), (override));
    MOCK_METHOD(void, DeRegisterNamedCallback, (NamedCallback_t *namedCallback), (override));
    MOCK_METHOD(void, CallCallbacks, (const String& name, T* object), (override));
};

template <typename T>
class MockNewRxTxValueCallerInterface : public NewRxTxValueCallerInterface<T> {
public:
    MOCK_METHOD(void, SetNewTxValue, (const T* object, const size_t count), (override));
    MOCK_METHOD(void, RegisterForNewValueNotification, (NewRxTxValueCalleeInterface<T>* NewCallee), (override));
    MOCK_METHOD(void, DeRegisterForNewValueNotification, (NewRxTxValueCalleeInterface<T>* Callee), (override));
    MOCK_METHOD(void, NotifyCallee, (const String& name, T* object), (override));
};

class MockNewRxTxVoidObjectCalleeInterface : public NewRxTxVoidObjectCalleeInterface {
public:
    MockNewRxTxVoidObjectCalleeInterface(size_t Count) : NewRxTxVoidObjectCalleeInterface(Count) {}
    MOCK_METHOD(bool, NewRxValueReceived, (void* object, size_t Count), (override));
    MOCK_METHOD(String, GetName, (), (override));
    MOCK_METHOD(size_t, GetCount, (), (override));
};

class MockNewRxTxVoidObjectCallerInterface : public NewRxTxVoidObjectCallerInterface {
public:
    MOCK_METHOD(void, RegisterForNewValueNotification, (NewRxTxVoidObjectCalleeInterface* NewCallee), (override));
    MOCK_METHOD(void, DeRegisterForNewValueNotification, (NewRxTxVoidObjectCalleeInterface* Callee), (override));
    MOCK_METHOD(void, RegisterNamedCallback, (NamedCallback_t *NamedCallback), (override));
    MOCK_METHOD(void, DeRegisterNamedCallback, (NamedCallback_t *NamedCallback), (override));
    MOCK_METHOD(void, NotifyCallee, (const String& name, void* object), (override));
    MOCK_METHOD(void, CallCallbacks, (const String& name, void* object), (override));
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
    MOCK_METHOD(String, SerializeDataToJson, (String Name, DataType_t DataType, void* Object, size_t Count), (override));
    MOCK_METHOD(bool, DeSerializeJsonToNamedObject, (String json, NamedObject_t &NamedObject), (override));
    MOCK_METHOD(void, FailPercentage, (), (override));
    MOCK_METHOD(bool, AllTagsExist, (JSONVar &jsonObject), (override));
};

// Mock for SerialPortMessageManager, if needed, inheriting the mock caller interface
class MockSerialPortMessageManager : public SerialPortMessageManager {
public:
    MockSerialPortMessageManager(const String& Name, HardwareSerial &Serial, DataSerializer &DataSerializer, BaseType_t coreId = 1)
        : SerialPortMessageManager(Name, Serial, DataSerializer, coreId) {}
    virtual ~MockSerialPortMessageManager() override 
    {
        ESP_LOGD("MockSerialPortMessageManager", "Deleting MockSerialPortMessageManager");
    }
    
    MOCK_METHOD(void, SetupSerialPortMessageManager, (), (override));
    MOCK_METHOD(bool, QueueMessageFromData, (const String& Name, DataType_t DataType, void* Object, size_t Count), (override));
    MOCK_METHOD(bool, QueueMessage, (const String& message), (override));
    MOCK_METHOD(String, GetName, (), (override));
    MOCK_METHOD(void, SerialPortMessageManager_RxTask, (), (override));
    MOCK_METHOD(void, SerialPortMessageManager_TxTask, (), (override));

    //NewRxTxValueCallerInterface
    MOCK_METHOD(void, RegisterForNewValueNotification, (NewRxTxVoidObjectCalleeInterface* NewCallee), (override));	
    MOCK_METHOD(void, DeRegisterForNewValueNotification, (NewRxTxVoidObjectCalleeInterface* Callee), (override));
    MOCK_METHOD(void, RegisterNamedCallback, (NamedCallback_t* NamedCallback), (override));	
    MOCK_METHOD(void, DeRegisterNamedCallback, (NamedCallback_t* NamedCallback), (override));
    MOCK_METHOD(void, NotifyCallee, (const String& name, void* object), (override));
    MOCK_METHOD(void, CallCallbacks, (const String& name, void* object), (override));
};