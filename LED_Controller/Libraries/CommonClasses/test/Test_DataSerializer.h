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

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "DataSerializer.h"
#include "Mock_DataItem.h"

TEST(DataSerializerInstantiation, Instantion_Destruction)
{
    DataSerializer *dataSerializer = new DataSerializer();
    delete dataSerializer;
}

using ::testing::_;
using ::testing::NotNull;
using ::testing::NiceMock;
using namespace testing;
// Test Fixture for DataSerializer Parse Failure Tests
class DataSerializerParsingFailureTests : public Test
{
    protected:
        const String name = "Test Name";
        DataSerializer *mp_dataSerializer;
        void SetUp() override
        {
            mp_dataSerializer = new DataSerializer();
        }
        void TearDown() override 
        {
            delete mp_dataSerializer;
        }
        void TestParseFailure(std::string jsonString)
        {
            NamedObject_t namedObject;
            EXPECT_EQ(false, mp_dataSerializer->DeSerializeJsonToNamedObject(jsonString, namedObject));
        }
};

TEST_F(DataSerializerParsingFailureTests, Data_Serializer_Parse_Mal_Formed_Failure_Test)
{
    // Correct JSON: "{\"Name\":\"Object Name\",\"Count\":1,\"Type\":\"Bool_t\",\"Bytes\":1,\"Data\":[\"01\"],\"Sum\":1}"
    TestParseFailure("\"Name\":\"Object Name\",\"Count\":1,\"Type\":\"Bool_t\",\"Bytes\":1,\"Data\":[\"01\"],\"Sum\":1}");
    TestParseFailure("{\"Name\":\"Object Name\",\"Count\":1,\"Type\":\"Bool_t\",\"Bytes\":1,\"Data\":[\"01\"],\"Sum\":1");
    TestParseFailure("{\"Name\":\"Object Name\",\"Count\":1,\"Type\":\"Bool_t\",\"Bytes\":1,\"Data\":\"01\"],\"Sum\":1}");
    TestParseFailure("{\"Name\":\"Object Name\",\"Count\":1,\"Type\":\"Bool_t\",\"Bytes\":1,\"Data\":[\"01\",\"Sum\":1}");
    TestParseFailure("{\"Name\"\"Object Name\",\"Count\":1,\"Type\":\"Bool_t\",\"Bytes\":1,\"Data\":[\"01\",\"Sum\":1}");
    TestParseFailure("{\"Name\":\"Object Name\"\"Count\":1,\"Type\":\"Bool_t\",\"Bytes\":1,\"Data\":[\"01\"],\"Sum\":1}");
    TestParseFailure("{\"Name\":\"Object Name\",\"Count\"1,\"Type\":\"Bool_t\",\"Bytes\":1,\"Data\":[\"01\"],\"Sum\":1}");
    TestParseFailure("{\"Name\":\"Object Name\",\"Count\":1\"Type\":\"Bool_t\",\"Bytes\":1,\"Data\":[\"01\"],\"Sum\":1}");
    TestParseFailure("{\"Name\":\"Object Name\",\"Count\":1,\"Type\"\"Bool_t\",\"Bytes\":1,\"Data\":[\"01\"],\"Sum\":1}");
    TestParseFailure("{\"Name\":\"Object Name\",\"Count\":1,\"Type\":\"Bool_t\"\"Bytes\":1,\"Data\":[\"01\"],\"Sum\":1}");
    TestParseFailure("{\"Name\":\"Object Name\",\"Count\":1,\"Type\":\"Bool_t\",\"Bytes\"1,\"Data\":[\"01\"],\"Sum\":1}");
    TestParseFailure("{\"Name\":\"Object Name\",\"Count\":1,\"Type\":\"Bool_t\",\"Bytes\":1\"Data\":[\"01\"],\"Sum\":1}");
    TestParseFailure("{\"Name\":\"Object Name\",\"Count\":1,\"Type\":\"Bool_t\",\"Bytes\":1,\"Data\"[\"01\"],\"Sum\":1}");
    TestParseFailure("{\"Name\":\"Object Name\",\"Count\":1,\"Type\":\"Bool_t\",\"Bytes\":1,\"Data\":[\"01\"]\"Sum\":1}");
    TestParseFailure("{\"Name\":\"Object Name\",\"Count\":1,\"Type\":\"Bool_t\",\"Bytes\":1,\"Data\":[\"01\"],\"Sum\"1}");
    TestParseFailure("");
}

