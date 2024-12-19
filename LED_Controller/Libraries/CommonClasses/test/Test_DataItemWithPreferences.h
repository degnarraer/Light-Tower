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
#include "DataItem/DataItemWithPreferences.h"
#include "Mock_SetupCallInterface.h"
#include "Mock_ValidValueChecker.h"
#include "Mock_SerialPortMessageManager.h"
#include "Mock_Preferences.h"
#include "Mock_Callbacks.h"

using ::testing::_;
using ::testing::NotNull;
using ::testing::NiceMock;
using namespace testing;

// Test Fixture for DataItemWithPreferencesFunctionCallTests
class DataItemWithPreferencesFunctionCallTests : public Test
{
    protected:
        const int32_t initialValue = 10;
        const std::string initialValueString = "10";
        const std::string spmm = "Serial Port Message Manager";
        const std::string name = "Test Name";
        const std::string key1 = "key1";
        MockSetupCallerInterface *mp_MockSetupCaller;
        MockSerialPortMessageManager *mp_MockSerialPortMessageManager;
        MockNamedCallback *mp_mockNamedCallback;
        NiceMock<MockPreferences> *mp_mockPreferences;
        DataItemWithPreferences<int32_t, 1> *mp_DataItemWithPreferences;
        DataItemWithPreferencesFunctionCallTests(): mp_MockSetupCaller(nullptr)
                                                  , mp_MockSerialPortMessageManager(nullptr)
                                                  , mp_mockPreferences(nullptr)
                                                  , mp_DataItemWithPreferences(nullptr)
        {}
        void SetUp() override
        {
            ESP_LOGD("SetUp", "Setting Up");
            mp_MockSetupCaller = new MockSetupCallerInterface();
            mp_MockSerialPortMessageManager = new MockSerialPortMessageManager();
            mp_mockPreferences = new NiceMock<MockPreferences>();
            mp_mockNamedCallback = new MockNamedCallback(name, nullptr);
            ON_CALL(*mp_MockSerialPortMessageManager, QueueMessageFromDataType(_,_,_,_,_)).WillByDefault(Return(true));
            ON_CALL(*mp_MockSerialPortMessageManager, GetName()).WillByDefault(Return(spmm));
            ON_CALL(*mp_mockPreferences, isKey( StrEq(key1.c_str()) )).WillByDefault(Return(false));
            ON_CALL(*mp_mockPreferences, getString( StrEq(key1.c_str()), initialValueString )).WillByDefault(Return(initialValueString));
            ON_CALL(*mp_mockPreferences, putString( StrEq(key1.c_str()), initialValueString )).WillByDefault(Return(initialValueString.length()));
            ESP_LOGD("SetUp", "SetUp Complete");
        }
        void CreateDataItemWithPreferences(RxTxType_t rxTxType,  uint16_t rate)
        {
            EXPECT_CALL(*mp_MockSetupCaller, RegisterForSetupCall(NotNull())).Times(1);
            mp_DataItemWithPreferences = new DataItemWithPreferences<int32_t, 1>( name 
                                                                                , initialValue
                                                                                , rxTxType
                                                                                , rate
                                                                                , mp_mockPreferences
                                                                                , mp_MockSerialPortMessageManager
                                                                                , mp_mockNamedCallback
                                                                                , mp_MockSetupCaller );
            ::testing::Mock::VerifyAndClearExpectations(&mp_MockSetupCaller);
            EXPECT_CALL(*mp_MockSerialPortMessageManager, RegisterForNewRxValueNotification(mp_DataItemWithPreferences)).Times(1);
            if( rxTxType == RxTxType_Tx_Periodic || rxTxType == RxTxType_Tx_On_Change || rxTxType == RxTxType_Tx_On_Change_With_Heartbeat )
            {
                EXPECT_CALL(*mp_MockSerialPortMessageManager, QueueMessageFromDataType(name,_,_,_,_))
                    .Times(1)
                    .WillOnce(Return(true));
            }
            EXPECT_CALL(mockNamedCallback_Callback, NewValueCallbackFunction(name,_,_)).Times(1);
            mp_DataItemWithPreferences->Setup();
            ::testing::Mock::VerifyAndClearExpectations(&mp_MockSerialPortMessageManager);
        }
        void TearDown() override
        {
            ESP_LOGD("TearDown", "Tearing Down");
            DestroyDataItemWithPreferences();
            if (mp_mockNamedCallback)
            {
                delete mp_mockNamedCallback;
                mp_mockNamedCallback = nullptr;
            }
            if (mp_mockPreferences)
            {
                delete mp_mockPreferences;
                mp_mockPreferences = nullptr;
            }
            if (mp_MockSerialPortMessageManager)
            {
                delete mp_MockSerialPortMessageManager;
                mp_MockSerialPortMessageManager = nullptr;
            }
            if (mp_MockSetupCaller)
            {
                delete mp_MockSetupCaller;
                mp_MockSetupCaller = nullptr;
            }
            ESP_LOGD("TearDown", "TearDown Complete");
        }
        void DestroyDataItemWithPreferences()
        {
            if(mp_DataItemWithPreferences)
            {
                if(mp_DataItemWithPreferences)
                {   
                    EXPECT_CALL(*mp_MockSetupCaller, DeRegisterForSetupCall(mp_DataItemWithPreferences)).Times(1);
                    EXPECT_CALL(*mp_MockSerialPortMessageManager, DeRegisterForNewRxValueNotification(mp_DataItemWithPreferences)).Times(1);
                    delete mp_DataItemWithPreferences;
                    mp_DataItemWithPreferences = nullptr;
                    ::testing::Mock::VerifyAndClearExpectations(&mp_MockSetupCaller);
                    ::testing::Mock::VerifyAndClearExpectations(&mp_MockSerialPortMessageManager);
                }
            }
        }
        void TestSetupCallRegistration(RxTxType_t rxtxtype,  size_t rate)
        {
            CreateDataItemWithPreferences(rxtxtype, rate);
            DestroyDataItemWithPreferences();
        }
        void TestNewValueNotificationRegistration(RxTxType_t rxtxtype,  size_t rate)
        {
            CreateDataItemWithPreferences(rxtxtype, rate);
            DestroyDataItemWithPreferences();
        }
};

