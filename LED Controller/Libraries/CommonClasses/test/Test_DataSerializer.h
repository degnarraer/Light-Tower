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

// Test Fixture for DataSerializerTests
class DataSerializerTests : public Test
{
    protected:
        const int32_t initialValue = 10;
        const String name = "Test Name";
        DataSerializer *mp_dataSerializer;
  
        MockDataItem<int32_t, 1> *mp_mockDataItem;
        void SetUp() override
        {
            mp_dataSerializer = new DataSerializer();
            mp_mockDataItem = new MockDataItem<int32_t, 1>(name, initialValue, RxTxType_Rx_Only, UpdateStoreType_On_Rx, 0);

        }
        void TearDown() override 
        {
            delete mp_dataSerializer;
            delete mp_mockDataItem;
        }
};

TEST_F(DataSerializerTests, Data_Serializer_Serializes_Correctly)
{
    int32_t testValue = 10;
    String resultString = mp_dataSerializer->SerializeDataToJson(name, DataType_Int32_t, &testValue, 1);
    EXPECT_STREQ(resultString.c_str(), "{\"Name\":\"Test Name\",\"Count\":1,\"Type\":\"Int32_t\",\"Bytes\":4,\"Data\":[\"0A000000\"],\"Sum\":10}");
}
