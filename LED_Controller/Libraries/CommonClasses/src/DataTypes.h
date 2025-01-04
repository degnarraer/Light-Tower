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

#include <iostream>
#include <string>
#include <sstream>
#include <type_traits>
#include "Streaming.h"

#define BT_NAME_LENGTH 50
#define BT_ADDRESS_LENGTH 18

#define THREAD_PRIORITY_RT configMAX_PRIORITIES - 1
#define THREAD_PRIORITY_HIGH configMAX_PRIORITIES - 5
#define THREAD_PRIORITY_MEDIUM configMAX_PRIORITIES - 10
#define THREAD_PRIORITY_LOW configMAX_PRIORITIES - 15
#define THREAD_PRIORITY_IDLE configMAX_PRIORITIES - 20

#define ENCODE_OBJECT_DIVIDER "|"
#define ENCODE_VALUE_DIVIDER ","


enum DeviceState
{
Installed,
Uninstalled,
Running,
Stopped
};
typedef DeviceState DeviceState_t;

struct UpdateStatus
{
    bool ValueChanged = false;
    bool ValidValue = false;
    bool UpdateAllowed = false;
    bool UpdateSuccessful = false;
    UpdateStatus()
    {
    }
    UpdateStatus(bool ValueChanged_in, bool ValidValue_in, bool UpdateAllowed_in, bool UpdateSuccessful_in)
    {
        bool ValueChanged = ValueChanged_in;
        bool ValidValue = ValidValue_in;
        bool UpdateAllowed = UpdateAllowed_in;
        bool UpdateSuccessful = UpdateSuccessful_in;
    }
    bool operator==(const UpdateStatus& other) const
    {
        return ( this->ValueChanged == other.ValueChanged &&
                 this->ValidValue == other.ValidValue &&
                 this->UpdateAllowed == other.UpdateAllowed &&
                 this->UpdateSuccessful == other.UpdateSuccessful );
    }

    UpdateStatus& operator|=(const UpdateStatus& other)
    {
        this->ValueChanged |= other.ValueChanged;
        this->ValidValue |= other.ValidValue;
        this->UpdateAllowed |= other.UpdateAllowed;
        this->UpdateSuccessful |= other.UpdateSuccessful;
        return *this;
    }
};
typedef UpdateStatus UpdateStatus_t;

template <typename T>
struct TemplateObjectWithChangeCount
{
    size_t ChangeCount;
    T Object;
};

template <typename T>
using TemplateObjectWithChangeCount_t = TemplateObjectWithChangeCount<T>;

class NamedItem
{
  public:
	NamedItem(std::string Title): m_Title(Title){}
	virtual ~NamedItem(){}
    std::string GetTitle()
    { 
      return m_Title;
    }
  private:
    std::string m_Title;
};

struct KeyValuePair
{
  std::string Key;
  std::string Value;
  KeyValuePair();
  KeyValuePair(std::string Key_in, std::string Value_in)
  {
    Key = Key_in;
    Value = Value_in;
  }
};
typedef KeyValuePair KVP;

template <typename T>
struct MyAllocator
{
	using value_type = T;

	T* allocate(std::size_t n)
	{
		return static_cast<T*>(malloc(sizeof(T) * n));
	}

	void deallocate(T* p, std::size_t n)
	{
		free(p);
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
		char name[BT_NAME_LENGTH+1] = "\0";
		char address[BT_ADDRESS_LENGTH+1] = "\0";
		int32_t rssi = 0;
        
		BT_Device_Info(){}
		BT_Device_Info(const char* name_In, int32_t rssi_In = 0)
		{
            strncpy(name, name_In, BT_NAME_LENGTH);
            name[BT_NAME_LENGTH] = '\0';
			rssi = rssi_In;
		}
		BT_Device_Info(const char* name_In, const char* address_In, int32_t rssi_In = 0)
		{
            strncpy(name, name_In, BT_NAME_LENGTH);
            name[BT_NAME_LENGTH] = '\0';
    
            strncpy(address, address_In, BT_ADDRESS_LENGTH);
            address[BT_ADDRESS_LENGTH] = '\0';
			rssi = rssi_In;
		}
		bool operator==(const BT_Device_Info& other) const
		{
			return ( strcmp(this->name, other.name) == 0 &&
				     strcmp(this->address, other.address) == 0 );
		}
		bool operator!=(const BT_Device_Info& other) const
		{
			return !(*this == other);
		}
};
typedef BT_Device_Info BT_Device_Info_t;

struct ActiveBluetoothDevice_t
{
public:
    char name[BT_NAME_LENGTH+1];
    char address[BT_ADDRESS_LENGTH+1];
    int32_t rssi;
    unsigned long lastUpdateTime;
    uint32_t timeSinceUpdate;
    
    ActiveBluetoothDevice_t() : rssi(0), lastUpdateTime(0), timeSinceUpdate(0)
    {
        name[0] = '\0';
        address[0] = '\0';
    }
    
	ActiveBluetoothDevice_t(const String &str)
	{
		*this =  fromString(str.c_str());
	}