TEST_F(DataItemWithPreferencesFunctionCallTests, Registration_With_Setup_Caller)
{
    TestSetupCallRegistration(RxTxType_Tx_Periodic, 1000);
    TestSetupCallRegistration(RxTxType_Tx_On_Change_With_Heartbeat, 1000);
    TestSetupCallRegistration(RxTxType_Tx_On_Change, 0);
    TestSetupCallRegistration(RxTxType_Rx_Only, 0);
    TestSetupCallRegistration(RxTxType_Rx_Echo_Value, 0);
}

TEST_F(DataItemWithPreferencesFunctionCallTests, Registration_For_New_Value_Notification)
{
    TestNewValueNotificationRegistration(RxTxType_Tx_Periodic, 1000);
    TestNewValueNotificationRegistration(RxTxType_Tx_On_Change_With_Heartbeat, 1000);
    TestNewValueNotificationRegistration(RxTxType_Tx_On_Change, 0);
    TestNewValueNotificationRegistration(RxTxType_Rx_Only, 0);
    TestNewValueNotificationRegistration(RxTxType_Rx_Echo_Value, 0);
}

// Test Fixture for DataItemWithPreferences Rx Tx Tests
class DataItemWithPreferencesRxTxTests : public Test
                        , public SetupCallerInterface
{
    protected:
        const int32_t initialValue = 10;
        const std::string initialValueString = "10";
        const std::string name = "Test Name1";
        const std::string spmm = "Serial Port Message Manager";
        const std::string key1 = "key1";
        NiceMock<MockSerialPortMessageManager> *mp_MockSerialPortMessageManager;
        MockNamedCallback *mp_mockNamedCallback;
        NiceMock<MockPreferences> *mp_mockPreferences;
        DataItemWithPreferences<int32_t, 1> *mp_DataItemWithPreferences;
        DataItemWithPreferencesRxTxTests()
            : mp_MockSerialPortMessageManager(nullptr)
            , mp_DataItemWithPreferences(nullptr)
        {}

        void SetUp() override
        {
            mp_MockSerialPortMessageManager = new NiceMock<MockSerialPortMessageManager>();
            mp_mockPreferences = new NiceMock<MockPreferences>();
            ON_CALL(*mp_MockSerialPortMessageManager, QueueMessageFromDataType(_,_,_,_,_)).WillByDefault(Return(true));
            ON_CALL(*mp_MockSerialPortMessageManager, GetName()).WillByDefault(Return(spmm));
            ON_CALL(*mp_mockPreferences, isKey( StrEq(key1.c_str()) )).WillByDefault(Return(false));
            ON_CALL(*mp_mockPreferences, getString( StrEq(key1.c_str()), StrEq(initialValueString.c_str()) )).WillByDefault(Return(initialValueString));
            ON_CALL(*mp_mockPreferences, putString( StrEq(key1.c_str()), initialValueString )).WillByDefault(Return(initialValueString.length()));
        }

        void CreateDataItemWithPreferences(RxTxType_t rxTxType,  uint16_t rate)
        {
            mp_mockNamedCallback = new MockNamedCallback(name, nullptr);
            mp_DataItemWithPreferences = new DataItemWithPreferences<int32_t, 1>( name 
                                                                                , initialValue
                                                                                , rxTxType
                                                                                , rate
                                                                                , mp_mockPreferences
                                                                                , mp_MockSerialPortMessageManager
                                                                                , mp_mockNamedCallback
                                                                                , this );
            EXPECT_CALL(mockNamedCallback_Callback, NewValueCallbackFunction(name,_,_)).Times(1);
            SetupAllSetupCallees();
            ::testing::Mock::VerifyAndClearExpectations(&mockNamedCallback_Callback);
            ::testing::Mock::VerifyAndClearExpectations(&mp_MockSerialPortMessageManager);
        }

        void DestroyDataItemWithPreferences()
        {
            if(mp_DataItemWithPreferences)
            {
                delete mp_DataItemWithPreferences;
                mp_DataItemWithPreferences = nullptr;
            }
        }

        void TearDown() override
        {
            DestroyDataItemWithPreferences();
            if (mp_mockNamedCallback)
            {
                delete mp_mockNamedCallback;
                mp_mockNamedCallback = nullptr;
            }
            if (mp_mockPreferences)
            {
                delete mp_mockPreferences;
                mp_mockPreferences = nullptr;
            }
            if (mp_MockSerialPortMessageManager)
            {
                delete mp_MockSerialPortMessageManager;
                mp_MockSerialPortMessageManager = nullptr;
            }
        }
};

