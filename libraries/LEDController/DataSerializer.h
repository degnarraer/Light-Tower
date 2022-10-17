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

#ifndef DataSerializer_H
#define DataSerializer_H
#include <Arduino.h>
#include <ArduinoJson.h>
#include <Helpers.h>
#include "Streaming.h"

class DataSerializer: public CommonUtils

{
	public:
		DataSerializer(){}
		virtual ~DataSerializer(){}
		void SetDataSerializerDataItems(DataItem_t& DataItem, size_t DataItemCount)
		{
			m_DataItems = &DataItem;
			m_DataItemsCount = DataItemCount;
		}
		
		
		String SerializeDataToJson(String Name, DataType_t DataType, void* Object, size_t Count)
		{
			int32_t CheckSum = 0;
			docOut.clear();
			docOut[m_NameTag] = Name;
			docOut[m_CountTag] = Count;
			docOut[m_DataTypeTag] = DataTypeStrings[DataType];
			JsonArray data = docOut.createNestedArray(m_DataTag);
			docOut[m_TotalByteCountTag] = GetSizeOfDataType(DataType) * Count;
			for(int i = 0; i < docOut[m_TotalByteCountTag]; ++i)
			{
				uint8_t Value = ((uint8_t*)Object)[i];
				CheckSum += Value;
				data.add(Value);
			}
			docOut[m_CheckSumTag] = CheckSum;
			String Result;
			serializeJson(docOut, Result);
			Result = m_Startinator + Result + m_Terminator;
			return Result;
		}
		void DeSerializeJsonToMatchingDataItem(String json)
		{
			DeserializationError error = deserializeJson(docIn, json.c_str());
			// Test if parsing succeeds.
			if (error)
			{
				ESP_LOGW("Serial_Datalink", "WARNING! Deserialize failed: %s. For String: %s", error.c_str(), json.c_str());
				return;
			}
			else
			{
				if(NULL != m_DataItems)
				{
					for(int i = 0; i < m_DataItemsCount; ++i)
					{
						const String ItemName = (m_DataItems[i]).Name;
						const String DocName = docIn[m_NameTag];
						if(true == ItemName.equals(DocName))
						{
							int CheckSumCalc = 0;
							int CheckSumIn = docIn[m_CheckSumTag];
							int CountIn = docIn[m_CountTag];
							int ByteCountIn = docIn[m_TotalByteCountTag];
							int DataByteCount = docIn[m_DataTag].size();
							uint8_t Buffer[DataByteCount];
							if(ByteCountIn == DataByteCount)
							{
								for(int j = 0; j < DataByteCount; ++j)
								{
									CheckSumCalc += (uint8_t)(docIn[m_DataTag][j]);
									Buffer[j] = (uint8_t)(docIn[m_DataTag][j]);
								}
								if(CheckSumCalc == CheckSumIn)
								{
									PushValueToQueue(Buffer, m_DataItems[i].QueueHandle_RX, false, ItemName.c_str(), m_DataItems[i].DataPushHasErrored);
								}
								else
								{
									ESP_LOGW("Serial_Datalink", "WARNING! Deserialize failed: Checksum Error for String: %s", json.c_str());
								}
							}
							else
							{
								ESP_LOGW("Serial_Datalink", "WARNING! Deserialize failed: Byte Count Error for String: %s", json.c_str());
							}
							return;
						}
					}
				}
			}
		}
		bool SerializeByteArray(uint8_t *InputBuffer, size_t InputBufferSize, uint8_t *OutputBuffer, size_t OutputBufferSize, size_t DataByteCount)
		{
			if(OutputBufferSize >= InputBufferSize + sizeof(DataByteCount) )
			{
				memcpy(OutputBuffer, &DataByteCount, sizeof(DataByteCount));
				memcpy(OutputBuffer + sizeof(DataByteCount), InputBuffer, InputBufferSize);
				return true;
			}
			else
			{
				return false;
			}
		}
		size_t DeSerializeByteArray(uint8_t *InputBuffer, size_t InputBufferSize, uint8_t *OutputBuffer, size_t OutputBufferSize)
		{
			/*
			Serial << "Input Buffer: ";
			for(int i = 0; i < 10; ++i)
			{
				Serial << InputBuffer[i] << ",";
			}
			Serial << "\n";
			*/
			size_t BufferedByteCount;
			assert(OutputBufferSize >= InputBufferSize - sizeof(BufferedByteCount) );
			BufferedByteCount = ((size_t*)InputBuffer)[0];
			Serial << "Buffered Bytes: " << BufferedByteCount << "\n";
			assert(OutputBufferSize >= BufferedByteCount);
			memcpy(OutputBuffer, InputBuffer + sizeof(BufferedByteCount), BufferedByteCount);
			/*
			Serial << "Output Buffer: ";
			for(int i = 0; i < 10; ++i)
			{
				Serial << OutputBuffer[i] << ",";
			}
			Serial << "\n";
			*/
			return BufferedByteCount;
		}
		size_t DeSerialize(String InputString, String Name, uint8_t *DataBuffer, size_t MaxBytes)
		{
			int16_t first = InputString.indexOf(m_Startinator) + m_Startinator.length();
			int16_t last = InputString.indexOf(m_Terminator);
			if(first >= 0 && last >= 0)
			{
				assert(last > first);
				String json = InputString.substring(first, last);
				DeserializationError error = deserializeJson(docIn, json.c_str());
				// Test if parsing succeeds.
				if (error)
				{
					ESP_LOGW("Serial_Datalink", "WARNING! Deserialize failed: %s. For String: %s", error.c_str(), json.c_str());
					return 0;
				}
				else
				{
					const String DocName = docIn[m_NameTag];
					if(true == DocName.equals(Name))
					{
						int CheckSumCalc = 0;
						int CheckSumIn = docIn[m_CheckSumTag];
						int CountIn = docIn[m_CountTag];
						int ByteCountIn = docIn[m_TotalByteCountTag];
						int ByteCountInCalc = CountIn * GetSizeOfDataType((DataType_t)GetDataTypeFromString(docIn[m_DataTypeTag]));
						int DataByteCount = docIn[m_DataTag].size();
						Serial << "DBC: " << DataByteCount << "\tBCI: " << ByteCountIn << "\n";
						if(ByteCountIn == DataByteCount && ByteCountIn <= MaxBytes)
						{
							for(int i = 0; i < DataByteCount; ++i)
							{
								CheckSumCalc += (uint8_t)(docIn[m_DataTag][i]);
							}
							if(CheckSumCalc == CheckSumIn)
							{
								for(int i = 0; i < DataByteCount; ++i)
								{
									DataBuffer[i] = (uint8_t)(docIn[m_DataTag][i]);
								}
								return DataByteCount;
							}
							else
							{
								ESP_LOGW("Serial_Datalink", "WARNING! Deserialize failed: Checksum Error for String: %s", json.c_str());
								return 0;
							}
						}
						else
						{
							ESP_LOGW("Serial_Datalink", "WARNING! Deserialize failed: Byte Count Error for String: %s", json.c_str());
						}
						return 0;
					}
					else
					{
						ESP_LOGW("Serial_Datalink", "WARNING! Deserialize failed: Data Packet Name Did Not Match: %s", Name.c_str());
						return 0;
					}
				}
			}
			else
			{
				return 0;
			}
		}
	private:
		StaticJsonDocument<10000> docIn;
		StaticJsonDocument<10000> docOut;
		DataItem_t* m_DataItems;
		size_t m_DataItemsCount = 0;
		//Tags
		String m_Startinator = "<PACKET_START>";
		String m_NameTag = "N";
		String m_CheckSumTag = "S";
		String m_CountTag = "C";
		String m_DataTag = "D";
		String m_DataTypeTag = "T";
		String m_TotalByteCountTag = "B";
		String m_Terminator = "<PACKET_END>";
};


#endif