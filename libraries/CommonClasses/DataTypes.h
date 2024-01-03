#ifndef DataTypes_H
#define DataTypes_H

#include "Arduino.h"
#include "Streaming.h"

class NamedItem
{
  public:
	NamedItem(String Title): m_Title(Title){}
	virtual ~NamedItem(){}
    String GetTitle()
    { 
      return m_Title;
    }
  private:
    String m_Title;
};

struct KeyValuePair
{
  String Key;
  String Value;
};
typedef KeyValuePair KVP;

struct KeyValueTuple
{
  String Key;
  String Value1;
  String Value2;
};
typedef KeyValueTuple KVT;

struct SSID_Info
{
	public:
		SSID_Info(){}
		SSID_Info(String SSID_In, int32_t RSSI_In = 0)
		{
			if(248 < SSID_In.length())
			{
				Serial << "Bad SSID: " << SSID_In.c_str() << " | " << SSID_In.length() << "\n";
				assert(248 >= SSID_In.length());
			}
			snprintf(SSID, 248, "%s", SSID_In.c_str());
			RSSI = RSSI_In;
		}
		SSID_Info(String SSID_In, String ADDRESS_In, int32_t RSSI_In = 0)
		{
			if(248 < SSID_In.length())
			{
				Serial << "Bad SSID: " << SSID_In.c_str() << " | " << SSID_In.length() << "\n";
				assert(248 >= SSID_In.length());
			}
			snprintf(SSID, 248, "%s", SSID_In.c_str());
			if(18 < ADDRESS_In.length())
			{
				Serial << "Bad ADDRESS: " << ADDRESS_In.c_str() << " | " << ADDRESS_In.length() << "\n";
				assert(18 >= ADDRESS_In.length());
			}
			snprintf(ADDRESS, 18, "%s", ADDRESS_In.c_str());
			RSSI = RSSI_In;
		}
		char SSID[248] = "\0";
		char ADDRESS[18] = "\0";
		int32_t RSSI = 0;
};
typedef SSID_Info SSID_Info_t;

struct BT_Info_With_LastUpdateTime
{
	public:
		BT_Info_With_LastUpdateTime(){}
		BT_Info_With_LastUpdateTime(String SSID_In, String ADDRESS_In, uint32_t TimeSinceUdpate_in, int32_t RSSI_In = 0)
		{
			if(248 < SSID_In.length())
			{
				Serial << "Bad SSID: " << SSID_In.c_str() << " | " << SSID_In.length() << "\n";
				assert(248 >= SSID_In.length());
			}
			snprintf(SSID, 248, "%s", SSID_In.c_str());
			if(18 < ADDRESS_In.length())
			{
				Serial << "Bad ADDRESS: " << ADDRESS_In.c_str() << " | " << ADDRESS_In.length() << "\n";
				assert(18 >= ADDRESS_In.length());
			}
			snprintf(ADDRESS, 18, "%s", ADDRESS_In.c_str());
			TimeSinceUdpate = TimeSinceUdpate_in;
			RSSI = RSSI_In;
		}
		BT_Info_With_LastUpdateTime& operator=(const BT_Info_With_LastUpdateTime& other)
		{
			strncpy(this->SSID, other.SSID, sizeof(this->SSID) - 1);
			this->SSID[sizeof(this->SSID) - 1] = '\0';  // Ensure null-terminated

			strncpy(this->ADDRESS, other.ADDRESS, sizeof(this->ADDRESS) - 1);
			this->ADDRESS[sizeof(this->ADDRESS) - 1] = '\0';  // Ensure null-terminated

			this->RSSI = other.RSSI;
			this->TimeSinceUdpate = other.TimeSinceUdpate;

			return *this;
		}
		bool operator==(const BT_Info_With_LastUpdateTime& other) const
		{
			if( strcmp(this->SSID, other.SSID) == 0 &&
				strcmp(this->ADDRESS, other.ADDRESS) == 0 &&
				this->RSSI == other.RSSI &&
				this->TimeSinceUdpate == other.TimeSinceUdpate) return true;
			else return false;
		}
		bool operator!=(const BT_Info_With_LastUpdateTime& other) const
		{
			if( strcmp(this->SSID, other.SSID) != 0 ||
				strcmp(this->ADDRESS, other.ADDRESS) != 0 ||
				this->RSSI != other.RSSI ||
				this->TimeSinceUdpate != other.TimeSinceUdpate) return true;
			else return false;
		}
		char SSID[248] = "\0";
		char ADDRESS[18] = "\0";
		int32_t RSSI = 0;
		uint32_t TimeSinceUdpate = 0;
};
typedef BT_Info_With_LastUpdateTime BT_Info_With_LastUpdateTime_t;