TEST_F(DataItemWithPreferencesRxTxTests, Tx_Called_Periodically)
{
    CreateDataItemWithPreferences(RxTxType_Tx_Periodic, 100);
    EXPECT_CALL(*mp_MockSerialPortMessageManager, QueueMessageFromDataType(name,_,_,_,_)).Times(10)
        .WillRepeatedly(Return(true));
    std::this_thread::sleep_for(std::chrono::milliseconds(1050));
    ::testing::Mock::VerifyAndClearExpectations(&mp_MockSerialPortMessageManager);
}

// Test Fixture for DataItemWithPreferencesGetAndSetValueTests
template <typename T, size_t COUNT>
class DataItemWithPreferencesGetAndSetValueTests : public Test, public SetupCallerInterface
{
protected:
    const ValidStringValues_t validValues = { "10", "20", "30" };
    const int32_t validValue10 = 10;
    const int32_t validValue20 = 20;
    const int32_t validValue30 = 30;
    const int32_t invalidValue = 40;
    const std::string validValue10String = std::to_string(validValue10);
    const std::string validValue20String = std::to_string(validValue20);
    const std::string validValue30String = std::to_string(validValue30);
    const std::string invalidValueString = std::to_string(invalidValue);
    const int32_t validValue10Array[10] = {10,10,10,10,10,10,10,10,10,10};
    const int32_t validValue20Array[10] = {20,20,20,20,20,20,20,20,20,20};
    const int32_t validValue30Array[10] = {30,30,30,30,30,30,30,30,30,30};
    const int32_t invalidValueArray[10] = {40,40,40,40,40,40,40,40,40,40};
    const std::string validValue10ArrayString = "10|10|10|10|10|10|10|10|10|10";
    const std::string validValue20ArrayString = "20|20|20|20|20|20|20|20|20|20";
    const std::string validValue30ArrayString = "30|30|30|30|30|30|30|30|30|30";
    const std::string invalidValueArrayString = "40|40|40|40|40|40|40|40|40|40";
    NiceMock<MockPreferences> *mp_mockPreferences;
    NiceMock<MockSerialPortMessageManager> *mp_MockSerialPortMessageManager;
    MockNamedCallback *mp_mockNamedCallback;
    DataItemWithPreferences<T, COUNT> *mp_DataItemWithPreferences;
    const std::string spmm = "Serial Port Message Manager";
    const std::string name1 = "Test Name1";
    const std::string name2 = "Test Name2";
    const std::string name3 = "Test Name3";
    const std::string name4 = "Test Name4";
    const String key1 = "key1";

    void SetUp() override
    {
        mp_MockSerialPortMessageManager = new NiceMock<MockSerialPortMessageManager>();
        mp_mockPreferences = new NiceMock<MockPreferences>();
        ON_CALL(*mp_MockSerialPortMessageManager, QueueMessageFromDataType(_,_,_,_,_)).WillByDefault(Return(true));
        ON_CALL(*mp_MockSerialPortMessageManager, GetName()).WillByDefault(Return(spmm));
        ON_CALL(*mp_mockPreferences, isKey( StrEq(key1.c_str()) )).WillByDefault(Return(false));
    }