    ActiveBluetoothDevice_t(const String& name_In, const String& address_In)
    {
        if (BT_NAME_LENGTH < name_In.length())
        {
            Serial << "Bad SSID: " << name_In.c_str() << " | " << name_In.length() << "\n";
            assert(BT_NAME_LENGTH >= name_In.length());
        }
        strncpy(this->name, name_In.c_str(), sizeof(this->name));
        this->name[sizeof(this->name)] = '\0';

        if (BT_ADDRESS_LENGTH < address_In.length())
        {
            Serial << "Bad address: " << address_In.c_str() << " | " << address_In.length() << "\n";
            assert(BT_ADDRESS_LENGTH >= address_In.length());
        }
        strncpy(this->address, address_In.c_str(), sizeof(this->address));
        this->address[sizeof(this->address)] = '\0';
    }

    ActiveBluetoothDevice_t(const String& name_In, const String& address_In, int32_t rssi_in, unsigned long lastUpdateTime_in, uint32_t timeSinceUpdate_in)
    {
        if (BT_NAME_LENGTH < name_In.length())
        {
            Serial << "Bad SSID: " << name_In.c_str() << " | " << name_In.length() << "\n";
            assert(BT_NAME_LENGTH >= name_In.length());
        }
        strncpy(this->name, name_In.c_str(), sizeof(this->name));
        this->name[sizeof(this->name)] = '\0';

        if (BT_ADDRESS_LENGTH < address_In.length())
        {
            Serial << "Bad address: " << address_In.c_str() << " | " << address_In.length() << "\n";
            assert(BT_ADDRESS_LENGTH >= address_In.length());
        }
        strncpy(this->address, address_In.c_str(), sizeof(this->address));
        this->address[sizeof(this->address)] = '\0';

        rssi = rssi_in;
        lastUpdateTime = lastUpdateTime_in;
        timeSinceUpdate = timeSinceUpdate_in;
    }
    
    ActiveBluetoothDevice_t(const BT_Device_Info& deviceInfo)
    {
        strncpy(this->name, deviceInfo.name, sizeof(this->name));
        this->name[sizeof(this->name)] = '\0';

        strncpy(this->address, deviceInfo.address, sizeof(this->address));
        this->address[sizeof(this->address)] = '\0';

        this->rssi = deviceInfo.rssi;
        this->lastUpdateTime = millis();
        this->timeSinceUpdate = 0;
    }

    ActiveBluetoothDevice_t& operator=(const ActiveBluetoothDevice_t& other)
    {
        if (this != &other)
        {
            strncpy(this->name, other.name, sizeof(this->name));
            this->name[sizeof(this->name)] = '\0';

            strncpy(this->address, other.address, sizeof(this->address));
            this->address[sizeof(this->address)] = '\0';

            rssi = other.rssi;
            lastUpdateTime = other.lastUpdateTime;
            timeSinceUpdate = other.timeSinceUpdate;
        }
        return *this;
    }

    ActiveBluetoothDevice_t& operator=(const BT_Device_Info& other)
    {
        strncpy(this->name, other.name, sizeof(this->name));
        this->name[sizeof(this->name)] = '\0';

        strncpy(this->address, other.address, sizeof(this->address));
        this->address[sizeof(this->address)] = '\0';

        rssi = other.rssi;
        lastUpdateTime = millis();
        timeSinceUpdate = 0;

        return *this;
    }

    bool operator==(const ActiveBluetoothDevice_t& other) const
    {
        return ( strcmp(this->name, other.name) == 0 &&
                 strcmp(this->address, other.address ) == 0 );
    }
    
    bool operator==(const BT_Device_Info& other) const
    {
        return ( strcmp(this->name, other.name) == 0 &&
                 strcmp(this->address, other.address ) == 0 );
    }

    bool operator!=(const ActiveBluetoothDevice_t& other) const
    {
        return !(*this == other);
    }

	operator std::string() const
	{
		return toString();
	}

    std::string toString() const
    {
        return std::string(name) + ENCODE_VALUE_DIVIDER + std::string(address) + ENCODE_VALUE_DIVIDER + std::to_string(rssi) + ENCODE_VALUE_DIVIDER + std::to_string(lastUpdateTime) + ENCODE_VALUE_DIVIDER + std::to_string(timeSinceUpdate);
    }