TEST_F(DataSerializerParsingFailureTests, Data_Serializer_Parse_Tags_Failure_Test)
{
    // Correct JSON: "{\"Name\":\"Object Name\",\"Count\":1,\"Type\":\"Bool_t\",\"Bytes\":1,\"Data\":[\"01\"],\"Sum\":1}"
    TestParseFailure("{\"Nam\":\"Object Name\",\"Count\":1,\"Type\":\"Bool_t\",\"Bytes\":1,\"Data\":[\"01\"],\"Sum\":1}");
    TestParseFailure("{\"Name\":\"Object Name\",\"Coun\":1,\"Type\":\"Bool_t\",\"Bytes\":1,\"Data\":[\"01\"],\"Sum\":1}");
    TestParseFailure("{\"Name\":\"Object Name\",\"Count\":1,\"Typ\":\"Bool_t\",\"Bytes\":1,\"Data\":[\"01\"],\"Sum\":1}");
    TestParseFailure("{\"Name\":\"Object Name\",\"Count\":1,\"Type\":\"Bool_t\",\"Byte\":1,\"Data\":[\"01\"],\"Sum\":1}");
    TestParseFailure("{\"Name\":\"Object Name\",\"Count\":1,\"Type\":\"Bool_t\",\"Bytes\":1,\"Dat\":[\"01\"],\"Sum\":1}");
    TestParseFailure("{\"Name\":\"Object Name\",\"Count\":1,\"Type\":\"Bool_t\",\"Bytes\":1,\"Data\":[\"01\"],\"Su\":1}");
}

TEST_F(DataSerializerParsingFailureTests, Data_Serializer_Parse_Checksum_Failure_Test)
{
    // Correct JSON: "{\"Name\":\"Object Name\",\"Count\":1,\"Type\":\"Bool_t\",\"Bytes\":1,\"Data\":[\"01\"],\"Sum\":1}"
    TestParseFailure("{\"Name\":\"Object Name\",\"Count\":1,\"Type\":\"Bool_t\",\"Bytes\":1,\"Data\":[\"01\"],\"Sum\":2}");
}

TEST_F(DataSerializerParsingFailureTests, Data_Serializer_Parse_Byte_Count_Failure_Test)
{
    // Correct JSON: "{\"Name\":\"Object Name\",\"Count\":1,\"Type\":\"Bool_t\",\"Bytes\":1,\"Data\":[\"01\"],\"Sum\":1}"
    TestParseFailure("{\"Name\":\"Object Name\",\"Count\":1,\"Type\":\"Bool_t\",\"Bytes\":2,\"Data\":[\"01\"],\"Sum\":1}");
}

TEST_F(DataSerializerParsingFailureTests, Data_Serializer_Parse_Data_Type_Failure_Test)
{
    // Correct JSON: "{\"Name\":\"Object Name\",\"Count\":1,\"Type\":\"Bool_t\",\"Bytes\":1,\"Data\":[\"01\"],\"Sum\":1}"
    TestParseFailure("{\"Name\":\"Object Name\",\"Count\":1,\"Type\":\"Bol_t\",\"Bytes\":2,\"Data\":[\"01\"],\"Sum\":1}");
}


// Test Fixture for DataSerializerTests
class DataSerializerTests : public Test
{
    protected:
        const String name = "Test Name";
        DataSerializer *mp_dataSerializer;
        void SetUp() override
        {
            mp_dataSerializer = new DataSerializer();
        }
        void TearDown() override 
        {
            delete mp_dataSerializer;
        }
};

TEST_F(DataSerializerTests, Data_Serializer_Serializes_Deserializes_Bool_t_Correctly)
{
    std::string objectName = "Object Name";
    bool testValue = true;
    NamedObject_t namedObject;
    std::string jsonString = mp_dataSerializer->SerializeDataItemToJson(objectName, DataType_Bool_t, &testValue, 1, 0);
    EXPECT_EQ(true, mp_dataSerializer->DeSerializeJsonToNamedObject(jsonString, namedObject));
    EXPECT_NE(nullptr, namedObject.Object);
    EXPECT_EQ(testValue, *(bool*)namedObject.Object);
    EXPECT_STREQ(objectName.c_str(), namedObject.Name.c_str());
}

