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

template <typename T>
struct MyAllocator
{
	using value_type = T;

	T* allocate(std::size_t n)
	{
		// Implement your custom allocation logic here
		return static_cast<T*>(heap_caps_malloc(sizeof(T) * n, MALLOC_CAP_SPIRAM));
	}

	void deallocate(T* p, std::size_t n)
	{
		// Implement your custom deallocation logic here
		heap_caps_free(p);
	}
};

struct KeyValueTuple
{
  String Key;
  String Value1;
  String Value2;
};
typedef KeyValueTuple KVT;

struct BT_Device_Info
{
	public:
		BT_Device_Info(){}
		BT_Device_Info(String NAME_In, int32_t RSSI_In = 0)
		{
			if(248 < NAME_In.length())
			{
				Serial << "Bad NAME: " << NAME_In.c_str() << " | " << NAME_In.length() << "\n";
				assert(248 >= NAME_In.length());
			}
			snprintf(NAME, 248, "%s", NAME_In.c_str());
			RSSI = RSSI_In;
		}
		BT_Device_Info(String NAME_In, String ADDRESS_In, int32_t RSSI_In = 0)
		{
			if(248 < NAME_In.length())
			{
				Serial << "Bad NAME: " << NAME_In.c_str() << " | " << NAME_In.length() << "\n";
				assert(248 >= NAME_In.length());
			}
			snprintf(NAME, 248, "%s", NAME_In.c_str());
			if(18 < ADDRESS_In.length())
			{
				Serial << "Bad ADDRESS: " << ADDRESS_In.c_str() << " | " << ADDRESS_In.length() << "\n";
				assert(18 >= ADDRESS_In.length());
			}
			snprintf(ADDRESS, 18, "%s", ADDRESS_In.c_str());
			RSSI = RSSI_In;
		}
		char NAME[248] = "\0";
		char ADDRESS[18] = "\0";
		int32_t RSSI = 0;
};
typedef BT_Device_Info BT_Device_Info_t;

struct BT_Device_Info_With_LastUpdateTime_t
{
	public:
		BT_Device_Info_With_LastUpdateTime_t(){}
		BT_Device_Info_With_LastUpdateTime_t(String NAME_In, String ADDRESS_In, uint32_t TimeSinceUdpate_in, int32_t RSSI_In = 0)
		{
			if(248 < NAME_In.length())
			{
				Serial << "Bad SSID: " << NAME_In.c_str() << " | " << NAME_In.length() << "\n";
				assert(248 >= NAME_In.length());
			}
			snprintf(NAME, 248, "%s", NAME_In.c_str());
			if(18 < ADDRESS_In.length())
			{
				Serial << "Bad ADDRESS: " << ADDRESS_In.c_str() << " | " << ADDRESS_In.length() << "\n";
				assert(18 >= ADDRESS_In.length());
			}
			snprintf(ADDRESS, 18, "%s", ADDRESS_In.c_str());
			TimeSinceUdpate = TimeSinceUdpate_in;
			RSSI = RSSI_In;
		}
		BT_Device_Info_With_LastUpdateTime_t& operator=(const BT_Device_Info_With_LastUpdateTime_t& other)
		{
			strncpy(this->NAME, other.NAME, sizeof(this->NAME) - 1);
			this->NAME[sizeof(this->NAME) - 1] = '\0';  // Ensure null-terminated

			strncpy(this->ADDRESS, other.ADDRESS, sizeof(this->ADDRESS) - 1);
			this->ADDRESS[sizeof(this->ADDRESS) - 1] = '\0';  // Ensure null-terminated

			this->RSSI = other.RSSI;
			this->TimeSinceUdpate = other.TimeSinceUdpate;

			return *this;
		}
		bool operator==(const BT_Device_Info_With_LastUpdateTime_t& other) const
		{
			if( strcmp(this->NAME, other.NAME) == 0 &&
				strcmp(this->ADDRESS, other.ADDRESS) == 0 &&
				this->RSSI == other.RSSI &&
				this->TimeSinceUdpate == other.TimeSinceUdpate) return true;
			else return false;
		}
		bool operator!=(const BT_Device_Info_With_LastUpdateTime_t& other) const
		{
			if( strcmp(this->NAME, other.NAME) != 0 ||
				strcmp(this->ADDRESS, other.ADDRESS) != 0 ||
				this->RSSI != other.RSSI ||
				this->TimeSinceUdpate != other.TimeSinceUdpate) return true;
			else return false;
		}
		char NAME[248] = "\0";
		char ADDRESS[18] = "\0";
		int32_t RSSI = 0;
		uint32_t TimeSinceUdpate = 0;
};
typedef BT_Device_Info_With_LastUpdateTime_t BT_Device_Info_With_LastUpdateTime_t_t;