    static ActiveBluetoothDevice_t fromString(const std::string &str)
    {
        const std::string delimiter = ENCODE_VALUE_DIVIDER; // Assuming ENCODE_VALUE_DIVIDER is a string or char
        size_t delimiterIndex = str.find(delimiter);

        if (delimiterIndex == std::string::npos)
        {
            return ActiveBluetoothDevice_t();  // Return default if delimiter not found
        }

        // Extract name
        std::string name = str.substr(0, delimiterIndex);

        // Find the next delimiter for the address
        size_t nextDelimiterIndex = str.find(delimiter, delimiterIndex + delimiter.length());
        if (nextDelimiterIndex == std::string::npos)
        {
            return ActiveBluetoothDevice_t();
        }

        // Extract address
        std::string address = str.substr(delimiterIndex + delimiter.length(), nextDelimiterIndex - delimiterIndex - delimiter.length());

        // Find the next delimiter for RSSI
        size_t nextDelimiterIndex2 = str.find(delimiter, nextDelimiterIndex + delimiter.length());
        if (nextDelimiterIndex2 == std::string::npos)
        {
            return ActiveBluetoothDevice_t();
        }

        // Extract RSSI
        std::string rssiStr = str.substr(nextDelimiterIndex + delimiter.length(), nextDelimiterIndex2 - nextDelimiterIndex - delimiter.length());
        int rssi = std::stoi(rssiStr);

        // Find the next delimiter for lastUpdateTime
        size_t nextDelimiterIndex3 = str.find(delimiter, nextDelimiterIndex2 + delimiter.length());
        if (nextDelimiterIndex3 == std::string::npos)
        {
            return ActiveBluetoothDevice_t();
        }

        // Extract lastUpdateTime
        std::string lastUpdateTimeStr = str.substr(nextDelimiterIndex2 + delimiter.length(), nextDelimiterIndex3 - nextDelimiterIndex2 - delimiter.length());
        unsigned long lastUpdateTime = std::stoul(lastUpdateTimeStr);

        // Extract timeSinceUpdate
        std::string timeSinceUpdateStr = str.substr(nextDelimiterIndex3 + delimiter.length());
        uint32_t timeSinceUpdate = std::stoul(timeSinceUpdateStr);

        // Create and return the ActiveBluetoothDevice_t object
        return ActiveBluetoothDevice_t(name.c_str(), address.c_str(), rssi, lastUpdateTime, timeSinceUpdate);
    }

	
    friend std::istream& operator>>(std::istream& is, ActiveBluetoothDevice_t& device) {
        std::string str;
        std::getline(is, str);
        device = ActiveBluetoothDevice_t::fromString(str);
        return is;
    }

    friend std::ostream& operator<<(std::ostream& os, const ActiveBluetoothDevice_t& device) {
        os << device.toString().c_str();
        return os;
    }
};

struct BT_Device_Info_With_Time_Since_Update
{
	public:
		char name[BT_NAME_LENGTH] = "\0";
		char address[BT_ADDRESS_LENGTH] = "\0";
		int32_t rssi = 0;
		uint32_t timeSinceUpdate = 0;

		BT_Device_Info_With_Time_Since_Update(){}
		BT_Device_Info_With_Time_Since_Update(const char* name_In, const char* address_In, int32_t rssi_In, uint32_t timeSinceUdpate_in)
		{
            strncpy(name, name_In, BT_NAME_LENGTH);
            name[BT_NAME_LENGTH] = '\0';
    
            strncpy(address, address_In, BT_ADDRESS_LENGTH);
            address[BT_ADDRESS_LENGTH] = '\0';
			rssi = rssi_In;
			timeSinceUpdate = timeSinceUdpate_in;
		}

        BT_Device_Info_With_Time_Since_Update(const ActiveBluetoothDevice_t device_in, uint32_t timeSinceUpdate_in)
        {
            strncpy(name, device_in.name, BT_NAME_LENGTH);
            name[BT_NAME_LENGTH] = '\0';
    
            strncpy(address, device_in.address, BT_ADDRESS_LENGTH);
            address[BT_ADDRESS_LENGTH] = '\0';
			rssi = device_in.rssi;
            timeSinceUpdate = timeSinceUpdate_in;
        }
        
		BT_Device_Info_With_Time_Since_Update& operator=(const BT_Device_Info_With_Time_Since_Update& other)
		{
			strncpy(this->name, other.name, sizeof(this->name));
			this->name[sizeof(this->name)] = '\0';

			strncpy(this->address, other.address, sizeof(this->address));
			this->address[sizeof(this->address)] = '\0';

			this->rssi = other.rssi;
			this->timeSinceUpdate = other.timeSinceUpdate;

			return *this;
		}
		bool operator==(const BT_Device_Info_With_Time_Since_Update& other) const
		{
			return ( strcmp(this->name, other.name) == 0 &&
				     strcmp(this->address, other.address) == 0 );
		}
		bool operator!=(const BT_Device_Info_With_Time_Since_Update& other) const
		{
			return !(*this == other);
		}

		operator std::string() const
        {
            return toString();
        }

        std::string toString() const
        {
            return std::string(name) + ENCODE_VALUE_DIVIDER + std::string(address) + ENCODE_VALUE_DIVIDER + std::to_string(rssi) + ENCODE_VALUE_DIVIDER + std::to_string(timeSinceUpdate);
        }

		static BT_Device_Info_With_Time_Since_Update fromString(const std::string &str)
        {
            const std::string delimiter = ENCODE_VALUE_DIVIDER;
            std::vector<std::string> fields;
            size_t start = 0, delimiterIndex;

            while ((delimiterIndex = str.find(delimiter, start)) != std::string::npos)
            {
                fields.push_back(str.substr(start, delimiterIndex - start));
                start = delimiterIndex + delimiter.length();
            }
            fields.push_back(str.substr(start));

            if (fields.size() != 4)
            {
                return BT_Device_Info_With_Time_Since_Update();
            }

            try
            {
                const std::string& name = fields[0];
                const std::string& address = fields[1];
                int32_t rssi = std::stoi(fields[2]);
                uint32_t timeSinceUpdate = std::stoul(fields[3]);

                return BT_Device_Info_With_Time_Since_Update(name.c_str(), address.c_str(), rssi, timeSinceUpdate);
            }
            catch (const std::exception &e)
            {
                return BT_Device_Info_With_Time_Since_Update();
            }
        }