TEST_F(DataSerializerTests, Data_Serializer_Serializes_Deserializes_Uint8_t_Correctly)
{
    std::string objectName = "Object Name";
    uint8_t testValue = 10;
    NamedObject_t namedObject;
    std::string jsonString = mp_dataSerializer->SerializeDataItemToJson(objectName, DataType_Uint8_t, &testValue, 1, 0);
    EXPECT_EQ(true, mp_dataSerializer->DeSerializeJsonToNamedObject(jsonString, namedObject));
    EXPECT_NE(nullptr, namedObject.Object);
    EXPECT_EQ(testValue, *(uint8_t*)namedObject.Object);
    EXPECT_STREQ(objectName.c_str(), namedObject.Name.c_str());
}

TEST_F(DataSerializerTests, Data_Serializer_Serializes_Deserializes_Uint16_t_Correctly)
{
    std::string objectName = "Object Name";
    uint16_t testValue = 10;
    NamedObject_t namedObject;
    std::string jsonString = mp_dataSerializer->SerializeDataItemToJson(objectName, DataType_Uint16_t, &testValue, 1, 0);
    EXPECT_EQ(true, mp_dataSerializer->DeSerializeJsonToNamedObject(jsonString, namedObject));
    EXPECT_NE(nullptr, namedObject.Object);
    EXPECT_EQ(testValue, *(uint16_t*)namedObject.Object);
    EXPECT_STREQ(objectName.c_str(), namedObject.Name.c_str());
}

TEST_F(DataSerializerTests, Data_Serializer_Serializes_Deserializes_Uint32_t_Correctly)
{
    std::string objectName = "Object Name";
    uint32_t testValue = 10;
    NamedObject_t namedObject;
    std::string jsonString = mp_dataSerializer->SerializeDataItemToJson(objectName, DataType_Uint32_t, &testValue, 1, 0);
    EXPECT_EQ(true, mp_dataSerializer->DeSerializeJsonToNamedObject(jsonString, namedObject));
    EXPECT_NE(nullptr, namedObject.Object);
    EXPECT_EQ(testValue, *(uint32_t*)namedObject.Object);
    EXPECT_STREQ(objectName.c_str(), namedObject.Name.c_str());
}

TEST_F(DataSerializerTests, Data_Serializer_Serializes_Deserializes_Int8_t_Correctly)
{
    std::string objectName = "Object Name";
    int8_t testValue = -10;
    NamedObject_t namedObject;
    std::string jsonString = mp_dataSerializer->SerializeDataItemToJson(objectName, DataType_Int8_t, &testValue, 1, 0);
    EXPECT_EQ(true, mp_dataSerializer->DeSerializeJsonToNamedObject(jsonString, namedObject));
    EXPECT_NE(nullptr, namedObject.Object);
    EXPECT_EQ(testValue, *(int8_t*)namedObject.Object);
    EXPECT_STREQ(objectName.c_str(), namedObject.Name.c_str());
}

TEST_F(DataSerializerTests, Data_Serializer_Serializes_Deserializes_Int16_t_Correctly)
{
    std::string objectName = "Object Name";
    int16_t testValue = -10;
    NamedObject_t namedObject;
    std::string jsonString = mp_dataSerializer->SerializeDataItemToJson(objectName, DataType_Int16_t, &testValue, 1, 0);
    EXPECT_EQ(true, mp_dataSerializer->DeSerializeJsonToNamedObject(jsonString, namedObject));
    EXPECT_NE(nullptr, namedObject.Object);
    EXPECT_EQ(testValue, *(int16_t*)namedObject.Object);
    EXPECT_STREQ(objectName.c_str(), namedObject.Name.c_str());
}

TEST_F(DataSerializerTests, Data_Serializer_Serializes_Deserializes_Int32_t_Correctly)
{
    std::string objectName = "Object Name";
    int32_t testValue = -10;
    NamedObject_t namedObject;
    std::string jsonString = mp_dataSerializer->SerializeDataItemToJson(objectName, DataType_Int32_t, &testValue, 1, 0);
    EXPECT_EQ(true, mp_dataSerializer->DeSerializeJsonToNamedObject(jsonString, namedObject));
    EXPECT_NE(nullptr, namedObject.Object);
    EXPECT_EQ(testValue, *(int32_t*)namedObject.Object);
    EXPECT_STREQ(objectName.c_str(), namedObject.Name.c_str());
}