struct ActiveCompatibleDevice_t
{
	String SSID;
	String ADDRESS;
	int32_t RSSI;
	unsigned long LastUpdateTime;
};
	
enum SoundState_t
{
  LastingSilenceDetected = 0,
  SilenceDetected = 1,
  Sound_Level1_Detected = 2,
  Sound_Level2_Detected = 3,
  Sound_Level3_Detected = 4,
  Sound_Level4_Detected = 5,
  Sound_Level5_Detected = 6,
  Sound_Level6_Detected = 7,
  Sound_Level7_Detected = 8,
  Sound_Level8_Detected = 9,
  Sound_Level9_Detected = 10,
  Sound_Level10_Detected = 11,
  Sound_Level11_Detected = 12,
};

enum Transciever_t
{
	Transciever_None,
	Transciever_TX,
	Transciever_RX,
	Transciever_TXRX
};

struct __attribute__((packed)) Frame_t {
  int16_t channel1;
  int16_t channel2;
};

enum BitLength_t
{
  BitLength_32,
  BitLength_16,
  BitLength_8,
};

enum DataType_t
{
  DataType_bool_t,
  DataType_Int8_t,
  DataType_Int16_t,
  DataType_Int32_t,
  DataType_Uint8_t,
  DataType_Uint16_t,
  DataType_Uint32_t,
  DataType_String_t,
  DataType_SSID_Info_t,
  DataType_BT_Info_With_LastUpdateTime_t,
  DataType_Float_t,
  DataType_Double_t,
  DataType_ProcessedSoundData_t,
  DataType_MaxBandSoundData_t,
  DataType_Frame_t,
  DataType_ProcessedSoundFrame_t,
  DataType_SoundState_t,
  DataType_ConnectionStatus_t,
  DataType_Undef,
};

static const char* DataTypeStrings[] =
{
  "bool_t",
  "Int8_t",
  "Int16_t",
  "Int32_t",
  "Uint8_t",
  "Uint16_t",
  "Uint32_t",
  "String_t",
  "SSID_Info_t",
  "BT_Info_With_LastUpdateTime_t",
  "Float_t",
  "Double_t",
  "ProcessedSoundData_t",
  "MaxBandSoundData_t",
  "Frame_t",
  "ProcessedSoundFrame_t",
  "SoundState_t",
  "ConnectionStatus_t",
  "Undefined_t"
};

struct DataItemConfig_t
{
  String Name;
  DataType_t DataType;
  size_t Count;
  Transciever_t TransceiverConfig;
  size_t QueueCount;
};

enum ConnectionStatus_t
{
	Disconnected,
	Waiting,
	Searching,
	Pairing,
	Paired
};

static const char* ConnectionStatusStrings[] =
{
	"Disconnected",
	"Waiting",
	"Searching",
	"Pairing",
	"Paired"
};

struct NamedObject_t
{
	void* Object;
	String Name = "";
	
	// Destructor to release memory when the object is destroyed.
	~NamedObject_t()
	{
		if (Object != nullptr)
		{
			// Assuming Object points to dynamically allocated memory.
			free(Object);
			Object = nullptr;
		}
	}
};

struct NamedCallback_t
{
	void (*Callback)(const String& name, void* callback);
	String Name = "";
};

struct DataItem_t
{
	String Name;
	QueueHandle_t QueueHandle_RX = NULL;
	QueueHandle_t QueueHandle_TX = NULL;
	Transciever_t TransceiverConfig;
	DataType_t DataType;
	size_t Count = 0;
	size_t TotalByteCount = 0;
	bool DataPushHasErrored = false;
};

struct ProcessedSoundData_t
{
	float NormalizedPower;
	int32_t Minimum;
	int32_t Maximum;
};