		friend std::istream& operator>>(std::istream& is, BT_Device_Info_With_Time_Since_Update& device) {
			std::string str;
			std::getline(is, str);
			device = BT_Device_Info_With_Time_Since_Update::fromString(str);
			return is;
		}

		friend std::ostream& operator<<(std::ostream& os, const BT_Device_Info_With_Time_Since_Update& device) {
			os << device.toString().c_str();
			return os;
		}
};
typedef BT_Device_Info_With_Time_Since_Update BT_Device_Info_With_Time_Since_Update_t;

struct  BluetoothDevice_t
{
	public:
		char name[BT_NAME_LENGTH+1] = "\0";
		char address[BT_ADDRESS_LENGTH+1] = "\0";
		
        BluetoothDevice_t(){}

		BluetoothDevice_t(const std::string &str)
		{
			*this = fromString(str);
		}

		BluetoothDevice_t(const char* name_In, const char* address_In)
		{
            strncpy(name, name_In, BT_NAME_LENGTH);
            name[BT_NAME_LENGTH] = '\0';
    
            strncpy(address, address_In, BT_ADDRESS_LENGTH);
            address[BT_ADDRESS_LENGTH] = '\0';
		}

		BluetoothDevice_t& operator=(const BluetoothDevice_t& other)
		{
			strncpy(this->name, other.name, sizeof(this->name));
			this->name[sizeof(this->name)] = '\0';

			strncpy(this->address, other.address, sizeof(this->address));
			this->address[sizeof(this->address)] = '\0';
			return *this;
		}

		bool operator==(const BluetoothDevice_t& other) const
		{
			return ( strcmp(this->name, other.name) == 0 &&
				     strcmp(this->address, other.address) == 0 );
		}

		bool operator!=(const BluetoothDevice_t& other) const
		{
			return !(*this == other);
		}

		operator std::string() const
        {
            return toString();
        }

        std::string toString() const
        {
            return std::string(name) + ENCODE_VALUE_DIVIDER + std::string(address);
        }

		static BluetoothDevice_t fromString(const std::string &str)
        {
            const std::string delimiter = ENCODE_VALUE_DIVIDER;
            size_t delimiterIndex = str.find(delimiter);

            if (delimiterIndex == std::string::npos)
            {
                return BluetoothDevice_t();
            }

            std::string name = str.substr(0, delimiterIndex);
            std::string address = str.substr(delimiterIndex + delimiter.length());
            if (name.empty() || address.empty())
            {
                return BluetoothDevice_t(); // Return a default object if fields are invalid
            }
            return BluetoothDevice_t(name.c_str(), address.c_str());
        }

		friend std::istream& operator>>(std::istream& is, BluetoothDevice_t& device) {
			std::string str;
			std::getline(is, str);
			device = BluetoothDevice_t::fromString(str);
			return is;
		}

		friend std::ostream& operator<<(std::ostream& os, const BluetoothDevice_t& device) {
			os << device.toString();
			return os;
		}
};

class SoundInputSource
{
    public:
        enum Value
        {
            OFF = 0,
            Microphone = 1,
            Bluetooth = 2,
            Count = 3
        };
 
        static std::string ToString(const Value &source)
        {
            switch (source)
            {
                case OFF: return "OFF";
                case Microphone: return "Microphone";
                case Bluetooth: return "Bluetooth";
                default: return "Unknown";
            }
        }

        static Value FromString(const std::string& str)
        {
            std::string input = str;
            std::transform(input.begin(), input.end(), input.begin(), ::tolower);
            if (input == "off") return OFF;
            if (input == "microphone") return Microphone;
            if (input == "bluetooth") return Bluetooth;
            return OFF;
        }

        friend std::ostream& operator<<(std::ostream& os, const SoundInputSource::Value& value) {
            std::string result = SoundInputSource::ToString(value);
            ESP_LOGD("SoundInputSource", "ostream input value: \"%i\" Converted to: \"%s\"", value, result);
            os << result;
            return os;
        }
        
        friend std::istream& operator>>(std::istream& is, SoundInputSource::Value& value) 
        {
            std::string token;
            is >> token;
            value = SoundInputSource::FromString(token.c_str());
            ESP_LOGD("SoundInputSource", "istream input value: \"%s\" Converted to: \"%i\"", token.c_str(), value);
            return is;
        }
};
typedef SoundInputSource::Value SoundInputSource_t;

class SoundOutputSource
{
    public:
        enum Value
        {
            OFF = 0,
            Bluetooth = 1,
            Count = 2
        };

        
        static std::string ToString(Value source)
        {
            switch (source)
            {
                case OFF:         return "OFF";
                case Bluetooth:   return "Bluetooth";
                default:          return "Unknown";
            }
        }

        static Value FromString(const std::string& str)
        {
            std::string input = str;
            std::transform(input.begin(), input.end(), input.begin(), ::tolower);
            if (input == "off" ) return OFF;
            if (input == "bluetooth" ) return Bluetooth;
            return OFF;
        }

