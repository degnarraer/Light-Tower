#ifndef DataTypes_H
#define DataTypes_H

#include "Arduino.h"
#include "Streaming.h"

#define BT_NAME_LENGTH 50
#define BT_ADDRESS_LENGTH 18

#define THREAD_PRIORITY_RT configMAX_PRIORITIES
#define THREAD_PRIORITY_HIGH configMAX_PRIORITIES-10
#define THREAD_PRIORITY_MEDIUM configMAX_PRIORITIES-20
#define THREAD_PRIORITY_LOW configMAX_PRIORITIES-30


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
		BT_Device_Info(const char* name_In, int32_t rssi_In = 0)
		{
			if(BT_NAME_LENGTH < String(name_In).length())
			{
				Serial << "Bad name: " << String(name_In).c_str() << " | " << String(name_In).length() << "\n";
				assert(BT_NAME_LENGTH >= String(name_In).length());
			}
			snprintf(name, BT_NAME_LENGTH, "%s", name_In);
			rssi = rssi_In;
		}
		BT_Device_Info(const char* name_In, const char* address_In, int32_t rssi_In = 0)
		{
			if(BT_NAME_LENGTH < String(name_In).length())
			{
				Serial << "Bad name: " << name_In << " | " << String(name_In).length() << "\n";
				assert(BT_NAME_LENGTH >= String(name_In).length());
			}
			snprintf(name, BT_NAME_LENGTH, "%s", name_In);
			if(BT_ADDRESS_LENGTH < String(address_In).length())
			{
				Serial << "Bad ADDRESS: " << address_In << " | " << String(address_In).length() << "\n";
				assert(BT_ADDRESS_LENGTH >= String(address_In).length());
			}
			snprintf(address, BT_ADDRESS_LENGTH, "%s", address_In);
			rssi = rssi_In;
		}
		char name[BT_NAME_LENGTH] = "\0";
		char address[BT_ADDRESS_LENGTH] = "\0";
		int32_t rssi = 0;
};
typedef BT_Device_Info BT_Device_Info_t;

struct BT_Device_Info_With_Time_Since_Update
{
	public:
		BT_Device_Info_With_Time_Since_Update(){}
		BT_Device_Info_With_Time_Since_Update(const char* name_In, const char* address_In, uint32_t timeSinceUdpate_in, int32_t rssi_In = 0)
		{
			if(BT_NAME_LENGTH < String(name_In).length())
			{
				Serial << "Bad SSID: " << name_In << " | " << String(name_In).length() << "\n";
				assert(BT_NAME_LENGTH >= String(name_In).length());
			}
			snprintf(name, BT_NAME_LENGTH, "%s", name_In);
			if(BT_ADDRESS_LENGTH < String(address_In).length())
			{
				Serial << "Bad address: " << address_In << " | " << String(address_In).length() << "\n";
				assert(BT_ADDRESS_LENGTH >= String(address_In).length());
			}
			snprintf(address, BT_ADDRESS_LENGTH, "%s", address_In);
			timeSinceUdpate = timeSinceUdpate_in;
			rssi = rssi_In;
		}
		BT_Device_Info_With_Time_Since_Update& operator=(const BT_Device_Info_With_Time_Since_Update& other)
		{
			strncpy(this->name, other.name, sizeof(this->name) - 1);
			this->name[sizeof(this->name) - 1] = '\0';  // Ensure null-terminated

			strncpy(this->address, other.address, sizeof(this->address) - 1);
			this->address[sizeof(this->address) - 1] = '\0';  // Ensure null-terminated

			this->rssi = other.rssi;
			this->timeSinceUdpate = other.timeSinceUdpate;

			return *this;
		}
		bool operator==(const BT_Device_Info_With_Time_Since_Update& other) const
		{
			if( strcmp(this->name, other.name) == 0 &&
				strcmp(this->address, other.address) == 0 &&
				this->rssi == other.rssi &&
				this->timeSinceUdpate == other.timeSinceUdpate) return true;
			else return false;
		}
		bool operator!=(const BT_Device_Info_With_Time_Since_Update& other) const
		{
			if( strcmp(this->name, other.name) != 0 ||
				strcmp(this->address, other.address) != 0 ||
				this->rssi != other.rssi ||
				this->timeSinceUdpate != other.timeSinceUdpate) return true;
			else return false;
		}
		char name[BT_NAME_LENGTH] = "\0";
		char address[BT_ADDRESS_LENGTH] = "\0";
		int32_t rssi = 0;
		uint32_t timeSinceUdpate = 0;
};
typedef BT_Device_Info_With_Time_Since_Update BT_Device_Info_With_Time_Since_Update_t;