struct  CompatibleDevice_t
{
	public:
		CompatibleDevice_t(){}
		CompatibleDevice_t(String NAME_In, String ADDRESS_In)
		{
			if(248 < NAME_In.length())
			{
				Serial << "Bad SSID: " << NAME_In.c_str() << " | " << NAME_In.length() << "\n";
				assert(248 >= NAME_In.length());
			}
			snprintf(NAME, 248, "%s", NAME_In.c_str());
			if(18 < ADDRESS_In.length())
			{
				Serial << "Bad ADDRESS: " << ADDRESS_In.c_str() << " | " << ADDRESS_In.length() << "\n";
				assert(18 >= ADDRESS_In.length());
			}
			snprintf(ADDRESS, 18, "%s", ADDRESS_In.c_str());
		}
		CompatibleDevice_t& operator=(const CompatibleDevice_t& other)
		{
			strncpy(this->NAME, other.NAME, sizeof(this->NAME) - 1);
			this->NAME[sizeof(this->NAME) - 1] = '\0';  // Ensure null-terminated

			strncpy(this->ADDRESS, other.ADDRESS, sizeof(this->ADDRESS) - 1);
			this->ADDRESS[sizeof(this->ADDRESS) - 1] = '\0';  // Ensure null-terminated
			return *this;
		}
		bool operator==(const CompatibleDevice_t& other) const
		{
			if( strcmp(this->NAME, other.NAME) == 0 &&
				strcmp(this->ADDRESS, other.ADDRESS) == 0) return true;
			else return false;
		}
		bool operator!=(const BT_Device_Info_With_LastUpdateTime_t& other) const
		{
			if( strcmp(this->NAME, other.NAME) != 0 ||
				strcmp(this->ADDRESS, other.ADDRESS) != 0) return true;
			else return false;
		}
		char NAME[248] = "\0";
		char ADDRESS[18] = "\0";
};

struct ActiveCompatibleDevice_t
{
	String NAME;
	String ADDRESS;
	int32_t RSSI;
	unsigned long LastUpdateTime;
};

enum SoundInputSource
{
  SoundInputSource_OFF,
  SoundInputSource_Microphone,
  SoundInputSource_Bluetooth,
  SoundInputSource_Count
};
typedef SoundInputSource SoundInputSource_t;

enum SoundOutputSource
{
  SoundOutputSource_OFF,
  SoundOutputSource_Bluetooth,
  SoundOutputSource_Count
};
typedef SoundOutputSource SoundOutputSource_t;

enum Mute_State_t
{
  Mute_State_Un_Muted = 0,
  Mute_State_Muted,
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
  DataType_Bool_t,
  DataType_Int8_t,
  DataType_Int16_t,
  DataType_Int32_t,
  DataType_Uint8_t,
  DataType_Uint16_t,
  DataType_Uint32_t,
  DataType_Char_t,
  DataType_String_t,
  DataType_BT_Device_Info_t,
  DataType_BT_Device_Info_With_LastUpdateTime_t,
  DataType_Float_t,
  DataType_Double_t,
  DataType_ProcessedSoundData_t,
  DataType_MaxBandSoundData_t,
  DataType_Frame_t,
  DataType_ProcessedSoundFrame_t,
  DataType_SoundState_t,
  DataType_ConnectionStatus_t,
  DataType_SoundInputSource_t,
  DataType_SoundOutputSource_t,
  DataType_Undef,
};