        friend std::ostream& operator<<(std::ostream& os, const SoundOutputSource::Value& value) {
            std::string result = SoundOutputSource::ToString(value);
            ESP_LOGD("SoundOutputSource", "ostream input value: \"%i\" Converted to: \"%s\"", value, result);
            os << result;
            return os;
        }
        
        friend std::istream& operator>>(std::istream& is, SoundOutputSource::Value& value) 
        {
            std::string token;
            is >> token;
            value = SoundOutputSource::FromString(token.c_str());
            ESP_LOGD("SoundOutputSource", "istream input value: \"%s\" Converted to: \"%i\"", token.c_str(), value);
            return is;
        }
};
typedef SoundOutputSource::Value SoundOutputSource_t;

class Mute_State
{
    public:
        enum Value
        {
            Mute_State_Un_Muted = 0,
            Mute_State_Muted = 1
        };

        static std::string ToString(Value state)
        {
            switch (state)
            {
                case Mute_State_Un_Muted: return "UnMuted";
                case Mute_State_Muted: return "Muted";
                default: return "Unknown";
            }
        }

        static Value FromString(const std::string& str)
        {
            std::string input = str;
            std::transform(input.begin(), input.end(), input.begin(), ::tolower);
            if (input == "unmuted") return Mute_State_Un_Muted;
            if (input == "muted") return Mute_State_Muted;
            return Mute_State_Un_Muted;
        }

        friend std::istream& operator>>(std::istream& is, Mute_State::Value& value) 
        {
            std::string token;
            is >> token;
            value = Mute_State::FromString(token);
            return is;
        }
        
        friend std::ostream& operator<<(std::ostream& os, const Mute_State::Value& value) {
            os << Mute_State::ToString(value);
            return os;
        }
};
typedef Mute_State::Value Mute_State_t;

class Bluetooth_Discovery_Mode
{
    public:
        enum Value
        {
            Discovery_Mode_Started = 0,
            Discovery_Mode_Stopped = 1,
            Discovery_Mode_Unknown = 2
        };

        static std::string ToString(Value state)
        {
            switch (state)
            {
                case Discovery_Mode_Started: return "Started";
                case Discovery_Mode_Stopped: return "Stopped";
                case Discovery_Mode_Unknown: return "Unknown";
                default: return "Discovery_Mode_Unknown";
            }
        }

        static Value FromString(const std::string& str)
        {
            std::string input = str;
            std::transform(input.begin(), input.end(), input.begin(), ::tolower);
            if (input == "started") return Discovery_Mode_Started;
            if (input == "stopped") return Discovery_Mode_Stopped;
            return Discovery_Mode_Unknown;
        }

        friend std::istream& operator>>(std::istream& is, Bluetooth_Discovery_Mode::Value& value) 
        {
            std::string token;
            is >> token;
            value = Bluetooth_Discovery_Mode::FromString(token);
            return is;
        }
        
        friend std::ostream& operator<<(std::ostream& os, const Bluetooth_Discovery_Mode::Value& value) {
            os << Bluetooth_Discovery_Mode::ToString(value);
            return os;
        }
};
typedef Bluetooth_Discovery_Mode::Value Bluetooth_Discovery_Mode_t;

class SoundState
{
    public:
        enum Value
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
            Sound_Level11_Detected = 12
        };

        static std::string ToString(Value state)
        {
            switch (state)
            {
                case LastingSilenceDetected:    return "Lasting Silence Detected";
                case SilenceDetected:           return "Silence Detected";
                case Sound_Level1_Detected:     return "Sound Level 1 Detected";
                case Sound_Level2_Detected:     return "Sound Level 2 Detected";
                case Sound_Level3_Detected:     return "Sound Level 3 Detected";
                case Sound_Level4_Detected:     return "Sound Level 4 Detected";
                case Sound_Level5_Detected:     return "Sound Level 5 Detected";
                case Sound_Level6_Detected:     return "Sound Level 6 Detected";
                case Sound_Level7_Detected:     return "Sound Level 7 Detected";
                case Sound_Level8_Detected:     return "Sound Level 8 Detected";
                case Sound_Level9_Detected:     return "Sound Level 9 Detected";
                case Sound_Level10_Detected:    return "Sound Level 10 Detected";
                case Sound_Level11_Detected:    return "Sound Level 11 Detected";
                default:                        return "Unknown";
            }
        }

        static Value FromString(const std::string& str)
        {
            std::string input = str;
            std::transform(input.begin(), input.end(), input.begin(), ::tolower);
            if (input == "lasting silence detected")    return LastingSilenceDetected;
            if (input == "silence detected")            return SilenceDetected;
            if (input == "sound level 1 detected")      return Sound_Level1_Detected;
            if (input == "sound level 2 detected")      return Sound_Level2_Detected;
            if (input == "sound level 3 detected")      return Sound_Level3_Detected;
            if (input == "sound level 4 detected")      return Sound_Level4_Detected;
            if (input == "sound level 5 detected")      return Sound_Level5_Detected;
            if (input == "sound level 6 detected")      return Sound_Level6_Detected;
            if (input == "sound level 7 detected")      return Sound_Level7_Detected;
            if (input == "sound level 8 detected")      return Sound_Level8_Detected;
            if (input == "sound level 9 detected")      return Sound_Level9_Detected;
            if (input == "sound level 10 detected")     return Sound_Level10_Detected;
            if (input == "sound level 11 detected")     return Sound_Level11_Detected;
            return  LastingSilenceDetected;
        }