    void CreateDataItemWithPreferences( const std::string name, int32_t initialValue, RxTxType_t rxTxType,  uint16_t rate, ValidStringValues_t *validStringValues )
    {
        mp_mockNamedCallback = new MockNamedCallback(name, nullptr);
        mp_DataItemWithPreferences = new DataItemWithPreferences<T, COUNT>( name, initialValue, rxTxType, rate, mp_mockPreferences, mp_MockSerialPortMessageManager, mp_mockNamedCallback, this, validStringValues);
        EXPECT_CALL(mockNamedCallback_Callback, NewValueCallbackFunction(name,_,_)).Times(1);
        SetupAllSetupCallees();
        ::testing::Mock::VerifyAndClearExpectations(&mockNamedCallback_Callback);
        ::testing::Mock::VerifyAndClearExpectations(&mp_MockSerialPortMessageManager);
    }

    void DestroyDataItemWithPreferences()
    {
        if(mp_DataItemWithPreferences)
        {
            delete mp_DataItemWithPreferences;
            mp_DataItemWithPreferences = nullptr;
        }
        if (mp_mockNamedCallback)
        {
            delete mp_mockNamedCallback;
            mp_mockNamedCallback = nullptr;
        }
    }

    void TearDown() override
    {
        DestroyDataItemWithPreferences();
        if (mp_mockPreferences)
        {
            delete mp_mockPreferences;
            mp_mockPreferences = nullptr;
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
                    EXPECT_CALL(*mp_MockSerialPortMessageManager, QueueMessageFromDataType(_,_,_,_,_)).Times(2).WillRepeatedly(Return(true));
                    ESP_LOGD( "SetRxTxCallExpectations", "EXPECT_CALL: QueueMessageFromDataType x 1");
                }
                else
                {
                    EXPECT_CALL(*mp_MockSerialPortMessageManager, QueueMessageFromDataType(_,_,_,_,_)).Times(1);
                    ESP_LOGD( "SetRxTxCallExpectations", "EXPECT_CALL: QueueMessageFromDataType x 0");
                }
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
                    EXPECT_CALL(*mp_MockSerialPortMessageManager, QueueMessageFromDataType(_,_,_,_,_)).Times(1).WillRepeatedly(Return(true));
                    ESP_LOGD( "SetRxTxCallExpectations", "EXPECT_CALL: QueueMessageFromDataType x 1");
                }
                else
                {
                    EXPECT_CALL(*mp_MockSerialPortMessageManager, QueueMessageFromDataType(_,_,_,_,_)).Times(0);
                    ESP_LOGD( "SetRxTxCallExpectations", "EXPECT_CALL: QueueMessageFromDataType x 0");
                }
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
                    EXPECT_CALL(*mp_MockSerialPortMessageManager, QueueMessageFromDataType(_,_,_,_,_)).Times(2).WillRepeatedly(Return(true));
                    ESP_LOGD( "SetRxTxCallExpectations", "EXPECT_CALL: QueueMessageFromDataType x 1");
                }
                else
                {
                    EXPECT_CALL(*mp_MockSerialPortMessageManager, QueueMessageFromDataType(_,_,_,_,_)).Times(1);
                    ESP_LOGD( "SetRxTxCallExpectations", "EXPECT_CALL: QueueMessageFromDataType x 0");
                }
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
                EXPECT_CALL(*mp_MockSerialPortMessageManager, QueueMessageFromDataType(_,_,_,_,_)).Times(0);
                ESP_LOGD( "SetRxTxCallExpectations", "EXPECT_CALL: QueueMessageFromDataType x 0");
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
                EXPECT_CALL(*mp_MockSerialPortMessageManager, QueueMessageFromDataType(_,_,_,_,_)).Times(0);
                ESP_LOGD( "SetRxTxCallExpectations", "EXPECT_CALL: QueueMessageFromDataType x 0");
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
    void TestNameIsSet( const std::string name, int32_t initialValue, std::string initialValueString, RxTxType_t rxTxType,  uint16_t rate, ValidStringValues_t *validStringValues )
    {
        EXPECT_CALL(*mp_mockPreferences, isKey( StrEq(name.c_str()) )).Times(1).WillOnce(Return(true));
        EXPECT_CALL(*mp_mockPreferences, getString( StrEq(name.c_str()), A<std::string>() )).Times(1).WillOnce(Return(initialValueString));
        CreateDataItemWithPreferences(name, initialValue, rxTxType, rate, validStringValues);
        EXPECT_STREQ(name.c_str(), mp_DataItemWithPreferences->GetName());
        ::testing::Mock::VerifyAndClearExpectations(&mp_mockPreferences);
    }

    void TestInitialValueIsSet( const std::string name, int32_t initialValue, std::string initialValueString, RxTxType_t rxTxType,  uint16_t rate, ValidStringValues_t *validStringValues )
    {
        EXPECT_CALL(*mp_mockPreferences, isKey( StrEq(name.c_str()) )).Times(1).WillOnce(Return(true));
        EXPECT_CALL(*mp_mockPreferences, getString( StrEq(name.c_str()), A<std::string>() )).Times(1).WillOnce(Return(initialValueString));
        CreateDataItemWithPreferences(name, initialValue, rxTxType, rate, validStringValues);
        for(size_t i = 0; i < mp_DataItemWithPreferences->GetCount(); ++i)
        {
            EXPECT_EQ(initialValue, mp_DataItemWithPreferences->GetValuePointer()[i]);
        }
        ::testing::Mock::VerifyAndClearExpectations(&mp_mockPreferences);
    }

    void TestInitialValueReturnedAsString( const std::string name, int32_t initialValue, std::string initialValueString, RxTxType_t rxTxType,  uint16_t rate, ValidStringValues_t *validStringValues )
    {
        EXPECT_CALL(*mp_mockPreferences, isKey( StrEq(name.c_str()) )).Times(1).WillOnce(Return(true));
        EXPECT_CALL(*mp_mockPreferences, getString( StrEq(name.c_str()), A<std::string>() )).Times(1).WillOnce(Return(initialValueString));
        CreateDataItemWithPreferences(name, validValue10, rxTxType, rate, validStringValues);
        EXPECT_STREQ(initialValueString.c_str(), mp_DataItemWithPreferences->GetInitialValueAsString().c_str());
        ::testing::Mock::VerifyAndClearExpectations(&mp_mockPreferences);
    }

    void TestSetValueFromValueConvertsToString( const int32_t* testValue, const std::string resultString, const std::string name, int32_t initialValue, std::string initialValueString, RxTxType_t rxTxType,  uint16_t rate, ValidStringValues_t *validStringValues )
    {
        EXPECT_CALL(*mp_mockPreferences, isKey( StrEq(name.c_str()) )).Times(1).WillOnce(Return(true));
        EXPECT_CALL(*mp_mockPreferences, getString( StrEq(name.c_str()), A<std::string>() )).Times(1).WillOnce(Return(initialValueString));
        CreateDataItemWithPreferences(name, initialValue, rxTxType, rate, validStringValues);
        SetRxTxCallExpectations(name, rxTxType, true);
        mp_DataItemWithPreferences->SetValue(testValue, COUNT);
        EXPECT_STREQ(resultString.c_str(), mp_DataItemWithPreferences->GetValueAsString().c_str());
        ::testing::Mock::VerifyAndClearExpectations(&mp_mockPreferences);
    }

    void TestSetValueFromStringConvertsToValue( const std::string testString, const int32_t* resultValue, const std::string name, int32_t initialValue, std::string initialValueString, RxTxType_t rxTxType,  uint16_t rate, ValidStringValues_t *validStringValues )
    {
        EXPECT_CALL(*mp_mockPreferences, isKey( StrEq(name.c_str()) )).Times(1).WillOnce(Return(true));
        EXPECT_CALL(*mp_mockPreferences, getString( StrEq(name.c_str()), A<std::string>() )).Times(1).WillOnce(Return(initialValueString));
        CreateDataItemWithPreferences(name, initialValue, rxTxType, rate, validStringValues);
        SetRxTxCallExpectations(name, rxTxType, true);
        mp_DataItemWithPreferences->SetValueFromString(testString);
        for(size_t i = 0; i < COUNT; ++i)
        {
            EXPECT_EQ(resultValue[i], mp_DataItemWithPreferences->GetValuePointer()[i]);
        }
        ::testing::Mock::VerifyAndClearExpectations(&mp_mockPreferences);
    }

    void TestSettingValue(const std::string name, const int32_t initialValue, std::string initialValueString, const int32_t* testValue, const std::string testValueString, bool expectEqual)
    {
        EXPECT_CALL(*mp_mockPreferences, isKey( StrEq(name.c_str()) )).Times(1).WillOnce(Return(true));
        EXPECT_CALL(*mp_mockPreferences, getString( StrEq(name.c_str()), A<std::string>() )).Times(1).WillOnce(Return(initialValueString));
        CreateDataItemWithPreferences(name, initialValue, RxTxType_Tx_On_Change, 0, &validValues);
        SetRxTxCallExpectations(name, RxTxType_Tx_On_Change, expectEqual);
        mp_DataItemWithPreferences->SetValue(testValue, COUNT);
        for(size_t i = 0; i < COUNT; ++i)
        {
            if(expectEqual)
            {
                EXPECT_EQ(testValue[i], mp_DataItemWithPreferences->GetValuePointer()[i]);
            }
            else
            {
                EXPECT_NE(testValue[i], mp_DataItemWithPreferences->GetValuePointer()[i]);
                EXPECT_EQ(initialValue, mp_DataItemWithPreferences->GetValuePointer()[i]);
            }
        }
        ::testing::Mock::VerifyAndClearExpectations(&mp_mockPreferences);
        mp_DataItemWithPreferences->SetValueFromString(testValueString);
        for(size_t i = 0; i < COUNT; ++i)
        {
            if(expectEqual)
            {
                EXPECT_EQ(testValue[i], mp_DataItemWithPreferences->GetValuePointer()[i]);
            }
            else
            {
                EXPECT_NE(testValue[i], mp_DataItemWithPreferences->GetValuePointer()[i]);
                EXPECT_EQ(initialValue, mp_DataItemWithPreferences->GetValuePointer()[i]);
            }
        }
    }
};