struct  CompatibleDevice_t
{
	public:
		CompatibleDevice_t(){}
		CompatibleDevice_t(const char* name_In, const char* address_In)
		{
			if(BT_NAME_LENGTH < String(name_In).length())
			{
				Serial << "Bad SSID: " << name_In << " | " << String(name_In).length() << "\n";
				assert(BT_NAME_LENGTH >= String(name_In).length());
			}
			snprintf(name, BT_NAME_LENGTH, "%s", name_In);
			if(BT_ADDRESS_LENGTH < String(address_In).length())
			{
				Serial << "Bad address: " << address_In << " | " << String(address_In).length() << "\n";
				assert(BT_ADDRESS_LENGTH >= String(address_In).length());
			}
			snprintf(address, BT_ADDRESS_LENGTH, "%s", address_In);
		}
		CompatibleDevice_t& operator=(const CompatibleDevice_t& other)
		{
			strncpy(this->name, other.name, sizeof(this->name) - 1);
			this->name[sizeof(this->name) - 1] = '\0';  // Ensure null-terminated

			strncpy(this->address, other.address, sizeof(this->address) - 1);
			this->address[sizeof(this->address) - 1] = '\0';  // Ensure null-terminated
			return *this;
		}
		bool operator==(const CompatibleDevice_t& other) const
		{
			if( strcmp(this->name, other.name) == 0 &&
				strcmp(this->address, other.address) == 0) return true;
			else return false;
		}
		bool operator!=(const CompatibleDevice_t& other) const
		{
			if( strcmp(this->name, other.name) != 0 ||
				strcmp(this->address, other.address) != 0) return true;
			else return false;
		}
		char name[BT_NAME_LENGTH] = "\0";
		char address[BT_ADDRESS_LENGTH] = "\0";
};

struct ActiveCompatibleDevice_t
{
	public:
		ActiveCompatibleDevice_t(){}
		ActiveCompatibleDevice_t(String name_In, String address_In)
		{
			if(BT_NAME_LENGTH < name_In.length())
			{
				Serial << "Bad SSID: " << name_In.c_str() << " | " << name_In.length() << "\n";
				assert(BT_NAME_LENGTH >= name_In.length());
			}
			strncpy(this->name, name_In.c_str(), sizeof(this->name) - 1);
			if(BT_ADDRESS_LENGTH < address_In.length())
			{
				Serial << "Bad address: " << address_In.c_str() << " | " << address_In.length() << "\n";
				assert(BT_ADDRESS_LENGTH >= address_In.length());
			}
			strncpy(this->address, address_In.c_str(), sizeof(this->address) - 1);
		}
		ActiveCompatibleDevice_t(String name_In, String address_In, int32_t rssi_in, unsigned long lastUpdateTime_in, uint32_t timeSinceUpdate_in)
		{
			if(BT_NAME_LENGTH < name_In.length())
			{
				Serial << "Bad SSID: " << name_In.c_str() << " | " << name_In.length() << "\n";
				assert(BT_NAME_LENGTH >= name_In.length());
			}
			snprintf(name, BT_NAME_LENGTH, "%s", name_In.c_str());
			if(BT_ADDRESS_LENGTH < address_In.length())
			{
				Serial << "Bad address: " << address_In.c_str() << " | " << address_In.length() << "\n";
				assert(BT_ADDRESS_LENGTH >= address_In.length());
			}
			snprintf(address, BT_ADDRESS_LENGTH, "%s", address_In.c_str());
			rssi = rssi_in;
			lastUpdateTime = lastUpdateTime_in;
			timeSinceUpdate = timeSinceUpdate_in;
		}
		ActiveCompatibleDevice_t& operator=(const ActiveCompatibleDevice_t& other)
		{
			strncpy(this->name, other.name, sizeof(this->name) - 1);
			this->name[sizeof(this->name) - 1] = '\0';  // Ensure null-terminated

			strncpy(this->address, other.address, sizeof(this->address) - 1);
			this->address[sizeof(this->address) - 1] = '\0';  // Ensure null-terminated
			return *this;
		}
		bool operator==(const ActiveCompatibleDevice_t& other) const
		{
			if( strcmp(this->name, other.name) == 0 &&
				strcmp(this->address, other.address) == 0) return true;
			else return false;
		}
		bool operator!=(const ActiveCompatibleDevice_t& other) const
		{
			if( strcmp(this->name, other.name) != 0 ||
				strcmp(this->address, other.address) != 0) return true;
			else return false;
		}
	char name[BT_NAME_LENGTH] = "\0";
	char address[BT_ADDRESS_LENGTH] = "\0";
	int32_t rssi;
	unsigned long lastUpdateTime;
	uint32_t timeSinceUpdate = 0;
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
  DataType_BT_Device_Info_With_Time_Since_Update_t,
  DataType_CompatibleDevice_t,
  DataType_ActiveCompatibleDevice_t,
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
	Disconnected = 0,
	Connecting = 1,
	Connected = 2,
	Disconnecting = 3
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
    String Name;
    void (*Callback)(const String& name, void* callback, void* arg);
    void* Arg;
	NamedCallback_t(){}
    NamedCallback_t(String name, void (*callback)(const String& name, void* callback, void* arg), void* arg = nullptr)
        : Name(name), Callback(callback), Arg(arg)
    {
    }