        friend std::istream& operator>>(std::istream& is, SoundState::Value& value) 
        {
            std::string token;
            is >> token;
            value = SoundState::FromString(token);
            return is;
        }
        
        friend std::ostream& operator<<(std::ostream& os, const SoundState::Value& value) {
            os << SoundState::ToString(value);
            return os;
        }
};
typedef SoundState::Value SoundState_t;

class Transciever
{
    public:
        enum Value
        {
            Transciever_None = 0,
            Transciever_TX = 1,
            Transciever_RX = 2,
            Transciever_TXRX = 3
        };

        static std::string ToString(Value transciever)
        {
            switch (transciever)
            {
                case Transciever_None: return "None";
                case Transciever_TX: return "TX";
                case Transciever_RX: return "RX";
                case Transciever_TXRX: return "TXRX";
                default: return "Unknown";
            }
        }

        static Value FromString(const std::string& str)
        {
            std::string input = str;
            std::transform(input.begin(), input.end(), input.begin(), ::tolower);
            if (input == "none") return Transciever_None;
            if (input == "tx") return Transciever_TX;
            if (input == "rx") return Transciever_RX;
            if (input == "txrx") return Transciever_TXRX;
            return Transciever_None;
        }

        friend std::istream& operator>>(std::istream& is, Transciever::Value& value) 
        {
            std::string token;
            is >> token;
            value = Transciever::FromString(token);
            return is;
        }
        
        friend std::ostream& operator<<(std::ostream& os, const Transciever::Value& value) {
            os << Transciever::ToString(value);
            return os;
        }

};
typedef Transciever::Value Transciever_t;

struct __attribute__((packed)) Frame_t {
  int16_t channel1;
  int16_t channel2;
  bool operator==(const Frame_t& other) const {
        return channel1 == other.channel1 && channel2 == other.channel2;
    }
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
  DataType_BluetoothDevice_t,
  DataType_ActiveBluetoothDevice_t,
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
  DataType_Bluetooth_Discovery_Mode_t,
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
  "BT_Device_Info_With_Time_Since_Update_t",
  "BluetoothDevice_t",
  "ActiveBluetoothDevice_t",
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
  "Bluetooth_Discovery_Mode_t",
  "Undefined_t"
};

struct DataItemConfig_t
{
  std::string Name;
  DataType_t DataType;
  size_t Count;
  Transciever_t TransceiverConfig;
  size_t QueueCount;
};

class Wifi_Mode
{
public:
    enum Value
    {
        Station = 0,
        AccessPoint = 1,
        Unknown = 2
    };

    static std::string ToString(Value status)
    {
        switch (status)
        {
            case Station: return "Station";
            case AccessPoint: return "Access Point";
            default: return "Unknown";
        }
    }

    static Value FromString(const std::string& str)
    {
        std::string input = str;
        std::transform(input.begin(), input.end(), input.begin(), ::tolower);
        if (input == "station") return Station;
        if (input == "access point") return AccessPoint;
        return Unknown; // Default or error value
    }

	friend std::istream& operator>>(std::istream& is, Wifi_Mode::Value& value) 
	{
		std::string token;
		is >> token;
		value = Wifi_Mode::FromString(token);
		return is;
	}
    
    friend std::ostream& operator<<(std::ostream& os, const Wifi_Mode::Value& value) {
        ESP_LOGD("operator<<", "Wifi Mode to String: \"%s\"", Wifi_Mode::ToString(value).c_str());
        os << Wifi_Mode::ToString(value);
        return os;
    }
    
};
typedef Wifi_Mode::Value Wifi_Mode_t;

class ConnectionStatus
{
public:
    enum Value
    {
        Disconnected = 0,
        Connecting = 1,
        Connected = 2,
        Disconnecting = 3,
        Unknown = 4
    };

    
    static std::string ToString(Value status)
    {
        switch (status)
        {
            case Disconnected: return "Disconnected";
            case Connecting: return "Connecting";
            case Connected: return "Connected";
            case Disconnecting: return "Disconnecting";
            case Unknown: return "Unknown";
            default: return "Unknown";
        }
    }

    
    static Value FromString(const std::string& str)
    {
        std::string input = str;
        std::transform(input.begin(), input.end(), input.begin(), ::tolower);
        if (input == "disconnected") return Disconnected;
        if (input == "vonnecting") return Connecting;
        if (input == "vonnected") return Connected;
        if (input == "disconnecting") return Disconnecting;
        return Unknown; // Default or error value
    }

    
	friend std::istream& operator>>(std::istream& is, ConnectionStatus::Value& value) 
	{
		std::string token;
		is >> token;
		value = ConnectionStatus::FromString(token);
		return is;
	}
    
    
    friend std::ostream& operator<<(std::ostream& os, const ConnectionStatus::Value& value) {
        ESP_LOGD("operator<<", "Connection Status to String: \"%s\"", ConnectionStatus::ToString(value));
        os << ConnectionStatus::ToString(value);
        return os;
    }
    
};
typedef ConnectionStatus::Value ConnectionStatus_t;

