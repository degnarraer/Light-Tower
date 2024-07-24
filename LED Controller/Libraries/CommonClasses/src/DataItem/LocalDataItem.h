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
#include "SetupCallInterfaces.h"
#include "ValidValueChecker.h"
#include "StringEncoderDecoder.h"

#define ENCODE_DIVIDER "|"

template <typename T, size_t COUNT>
class 
LocalDataItem: public NamedCallbackInterface<T>
			 , public SetupCalleeInterface
			 , public DataTypeFunctions
			 , public ValidValueChecker
			 , public StringEncoderDecoder<T>
{
	public:
		LocalDataItem( const String name
					 , const T* initialValue )
					 : m_Name(name)
					 , mp_InitialValuePtr(initialValue){}
		LocalDataItem( const String name
					 , const T& initialValue )
					 : m_Name(name)
					 , mp_InitialValuePtr(&initialValue){}
		LocalDataItem( const String name
					 , const T* initialValue
					 , NamedCallback_t *namedCallback
					 , SetupCallerInterface *setupCallerInterface )
					 : m_ValidValueChecker(ValidValueChecker())
					 , m_Name(name)
					 , mp_InitialValuePtr(initialValue)
					 , mp_NamedCallback(namedCallback)
					 , mp_SetupCallerInterface(setupCallerInterface)
		{
			ESP_LOGI("LocalDataItem", "LocalDataItem Instantiated: Constructor 1");
			RegisterForSetup();
		}
		
		LocalDataItem( const String name
					 , const T& initialValue
					 , NamedCallback_t *namedCallback
					 , SetupCallerInterface *setupCallerInterface )
					 : m_ValidValueChecker(ValidValueChecker())
					 , m_Name(name)
					 , mp_InitialValuePtr(&initialValue)
					 , mp_NamedCallback(namedCallback)
					 , mp_SetupCallerInterface(setupCallerInterface)
		{
			ESP_LOGI("LocalDataItem", "LocalDataItem Instantiated: Constructor 2");
			RegisterForSetup();
		}

		LocalDataItem( const String name
					 , const T* initialValue
					 , NamedCallback_t *namedCallback
					 , SetupCallerInterface *setupCallerInterface
					 , const ValidStringValues_t* const validStringValues )
					 : m_ValidValueChecker(ValidValueChecker(validStringValues))
					 , m_Name(name)
					 , mp_InitialValuePtr(initialValue)
					 , mp_NamedCallback(namedCallback)
					 , mp_SetupCallerInterface(setupCallerInterface)
		{
			ESP_LOGI("LocalDataItem", "LocalDataItem Instantiated: Constructor 3");
			RegisterForSetup();
		}
		
		LocalDataItem( const String name
					 , const T& initialValue
					 , NamedCallback_t *namedCallback
					 , SetupCallerInterface *setupCallerInterface
					 , const ValidStringValues_t* const validStringValues )
					 : m_ValidValueChecker(ValidValueChecker(validStringValues))
					 , m_Name(name)
					 , mp_InitialValuePtr(&initialValue)
					 , mp_NamedCallback(namedCallback)
					 , mp_SetupCallerInterface(setupCallerInterface)
		{
			ESP_LOGI("LocalDataItem", "LocalDataItem Instantiated: Constructor 4");
			RegisterForSetup();
		}
		
		virtual ~LocalDataItem()
		{
			ESP_LOGI("DataItem<T, COUNT>::Setup()", "\"%s\": LocalDataItem Freeing Memory", m_Name.c_str());
			if(mp_SetupCallerInterface)
			{
        		ESP_LOGD("~LocalDataItem", "DeRegistering for Setup Call");
				mp_SetupCallerInterface->DeRegisterForSetupCall(this);
			}
			if(mp_NamedCallback)
			{
        		ESP_LOGD("~LocalDataItem", "DeRegistering Named Callback");
				this->DeRegisterNamedCallback(mp_NamedCallback);
			}
			if(mp_Value)
			{
        		ESP_LOGD("~LocalDataItem", "freeing mp_Value Memory");
				heap_caps_free(mp_Value);
			}
			if(mp_InitialValue) 
			{
        		ESP_LOGD("~LocalDataItem", "freeing mp_InitialValue Memory");
				heap_caps_free(mp_InitialValue);
			}
		}

		virtual void RegisterForSetup()
		{
			if(mp_SetupCallerInterface)
			{
        		ESP_LOGD("RegisterForSetup", "Registering for Setup Call");
				mp_SetupCallerInterface->RegisterForSetupCall(this);
			}
			else
			{
				ESP_LOGE("LocalDataItem()", "ERROR! Unable to register for Setup, NULL pointer.");
			}
		}

		virtual void Setup()
		{
			ESP_LOGD("DataItem<T, COUNT>::Setup()", "\"%s\": Allocating Memory", m_Name.c_str());
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
					ESP_LOGI( "DataItem<T, COUNT>::Setup()", "\"%s\": Set initial value <char>: \"%s\""
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
					ESP_LOGI( "DataItem<T, COUNT>::Setup()", "\"%s\": Set initial value <T>: \"%s\""
							, m_Name.c_str()
							, GetInitialValueAsString().c_str());
					this->CallCallbacks(m_Name.c_str(), mp_Value);
				}
			}
			else
			{
				ESP_LOGE("DataItem<T, COUNT>::Setup()", "Failed to allocate memory on spi ram.");
			}
		}
		String GetName() const
		{
			return m_Name;
		}
		size_t GetCount() const
		{
			return m_Count;
		}
		size_t GetChangeCount() const
		{
			return m_ValueChangeCount;
		}
		void GetValue(void* Object, size_t Count) const
		{
			assert((Count == COUNT) && "Counts must be equal");
			if(mp_Value)
			{
				memcpy(Object, mp_Value, sizeof(T)*Count);
			}
			else
			{
				ESP_LOGE("GetValueAsString", "ERROR! NULL Pointer.");
				*reinterpret_cast<T**>(Object) = nullptr;
			}
		}

		T* GetValuePointer() const
		{
			if(!mp_Value)
			{
				ESP_LOGE("GetValueAsString", "ERROR! \"%s\": NULL Pointer.", m_Name.c_str());
			}
			return mp_Value;
		}

		T GetValue() const
		{
			assert((1 == COUNT) && "Count must 1 to use this function");
			if(mp_Value)
			{
				return static_cast<T>(*mp_Value);
			}
			else
			{
				ESP_LOGE("GetValueAsString", "ERROR! \"%s\": NULL Pointer.", m_Name.c_str());
				return T();
			}
		}

		virtual bool GetInitialValueAsString(String &stringValue) const
		{
			stringValue = "";
			if (mp_InitialValue && COUNT > 0)
			{
				std::vector<String> valueStrings;
				for (size_t i = 0; i < COUNT; ++i)
				{
					valueStrings.push_back(StringEncoderDecoder<T>::EncodeToString(*mp_InitialValue));
				}
				
				for (size_t i = 0; i < COUNT - 1; ++i)
				{
					stringValue += valueStrings[i];
					stringValue += ENCODE_DIVIDER;
				}
				stringValue += valueStrings[COUNT - 1];
				return true;
			}
			else
			{
				ESP_LOGE("GetInitialValueAsString", "ERROR! \"%s\": NULL Pointer.", m_Name.c_str());
				return false;
			}
		}

		virtual String GetInitialValueAsString() const
		{
			String value;
			if(!GetInitialValueAsString(value))
			{
				ESP_LOGE("GetInitialValueAsString", "ERROR! \"%s\": Unable to Get String Value! Returning Empty String.", m_Name.c_str());
				value = "";
			}
			return value;
		}

		virtual bool GetValueAsString(String &stringValue) const
		{
			stringValue = "";
			if (mp_Value && COUNT > 0)
			{
				std::vector<String> valueStrings;
				for (size_t i = 0; i < COUNT; ++i)
				{
					valueStrings.push_back(StringEncoderDecoder<T>::EncodeToString(mp_Value[i]));
				}
				
				for (size_t i = 0; i < COUNT - 1; ++i)
				{
					stringValue += valueStrings[i];
					stringValue += ENCODE_DIVIDER;
				}
				stringValue += valueStrings[COUNT - 1];
				ESP_LOGD("GetValueAsString", "\"%s\": String Value: \"%s\"", m_Name.c_str(), stringValue.c_str());
				return true;
			}
			else
			{
				ESP_LOGE("GetValueAsString", "ERROR! \"%s\": NULL Pointer.", m_Name.c_str());
				return false;
			}
		}

		virtual String GetValueAsString() const
		{
			String value;
			if(!GetValueAsString(value))
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
            		ESP_LOGE("StaticSetValueFromString", "ERROR! Null Object.");
					return false;
				}
			}
			else
			{
            	ESP_LOGE("StaticSetValueFromString", "ERROR! Null Object Pointer.");
				return false;
			}
		}

		virtual bool SetValueFromString(const String& stringValue)
		{
			ESP_LOGD("LocalDataItem::SetValueFromString"
					, "Name: \"%s\" String Value: \"%s\""
					, m_Name.c_str()
					, stringValue.c_str());
			T value[COUNT];
			std::vector<String> substrings;
			size_t start = 0;
			size_t end = stringValue.indexOf(ENCODE_DIVIDER);

			// Split the input string by ENCODE_DIVIDER
			while (end != -1) 
			{
				String parsedString = stringValue.substring(start, end);
				ESP_LOGD("SetValueFromString", "Parsed String: \"%s\"", parsedString.c_str());
				substrings.push_back(parsedString);
				start = end + 1;
				end = stringValue.indexOf(ENCODE_DIVIDER, start);
			}
			String parsedString = stringValue.substring(start);
			ESP_LOGD("SetValueFromString", "Parsed String: \"%s\"", parsedString.c_str());
			substrings.push_back(parsedString);

			// Check if the number of substrings matches the expected COUNT
			if (substrings.size() != COUNT) 
			{
				ESP_LOGE( "SetValueFromString",
						  "Expected %zu substrings but got %zu in string: \"%s\".",
						  COUNT, substrings.size(), stringValue.c_str());
				return false;
			}

			// Decode each substring and store it in the value array
			ESP_LOGD("SetValue", "\"%s\" Parsed %i Strings.", m_Name.c_str(), substrings.size() );
			for (size_t i = 0; i < substrings.size(); ++i) 
			{
				if(false == m_ValidValueChecker.IsValidStringValue(substrings[i]))
				{
					ESP_LOGE("SetValue", "\"%s\" Value Rejected: \"%s\".", m_Name.c_str(), substrings[i].c_str() );
					return false;
				}
				value[i] = StringEncoderDecoder<T>::DecodeFromString(substrings[i]);
			}

			// Set the decoded values
			return SetValue(value, COUNT);
		}

		virtual bool SetValue(const T *value, size_t count)
		{
			assert(value != nullptr);
			assert(mp_Value != nullptr);
			assert(COUNT > 0);
			assert(COUNT == count);
			bool valueChanged = (memcmp(mp_Value, value, sizeof(T) * COUNT) != 0);
			bool validValue = true;
			if(true == valueChanged)
			{
				for(int i = 0; i < COUNT; ++i)
				{
					String stringValue = StringEncoderDecoder<T>::EncodeToString(value[i]);
					if(false == m_ValidValueChecker.IsValidStringValue(stringValue))
					{
						ESP_LOGE("SetValue", "\"%s\" Value Rejected: \"%s\".", m_Name.c_str(), stringValue.c_str() );
						validValue = false;
					}
				}
				if(true == validValue)
				{
					memcpy(mp_Value, value, sizeof(T) * COUNT);
					++m_ValueChangeCount;
					ESP_LOGD( "LocalDataItem: SetValue"
							, "\"%s\" Set Value: \"%s\""
							, m_Name.c_str()
							, GetValueAsString().c_str());
					this->CallCallbacks(m_Name.c_str(), mp_Value);
				}
			}
			return (valueChanged && validValue);
		}

		virtual bool SetValue(T value)
		{
			assert(COUNT == 1);
			assert(mp_Value != nullptr);	
			bool valueChanged = (*mp_Value != value);
			bool validValue = true;
			const String stringValue = StringEncoderDecoder<T>::EncodeToString(value);
			if(true == valueChanged && false == m_ValidValueChecker.IsValidStringValue(stringValue))
			{
				validValue = false;
			}
			if(true == valueChanged && true == validValue)
			{
				*mp_Value = value;
				++m_ValueChangeCount;
				ESP_LOGD( "LocalDataItem: SetValue"
						, "\"%s\" Set Value: \"%s\""
						, m_Name.c_str()
						, GetValueAsString().c_str());
				this->CallCallbacks(m_Name.c_str(), mp_Value);
			}
			return (valueChanged && validValue);
		}

		bool EqualsValue(T *Object, size_t Count)
		{
			assert((Count == COUNT) && "Counts must equal");
			return (memcmp(mp_Value, Object, Count) == 0);
		}
	private:
		ValidValueChecker m_ValidValueChecker;
	protected:
		String m_Name;
		const T* const mp_InitialValuePtr;
		T *mp_Value = nullptr;
		T *mp_InitialValue = nullptr;
		NamedCallback_t *mp_NamedCallback = nullptr;
		size_t m_ValueChangeCount = 0;
	private:
		SetupCallerInterface *mp_SetupCallerInterface = nullptr;
		size_t m_Count = COUNT;
};