struct ProcessedSoundFrame_t
{
	ProcessedSoundData_t Channel1;
	ProcessedSoundData_t Channel2;
};

struct MaxBandSoundData_t
{
	float MaxBandNormalizedPower;
	int16_t MaxBandIndex;
	int16_t TotalBands;
};


class DataTypeFunctions
{
	public:			
		DataType_t GetDataTypeFromString(String DataType)
		{
			for(int i = 0; i < sizeof(DataTypeStrings) / sizeof(DataTypeStrings[0]); ++i)
			{
				if(DataType.equals(DataTypeStrings[i]))return (DataType_t)i;
			}
			return DataType_Undef;
		}

		template <typename T>
		DataType_t GetDataTypeFromTemplateType()
		{
			DataType_t Result;
			if(		std::is_same<T, bool>::value) 								return DataType_bool_t;
			else if(std::is_same<T, int8_t>::value) 							return DataType_Int8_t;
			else if(std::is_same<T, int16_t>::value) 							return DataType_Int16_t;
			else if(std::is_same<T, int32_t>::value) 							return DataType_Int32_t;
			else if(std::is_same<T, uint8_t>::value) 							return DataType_Uint8_t;
			else if(std::is_same<T, uint16_t>::value) 							return DataType_Uint16_t;
			else if(std::is_same<T, uint32_t>::value) 							return DataType_Uint32_t;
			else if(std::is_same<T, String>::value) 							return DataType_String_t;
			else if(std::is_same<T, SSID_Info_t>::value) 						return DataType_SSID_Info_t;
			else if(std::is_same<T, BT_Info_With_LastUpdateTime_t>::value) 	return DataType_BT_Info_With_LastUpdateTime_t;
			else if(std::is_same<T, float>::value) 								return DataType_Float_t;
			else if(std::is_same<T, double>::value) 							return DataType_Double_t;
			else if(std::is_same<T, ProcessedSoundData_t>::value) 				return DataType_ProcessedSoundData_t;
			else if(std::is_same<T, MaxBandSoundData_t>::value) 				return DataType_MaxBandSoundData_t;
			else if(std::is_same<T, Frame_t>::value) 							return DataType_Frame_t;
			else if(std::is_same<T, ProcessedSoundFrame_t>::value) 				return DataType_ProcessedSoundFrame_t;
			else if(std::is_same<T, SoundState_t>::value) 						return DataType_SoundState_t;
			else if(std::is_same<T, ConnectionStatus_t>::value) 				return DataType_ConnectionStatus_t;
			else 																return DataType_Undef;
		}
		size_t GetSizeOfDataType(DataType_t DataType)
		{
			uint32_t Result = 0;
			switch(DataType)
			{
				case DataType_bool_t:
					Result = sizeof(bool);
				break;
				
				case DataType_Int8_t:
					Result = sizeof(int8_t);
				break;
				
				case DataType_Int16_t:
					Result = sizeof(int16_t);
				break;
				
				case DataType_Int32_t:
					Result = sizeof(int32_t);
				break;
				
				case DataType_Uint8_t:
					Result = sizeof(uint8_t);
				break;
				
				case DataType_Uint16_t:
					Result = sizeof(uint16_t);
				break;
				
				case DataType_Uint32_t:
					Result = sizeof(uint32_t);
				break;
				
				case DataType_String_t:
					Result = sizeof(String);
				break;
				
				case DataType_SSID_Info_t:
					Result = sizeof(SSID_Info_t);
				break;
				
				case DataType_BT_Info_With_LastUpdateTime_t:
					Result = sizeof(BT_Info_With_LastUpdateTime_t);
				break;
				
				case DataType_Float_t:
					Result = sizeof(float);
				break;
				
				case DataType_Double_t:
					Result = sizeof(double);
				break;
				
				case DataType_ProcessedSoundData_t:
					Result = sizeof(ProcessedSoundData_t);
				break;
				
				case DataType_MaxBandSoundData_t:
					Result = sizeof(MaxBandSoundData_t);
				break;
				
				case DataType_Frame_t:
					Result = sizeof(Frame_t);
				break;
				
				case DataType_ProcessedSoundFrame_t:
					Result = sizeof(ProcessedSoundFrame_t);
				break;
				
				case DataType_SoundState_t:
					Result = sizeof(SoundState_t);
				break;
				
				case DataType_ConnectionStatus_t:
					Result = sizeof(ConnectionStatus_t);
				break;
				
				default:
					Result = 0;
				break;
			}
			return Result;
		}
		bool SetValueFromFromStringForDataType(void *Buffer, String Value, DataType_t DataType)
		{
			bool Result = true;
			switch (DataType)
			{
			case DataType_bool_t:
				ESP_LOGD("DataTypeFunctions: SetValueFromFromStringForDataType", "Bool Received: %s", Value.c_str());
				*((bool *)Buffer) = Value.equals("true");
				break;
			case DataType_Int8_t:
				ESP_LOGD("DataTypeFunctions: SetValueFromFromStringForDataType", "Int8_t Received: %s", Value.c_str());
				*(int8_t *)Buffer = Value.toInt();
				break;
			case DataType_Int16_t:
				ESP_LOGD("DataTypeFunctions: SetValueFromFromStringForDataType", "Int16_t Received: %s", Value.c_str());
				*(int16_t *)Buffer = Value.toInt();
				break;
			case DataType_Int32_t:
				ESP_LOGD("DataTypeFunctions: SetValueFromFromStringForDataType", "Int32_t Received: %s", Value.c_str());
				*(int32_t *)Buffer = Value.toInt();
				break;
			case DataType_Uint8_t:
			case DataType_Uint16_t:
			case DataType_Uint32_t:
				ESP_LOGD("DataTypeFunctions: SetValueFromFromStringForDataType", "UInt Received: %s", Value.c_str());
				Value.getBytes((byte *)Buffer, Value.length());
				break;
			case DataType_Float_t:
				ESP_LOGD("DataTypeFunctions: SetValueFromFromStringForDataType", "Float_t Received: %s", Value.c_str());
				*(float *)Buffer = Value.toFloat();
				break;
			case DataType_Double_t:
				ESP_LOGD("DataTypeFunctions: SetValueFromFromStringForDataType", "Double_t Received: %s", Value.c_str());
				*(double *)Buffer = Value.toDouble();
				break;
			case DataType_String_t:
			case DataType_SSID_Info_t:
			case DataType_BT_Info_With_LastUpdateTime_t:
			case DataType_ProcessedSoundData_t:
			case DataType_MaxBandSoundData_t:
			case DataType_Frame_t:
			case DataType_ProcessedSoundFrame_t:
			case DataType_SoundState_t:
			case DataType_ConnectionStatus_t:
				Result = false;
				break;
			default:
				Result = false;
				break;
			}

			return Result;
		}
		String GetValueAsStringForDataType(const void *Buffer, DataType_t DataType, size_t Count)
		{
			String resultString;
			for (int i = 0; i < Count; ++i)
			{
				if (i > 0)
					resultString += " ";

				switch (DataType)
				{
				case DataType_bool_t:
					resultString += (*((const bool *)Buffer + i) ? "true" : "false");
					break;
				case DataType_Int8_t:
					resultString += String(*((const int8_t *)Buffer + i));
					break;
				case DataType_Int16_t:
					resultString += String(*((const int16_t *)Buffer + i));
					break;
				case DataType_Int32_t:
				case DataType_ConnectionStatus_t:
					resultString += String(*((const int32_t *)Buffer + i));
					break;
				case DataType_Uint8_t:
				case DataType_Uint16_t:
				case DataType_Uint32_t:
					resultString += String(((const char *)Buffer)[i]);
					break;
				case DataType_Float_t:
					resultString += String(*((const float *)Buffer + i));
					break;
				case DataType_Double_t:
					resultString += String(*((const double *)Buffer + i));
					break;
				case DataType_String_t:
				case DataType_SSID_Info_t:
				case DataType_BT_Info_With_LastUpdateTime_t:
				case DataType_ProcessedSoundData_t:
				case DataType_MaxBandSoundData_t:
				case DataType_Frame_t:
				case DataType_ProcessedSoundFrame_t:
				case DataType_SoundState_t:
					resultString = "Unsupported Data Type";
					break;
				default:
					resultString = "Invalid Data Type";
					break;
				}
			}

			return resultString;
		}
};

#endif