TEST_F(DataSerializerTests, Data_Serializer_Serializes_Deserializes_Char_Correctly)
{
    std::string objectName = "Object Name";
    char testValue = 'a';
    NamedObject_t namedObject;
    std::string jsonString = mp_dataSerializer->SerializeDataItemToJson(objectName, DataType_Char_t, &testValue, 1, 0);
    EXPECT_EQ(true, mp_dataSerializer->DeSerializeJsonToNamedObject(jsonString, namedObject));
    EXPECT_NE(nullptr, namedObject.Object);
    EXPECT_EQ(testValue, *(char*)namedObject.Object);
    EXPECT_STREQ(objectName.c_str(), namedObject.Name.c_str());
}

TEST_F(DataSerializerTests, Data_Serializer_Serializes_Deserializes_Bt_Device_Info_Correctly)
{
    std::string objectName = "Object Name";
    char name[BT_NAME_LENGTH] = "LED Tower of Power";
	char address[BT_ADDRESS_LENGTH] = "AA:BB:CC:DD:EE:FF";
	int32_t rssi = 100;
    BT_Device_Info_t testValue = { name, address, rssi };
    NamedObject_t namedObject;
    std::string jsonString = mp_dataSerializer->SerializeDataItemToJson(objectName, DataType_BT_Device_Info_t, &testValue, 1, 0);
    EXPECT_EQ(true, mp_dataSerializer->DeSerializeJsonToNamedObject(jsonString, namedObject));
    EXPECT_NE(nullptr, namedObject.Object);
    EXPECT_EQ(testValue, *(BT_Device_Info_t*)namedObject.Object);
    EXPECT_STREQ(objectName.c_str(), namedObject.Name.c_str());
}

TEST_F(DataSerializerTests, Data_Serializer_Serializes_Deserializes_BT_Device_Info_With_Time_Since_Update_Correctly)
{
    std::string objectName = "Object Name";
    char name[BT_NAME_LENGTH] = "LED Tower of Power";
	char address[BT_ADDRESS_LENGTH] = "AA:BB:CC:DD:EE:FF";
	int32_t rssi = 100;
    uint32_t timeSinceUdpate = 200;
    BT_Device_Info_With_Time_Since_Update_t testValue = { name, address, timeSinceUdpate, rssi };
    NamedObject_t namedObject;
    std::string jsonString = mp_dataSerializer->SerializeDataItemToJson(objectName, DataType_BT_Device_Info_With_Time_Since_Update_t, &testValue, 1, 0);
    EXPECT_EQ(true, mp_dataSerializer->DeSerializeJsonToNamedObject(jsonString, namedObject));
    EXPECT_NE(nullptr, namedObject.Object);
    EXPECT_EQ(testValue, *(BT_Device_Info_With_Time_Since_Update_t*)namedObject.Object);
    EXPECT_STREQ(objectName.c_str(), namedObject.Name.c_str());
}

TEST_F(DataSerializerTests, Data_Serializer_Serializes_Deserializes_CompatibleDevice_Correctly)
{
    std::string objectName = "Object Name";
    char name[BT_NAME_LENGTH] = "LED Tower of Power";
	char address[BT_ADDRESS_LENGTH] = "AA:BB:CC:DD:EE:FF";
    BluetoothDevice_t testValue = { name, address };
    NamedObject_t namedObject;
    std::string jsonString = mp_dataSerializer->SerializeDataItemToJson(objectName, DataType_BluetoothDevice_t, &testValue, 1, 0);
    EXPECT_EQ(true, mp_dataSerializer->DeSerializeJsonToNamedObject(jsonString, namedObject));
    EXPECT_NE(nullptr, namedObject.Object);
    EXPECT_EQ(testValue, *(BluetoothDevice_t*)namedObject.Object);
    EXPECT_STREQ(objectName.c_str(), namedObject.Name.c_str());
}

TEST_F(DataSerializerTests, Data_Serializer_Serializes_Deserializes_ActiveCompatibleDevice_Correctly)
{
    std::string objectName = "Object Name";
    char name[BT_NAME_LENGTH] = "LED Tower of Power";
	char address[BT_ADDRESS_LENGTH] = "AA:BB:CC:DD:EE:FF";
    int32_t rssi = 100;
    unsigned long lastUpdateTime = 200;
    uint32_t timeSinceUpdate = 300;
    ActiveBluetoothDevice_t testValue = { name, address, rssi, lastUpdateTime, timeSinceUpdate };
    NamedObject_t namedObject;
    std::string jsonString = mp_dataSerializer->SerializeDataItemToJson(objectName, DataType_ActiveBluetoothDevice_t, &testValue, 1, 0);
    EXPECT_EQ(true, mp_dataSerializer->DeSerializeJsonToNamedObject(jsonString, namedObject));
    EXPECT_NE(nullptr, namedObject.Object);
    EXPECT_EQ(testValue, *(ActiveBluetoothDevice_t*)namedObject.Object);
    EXPECT_STREQ(objectName.c_str(), namedObject.Name.c_str());
}

