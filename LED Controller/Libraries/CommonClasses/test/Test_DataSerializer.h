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
    String objectName = "Object Name";
    bool testValue = true;
    NamedObject_t namedObject;
    String jsonString = mp_dataSerializer->SerializeDataToJson(objectName, DataType_Bool_t, &testValue, 1);
    EXPECT_EQ(true, mp_dataSerializer->DeSerializeJsonToNamedObject(jsonString, namedObject));
    EXPECT_NE(nullptr, namedObject.Object);
    EXPECT_EQ(testValue, *(bool*)namedObject.Object);
    EXPECT_STREQ(objectName.c_str(), namedObject.Name.c_str());
}

TEST_F(DataSerializerTests, Data_Serializer_Serializes_Deserializes_Uint8_t_Correctly)
{
    String objectName = "Object Name";
    uint8_t testValue = 10;
    NamedObject_t namedObject;
    String jsonString = mp_dataSerializer->SerializeDataToJson(objectName, DataType_Uint8_t, &testValue, 1);
    EXPECT_EQ(true, mp_dataSerializer->DeSerializeJsonToNamedObject(jsonString, namedObject));
    EXPECT_NE(nullptr, namedObject.Object);
    EXPECT_EQ(testValue, *(uint8_t*)namedObject.Object);
    EXPECT_STREQ(objectName.c_str(), namedObject.Name.c_str());
}

TEST_F(DataSerializerTests, Data_Serializer_Serializes_Deserializes_Uint16_t_Correctly)
{
    String objectName = "Object Name";
    uint16_t testValue = 10;
    NamedObject_t namedObject;
    String jsonString = mp_dataSerializer->SerializeDataToJson(objectName, DataType_Uint16_t, &testValue, 1);
    EXPECT_EQ(true, mp_dataSerializer->DeSerializeJsonToNamedObject(jsonString, namedObject));
    EXPECT_NE(nullptr, namedObject.Object);
    EXPECT_EQ(testValue, *(uint16_t*)namedObject.Object);
    EXPECT_STREQ(objectName.c_str(), namedObject.Name.c_str());
}

TEST_F(DataSerializerTests, Data_Serializer_Serializes_Deserializes_Uint32_t_Correctly)
{
    String objectName = "Object Name";
    uint32_t testValue = 10;
    NamedObject_t namedObject;
    String jsonString = mp_dataSerializer->SerializeDataToJson(objectName, DataType_Uint32_t, &testValue, 1);
    EXPECT_EQ(true, mp_dataSerializer->DeSerializeJsonToNamedObject(jsonString, namedObject));
    EXPECT_NE(nullptr, namedObject.Object);
    EXPECT_EQ(testValue, *(uint32_t*)namedObject.Object);
    EXPECT_STREQ(objectName.c_str(), namedObject.Name.c_str());
}

TEST_F(DataSerializerTests, Data_Serializer_Serializes_Deserializes_Int8_t_Correctly)
{
    String objectName = "Object Name";
    int8_t testValue = -10;
    NamedObject_t namedObject;
    String jsonString = mp_dataSerializer->SerializeDataToJson(objectName, DataType_Int8_t, &testValue, 1);
    EXPECT_EQ(true, mp_dataSerializer->DeSerializeJsonToNamedObject(jsonString, namedObject));
    EXPECT_NE(nullptr, namedObject.Object);
    EXPECT_EQ(testValue, *(int8_t*)namedObject.Object);
    EXPECT_STREQ(objectName.c_str(), namedObject.Name.c_str());
}

TEST_F(DataSerializerTests, Data_Serializer_Serializes_Deserializes_Int16_t_Correctly)
{
    String objectName = "Object Name";
    int16_t testValue = -10;
    NamedObject_t namedObject;
    String jsonString = mp_dataSerializer->SerializeDataToJson(objectName, DataType_Int16_t, &testValue, 1);
    EXPECT_EQ(true, mp_dataSerializer->DeSerializeJsonToNamedObject(jsonString, namedObject));
    EXPECT_NE(nullptr, namedObject.Object);
    EXPECT_EQ(testValue, *(int16_t*)namedObject.Object);
    EXPECT_STREQ(objectName.c_str(), namedObject.Name.c_str());
}

