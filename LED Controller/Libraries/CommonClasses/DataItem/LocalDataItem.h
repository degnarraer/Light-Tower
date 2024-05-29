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

#include <Helpers.h>
#include <Preferences.h>
#include <nvs_flash.h>
#include <esp_timer.h>
#include <Arduino.h>
#include <esp_heap_caps.h>
#include "SerialMessageManager.h"
#include "ValidValueChecker.h"

#define ENCODE_DIVIDER "|"

template <typename T, size_t COUNT>
class LocalDataItem: public NamedCallbackInterface<T>
				   , public SetupCalleeInterface
				   , public DataTypeFunctions
				   , public ValidValueChecker
{
	public:
		LocalDataItem( const String name
					 , const T* initialValue
					 , NamedCallback_t *namedCallback )
					 : ValidValueChecker()
					 , m_Name(name)
					 , mp_InitialValuePtr(initialValue)
		{}
		
		LocalDataItem( const String name
					 , const T& initialValue
					 , NamedCallback_t *namedCallback )
					 : ValidValueChecker()
					 , m_Name(name)
					 , mp_InitialValuePtr(&initialValue)
		{}

		LocalDataItem( const String name
					 , const T* initialValue
					 , NamedCallback_t *namedCallback
					 , const ValidStringValues_t *validStringValues )
					 : ValidValueChecker(validStringValues)
					 , m_Name(name)
					 , mp_InitialValuePtr(initialValue)
		{}
		
		LocalDataItem( const String name
					 , const T& initialValue
					 , NamedCallback_t *namedCallback
					 , const ValidStringValues_t *validStringValues )
					 : ValidValueChecker(validStringValues)
					 , m_Name(name)
					 , mp_InitialValuePtr(&initialValue)
		{}
		
		virtual ~LocalDataItem()
		{
			ESP_LOGI("DataItem<T, COUNT>::Setup()", "\"%s\": Freeing Memory", m_Name.c_str());
			if(mp_NamedCallback) this->DeRegisterNamedCallback(mp_NamedCallback);
			heap_caps_free(mp_Value);
			heap_caps_free(mp_InitialValue);
		}
		virtual void Setup()
		{
			ESP_LOGI("DataItem<T, COUNT>::Setup()", "\"%s\": Allocating Memory", m_Name.c_str());
			if(mp_NamedCallback) this->RegisterNamedCallback(mp_NamedCallback);
			mp_Value = (T*)heap_caps_malloc(sizeof(T)*COUNT, MALLOC_CAP_SPIRAM);
			mp_InitialValue = (T*)heap_caps_malloc(sizeof(T)*COUNT, MALLOC_CAP_SPIRAM);
			if (mp_Value && mp_InitialValue && mp_InitialValuePtr)
			{
				if (std::is_same<T, char>::value)
				{
					String InitialValue = String((char*)mp_InitialValuePtr);
					for (size_t i = 0; i < COUNT; ++i)
					{
						char value;
						memcpy(&value, mp_InitialValuePtr+i, sizeof(char));
						if (i >= InitialValue.length())
						{
							value = '\0';
						}
						memcpy(mp_Value+i, &value, sizeof(char));
						memcpy(mp_InitialValue+i, &value, sizeof(char));
					}
					this->CallCallbacks(m_Name.c_str(), mp_Value);
					ESP_LOGE( "DataItem<T, COUNT>::Setup()", "\"%s\": Set initial value <char>: \"%s\""
							, m_Name.c_str()
							, GetInitialValueAsString().c_str());
				}
				else
				{
					for (size_t i = 0; i < COUNT; ++i)
					{
						memcpy(mp_Value+i, mp_InitialValuePtr, sizeof(T));
						memcpy(mp_InitialValue+i, mp_InitialValuePtr, sizeof(T));
					}
					this->CallCallbacks(m_Name.c_str(), mp_Value);
					ESP_LOGE( "DataItem<T, COUNT>::Setup()", "\"%s\": Set initial value <T>: \"%s\""
							, m_Name.c_str()
							, GetInitialValueAsString().c_str());
				}
			}
			else
			{
				ESP_LOGE("DataItem<T, COUNT>::Setup()", "Failed to allocate memory on SPI RAM");
			}
		}
		String GetName()
		{
			return m_Name;
		}
		size_t GetCount()
		{
			return m_Count;
		}
		size_t GetChangeCount()
		{
			return m_ValueChangeCount;
		}
		void GetValue(void* Object, size_t Count)
		{
			assert(Count == COUNT && "Counts must be equal");
			if(mp_Value)
			{
				memcpy(Object, mp_Value, sizeof(T)*Count);
			}
			else
			{
				ESP_LOGE("GetValueAsString", "NULL Pointer!");
				*reinterpret_cast<T**>(Object) = nullptr;
			}
		}

		T* GetValuePointer()
		{
			if(!mp_Value)
			{
				ESP_LOGE("GetValueAsString", "\"%s\": NULL Pointer!", m_Name.c_str());
			}
			return mp_Value;
		}

		T GetValue()
		{
			assert(1 == COUNT && "Count must 1 to use this function");
			if(mp_Value)
			{
				return static_cast<T>(*mp_Value);
			}
			else
			{
				ESP_LOGE("GetValueAsString", "\"%s\": NULL Pointer!", m_Name.c_str());
				return T();
			}
		}

		virtual bool GetStringInitialValue(String &stringValue)
		{
			stringValue = String(*mp_InitialValue);
			return true;
		}
		
		String GetInitialValueAsString()
		{
			String value;
			if(!GetStringInitialValue(value))
			{
				value = "";
			}
			return value;
		}

		virtual bool GetStringValue(String &stringValue)
		{
			if (COUNT == 0) return false;

			std::vector<String> valueStrings;
			for (size_t i = 0; i < COUNT; ++i)
			{
				valueStrings.push_back(String(mp_Value[i]).c_str());
			}

			stringValue = "";
			
			for (size_t i = 0; i < valueStrings.size() - 1; ++i)
			{
				stringValue += valueStrings[i];
				stringValue += ENCODE_DIVIDER;
			}
			stringValue += valueStrings[COUNT - 1];
			ESP_LOGD("GetStringValue"
					, "\"%s\": GetStringValue: %s"
					, m_Name.c_str()
					, stringValue.c_str());
			return true;
		}

		virtual String& GetValueString()
		{
			if(!GetStringValue(m_StringValue))
			{
				m_StringValue = "";
			}
			return m_StringValue;
		}

		virtual String GetValueAsString()
		{
			String value;
			if(!GetStringValue(value))
			{
				value = "";
			}
			return value;
		}

		static bool StaticSetValueFromString(const String& stringValue, void* objectptr)
		{
			if(objectptr)
			{
				LocalDataItem<T, COUNT>* object = static_cast<LocalDataItem<T, COUNT>*>(objectptr);
				if(object)
				{
					return object->SetValueFromString(stringValue);
				}
				else
				{
					return false;
				}
			}
			else
			{
            	ESP_LOGE("StaticSetValueFromString", "Null Object Pointer!");
				return false;
			}
		}

		virtual bool SetValueFromString(const String& stringValue)
		{
			T value[COUNT];
			std::vector<String> substrings;
			size_t start = 0;
			size_t end = stringValue.indexOf(ENCODE_DIVIDER);

			// Split the input string by ENCODE_DIVIDER
			while (end != -1) 
			{
				Serial.println(stringValue.substring(start, end).c_str());
				substrings.push_back(stringValue.substring(start, end));
				start = end + 1;
				end = stringValue.indexOf(ENCODE_DIVIDER, start);
			}
			substrings.push_back(stringValue.substring(start));
			Serial.println(stringValue.substring(start).c_str());

			// Check if the number of substrings matches the expected COUNT
			if (substrings.size() != COUNT) 
			{
				ESP_LOGE("SetValueFromString",
						"Expected %zu substrings but got %zu in string: \"%s\"",
						COUNT, substrings.size(), stringValue.c_str());
				return false;
			}

			// Decode each substring and store it in the value array
			for (size_t i = 0; i < COUNT; ++i) 
			{
				ESP_LOGE("SetValueFromString",
						"\"%s\": Set Value From String: \"%s\"",
						m_Name.c_str(), substrings[i].c_str());
				if(IsValidValue(substrings[i]))
				{
					value[i] = decodeFromString(substrings[i]);
				}
				else
				{
					return false;
				}
			}

			// Set the decoded values
			return SetValue(value, COUNT);
		}

		T decodeFromString(String str) {
			std::string stdStr = str.c_str();
			std::istringstream iss(stdStr);
			T value;
			iss >> value;
			return value;
		}

		String encodeToString(T value)
		{
			std::ostringstream oss;
			oss << value;
			if (oss.fail())
			{
				assert(false && "Failed to encode value to string");
			}
			return String(oss.str().c_str());
		}

		virtual bool SetValue(const T *value, size_t count)
		{
			assert(value != nullptr && "Value must not be null");
			assert(mp_Value != nullptr && "mp_Value must not be null");
			assert(COUNT > 0 && "COUNT must be a valid index range for mp_Value");
			assert(COUNT == count && "Counts must match");
			ESP_LOGD( "LocalDataItem: SetValue"
					, "\"%s\" Set Value: \"%s\""
					, m_Name.c_str()
					, GetValueAsStringForDataType(Value, GetDataTypeFromTemplateType<T>(), COUNT, "").c_str());
			bool valueChanged = (memcmp(mp_Value, &value, sizeof(T) * COUNT) != 0);
			bool validValue = true;
			if(valueChanged)
			{
				for(int i = 0; i< COUNT; ++i)
				{
					String stringValue = encodeToString(mp_Value[i]);
					if(!this->IsValidValue( stringValue ) )
					{
						ESP_LOGE("SetValue", "\"%s\" Value Rejected: \"%s\"", m_Name.c_str(), stringValue.c_str() );
						validValue = false;
					}
				}
				if(validValue)
				{
					memcpy(mp_Value, &value, sizeof(T) * COUNT);
					++m_ValueChangeCount;
					this->CallCallbacks(m_Name.c_str(), mp_Value);
				}
			}
			return valueChanged;
		}

		virtual bool SetValue(T Value)
		{
			assert(COUNT == 1 && "COUNT must be 1 to use this");
			assert(mp_Value != nullptr && "mp_Value must not be null");
			ESP_LOGD( "LocalDataItem: SetValue"
					, "\"%s\" Set Value: \"%s\""
					, this->m_Name.c_str()
					, this->GetStringValue().c_str());
			
			bool ValueChanged = (memcmp(mp_Value, &Value, sizeof(T) * COUNT) != 0);
			if(ValueChanged)
			{
				memcpy(mp_Value, &Value, sizeof(T) * COUNT);
				++this->m_ValueChangeCount;
				this->CallCallbacks(this->m_Name.c_str(), mp_Value);
			}
			return ValueChanged;
		}
		bool EqualsValue(T *Object, size_t Count)
		{
			assert(Count == COUNT && "Counts must equal");
			return (memcmp(mp_Value, Object, Count) == 0);
		}
	protected:
		const String m_Name;
		const T* const mp_InitialValuePtr;
		T *mp_Value;
		String m_StringValue;
		T *mp_InitialValue;
		NamedCallback_t *mp_NamedCallback = NULL;
		size_t m_ValueChangeCount = 0;
	private:
		size_t m_Count = COUNT;
};