    NamedCallback_t(const NamedCallback_t& other)
        : Name(other.Name), Callback(other.Callback), Arg(other.Arg)
    {
    }

    bool operator==(const NamedCallback_t& other) const
    {
        return this->Name.equals(other.Name) && this->Callback == other.Callback && this->Arg == other.Arg;
    }

    NamedCallback_t& operator=(const NamedCallback_t& other)
    {
        if (this != &other)
		{
			Name = other.Name;
			Callback = other.Callback;
			Arg = other.Arg;
		}
		return *this;
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
			if(	std::is_same<T, bool>::value) 											return DataType_Bool_t;
			else if(std::is_same<T, int8_t>::value) 									return DataType_Int8_t;
			else if(std::is_same<T, int16_t>::value) 									return DataType_Int16_t;
			else if(std::is_same<T, int32_t>::value) 									return DataType_Int32_t;
			else if(std::is_same<T, uint8_t>::value) 									return DataType_Uint8_t;
			else if(std::is_same<T, uint16_t>::value) 									return DataType_Uint16_t;
			else if(std::is_same<T, uint32_t>::value) 									return DataType_Uint32_t;
			else if(std::is_same<T, char>::value) 										return DataType_Char_t;
			else if(std::is_same<T, String>::value) 									return DataType_String_t;
			else if(std::is_same<T, BT_Device_Info_t>::value) 							return DataType_BT_Device_Info_t;
			else if(std::is_same<T, BT_Device_Info_With_Time_Since_Update_t>::value) 	return DataType_BT_Device_Info_With_Time_Since_Update_t;
			else if(std::is_same<T, CompatibleDevice_t>::value)							return DataType_CompatibleDevice_t;
			else if(std::is_same<T, ActiveCompatibleDevice_t>::value)					return DataType_ActiveCompatibleDevice_t;
			else if(std::is_same<T, float>::value) 										return DataType_Float_t;
			else if(std::is_same<T, double>::value) 									return DataType_Double_t;
			else if(std::is_same<T, ProcessedSoundData_t>::value) 						return DataType_ProcessedSoundData_t;
			else if(std::is_same<T, MaxBandSoundData_t>::value) 						return DataType_MaxBandSoundData_t;
			else if(std::is_same<T, Frame_t>::value) 									return DataType_Frame_t;
			else if(std::is_same<T, ProcessedSoundFrame_t>::value) 						return DataType_ProcessedSoundFrame_t;
			else if(std::is_same<T, SoundState_t>::value) 								return DataType_SoundState_t;
			else if(std::is_same<T, ConnectionStatus_t>::value) 						return DataType_ConnectionStatus_t;
			else if(std::is_same<T, SoundInputSource_t>::value)							return DataType_SoundInputSource_t;
			else if(std::is_same<T, SoundOutputSource_t>::value)						return DataType_SoundOutputSource_t;
			else
			{
				ESP_LOGE("DataTypes: GetDataTypeFromTemplateType", "Undefined Data Type");
				return DataType_Undef;
			}
		}
		size_t GetSizeOfDataType(DataType_t DataType)
		{
			uint32_t result = 0;
			switch(DataType)
			{
				case DataType_Bool_t:
					result = sizeof(bool);
				break;
				
				case DataType_Int8_t:
					result = sizeof(int8_t);
				break;
				
				case DataType_Int16_t:
					result = sizeof(int16_t);
				break;
				
				case DataType_Int32_t:
					result = sizeof(int32_t);
				break;
				
				case DataType_Uint8_t:
					result = sizeof(uint8_t);
				break;
				
				case DataType_Uint16_t:
					result = sizeof(uint16_t);
				break;
				
				case DataType_Uint32_t:
					result = sizeof(uint32_t);
				break;
				
				case DataType_Char_t:
					result = sizeof(char);
				break;
				
				case DataType_String_t:
					result = sizeof(String);
				break;
				
				case DataType_BT_Device_Info_t:
					result = sizeof(BT_Device_Info_t);
				break;
				
				case DataType_BT_Device_Info_With_Time_Since_Update_t:
					result = sizeof(BT_Device_Info_With_Time_Since_Update_t);
				break;
				
				case DataType_CompatibleDevice_t:
					result = sizeof(CompatibleDevice_t);
				break;
				
				case DataType_ActiveCompatibleDevice_t:
					result = sizeof(ActiveCompatibleDevice_t);
				break;
				
				case DataType_Float_t:
					result = sizeof(float);
				break;
				
				case DataType_Double_t:
					result = sizeof(double);
				break;
				
				case DataType_ProcessedSoundData_t:
					result = sizeof(ProcessedSoundData_t);
				break;
				
				case DataType_MaxBandSoundData_t:
					result = sizeof(MaxBandSoundData_t);
				break;
				
				case DataType_Frame_t:
					result = sizeof(Frame_t);
				break;
				
				case DataType_ProcessedSoundFrame_t:
					result = sizeof(ProcessedSoundFrame_t);
				break;
				
				case DataType_SoundState_t:
					result = sizeof(SoundState_t);
				break;
				
				case DataType_ConnectionStatus_t:
					result = sizeof(ConnectionStatus_t);
				break;
				
				case DataType_SoundInputSource_t:
					result = sizeof(SoundInputSource_t);
				break;
				
				case DataType_SoundOutputSource_t:
					result = sizeof(SoundOutputSource_t);
				break;
				
				default:
					ESP_LOGE("DataTypes: GetSizeOfDataType: %s", "GetSizeOfDataType: \"%s\": Undefined Data Type", DataTypeStrings[DataType]);
					result = 0;
				break;
			}
			return result;
		}
		bool SetValueFromStringForDataType(void *buffer, String stringValue, DataType_t dataType)
		{
			bool result = true;
			switch (dataType)
			{
			case DataType_Bool_t:
				ESP_LOGD("DataTypeFunctions: SetValueFromFromStringForDataType", "DataType_Bool_t Received: %s", value.c_str());
				*((bool *)buffer) = stringValue.equals("True");
				break;
			case DataType_Int8_t:
				ESP_LOGD("DataTypeFunctions: SetValueFromFromStringForDataType", "DataType_Int8_t Received: %s", value.c_str());
				*(int8_t *)buffer = stringValue.toInt();
				break;
			case DataType_Int16_t:
				ESP_LOGD("DataTypeFunctions: SetValueFromFromStringForDataType", "DataType_Int16_t Received: %s", value.c_str());
				*(int16_t *)buffer = stringValue.toInt();
				break;
			case DataType_Int32_t:
				ESP_LOGD("DataTypeFunctions: SetValueFromFromStringForDataType", "DataType_Int32_t Received: %s", value.c_str());
				*(int32_t *)buffer = stringValue.toInt();
				break;
			case DataType_Uint8_t:
			case DataType_Uint16_t:
			case DataType_Uint32_t:
				ESP_LOGD("DataTypeFunctions: SetValueFromFromStringForDataType", "DataType_Uint Received: %s", value.c_str());
				stringValue.getBytes((byte *)buffer, stringValue.length());
				break;
			case DataType_Float_t:
				ESP_LOGD("DataTypeFunctions: SetValueFromFromStringForDataType", "DataType_Float_t Received: %s", value.c_str());
				*(float *)buffer = stringValue.toFloat();
				break;
			case DataType_Double_t:
				ESP_LOGD("DataTypeFunctions: SetValueFromFromStringForDataType", "DataType_Double_t Received: %s", value.c_str());
				*(double *)buffer = stringValue.toDouble();
				break;
			case DataType_Char_t:
				ESP_LOGD("DataTypeFunctions: SetValueFromFromStringForDataType", "DataType_Char_t Received: %s", value.c_str());
				*(char *)buffer = stringValue[0];
				break;
			case DataType_SoundInputSource_t:
				ESP_LOGD("DataTypeFunctions: SetValueFromFromStringForDataType", "DataType_SoundInputSource_t Received: %s", value.c_str());
				*(SoundInputSource_t *)buffer = static_cast<SoundInputSource_t>(stringValue.toInt());
				break;
			case DataType_SoundOutputSource_t:
				ESP_LOGD("DataTypeFunctions: SetValueFromFromStringForDataType", "DataType_SoundOutputSource_t Received: %s", value.c_str());
				*(SoundOutputSource_t *)buffer = static_cast<SoundOutputSource_t>(stringValue.toInt());
				break;
			case DataType_ConnectionStatus_t:
				ESP_LOGD("DataTypeFunctions: SetValueFromFromStringForDataType", "DataType_ConnectionStatus_t Received: %s", value.c_str());
				*(SoundOutputSource_t *)buffer = static_cast<SoundOutputSource_t>(stringValue.toInt());
				break;
			case DataType_String_t:
			case DataType_BT_Device_Info_t:
			case DataType_BT_Device_Info_With_Time_Since_Update_t:
			case DataType_CompatibleDevice_t:
			case DataType_ActiveCompatibleDevice_t:
			case DataType_ProcessedSoundData_t:
			case DataType_MaxBandSoundData_t:
			case DataType_Frame_t:
			case DataType_ProcessedSoundFrame_t:
			case DataType_SoundState_t:
				ESP_LOGE( "DataTypes: SetValueFromFromStringForDataType", "Data Type Conversion to String for \"%s\": Not Yet Supported!", DataTypeStrings[dataType]);
				result = false;
				break;
			default:
				ESP_LOGE("DataTypes: SetValueFromFromStringForDataType", "SetValueFromStringForDataType: \"%s\": Undefined Data Type", DataTypeStrings[dataType]);
				result = false;
				break;
			}
			return result;
		}

