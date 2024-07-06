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
#include "Mock_SerialMessageManager.h"
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
        const String spmm = "Serial Port Message Manager";
        const String name = "Test Name";
        NiceMock<MockSetupCallerInterface> *mp_MockSetupCaller;
        NiceMock<MockSerialPortMessageManager> *mp_MockSerialPortMessageManager;
        MockNamedCallback *mp_mockNamedCallback;
        MockHardwareSerial m_MockHardwareSerial;
        MockDataSerializer m_MockDataSerializer;
        MockPreferences *mp_mockPreferences;
        DataItemWithPreferences<int32_t, 1> *mp_DataItemWithPreferences;
        DataItemWithPreferencesFunctionCallTests(): mp_MockSetupCaller(nullptr)
                                                  , mp_MockSerialPortMessageManager(nullptr)
                                                  , mp_mockPreferences(nullptr)
                                                  , mp_DataItemWithPreferences(nullptr)
        {}
        void SetUp() override
        {
            ESP_LOGD("SetUp", "Setting Up");
            mp_MockSetupCaller = new NiceMock<MockSetupCallerInterface>();
            mp_MockSerialPortMessageManager = new NiceMock<MockSerialPortMessageManager>( name, m_MockHardwareSerial, m_MockDataSerializer, 0 );
            mp_mockPreferences = new MockPreferences();
            mp_mockNamedCallback = new MockNamedCallback(name, nullptr);
            ON_CALL(*mp_MockSerialPortMessageManager, QueueMessageFromData(_,_,_,_)).WillByDefault(Return(true));
            ON_CALL(*mp_MockSerialPortMessageManager, GetName()).WillByDefault(Return(spmm));
            ESP_LOGD("SetUp", "SetUp Complete");
        }
        void CreateDataItemWithPreferences(RxTxType_t rxTxType, UpdateStoreType_t updateStoreType, uint16_t rate)
        {
            EXPECT_CALL(*mp_MockSetupCaller, RegisterForSetupCall(NotNull())).Times(1);
            mp_DataItemWithPreferences = new DataItemWithPreferences<int32_t, 1>( name 
                                                                                , initialValue
                                                                                , rxTxType
                                                                                , updateStoreType
                                                                                , rate
                                                                                , mp_mockPreferences
                                                                                , mp_MockSerialPortMessageManager
                                                                                , mp_mockNamedCallback
                                                                                , mp_MockSetupCaller );
            EXPECT_CALL(*mp_MockSerialPortMessageManager, RegisterForNewValueNotification(mp_DataItemWithPreferences)).Times(1);
            mp_DataItemWithPreferences->Setup();
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
                EXPECT_CALL(*mp_MockSetupCaller, DeRegisterForSetupCall(mp_DataItemWithPreferences)).Times(1);
                EXPECT_CALL(*mp_MockSerialPortMessageManager, DeRegisterForNewValueNotification(mp_DataItemWithPreferences)).Times(1);
                delete mp_DataItemWithPreferences;
                mp_DataItemWithPreferences = nullptr;
            }
        }
        void TestSetupCallRegistration(RxTxType_t rxtxtype)
        {
            CreateDataItemWithPreferences(rxtxtype, UpdateStoreType_On_Rx, 1000);
            DestroyDataItemWithPreferences();
        }
        void TestNewValueNotificationRegistration(RxTxType_t rxtxtype)
        {
            CreateDataItemWithPreferences(rxtxtype, UpdateStoreType_On_Rx, 1000);
            DestroyDataItemWithPreferences();
        }
};

TEST_F(DataItemWithPreferencesFunctionCallTests, Registration_With_Setup_Caller)
{
    TestSetupCallRegistration(RxTxType_Tx_Periodic);
    TestSetupCallRegistration(RxTxType_Tx_On_Change_With_Heartbeat);
    TestSetupCallRegistration(RxTxType_Tx_On_Change);
    TestSetupCallRegistration(RxTxType_Rx_Only);
    TestSetupCallRegistration(RxTxType_Rx_Echo_Value);
}