using DataItemWithPreferencesGetAndSetValueTestsInt1 = DataItemWithPreferencesGetAndSetValueTests<int32_t, 1>;
using DataItemWithPreferencesGetAndSetValueTestsInt10 = DataItemWithPreferencesGetAndSetValueTests<int32_t, 10>;

// ************ Name is set ******************
TEST_F(DataItemWithPreferencesGetAndSetValueTestsInt1, DataItemWithPreferences_Name_Is_Set)
{
    TestNameIsSet(name1, validValue10, validValue10String, RxTxType_Rx_Only, 0, nullptr);
}
TEST_F(DataItemWithPreferencesGetAndSetValueTestsInt10, DataItemWithPreferencesArray_Name_Is_Set)
{
    TestNameIsSet(name1, validValue10, validValue10ArrayString, RxTxType_Rx_Only, 0, nullptr);
}
TEST_F(DataItemWithPreferencesGetAndSetValueTestsInt1, DataItemWithPreferencesWithValidation_Name_Is_Set)
{
    TestNameIsSet(name1, validValue10, validValue10String, RxTxType_Rx_Only, 0, &validValues);
}

TEST_F(DataItemWithPreferencesGetAndSetValueTestsInt10, DataItemWithPreferencesArrayWithValidation_Name_Is_Set)
{
    TestNameIsSet(name1, validValue10, validValue10ArrayString, RxTxType_Rx_Only, 0, &validValues);
}