		bool GetStringValueForDataType(String &stringValue, const void *buffer, DataType_t dataType, size_t count, const String &divider)
		{
			bool success = false;
			if(!buffer) return success;
			for (int i = 0; i < count; ++i)
			{
				if (i > 0 && divider.length() > 0) stringValue += divider;
				switch (dataType)
				{
				case DataType_Bool_t:
					stringValue += (*((const bool *)buffer + i) ? "True" : "False");
					success = true;
					break;
				case DataType_Int8_t:
					stringValue += String(*((const int8_t *)buffer + i));
					success = true;
					break;
				case DataType_Int16_t:
					stringValue += String(*((const int16_t *)buffer + i));
					success = true;
					break;
				case DataType_Int32_t:
				case DataType_ConnectionStatus_t:
					stringValue += String(*((const int32_t *)buffer + i));
					success = true;
					break;
				case DataType_Uint8_t:
				case DataType_Uint16_t:
				case DataType_Uint32_t:
					stringValue += String(((const char *)buffer)[i]);
					success = true;
					break;
				case DataType_Float_t:
					stringValue += String(*((const float *)buffer + i));
					success = true;
					break;
				case DataType_Double_t:
					stringValue += String(*((const double *)buffer + i));
					success = true;
					break;
				case DataType_Char_t:
					stringValue += String(*((const char *)buffer + i));
					success = true;
					break;
				case DataType_SoundInputSource_t:
					stringValue += String(*((const SoundInputSource_t *)buffer + i));
					success = true;
				break;
				case DataType_SoundOutputSource_t:
					stringValue += String(*((const SoundOutputSource_t *)buffer + i));
					success = true;
				break;
				case DataType_BT_Device_Info_With_Time_Since_Update_t:
				{
					const uint8_t* bufferPtr = reinterpret_cast<const uint8_t*>(buffer);
					const BT_Device_Info_With_Time_Since_Update_t* deviceInfo = reinterpret_cast<const BT_Device_Info_With_Time_Since_Update_t*>(bufferPtr + i);
					stringValue += String(deviceInfo->name) + " | ";
					stringValue += String(deviceInfo->address) + " | ";
					stringValue += String(deviceInfo->rssi) + " | ";
					stringValue += String(deviceInfo->timeSinceUdpate);
					success = true;
					break;
				}
				case DataType_CompatibleDevice_t:
				{
					const uint8_t* bufferPtr = reinterpret_cast<const uint8_t*>(buffer);
					const CompatibleDevice_t* compatibleDevice = reinterpret_cast<const CompatibleDevice_t*>(bufferPtr + i);
					stringValue += String(compatibleDevice->name) + " | ";
					stringValue += String(compatibleDevice->address);
					success = true;
					break;
				}
				case DataType_ActiveCompatibleDevice_t:
				{
					const uint8_t* bufferPtr = reinterpret_cast<const uint8_t*>(buffer);
					const ActiveCompatibleDevice_t* activeCompatibleDevice = reinterpret_cast<const ActiveCompatibleDevice_t*>(bufferPtr + i);
					stringValue += String(activeCompatibleDevice->name) + " | ";
					stringValue += String(activeCompatibleDevice->address) + " | ";
					stringValue += String(activeCompatibleDevice->rssi) + " | ";
					stringValue += String(activeCompatibleDevice->lastUpdateTime) + " | ";
					stringValue += String(activeCompatibleDevice->timeSinceUpdate);
					success = true;
					break;
				}
				case DataType_String_t:
				case DataType_BT_Device_Info_t:
				case DataType_ProcessedSoundData_t:
				case DataType_MaxBandSoundData_t:
				case DataType_Frame_t:
				case DataType_ProcessedSoundFrame_t:
				case DataType_SoundState_t:
					stringValue = "Unsupported Data Type";
					break;
				default:
					ESP_LOGE("DataTypes: GetValueAsStringForDataType", "GetValueFromStringForDataType: \"%s\": Undefined Data Type", DataTypeStrings[dataType]);
					stringValue = "Invalid Data Type";
					break;
				}
			}
			return success;
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
					resultString += (*((const bool *)Buffer + i) ? "True" : "False");
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
				case DataType_BT_Device_Info_With_Time_Since_Update_t:
				{
					const uint8_t* bufferPtr = reinterpret_cast<const uint8_t*>(Buffer);
					const BT_Device_Info_With_Time_Since_Update_t* deviceInfo = reinterpret_cast<const BT_Device_Info_With_Time_Since_Update_t*>(bufferPtr + i);
					resultString += String(deviceInfo->name) + " | ";
					resultString += String(deviceInfo->address) + " | ";
					resultString += String(deviceInfo->rssi) + " | ";
					resultString += String(deviceInfo->timeSinceUdpate);
					break;
				}
				case DataType_CompatibleDevice_t:
				{
					const uint8_t* bufferPtr = reinterpret_cast<const uint8_t*>(Buffer);
					const CompatibleDevice_t* compatibleDevice = reinterpret_cast<const CompatibleDevice_t*>(bufferPtr + i);
					resultString += String(compatibleDevice->name) + " | ";
					resultString += String(compatibleDevice->address);
					break;
				}
				case DataType_ActiveCompatibleDevice_t:
				{
					const uint8_t* bufferPtr = reinterpret_cast<const uint8_t*>(Buffer);
					const ActiveCompatibleDevice_t* activeCompatibleDevice = reinterpret_cast<const ActiveCompatibleDevice_t*>(bufferPtr + i);
					resultString += String(activeCompatibleDevice->name) + " | ";
					resultString += String(activeCompatibleDevice->address) + " | ";
					resultString += String(activeCompatibleDevice->rssi) + " | ";
					resultString += String(activeCompatibleDevice->lastUpdateTime) + " | ";
					resultString += String(activeCompatibleDevice->timeSinceUpdate);
					break;
				}
				case DataType_String_t:
				case DataType_BT_Device_Info_t:
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