TEST_F(DataSerializerTests, Data_Serializer_Serializes_Deserializes_Flaot_Correctly)
{
    std::string objectName = "Object Name";
    float testValue = 1.23456;
    NamedObject_t namedObject;
    std::string jsonString = mp_dataSerializer->SerializeDataItemToJson(objectName, DataType_Float_t, &testValue, 1, 0);
    EXPECT_EQ(true, mp_dataSerializer->DeSerializeJsonToNamedObject(jsonString, namedObject));
    EXPECT_NE(nullptr, namedObject.Object);
    EXPECT_EQ(testValue, *(float*)namedObject.Object);
    EXPECT_STREQ(objectName.c_str(), namedObject.Name.c_str());
}

TEST_F(DataSerializerTests, Data_Serializer_Serializes_Deserializes_Double_t_Correctly)
{
    std::string objectName = "Object Name";
    double testValue = 10.10;
    NamedObject_t namedObject;
    std::string jsonString = mp_dataSerializer->SerializeDataItemToJson(objectName, DataType_Double_t, &testValue, 1, 0);
    EXPECT_EQ(true, mp_dataSerializer->DeSerializeJsonToNamedObject(jsonString, namedObject));
    EXPECT_NE(nullptr, namedObject.Object);
    EXPECT_EQ(testValue, *(double*)namedObject.Object);
    EXPECT_STREQ(objectName.c_str(), namedObject.Name.c_str());
}

TEST_F(DataSerializerTests, Data_Serializer_Serializes_Deserializes_ProcessedSoundData_Correctly)
{
    std::string objectName = "Object Name";
	float NormalizedPower = 0.10;
	int32_t Minimum = 100;
	int32_t Maximum = 200;
    ProcessedSoundData_t testValue = { NormalizedPower, Minimum, Maximum };
    NamedObject_t namedObject;
    std::string jsonString = mp_dataSerializer->SerializeDataItemToJson(objectName, DataType_ProcessedSoundData_t, &testValue, 1, 0);
    EXPECT_EQ(true, mp_dataSerializer->DeSerializeJsonToNamedObject(jsonString, namedObject));
    EXPECT_NE(nullptr, namedObject.Object);
    EXPECT_EQ(testValue, *(ProcessedSoundData_t*)namedObject.Object);
    EXPECT_STREQ(objectName.c_str(), namedObject.Name.c_str());
}

TEST_F(DataSerializerTests, Data_Serializer_Serializes_Deserializes_MaxBandSoundData_Correctly)
{
    std::string objectName = "Object Name";
	float MaxBandNormalizedPower = 0.10;
	int16_t MaxBandIndex = 100;
	int16_t TotalBands = 200;
    MaxBandSoundData_t testValue = { MaxBandNormalizedPower, MaxBandIndex, TotalBands };
    NamedObject_t namedObject;
    std::string jsonString = mp_dataSerializer->SerializeDataItemToJson(objectName, DataType_MaxBandSoundData_t, &testValue, 1, 0);
    EXPECT_EQ(true, mp_dataSerializer->DeSerializeJsonToNamedObject(jsonString, namedObject));
    EXPECT_NE(nullptr, namedObject.Object);
    EXPECT_EQ(testValue, *(MaxBandSoundData_t*)namedObject.Object);
    EXPECT_STREQ(objectName.c_str(), namedObject.Name.c_str());
}

TEST_F(DataSerializerTests, Data_Serializer_Serializes_Deserializes_Frame_Correctly)
{
    std::string objectName = "Object Name";
	int16_t channel1 = 100;
	int16_t channel2 = 200;
    Frame_t testValue = { channel1, channel2 };
    NamedObject_t namedObject;
    std::string jsonString = mp_dataSerializer->SerializeDataItemToJson(objectName, DataType_Frame_t, &testValue, 1, 0);
    EXPECT_EQ(true, mp_dataSerializer->DeSerializeJsonToNamedObject(jsonString, namedObject));
    EXPECT_NE(nullptr, namedObject.Object);
    EXPECT_EQ(testValue, *(Frame_t*)namedObject.Object);
    EXPECT_STREQ(objectName.c_str(), namedObject.Name.c_str());
}