// ************ Initial Value is set ******************
TEST_F(DataItemWithPreferencesGetAndSetValueTestsInt1, DataItemWithPreferences_Initial_Value_Is_Set)
{
    TestInitialValueIsSet(name1, validValue10, validValue10String, RxTxType_Rx_Only, 0, nullptr);
}
TEST_F(DataItemWithPreferencesGetAndSetValueTestsInt10, DataItemWithPreferencesArray_Initial_Value_Is_Set)
{
    TestInitialValueIsSet(name1, validValue10, validValue10ArrayString, RxTxType_Rx_Only, 0, nullptr);
}
TEST_F(DataItemWithPreferencesGetAndSetValueTestsInt1, DataItemWithPreferencesWithValidation_Initial_Value_Is_Set)
{
    TestInitialValueIsSet(name1, validValue10, validValue10String, RxTxType_Rx_Only, 0, &validValues);
}
TEST_F(DataItemWithPreferencesGetAndSetValueTestsInt10, DataItemWithPreferencesArrayWithValidation_Initial_Value_Is_Set)
{
    TestInitialValueIsSet(name1, validValue10, validValue10ArrayString, RxTxType_Rx_Only, 0, &validValues);
}

// ************ Initial Value Returned as String ******************
TEST_F(DataItemWithPreferencesGetAndSetValueTestsInt1, DataItemWithPreferences_Initial_Value_Is_Returned_As_String)
{
    TestInitialValueReturnedAsString(name1, validValue10, validValue10String, RxTxType_Rx_Only, 0, nullptr);
}
TEST_F(DataItemWithPreferencesGetAndSetValueTestsInt10, DataItemWithPreferencesArray_Initial_Value_Is_Returned_As_String)
{
    TestInitialValueReturnedAsString(name1, validValue10, validValue10ArrayString, RxTxType_Rx_Only, 0, nullptr);
}
TEST_F(DataItemWithPreferencesGetAndSetValueTestsInt1, DataItemWithPreferencesWithValidation_Initial_Value_Is_Returned_As_String)
{
    TestInitialValueReturnedAsString(name1, validValue10, validValue10String, RxTxType_Rx_Only, 0, &validValues);
}
TEST_F(DataItemWithPreferencesGetAndSetValueTestsInt10, DataItemWithPreferencesArrayWithValidation_Initial_Value_Is_Returned_As_String)
{
    TestInitialValueReturnedAsString(name1, validValue10, validValue10ArrayString, RxTxType_Rx_Only, 0, &validValues);
}