TEST_F(DataItemWithPreferencesFunctionCallTests, Registration_For_New_Value_Notification)
{
    TestNewValueNotificationRegistration(RxTxType_Tx_Periodic);
    TestNewValueNotificationRegistration(RxTxType_Tx_On_Change_With_Heartbeat);
    TestNewValueNotificationRegistration(RxTxType_Tx_On_Change);
    TestNewValueNotificationRegistration(RxTxType_Rx_Only);
    TestNewValueNotificationRegistration(RxTxType_Rx_Echo_Value);
}

// Test Fixture for DataItemWithPreferences Rx Tx Tests
class DataItemWithPreferencesRxTxTests : public Test
                        , public SetupCallerInterface
{
    protected:
        const int32_t initialValue = 10;
        const String name = "Test Name1";
        const String spmm = "Serial Port Message Manager";
        MockHardwareSerial m_MockHardwareSerial;
        MockDataSerializer m_MockDataSerializer;
        NiceMock<MockSerialPortMessageManager> *mp_MockSerialPortMessageManager;
        MockNamedCallback *mp_mockNamedCallback;
        MockPreferences *mp_mockPreferences;
        DataItemWithPreferences<int32_t, 1> *mp_DataItemWithPreferences;
        DataItemWithPreferencesRxTxTests()
            : mp_MockSerialPortMessageManager(nullptr)
            , mp_DataItemWithPreferences(nullptr)
        {}

        void SetUp() override
        {
            mp_MockSerialPortMessageManager = new NiceMock<MockSerialPortMessageManager>( name, m_MockHardwareSerial, m_MockDataSerializer, 0 );
            mp_mockPreferences = new MockPreferences();
            ON_CALL(*mp_MockSerialPortMessageManager, GetName()).WillByDefault(Return(spmm));
        }

        void CreateDataItemWithPreferences(RxTxType_t rxTxType, UpdateStoreType_t updateStoreType, uint16_t rate)
        {
            mp_mockNamedCallback = new MockNamedCallback(name, nullptr);
            mp_DataItemWithPreferences = new DataItemWithPreferences<int32_t, 1>( name 
                                                                                , initialValue
                                                                                , rxTxType
                                                                                , updateStoreType
                                                                                , rate
                                                                                , mp_mockPreferences
                                                                                , mp_MockSerialPortMessageManager
                                                                                , mp_mockNamedCallback
                                                                                , this );
            SetupAllSetupCallees();
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
    EXPECT_CALL(*mp_MockSerialPortMessageManager, QueueMessageFromData(_,_,_,_)).Times(10)
        .WillRepeatedly(Return(true));
    CreateDataItemWithPreferences(RxTxType_Tx_Periodic, UpdateStoreType_On_Rx, 100);
    std::this_thread::sleep_for(std::chrono::milliseconds(1050));
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
    const String validValue10String = String(validValue10);
    const String validValue20String = String(validValue20);
    const String validValue30String = String(validValue30);
    const String invalidValueString = String(invalidValue);
    const int32_t validValue10Array[10] = {10,10,10,10,10,10,10,10,10,10};
    const int32_t validValue20Array[10] = {20,20,20,20,20,20,20,20,20,20};
    const int32_t validValue30Array[10] = {30,30,30,30,30,30,30,30,30,30};
    const int32_t invalidValueArray[10] = {40,40,40,40,40,40,40,40,40,40};
    const String validValue10ArrayString = "10|10|10|10|10|10|10|10|10|10";
    const String validValue20ArrayString = "20|20|20|20|20|20|20|20|20|20";
    const String validValue30ArrayString = "30|30|30|30|30|30|30|30|30|30";
    const String invalidValueArrayString = "40|40|40|40|40|40|40|40|40|40";
    MockHardwareSerial m_MockHardwareSerial;
    MockDataSerializer m_MockDataSerializer;
    MockPreferences *mp_mockPreferences;
    NiceMock<MockSerialPortMessageManager> *mp_MockSerialPortMessageManager;
    MockNamedCallback *mp_mockNamedCallback;
    DataItemWithPreferences<T, COUNT> *mp_DataItemWithPreferences;
    const String spmm = "Serial Port Message Manager";
    const String name1 = "Test Name1";
    const String name2 = "Test Name2";
    const String name3 = "Test Name3";
    const String name4 = "Test Name4";

    void SetUp() override
    {
        mp_MockSerialPortMessageManager = new NiceMock<MockSerialPortMessageManager>( spmm, m_MockHardwareSerial, m_MockDataSerializer, 0 );
        mp_mockPreferences = new MockPreferences();
        EXPECT_CALL(*mp_MockSerialPortMessageManager, GetName()).WillRepeatedly(Return(spmm));
    }

    void CreateDataItemWithPreferences( const String name, int32_t initialValue, RxTxType_t rxTxType, UpdateStoreType_t updateStoreType, uint16_t rate, ValidStringValues_t *validStringValues )
    {
        mp_mockNamedCallback = new MockNamedCallback(name, nullptr);
        mp_DataItemWithPreferences = new DataItemWithPreferences<T, COUNT>( name, initialValue, rxTxType, updateStoreType, rate, mp_mockPreferences, mp_MockSerialPortMessageManager, mp_mockNamedCallback, this, validStringValues);
        SetupAllSetupCallees();
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

    void TestNameIsSet( const String name, int32_t initialValue, String initialValueString, RxTxType_t rxTxType, UpdateStoreType_t updateStoreType, uint16_t rate, ValidStringValues_t *validStringValues )
    {
        EXPECT_CALL(*mp_mockPreferences, isKey( StrEq(name.c_str()) )).Times(1).WillOnce(Return(true));
        EXPECT_CALL(*mp_mockPreferences, getString( StrEq(name.c_str()), A<String>() )).Times(1).WillOnce(Return(initialValueString));
        CreateDataItemWithPreferences(name, initialValue, rxTxType, updateStoreType, rate, validStringValues);
        EXPECT_STREQ(name.c_str(), mp_DataItemWithPreferences->GetName().c_str());
    }

    void TestInitialValueIsSet( const String name, int32_t initialValue, String initialValueString, RxTxType_t rxTxType, UpdateStoreType_t updateStoreType, uint16_t rate, ValidStringValues_t *validStringValues )
    {
        EXPECT_CALL(*mp_mockPreferences, isKey( StrEq(name.c_str()) )).Times(1).WillOnce(Return(true));
        EXPECT_CALL(*mp_mockPreferences, getString( StrEq(name.c_str()), A<String>() )).Times(1).WillOnce(Return(initialValueString));
        CreateDataItemWithPreferences(name, initialValue, rxTxType, updateStoreType, rate, validStringValues);
        for(size_t i = 0; i < mp_DataItemWithPreferences->GetCount(); ++i)
        {
            EXPECT_EQ(initialValue, mp_DataItemWithPreferences->GetValuePointer()[i]);
        }
    }

    void TestInitialValueReturnedAsString( const String name, int32_t initialValue, String initialValueString, RxTxType_t rxTxType, UpdateStoreType_t updateStoreType, uint16_t rate, ValidStringValues_t *validStringValues )
    {
        EXPECT_CALL(*mp_mockPreferences, isKey( StrEq(name.c_str()) )).Times(1).WillOnce(Return(true));
        EXPECT_CALL(*mp_mockPreferences, getString( StrEq(name.c_str()), A<String>() )).Times(1).WillOnce(Return(initialValueString));
        CreateDataItemWithPreferences(name, validValue10, rxTxType, updateStoreType, rate, validStringValues);
        EXPECT_STREQ(initialValueString.c_str(), mp_DataItemWithPreferences->GetInitialValueAsString().c_str());
    }

    void TestSetValueFromValueConvertsToString( const int32_t* testValue, const String resultString, const String name, int32_t initialValue, String initialValueString, RxTxType_t rxTxType, UpdateStoreType_t updateStoreType, uint16_t rate, ValidStringValues_t *validStringValues )
    {
        EXPECT_CALL(*mp_mockPreferences, isKey( StrEq(name.c_str()) )).Times(1).WillOnce(Return(true));
        EXPECT_CALL(*mp_mockPreferences, getString( StrEq(name.c_str()), A<String>() )).Times(1).WillOnce(Return(initialValueString));
        CreateDataItemWithPreferences(name, initialValue, rxTxType, updateStoreType, rate, validStringValues);
        mp_DataItemWithPreferences->SetValue(testValue, COUNT);
        EXPECT_STREQ(resultString.c_str(), mp_DataItemWithPreferences->GetValueAsString().c_str());
    }

    void TestSetValueFromStringConvertsToValue( const String testString, const int32_t* resultValue, const String name, int32_t initialValue, String initialValueString, RxTxType_t rxTxType, UpdateStoreType_t updateStoreType, uint16_t rate, ValidStringValues_t *validStringValues )
    {
        EXPECT_CALL(*mp_mockPreferences, isKey( StrEq(name.c_str()) )).Times(1).WillOnce(Return(true));
        EXPECT_CALL(*mp_mockPreferences, getString( StrEq(name.c_str()), A<String>() )).Times(1).WillOnce(Return(initialValueString));
        CreateDataItemWithPreferences(name, initialValue, rxTxType, updateStoreType, rate, validStringValues);
        mp_DataItemWithPreferences->SetValueFromString(testString);
        for(size_t i = 0; i < COUNT; ++i)
        {
            EXPECT_EQ(resultValue[i], mp_DataItemWithPreferences->GetValuePointer()[i]);
        }
    }

    void TestSettingValue(const String name, const int32_t initialValue, String initialValueString, const int32_t* testValue, const String testValueString, bool expectEqual)
    {
        EXPECT_CALL(*mp_mockPreferences, isKey( StrEq(name.c_str()) )).Times(1).WillOnce(Return(true));
        EXPECT_CALL(*mp_mockPreferences, getString( StrEq(name.c_str()), A<String>() )).Times(1).WillOnce(Return(initialValueString));
        CreateDataItemWithPreferences(name, initialValue, RxTxType_Rx_Only, UpdateStoreType_On_Rx, 0, &validValues);
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
    TestNameIsSet(name1, validValue10, validValue10String, RxTxType_Rx_Only, UpdateStoreType_On_Rx, 0, nullptr);
}
TEST_F(DataItemWithPreferencesGetAndSetValueTestsInt10, DataItemWithPreferencesArray_Name_Is_Set)
{
    TestNameIsSet(name1, validValue10, validValue10ArrayString, RxTxType_Rx_Only, UpdateStoreType_On_Rx, 0, nullptr);
}
TEST_F(DataItemWithPreferencesGetAndSetValueTestsInt1, DataItemWithPreferencesWithValidation_Name_Is_Set)
{
    TestNameIsSet(name1, validValue10, validValue10String, RxTxType_Rx_Only, UpdateStoreType_On_Rx, 0, &validValues);
}

TEST_F(DataItemWithPreferencesGetAndSetValueTestsInt10, DataItemWithPreferencesArrayWithValidation_Name_Is_Set)
{
    TestNameIsSet(name1, validValue10, validValue10ArrayString, RxTxType_Rx_Only, UpdateStoreType_On_Rx, 0, &validValues);
}


// ************ Initial Value is set ******************
TEST_F(DataItemWithPreferencesGetAndSetValueTestsInt1, DataItemWithPreferences_Initial_Value_Is_Set)
{
    TestInitialValueIsSet(name1, validValue10, validValue10String, RxTxType_Rx_Only, UpdateStoreType_On_Rx, 0, nullptr);
}
TEST_F(DataItemWithPreferencesGetAndSetValueTestsInt10, DataItemWithPreferencesArray_Initial_Value_Is_Set)
{
    TestInitialValueIsSet(name1, validValue10, validValue10ArrayString, RxTxType_Rx_Only, UpdateStoreType_On_Rx, 0, nullptr);
}
TEST_F(DataItemWithPreferencesGetAndSetValueTestsInt1, DataItemWithPreferencesWithValidation_Initial_Value_Is_Set)
{
    TestInitialValueIsSet(name1, validValue10, validValue10String, RxTxType_Rx_Only, UpdateStoreType_On_Rx, 0, &validValues);
}
TEST_F(DataItemWithPreferencesGetAndSetValueTestsInt10, DataItemWithPreferencesArrayWithValidation_Initial_Value_Is_Set)
{
    TestInitialValueIsSet(name1, validValue10, validValue10ArrayString, RxTxType_Rx_Only, UpdateStoreType_On_Rx, 0, &validValues);
}

// ************ Initial Value Returned as String ******************
TEST_F(DataItemWithPreferencesGetAndSetValueTestsInt1, DataItemWithPreferences_Initial_Value_Is_Returned_As_String)
{
    TestInitialValueReturnedAsString(name1, validValue10, validValue10String, RxTxType_Rx_Only, UpdateStoreType_On_Rx, 0, nullptr);
}
TEST_F(DataItemWithPreferencesGetAndSetValueTestsInt10, DataItemWithPreferencesArray_Initial_Value_Is_Returned_As_String)
{
    TestInitialValueReturnedAsString(name1, validValue10, validValue10ArrayString, RxTxType_Rx_Only, UpdateStoreType_On_Rx, 0, nullptr);
}
TEST_F(DataItemWithPreferencesGetAndSetValueTestsInt1, DataItemWithPreferencesWithValidation_Initial_Value_Is_Returned_As_String)
{
    TestInitialValueReturnedAsString(name1, validValue10, validValue10String, RxTxType_Rx_Only, UpdateStoreType_On_Rx, 0, &validValues);
}
TEST_F(DataItemWithPreferencesGetAndSetValueTestsInt10, DataItemWithPreferencesArrayWithValidation_Initial_Value_Is_Returned_As_String)
{
    TestInitialValueReturnedAsString(name1, validValue10, validValue10ArrayString, RxTxType_Rx_Only, UpdateStoreType_On_Rx, 0, &validValues);
}

// ************ Set Value From Value Converts To String ******************
TEST_F(DataItemWithPreferencesGetAndSetValueTestsInt1, DataItemWithPreferences_Set_Value_From_Value_Converts_To_String)
{
    TestSetValueFromValueConvertsToString(&validValue20, validValue20String, name1, validValue10, validValue10String, RxTxType_Rx_Only, UpdateStoreType_On_Rx, 0, nullptr);
}
TEST_F(DataItemWithPreferencesGetAndSetValueTestsInt1, DataItemWithPreferencesWithValidation_Set_Value_From_Value_Converts_To_String)
{
    TestSetValueFromValueConvertsToString(&validValue20, validValue20String, name1, validValue10, validValue10String, RxTxType_Rx_Only, UpdateStoreType_On_Rx, 0, &validValues);
}
TEST_F(DataItemWithPreferencesGetAndSetValueTestsInt10, DataItemWithPreferencesArray_Set_Value_From_Value_Converts_To_String)
{
    TestSetValueFromValueConvertsToString(validValue20Array, validValue20ArrayString, name1, validValue10, validValue10ArrayString, RxTxType_Rx_Only, UpdateStoreType_On_Rx, 0, nullptr);
}
TEST_F(DataItemWithPreferencesGetAndSetValueTestsInt10, DataItemWithPreferencesArrayWithValidation_Set_Value_From_Value_Converts_To_String)
{
    TestSetValueFromValueConvertsToString(validValue20Array, validValue20ArrayString, name1, validValue10, validValue10ArrayString, RxTxType_Rx_Only, UpdateStoreType_On_Rx, 0, &validValues);
}


// ************ Set Value From String Converts To Value ******************
TEST_F(DataItemWithPreferencesGetAndSetValueTestsInt1, DataItemWithPreferences_Set_Value_From_String_Converts_To_Value)
{
    TestSetValueFromStringConvertsToValue(validValue20String, &validValue20, name1, validValue10, validValue10String, RxTxType_Rx_Only, UpdateStoreType_On_Rx, 0, nullptr);
}
TEST_F(DataItemWithPreferencesGetAndSetValueTestsInt1, DataItemWithPreferencesWithValidation_Set_Value_From_String_Converts_To_Value)
{
    TestSetValueFromStringConvertsToValue(validValue20String, &validValue20, name1, validValue10, validValue10String, RxTxType_Rx_Only, UpdateStoreType_On_Rx, 0, &validValues);
}
TEST_F(DataItemWithPreferencesGetAndSetValueTestsInt10, DataItemWithPreferencesArray_Set_Value_From_String_Converts_To_Value)
{
    TestSetValueFromStringConvertsToValue(validValue20ArrayString, validValue20Array, name1, validValue10, validValue10ArrayString, RxTxType_Rx_Only, UpdateStoreType_On_Rx, 0, nullptr);
}
TEST_F(DataItemWithPreferencesGetAndSetValueTestsInt10, DataItemWithPreferencesArrayWithValidation_Set_Value_From_String_Converts_To_Value)
{
    TestSetValueFromStringConvertsToValue(validValue20ArrayString, validValue20Array, name1, validValue10, validValue10ArrayString, RxTxType_Rx_Only, UpdateStoreType_On_Rx, 0, &validValues);
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
    CreateDataItemWithPreferences(name1, validValue10, RxTxType_Rx_Only, UpdateStoreType_On_Rx, 0, &validValues);
    EXPECT_EQ(0, mp_DataItemWithPreferences->GetChangeCount());
    mp_DataItemWithPreferences->SetValue(validValue20);
    EXPECT_EQ(1, mp_DataItemWithPreferences->GetChangeCount());
    mp_DataItemWithPreferences->SetValue(validValue20);
    EXPECT_EQ(1, mp_DataItemWithPreferences->GetChangeCount());
    mp_DataItemWithPreferences->SetValue(invalidValue);
    EXPECT_EQ(1, mp_DataItemWithPreferences->GetChangeCount());
    mp_DataItemWithPreferences->SetValue(validValue30);
    EXPECT_EQ(2, mp_DataItemWithPreferences->GetChangeCount());
}

TEST_F(DataItemWithPreferencesGetAndSetValueTestsInt1, Change_Count_Changes_Properly_When_Validation_Not_Used)
{
    CreateDataItemWithPreferences(name1, validValue10, RxTxType_Rx_Only, UpdateStoreType_On_Rx, 0, nullptr);
    EXPECT_EQ(0, mp_DataItemWithPreferences->GetChangeCount());
    mp_DataItemWithPreferences->SetValue(validValue20);
    EXPECT_EQ(1, mp_DataItemWithPreferences->GetChangeCount());
    mp_DataItemWithPreferences->SetValue(validValue20);
    EXPECT_EQ(1, mp_DataItemWithPreferences->GetChangeCount());
    mp_DataItemWithPreferences->SetValue(invalidValue);
    EXPECT_EQ(2, mp_DataItemWithPreferences->GetChangeCount());
    mp_DataItemWithPreferences->SetValue(validValue30);
    EXPECT_EQ(3, mp_DataItemWithPreferences->GetChangeCount());
}