static const char* DataTypeStrings[] =
{
  "Bool_t",
  "Int8_t",
  "Int16_t",
  "Int32_t",
  "Uint8_t",
  "Uint16_t",
  "Uint32_t",
  "Char_t",
  "String_t",
  "BT_Device_Info_t",
  "BT_Info_With_LastUpdateTime_t",
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
	const String& Name;
    void (*Callback)(const String& name, void* callback);

    NamedCallback_t(const String& name, void (*callback)(const String& name, void* callback))
        : Name(name), Callback(callback)
    {
    }
	NamedCallback_t(const NamedCallback_t& other)
        : Name(other.Name), Callback(other.Callback)
    {
    }
    bool operator==(const NamedCallback_t& other) const
    {
        return this->Name.equals(other.Name) && this->Callback == other.Callback;
    }

    NamedCallback_t& operator=(const NamedCallback_t& other)
    {
        // Use the constructor with the member initializer list to initialize 'Name'
        return *this = NamedCallback_t(other.Name, other.Callback);
    }
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
			ESP_LOGE("DataTypes: GetDataTypeFromString: %s", "GetDataTypeFromString: \"%s\": Undefined Data Type", DataType.c_str());
			return DataType_Undef;
		}

		template <typename T>
		DataType_t GetDataTypeFromTemplateType()
		{
			DataType_t Result;
			if(	std::is_same<T, bool>::value) 										return DataType_Bool_t;
			else if(std::is_same<T, int8_t>::value) 								return DataType_Int8_t;
			else if(std::is_same<T, int16_t>::value) 								return DataType_Int16_t;
			else if(std::is_same<T, int32_t>::value) 								return DataType_Int32_t;
			else if(std::is_same<T, uint8_t>::value) 								return DataType_Uint8_t;
			else if(std::is_same<T, uint16_t>::value) 								return DataType_Uint16_t;
			else if(std::is_same<T, uint32_t>::value) 								return DataType_Uint32_t;
			else if(std::is_same<T, char>::value) 									return DataType_Char_t;
			else if(std::is_same<T, String>::value) 								return DataType_String_t;
			else if(std::is_same<T, BT_Device_Info_t>::value) 						return DataType_BT_Device_Info_t;
			else if(std::is_same<T, BT_Device_Info_With_LastUpdateTime_t>::value) 	return DataType_BT_Device_Info_With_LastUpdateTime_t;
			else if(std::is_same<T, float>::value) 									return DataType_Float_t;
			else if(std::is_same<T, double>::value) 								return DataType_Double_t;
			else if(std::is_same<T, ProcessedSoundData_t>::value) 					return DataType_ProcessedSoundData_t;
			else if(std::is_same<T, MaxBandSoundData_t>::value) 					return DataType_MaxBandSoundData_t;
			else if(std::is_same<T, Frame_t>::value) 								return DataType_Frame_t;
			else if(std::is_same<T, ProcessedSoundFrame_t>::value) 					return DataType_ProcessedSoundFrame_t;
			else if(std::is_same<T, SoundState_t>::value) 							return DataType_SoundState_t;
			else if(std::is_same<T, ConnectionStatus_t>::value) 					return DataType_ConnectionStatus_t;
			else if(std::is_same<T, SoundInputSource_t>::value)						return DataType_SoundInputSource_t;
			else if(std::is_same<T, SoundOutputSource_t>::value)					return DataType_SoundOutputSource_t;
			else
			{
				ESP_LOGE("DataTypes: GetDataTypeFromTemplateType", "Undefined Data Type");
				return DataType_Undef;
			}
		}
		size_t GetSizeOfDataType(DataType_t DataType)
		{
			uint32_t Result = 0;
			switch(DataType)
			{
				case DataType_Bool_t:
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
				
				case DataType_Char_t:
					Result = sizeof(char);
				break;
				
				case DataType_String_t:
					Result = sizeof(String);
				break;
				
				case DataType_BT_Device_Info_t:
					Result = sizeof(BT_Device_Info_t);
				break;
				
				case DataType_BT_Device_Info_With_LastUpdateTime_t:
					Result = sizeof(BT_Device_Info_With_LastUpdateTime_t);
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
				
				case DataType_SoundInputSource_t:
					Result = sizeof(SoundInputSource_t);
				break;
				
				case DataType_SoundOutputSource_t:
					Result = sizeof(SoundOutputSource_t);
				break;
				
				default:
					ESP_LOGE("DataTypes: GetSizeOfDataType: %s", "GetSizeOfDataType: \"%s\": Undefined Data Type", DataTypeStrings[DataType]);
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
			case DataType_Bool_t:
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
			case DataType_Char_t:
				ESP_LOGD("DataTypeFunctions: SetValueFromFromStringForDataType", "Char_t Received: %s", Value.c_str());
				*(char *)Buffer = Value[0];
				break;
			case DataType_SoundInputSource_t:
				ESP_LOGD("DataTypeFunctions: SetValueFromFromStringForDataType", "SoundInputSource_t Received: %s", Value.c_str());
				*(SoundInputSource_t *)Buffer = static_cast<SoundInputSource_t>(Value.toInt());
				break;
			case DataType_SoundOutputSource_t:
				ESP_LOGD("DataTypeFunctions: SetValueFromFromStringForDataType", "SoundOutputSource_t Received: %s", Value.c_str());
				*(SoundOutputSource_t *)Buffer = static_cast<SoundOutputSource_t>(Value.toInt());
				break;
			case DataType_String_t:
			case DataType_BT_Device_Info_t:
			case DataType_BT_Device_Info_With_LastUpdateTime_t:
			case DataType_ProcessedSoundData_t:
			case DataType_MaxBandSoundData_t:
			case DataType_Frame_t:
			case DataType_ProcessedSoundFrame_t:
			case DataType_SoundState_t:
			case DataType_ConnectionStatus_t:
				Result = false;
				break;
			default:
				ESP_LOGE("DataTypes: SetValueFromFromStringForDataType", "SetValueFromStringForDataType: \"%s\": Undefined Data Type", DataTypeStrings[DataType]);
				Result = false;
				break;
			}

			return Result;
		}
		String GetValueAsStringForDataType(const void *Buffer, DataType_t DataType, size_t Count, const String &Divider)
		{
			String resultString;
			for (int i = 0; i < Count; ++i)
			{
				if (i > 0 && Divider.length() > 0) resultString += Divider;
				switch (DataType)
				{
				case DataType_Bool_t:
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
				case DataType_Char_t:
					resultString += String(*((const char *)Buffer + i));
					break;
				case DataType_SoundInputSource_t:
					resultString += String(*((const SoundInputSource_t *)Buffer + i));
				break;
				case DataType_SoundOutputSource_t:
					resultString += String(*((const SoundOutputSource_t *)Buffer + i));
				break;
				case DataType_String_t:
				case DataType_BT_Device_Info_t:
				case DataType_BT_Device_Info_With_LastUpdateTime_t:
				case DataType_ProcessedSoundData_t:
				case DataType_MaxBandSoundData_t:
				case DataType_Frame_t:
				case DataType_ProcessedSoundFrame_t:
				case DataType_SoundState_t:
					resultString = "Unsupported Data Type";
					break;
				default:
					ESP_LOGE("DataTypes: GetValueAsStringForDataType", "GetValueFromStringForDataType: \"%s\": Undefined Data Type", DataTypeStrings[DataType]);
					resultString = "Invalid Data Type";
					break;
				}
			}

			return resultString;
		}
};

#endif