TEST_F(DataSerializerTests, Data_Serializer_Serializes_Deserializes_Int32_t_Correctly)
{
    String objectName = "Object Name";
    int32_t testValue = -10;
    NamedObject_t namedObject;
    String jsonString = mp_dataSerializer->SerializeDataToJson(objectName, DataType_Int32_t, &testValue, 1);
    EXPECT_EQ(true, mp_dataSerializer->DeSerializeJsonToNamedObject(jsonString, namedObject));
    EXPECT_NE(nullptr, namedObject.Object);
    EXPECT_EQ(testValue, *(int32_t*)namedObject.Object);
    EXPECT_STREQ(objectName.c_str(), namedObject.Name.c_str());
}

TEST_F(DataSerializerTests, Data_Serializer_Serializes_Deserializes_Double_t_Correctly)
{
    String objectName = "Object Name";
    double testValue = 10.10;
    NamedObject_t namedObject;
    String jsonString = mp_dataSerializer->SerializeDataToJson(objectName, DataType_Double_t, &testValue, 1);
    EXPECT_EQ(true, mp_dataSerializer->DeSerializeJsonToNamedObject(jsonString, namedObject));
    EXPECT_NE(nullptr, namedObject.Object);
    EXPECT_EQ(testValue, *(double*)namedObject.Object);
    EXPECT_STREQ(objectName.c_str(), namedObject.Name.c_str());
}

TEST_F(DataSerializerTests, Data_Serializer_Serializes_Deserializes_Bt_Device_Info_Correctly)
{
    String objectName = "Object Name";
    char name[BT_NAME_LENGTH] = "LED Tower of Power";
	char address[BT_ADDRESS_LENGTH] = "AA:BB:CC:DD:EE:FF";
	int32_t rssi = 100;
    BT_Device_Info_t testValue = { name, address, rssi };
    NamedObject_t namedObject;
    String jsonString = mp_dataSerializer->SerializeDataToJson(objectName, DataType_BT_Device_Info_t, &testValue, 1);
    EXPECT_EQ(true, mp_dataSerializer->DeSerializeJsonToNamedObject(jsonString, namedObject));
    EXPECT_NE(nullptr, namedObject.Object);
    EXPECT_EQ(testValue, *(BT_Device_Info_t*)namedObject.Object);
    EXPECT_STREQ(objectName.c_str(), namedObject.Name.c_str());
}

TEST_F(DataSerializerTests, Data_Serializer_Serializes_Deserializes_BT_Device_Info_With_Time_Since_Update_Correctly)
{
    String objectName = "Object Name";
    char name[BT_NAME_LENGTH] = "LED Tower of Power";
	char address[BT_ADDRESS_LENGTH] = "AA:BB:CC:DD:EE:FF";
	int32_t rssi = 100;
    uint32_t timeSinceUdpate = 200;
    BT_Device_Info_With_Time_Since_Update_t testValue = { name, address, rssi, timeSinceUdpate };
    NamedObject_t namedObject;
    String jsonString = mp_dataSerializer->SerializeDataToJson(objectName, DataType_BT_Device_Info_With_Time_Since_Update_t, &testValue, 1);
    EXPECT_EQ(true, mp_dataSerializer->DeSerializeJsonToNamedObject(jsonString, namedObject));
    EXPECT_NE(nullptr, namedObject.Object);
    EXPECT_EQ(testValue, *(BT_Device_Info_With_Time_Since_Update_t*)namedObject.Object);
    EXPECT_STREQ(objectName.c_str(), namedObject.Name.c_str());
}

/*
  "Int8_t",
  "Int16_t",
  "Uint8_t",
  "Uint16_t",
  "Char_t",
  "String_t",
  "BT_Device_Info_t",
  "BT_Info_With_LastUpdateTime_t",
  "CompatibleDevice_t",
  "ActiveCompatibleDevice_t",
  "Float_t",
  "Double_t",
  "ProcessedSoundData_t",
  "MaxBandSoundData_t",
  "Frame_t",
  "ProcessedSoundFrame_t",
  "SoundState_t",
  "ConnectionStatus_t",
  "SoundInputSource_t",
  "SoundOutputSource_t",
  */