TEST_F(DataSerializerTests, Data_Serializer_Serializes_Deserializes_ProcessedSoundFrame_Correctly)
{
    std::string objectName = "Object Name";
	float NormalizedPower1 = 0.10;
	int32_t Minimum1 = 100;
	int32_t Maximum1 = 200;
    ProcessedSoundData_t channel1 = { NormalizedPower1, Minimum1, Maximum1};
	float NormalizedPower2 = 0.20;
	int32_t Minimum2 = 300;
	int32_t Maximum2 = 400;
    ProcessedSoundData_t channel2 = { NormalizedPower2, Minimum2, Maximum2};
    ProcessedSoundFrame_t testValue = { channel1, channel2 };
    NamedObject_t namedObject;
    std::string jsonString = mp_dataSerializer->SerializeDataItemToJson(objectName, DataType_ProcessedSoundFrame_t, &testValue, 1, 0);
    EXPECT_EQ(true, mp_dataSerializer->DeSerializeJsonToNamedObject(jsonString, namedObject));
    EXPECT_NE(nullptr, namedObject.Object);
    EXPECT_EQ(testValue, *(ProcessedSoundFrame_t*)namedObject.Object);
    EXPECT_STREQ(objectName.c_str(), namedObject.Name.c_str());
}

TEST_F(DataSerializerTests, Data_Serializer_Serializes_Deserializes_SoundState_Correctly)
{
    std::string objectName = "Object Name";
    SoundState_t testValue = SoundState::Sound_Level9_Detected;
    NamedObject_t namedObject;
    std::string jsonString = mp_dataSerializer->SerializeDataItemToJson(objectName, DataType_SoundState_t, &testValue, 1, 0);
    EXPECT_EQ(true, mp_dataSerializer->DeSerializeJsonToNamedObject(jsonString, namedObject));
    EXPECT_NE(nullptr, namedObject.Object);
    EXPECT_EQ(testValue, *(SoundState_t*)namedObject.Object);
    EXPECT_STREQ(objectName.c_str(), namedObject.Name.c_str());
}

TEST_F(DataSerializerTests, Data_Serializer_Serializes_Deserializes_ConnectionStatus_Correctly)
{
    std::string objectName = "Object Name";
    ConnectionStatus_t testValue = ConnectionStatus::Disconnecting;
    NamedObject_t namedObject;
    std::string jsonString = mp_dataSerializer->SerializeDataItemToJson(objectName, DataType_ConnectionStatus_t, &testValue, 1, 0);
    EXPECT_EQ(true, mp_dataSerializer->DeSerializeJsonToNamedObject(jsonString, namedObject));
    EXPECT_NE(nullptr, namedObject.Object);
    EXPECT_EQ(testValue, *(ConnectionStatus_t*)namedObject.Object);
    EXPECT_STREQ(objectName.c_str(), namedObject.Name.c_str());
}

TEST_F(DataSerializerTests, Data_Serializer_Serializes_Deserializes_SoundInputSource_Correctly)
{
    std::string objectName = "Object Name";
    SoundInputSource_t testValue = SoundInputSource_t::Microphone;
    NamedObject_t namedObject;
    std::string jsonString = mp_dataSerializer->SerializeDataItemToJson(objectName, DataType_ConnectionStatus_t, &testValue, 1, 0);
    EXPECT_EQ(true, mp_dataSerializer->DeSerializeJsonToNamedObject(jsonString, namedObject));
    EXPECT_NE(nullptr, namedObject.Object);
    EXPECT_EQ(testValue, *(SoundInputSource_t*)namedObject.Object);
    EXPECT_STREQ(objectName.c_str(), namedObject.Name.c_str());
}

TEST_F(DataSerializerTests, Data_Serializer_Serializes_Deserializes_SoundOutputSource_Correctly)
{
    std::string objectName = "Object Name";
    SoundOutputSource_t testValue = SoundOutputSource_t::Bluetooth;
    NamedObject_t namedObject;
    std::string jsonString = mp_dataSerializer->SerializeDataItemToJson(objectName, DataType_SoundOutputSource_t, &testValue, 1, 0);
    EXPECT_EQ(true, mp_dataSerializer->DeSerializeJsonToNamedObject(jsonString, namedObject));
    EXPECT_NE(nullptr, namedObject.Object);
    EXPECT_EQ(testValue, *(SoundOutputSource_t*)namedObject.Object);
    EXPECT_STREQ(objectName.c_str(), namedObject.Name.c_str());
}