struct NamedObject_t
{
	void* Object;
	std::string Name = "";
    size_t ChangeCount = 0;
	
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
    public:
        std::string Name;
        void (*Callback)(const std::string& name, void* callback, void* arg);
        void* Arg;
        NamedCallback_t(){}
        NamedCallback_t(std::string name, void (*callback)(const std::string& name, void* callback, void* arg), void* arg = nullptr)
            : Name(name), Callback(callback), Arg(arg)
        {
        }

        NamedCallback_t(const NamedCallback_t& other)
            : Name(other.Name), Callback(other.Callback), Arg(other.Arg)
        {
        }

        bool operator==(const NamedCallback_t& other) const
        {
            return this->Name == other.Name && this->Callback == other.Callback && this->Arg == other.Arg;
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
	std::string Name;
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

    ProcessedSoundData_t()
    {
        NormalizedPower = 0.0;
        Minimum = 0;
        Maximum = 0;
    }

    ProcessedSoundData_t(float NormalizedPower_In, int32_t Minimum_In, int32_t Maximum_In)
    {
        NormalizedPower = NormalizedPower_In;
        Minimum = Minimum_In;
        Maximum = Maximum_In;
    }
    bool operator==(const ProcessedSoundData_t& other) const
    {
        return this->NormalizedPower == other.NormalizedPower && this->Minimum == other.Minimum && this->Maximum == other.Maximum;
    }

    bool operator!=(const ProcessedSoundData_t& other) const
    {
        return !(*this == other);
    }

    operator std::string() const
    {
        return toString();
    }

    std::string toString() const
    {
        return std::to_string(NormalizedPower) + ENCODE_VALUE_DIVIDER + std::to_string(Minimum) + ENCODE_VALUE_DIVIDER + std::to_string(Maximum);
    }
    
    static ProcessedSoundData_t fromString(const std::string &str)
    {
        const std::string delimiter = ENCODE_VALUE_DIVIDER;
        std::vector<std::string> fields;
        size_t start = 0, delimiterIndex;

        // Split the string into fields using the delimiter
        while ((delimiterIndex = str.find(delimiter, start)) != std::string::npos)
        {
            fields.push_back(str.substr(start, delimiterIndex - start));
            start = delimiterIndex + delimiter.length();
        }
        fields.push_back(str.substr(start)); // Add the last field

        // Ensure we have exactly three fields
        if (fields.size() != 3)
        {
            return ProcessedSoundData_t(); // Return default object on failure
        }

        try
        {
            float NormalizedPower = std::stof(fields[0]);
            int32_t Minimum = std::stoi(fields[1]);
            int32_t Maximum = std::stoi(fields[2]);

            return ProcessedSoundData_t(NormalizedPower, Minimum, Maximum);
        }
        catch (const std::exception &e)
        {
            return ProcessedSoundData_t();
        }
    }

    friend std::istream& operator>>(std::istream& is, ProcessedSoundData_t& psd) {
        std::string str;
        std::getline(is, str);
        psd = ProcessedSoundData_t::fromString(str);
        return is;
    }

    friend std::ostream& operator<<(std::ostream& os, const ProcessedSoundData_t& psd) {
        os << psd.toString().c_str();
        return os;
    }
};

struct ProcessedSoundFrame_t
{
	ProcessedSoundData_t Channel1;
	ProcessedSoundData_t Channel2;
    ProcessedSoundFrame_t()
    {
        Channel1 = ProcessedSoundData_t();
        Channel2 = ProcessedSoundData_t();
    }

    ProcessedSoundFrame_t(ProcessedSoundData_t Channel1_In, ProcessedSoundData_t Channel2_In)
    {
        Channel1_In = Channel1_In;
        Channel2_In = Channel2_In;
    }

    bool operator==(const ProcessedSoundFrame_t& other) const
    {
        return this->Channel1 == other.Channel1 && this->Channel2 == other.Channel2;
    }

    bool operator!=(const ProcessedSoundFrame_t& other) const
    {
        return !(*this == other);
    }

    operator std::string() const
    {
        return toString();
    }

    std::string toString() const
    {
        return std::string(Channel1) + ENCODE_VALUE_DIVIDER + std::string(Channel2);
    }
    
    static ProcessedSoundFrame_t fromString(const std::string &str)
    {
        const std::string delimiter = ENCODE_VALUE_DIVIDER;
        std::vector<std::string> fields;
        size_t start = 0, delimiterIndex;

        // Split the string into fields using the delimiter
        while ((delimiterIndex = str.find(delimiter, start)) != std::string::npos)
        {
            fields.push_back(str.substr(start, delimiterIndex - start));
            start = delimiterIndex + delimiter.length();
        }
        fields.push_back(str.substr(start)); // Add the last field

        // Ensure we have exactly two fields
        if (fields.size() != 2)
        {
            return ProcessedSoundFrame_t(); // Return default object on failure
        }

        try
        {
            // Parse the fields into respective types
            ProcessedSoundData_t channel1 = ProcessedSoundData_t::fromString(fields[0]);
            ProcessedSoundData_t channel2 = ProcessedSoundData_t::fromString(fields[1]);

            return ProcessedSoundFrame_t(channel1, channel2);
        }
        catch (const std::exception &e)
        {
            // Handle parsing errors gracefully
            return ProcessedSoundFrame_t();
        }
    }