// ************ Set Value From Value Converts To String ******************
TEST_F(DataItemWithPreferencesGetAndSetValueTestsInt1, DataItemWithPreferences_Set_Value_From_Value_Converts_To_String)
{
    TestSetValueFromValueConvertsToString(&validValue20, validValue20String, name1, validValue10, validValue10String, RxTxType_Tx_On_Change, 0, nullptr);
}
TEST_F(DataItemWithPreferencesGetAndSetValueTestsInt1, DataItemWithPreferencesWithValidation_Set_Value_From_Value_Converts_To_String)
{
    TestSetValueFromValueConvertsToString(&validValue20, validValue20String, name1, validValue10, validValue10String, RxTxType_Tx_On_Change, 0, &validValues);
}
TEST_F(DataItemWithPreferencesGetAndSetValueTestsInt10, DataItemWithPreferencesArray_Set_Value_From_Value_Converts_To_String)
{
    TestSetValueFromValueConvertsToString(validValue20Array, validValue20ArrayString, name1, validValue10, validValue10ArrayString, RxTxType_Tx_On_Change, 0, nullptr);
}
TEST_F(DataItemWithPreferencesGetAndSetValueTestsInt10, DataItemWithPreferencesArrayWithValidation_Set_Value_From_Value_Converts_To_String)
{
    TestSetValueFromValueConvertsToString(validValue20Array, validValue20ArrayString, name1, validValue10, validValue10ArrayString, RxTxType_Tx_On_Change, 0, &validValues);
}


// ************ Set Value From String Converts To Value ******************
TEST_F(DataItemWithPreferencesGetAndSetValueTestsInt1, DataItemWithPreferences_Set_Value_From_String_Converts_To_Value)
{
    TestSetValueFromStringConvertsToValue(validValue20String, &validValue20, name1, validValue10, validValue10String, RxTxType_Tx_On_Change, 0, nullptr);
}
TEST_F(DataItemWithPreferencesGetAndSetValueTestsInt1, DataItemWithPreferencesWithValidation_Set_Value_From_String_Converts_To_Value)
{
    TestSetValueFromStringConvertsToValue(validValue20String, &validValue20, name1, validValue10, validValue10String, RxTxType_Tx_On_Change, 0, &validValues);
}
TEST_F(DataItemWithPreferencesGetAndSetValueTestsInt10, DataItemWithPreferencesArray_Set_Value_From_String_Converts_To_Value)
{
    TestSetValueFromStringConvertsToValue(validValue20ArrayString, validValue20Array, name1, validValue10, validValue10ArrayString, RxTxType_Tx_On_Change, 0, nullptr);
}
TEST_F(DataItemWithPreferencesGetAndSetValueTestsInt10, DataItemWithPreferencesArrayWithValidation_Set_Value_From_String_Converts_To_Value)
{
    TestSetValueFromStringConvertsToValue(validValue20ArrayString, validValue20Array, name1, validValue10, validValue10ArrayString, RxTxType_Tx_On_Change, 0, &validValues);
}

TEST_F(DataItemWithPreferencesGetAndSetValueTestsInt1, DataItemWithPreferences_Set_Valid_Values_When_Validation_Is_Used)
{
    TestSettingValue(name1, validValue10, validValue10String, &validValue20, validValue20String, true);
}
TEST_F(DataItemWithPreferencesGetAndSetValueTestsInt1, DataItemWithPreferences_Reject_Invalid_Value_When_Validation_Is_Used)
{
    TestSettingValue(name1, validValue10, validValue10String, &invalidValue, invalidValueString, false);
}
TEST_F(DataItemWithPreferencesGetAndSetValueTestsInt10, DataItemWithPreferencesArray_Set_Valid_Array_Values_When_Validation_Is_Used)
{
    TestSettingValue(name1, validValue10, validValue10ArrayString, validValue20Array, validValue20ArrayString, true);
}
TEST_F(DataItemWithPreferencesGetAndSetValueTestsInt10, DataItemWithPreferencesArray_Reject_Invalid_Array_Values_When_Validation_Is_Used)
{
    TestSettingValue(name1, validValue10, validValue10ArrayString, invalidValueArray, invalidValueArrayString, false);
}