    friend std::istream& operator>>(std::istream& is, ProcessedSoundFrame_t& psf) {
        std::string str;
        std::getline(is, str);
        psf = ProcessedSoundFrame_t::fromString(str);
        return is;
    }

    friend std::ostream& operator<<(std::ostream& os, const ProcessedSoundFrame_t& psf) {
        os << psf.toString();
        return os;
    }
};

struct MaxBandSoundData_t
{
	float MaxBandNormalizedPower;
	int16_t MaxBandIndex;
	int16_t TotalBands;
    MaxBandSoundData_t()
    {
	    float MaxBandNormalizedPower = 0.0;
	    int16_t MaxBandIndex = 0;
	    int16_t TotalBands = 0;
    }
    MaxBandSoundData_t(float MaxBandNormalizedPower_In, int16_t MaxBandIndex_In, int16_t TotalBands_In)
    {
        MaxBandNormalizedPower = MaxBandNormalizedPower_In;
        MaxBandIndex = MaxBandIndex_In;
        TotalBands = TotalBands_In;
    }
    bool operator==(const MaxBandSoundData_t& other) const
    {
        return this->MaxBandNormalizedPower == other.MaxBandNormalizedPower && this->MaxBandIndex == other.MaxBandIndex && this->TotalBands == other.TotalBands;
    }

    bool operator!=(const MaxBandSoundData_t& other) const
    {
        return !(*this == other);
    }

    operator std::string() const
    {
        return toString();
    }

    std::string toString() const
    {
        return std::to_string(MaxBandNormalizedPower) + ENCODE_VALUE_DIVIDER + std::to_string(MaxBandIndex) + ENCODE_VALUE_DIVIDER + std::to_string(TotalBands);
    }

    static MaxBandSoundData_t fromString(const std::string &str)
    {
        const std::string delimiter = ENCODE_VALUE_DIVIDER;
        std::vector<std::string> fields;
        size_t start = 0, delimiterIndex;

        // Split the string into fields using the delimiter
        while ((delimiterIndex = str.find(delimiter, start)) != std::string::npos)
        {
            fields.push_back(str.substr(start, delimiterIndex - start));
            start = delimiterIndex + delimiter.length();
        }
        fields.push_back(str.substr(start)); // Add the last field

        // Ensure we have exactly three fields
        if (fields.size() != 3)
        {
            return MaxBandSoundData_t(); // Return default object on failure
        }

        try
        {
            // Parse the fields into respective types
            float maxBandNormalizedPower = std::stof(fields[0]);
            int16_t maxBandIndex = static_cast<int16_t>(std::stoi(fields[1]));
            int16_t totalBands = static_cast<int16_t>(std::stoi(fields[2]));

            return MaxBandSoundData_t(maxBandNormalizedPower, maxBandIndex, totalBands);
        }
        catch (const std::exception &e)
        {
            // Handle parsing errors gracefully
            return MaxBandSoundData_t();
        }
    }

    friend std::istream& operator>>(std::istream& is, MaxBandSoundData_t& device) {
        std::string str;
        std::getline(is, str);
        device = MaxBandSoundData_t::fromString(str);
        return is;
    }

    friend std::ostream& operator<<(std::ostream& os, const MaxBandSoundData_t& device) {
        os << device.toString().c_str();
        return os;
    }
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
			ESP_LOGE("DataTypes: GetDataTypeFromString: %s", "ERROR! \"%s\": Undefined Data Type.", DataType.c_str());
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
			else if(std::is_same<T, std::string>::value) 								return DataType_String_t;
			else if(std::is_same<T, BT_Device_Info_t>::value) 							return DataType_BT_Device_Info_t;
			else if(std::is_same<T, BT_Device_Info_With_Time_Since_Update_t>::value) 	return DataType_BT_Device_Info_With_Time_Since_Update_t;
			else if(std::is_same<T, BluetoothDevice_t>::value)							return DataType_BluetoothDevice_t;
			else if(std::is_same<T, ActiveBluetoothDevice_t>::value)					return DataType_ActiveBluetoothDevice_t;
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
			else if(std::is_same<T, Bluetooth_Discovery_Mode_t>::value)					return DataType_Bluetooth_Discovery_Mode_t;
			else
			{
				ESP_LOGE("DataTypes: GetDataTypeFromTemplateType", "ERROR! Undefined Data Type.");
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
				
				case DataType_BluetoothDevice_t:
					result = sizeof(BluetoothDevice_t);
				break;
				
				case DataType_ActiveBluetoothDevice_t:
					result = sizeof(ActiveBluetoothDevice_t);
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

                case DataType_Bluetooth_Discovery_Mode_t:
                    result = sizeof(Bluetooth_Discovery_Mode_t);
				break;
				
				default:
					ESP_LOGE("DataTypes: GetSizeOfDataType: %s", "ERROR! \"%s\": Undefined Data Type.", DataTypeStrings[DataType]);
					result = 0;
				break;
			}
			return result;
		}
};