TEST_F(DataItemWithPreferencesGetAndSetValueTestsInt1, Change_Count_Changes_Properly_When_Validation_Used)
{
    CreateDataItemWithPreferences(name1, validValue10, RxTxType_Tx_On_Change, 0, &validValues);
    EXPECT_EQ(0, mp_DataItemWithPreferences->GetChangeCount());

    SetRxTxCallExpectations(name1, RxTxType_Tx_On_Change, true);
    mp_DataItemWithPreferences->SetValue(validValue20);
    EXPECT_EQ(1, mp_DataItemWithPreferences->GetChangeCount());
    ::testing::Mock::VerifyAndClearExpectations(&mp_MockSerialPortMessageManager);
    ::testing::Mock::VerifyAndClearExpectations(&mockNamedCallback_Callback);

    SetRxTxCallExpectations(name1, RxTxType_Tx_On_Change, false);
    mp_DataItemWithPreferences->SetValue(validValue20);
    EXPECT_EQ(1, mp_DataItemWithPreferences->GetChangeCount());
    ::testing::Mock::VerifyAndClearExpectations(&mp_MockSerialPortMessageManager);
    ::testing::Mock::VerifyAndClearExpectations(&mockNamedCallback_Callback);

    SetRxTxCallExpectations(name1, RxTxType_Tx_On_Change, false);
    mp_DataItemWithPreferences->SetValue(invalidValue);
    EXPECT_EQ(1, mp_DataItemWithPreferences->GetChangeCount());
    ::testing::Mock::VerifyAndClearExpectations(&mp_MockSerialPortMessageManager);
    ::testing::Mock::VerifyAndClearExpectations(&mockNamedCallback_Callback);

    SetRxTxCallExpectations(name1, RxTxType_Tx_On_Change, true);
    mp_DataItemWithPreferences->SetValue(validValue30);
    EXPECT_EQ(2, mp_DataItemWithPreferences->GetChangeCount());
    ::testing::Mock::VerifyAndClearExpectations(&mp_MockSerialPortMessageManager);
    ::testing::Mock::VerifyAndClearExpectations(&mockNamedCallback_Callback);
}

TEST_F(DataItemWithPreferencesGetAndSetValueTestsInt1, Change_Count_Changes_Properly_When_Validation_Not_Used)
{
    CreateDataItemWithPreferences(name1, validValue10, RxTxType_Tx_On_Change, 0, nullptr);
    EXPECT_EQ(0, mp_DataItemWithPreferences->GetChangeCount());
    
    SetRxTxCallExpectations(name1, RxTxType_Tx_On_Change, true);
    mp_DataItemWithPreferences->SetValue(validValue20);
    EXPECT_EQ(1, mp_DataItemWithPreferences->GetChangeCount());
    ::testing::Mock::VerifyAndClearExpectations(&mp_MockSerialPortMessageManager);
    ::testing::Mock::VerifyAndClearExpectations(&mockNamedCallback_Callback);
    
    SetRxTxCallExpectations(name1, RxTxType_Tx_On_Change, false);
    mp_DataItemWithPreferences->SetValue(validValue20);
    EXPECT_EQ(1, mp_DataItemWithPreferences->GetChangeCount());
    ::testing::Mock::VerifyAndClearExpectations(&mp_MockSerialPortMessageManager);
    ::testing::Mock::VerifyAndClearExpectations(&mockNamedCallback_Callback);
    
    SetRxTxCallExpectations(name1, RxTxType_Tx_On_Change, true);
    mp_DataItemWithPreferences->SetValue(invalidValue);
    EXPECT_EQ(2, mp_DataItemWithPreferences->GetChangeCount());
    ::testing::Mock::VerifyAndClearExpectations(&mp_MockSerialPortMessageManager);
    ::testing::Mock::VerifyAndClearExpectations(&mockNamedCallback_Callback);
    
    SetRxTxCallExpectations(name1, RxTxType_Tx_On_Change, true);
    mp_DataItemWithPreferences->SetValue(validValue30);
    EXPECT_EQ(3, mp_DataItemWithPreferences->GetChangeCount());
    ::testing::Mock::VerifyAndClearExpectations(&mp_MockSerialPortMessageManager);
    ::testing::Mock::VerifyAndClearExpectations(&mockNamedCallback_Callback);
}