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

#include <Preferences.h>
#include <nvs_flash.h>
#include <esp_timer.h>
#include <Arduino.h>
#include <esp_heap_caps.h>
#include "DataItemInterface.h"
#include "SerialMessageManager.h"
#include "SetupCallInterfaces.h"
#include "ValidValueChecker.h"
#include "StringEncoderDecoder.h"


template <typename T, size_t COUNT>
class LocalDataItem: public DataItemInterface<T, COUNT>
			 	   , public SetupCalleeInterface
				   , public Named_Callback_Caller_Interface<T>
			 	   , public DataTypeFunctions
			 	   , public ValidValueChecker
			 	   , public StringEncoderDecoder<T>
{
	public:
		LocalDataItem( const std::string name
					 , const T* initialValue )
					 : m_Name(name)
					 , mp_InitialValuePtr(initialValue)
		{
			ESP_LOGD("LocalDataItem", "LocalDataItem Instantiated: Constructor 1");
			CommonSetup();
		}
		LocalDataItem( const std::string name
					 , const T& initialValue )
					 : m_Name(name)
					 , mp_InitialValuePtr(&initialValue)
		{
			ESP_LOGD("LocalDataItem", "LocalDataItem Instantiated: Constructor 2");
			CommonSetup();
		}
		LocalDataItem( const std::string name
					 , const T* initialValue
					 , NamedCallback_t *namedCallback
					 , SetupCallerInterface *setupCallerInterface )
					 : m_ValidValueChecker(ValidValueChecker())
					 , m_Name(name)
					 , mp_InitialValuePtr(initialValue)
					 , mp_NamedCallback(namedCallback)
					 , mp_SetupCallerInterface(setupCallerInterface)
		{
			ESP_LOGD("LocalDataItem", "LocalDataItem Instantiated: Constructor 3");
			CommonSetup();
		}
		
		LocalDataItem( const std::string name
					 , const T& initialValue
					 , NamedCallback_t *namedCallback
					 , SetupCallerInterface *setupCallerInterface )
					 : m_ValidValueChecker(ValidValueChecker())
					 , m_Name(name)
					 , mp_InitialValuePtr(&initialValue)
					 , mp_NamedCallback(namedCallback)
					 , mp_SetupCallerInterface(setupCallerInterface)
		{
			ESP_LOGD("LocalDataItem", "LocalDataItem Instantiated: Constructor 4");
			CommonSetup();
		}

		LocalDataItem( const std::string name
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
			ESP_LOGD("LocalDataItem", "LocalDataItem Instantiated: Constructor 5");
			CommonSetup();
		}
		
		LocalDataItem( const std::string name
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
			ESP_LOGD("LocalDataItem", "LocalDataItem Instantiated: Constructor 6");
			CommonSetup();
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
			if(xSemaphoreTakeRecursive(m_ValueSemaphore, portMAX_DELAY) == pdTRUE)
			{
				if(mp_Value)
				{
					ESP_LOGD("~LocalDataItem", "freeing mp_Value Memory");
					free(mp_Value);
					mp_Value = nullptr;
				}
				if(mp_InitialValue) 
				{
					ESP_LOGD("~LocalDataItem", "freeing mp_InitialValue Memory");
					free(mp_InitialValue);
					mp_InitialValue = nullptr;
				}
				xSemaphoreGiveRecursive(m_ValueSemaphore);
			}
			if (m_ValueSemaphore)
			{
				vSemaphoreDelete(m_ValueSemaphore);
				m_ValueSemaphore = nullptr;
			}
		}

		void CommonSetup()
		{
			RegisterForSetup();
			m_ValueSemaphore = xSemaphoreCreateMutex();
			if (m_ValueSemaphore == nullptr)
			{
				ESP_LOGE("AudioBuffer", "ERROR! Failed to create semaphore.");
			}
		}
		//SetupCalleeInterface
		virtual void Setup() override
		{
			if(xSemaphoreTakeRecursive(m_ValueSemaphore, portMAX_DELAY) == pdTRUE)
			{
				ESP_LOGD("DataItem<T, COUNT>::Setup()", "\"%s\": Allocating Memory", m_Name.c_str());
				if(mp_NamedCallback) this->RegisterNamedCallback(mp_NamedCallback);
				mp_Value = (T*)malloc(sizeof(T)*COUNT);
				mp_InitialValue = (T*)malloc(sizeof(T)*COUNT);
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
						ESP_LOGD( "DataItem<T, COUNT>::Setup()", "\"%s\": Set initial value <char>: \"%s\""
								, m_Name.c_str()
								, GetInitialValueAsString().c_str());
						this->CallNamedCallbacks(mp_Value);
					}
					else
					{
						for (size_t i = 0; i < COUNT; ++i)
						{
							memcpy(mp_Value+i, mp_InitialValuePtr, sizeof(T));
							memcpy(mp_InitialValue+i, mp_InitialValuePtr, sizeof(T));
						}
						ESP_LOGD( "DataItem<T, COUNT>::Setup()", "\"%s\": Set initial value <T>: \"%s\""
								, m_Name.c_str()
								, GetInitialValueAsString().c_str());
						this->CallNamedCallbacks(mp_Value);
					}
				}
				else
				{
					ESP_LOGE("DataItem<T, COUNT>::Setup()", "Failed to allocate memory on spi ram.");
				}
				xSemaphoreGiveRecursive(m_ValueSemaphore);
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

		//DataItemInterface
		String GetName() const
		{
			return String(m_Name.c_str());
		}

		virtual size_t GetCount() const
		{
			return m_Count;
		}

		virtual size_t GetChangeCount() const
		{
			return m_ChangeCount;
		}

		DataType_t GetDataType()
		{
			return GetDataTypeFromTemplateType<T>();
		}

		void ResetToDefaultValue()
		{
			SetValue(mp_InitialValue, COUNT);
		}

		void GetValue(void* object, size_t count) const
		{
			if(xSemaphoreTakeRecursive(m_ValueSemaphore, portMAX_DELAY) == pdTRUE)
			{
				assert((count == COUNT) && "Counts must be equal");
				if (mp_Value)
				{
					memcpy(object, mp_Value, sizeof(T) * count);
				}
				else
				{
					ESP_LOGE("GetValue", "ERROR! NULL Pointer in mp_Value.");
					if constexpr (std::is_pointer_v<T>)
					{
						*reinterpret_cast<T*>(object) = nullptr;
					}
				}
				xSemaphoreGiveRecursive(m_ValueSemaphore);
			}
		}

		T* GetValuePointer() const
		{
			if(xSemaphoreTakeRecursive(m_ValueSemaphore, portMAX_DELAY) == pdTRUE)
			{
				if(!mp_Value)
				{
					ESP_LOGE("GetValueAsString", "ERROR! \"%s\": NULL Pointer.", m_Name.c_str());
				}
				xSemaphoreGiveRecursive(m_ValueSemaphore);
			}
			return mp_Value;
		}

		virtual T GetValue() const
		{
			if(xSemaphoreTakeRecursive(m_ValueSemaphore, portMAX_DELAY) == pdTRUE)
			{
				assert((1 == COUNT) && "Count must 1 to use this function");
				if(mp_Value)
				{
					xSemaphoreGiveRecursive(m_ValueSemaphore);
					return static_cast<T>(*mp_Value);
				}
				else
				{
					ESP_LOGE("GetValueAsString", "ERROR! \"%s\": NULL Pointer.", m_Name.c_str());
					xSemaphoreGiveRecursive(m_ValueSemaphore);
					return T();
				}
			}
			return T();
		}

		virtual bool GetInitialValueAsString(String &stringValue) const
		{
			stringValue = "";
			if (mp_InitialValue && COUNT > 0)
			{
				stringValue = ConvertValueToString(mp_InitialValue, COUNT);
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
			if(xSemaphoreTakeRecursive(m_ValueSemaphore, portMAX_DELAY) == pdTRUE)
			{
				stringValue = "";
				if (mp_Value && COUNT > 0)
				{
					stringValue = ConvertValueToString(mp_Value, COUNT);
					ESP_LOGV("GetValueAsString", "\"%s\": String Value: \"%s\"", m_Name.c_str(), stringValue.c_str());
					xSemaphoreGiveRecursive(m_ValueSemaphore);
					return true;
				}
				else
				{
					ESP_LOGE("GetValueAsString", "ERROR! \"%s\": NULL Pointer.", m_Name.c_str());
					xSemaphoreGiveRecursive(m_ValueSemaphore);
					return false;
				}
			}
			return false;
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

		static UpdateStatus_t StaticSetValueFromString(const String& stringValue, void* objectptr)
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
					return UpdateStatus_t();
				}
			}
			else
			{
            	ESP_LOGE("StaticSetValueFromString", "ERROR! Null Object Pointer.");
				return UpdateStatus_t();
			}
		}

		virtual size_t ParseStringValueIntoValues(const String& stringValue, T* values)
		{
			std::vector<String> substrings;
			size_t start = 0;
			size_t end = stringValue.indexOf(ENCODE_OBJECT_DIVIDER);

			// Split the input string by ENCODE_OBJECT_DIVIDER
			while (end != -1)
			{
				String parsedString = stringValue.substring(start, end);
				ESP_LOGD("SetValueFromString", "Parsed String: \"%s\"", parsedString.c_str());
				substrings.push_back(parsedString);
				start = end + 1;
				end = stringValue.indexOf(ENCODE_OBJECT_DIVIDER, start);
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
				return 0;
			}

			// Decode each substring and store it in the value array
			ESP_LOGD("ParseStringValueIntoValues", "\"%s\" Parsed %i Strings.", m_Name.c_str(), substrings.size() );
			for (size_t i = 0; i < substrings.size(); ++i) 
			{
				if(false == m_ValidValueChecker.IsValidStringValue(substrings[i]))
				{
					ESP_LOGW("SetValue", "WARNING! \"%s\" Value Rejected: \"%s\".", m_Name.c_str(), substrings[i].c_str() );
					return 0;
				}
				values[i] = StringEncoderDecoder<T>::DecodeFromString(substrings[i]);
			}
			return substrings.size();
		}

		virtual UpdateStatus_t SetValueFromString(const String& stringValue)
		{
			ESP_LOGI( "LocalDataItem::SetValueFromString"
					, "Name: \"%s\" String Value: \"%s\""
					, m_Name.c_str()
					, stringValue.c_str() );
			T values[COUNT];
			size_t parseCount = ParseStringValueIntoValues(stringValue, values);
			if(parseCount == COUNT)
			{
				return SetValue(values, parseCount);
			}
			else
			{
				ESP_LOGE("SetValueFromString", "Name: \"%s\" Count Error!", this->GetName().c_str() );
				return UpdateStatus_t();
			}
		}

		virtual UpdateStatus_t SetValue(const T *values, size_t count)
		{
			ESP_LOGD( "LocalDataItem: SetValue"
					, "\"%s\" Set Value: \"%s\""
					, m_Name.c_str()
					, this->ConvertValueToString(values, count).c_str() );
			return UpdateStore(values, GetChangeCount()+1);
		}

		virtual UpdateStatus_t SetValue(const T& value)
		{
			assert(COUNT == 1);
			assert(mp_Value != nullptr);	
			return SetValue(&value, 1);
		}

		bool EqualsValue(T *values, size_t count) const
		{
			if(xSemaphoreTakeRecursive(m_ValueSemaphore, portMAX_DELAY) == pdTRUE)
			{
				if(COUNT == count)
				{
					bool result = (memcmp(mp_Value, values, sizeof(T)*count) == 0);
					xSemaphoreGiveRecursive(m_ValueSemaphore);
					return result;
				}
				xSemaphoreGiveRecursive(m_ValueSemaphore);
			}
			return false;
		}

		virtual String ConvertValueToString(const T *pvalue, size_t count) const
		{
			std::vector<String> valueStrings;
			if(pvalue && count > 0)
			{
				for (size_t i = 0; i < count; ++i)
				{
					String encodedString = StringEncoderDecoder<T>::EncodeToString(pvalue[i]);
					ESP_LOGD("ConvertValueToString", "Encoded String \"%s\"", encodedString.c_str());
					valueStrings.push_back(encodedString);
				}
			}
			String stringValue = "";
			for (size_t i = 0; i < count - 1; ++i)
			{
				stringValue += valueStrings[i];
				stringValue += ENCODE_OBJECT_DIVIDER;
			}
			stringValue += valueStrings[COUNT - 1];
			return stringValue;
		}

	protected:
		ValidValueChecker m_ValidValueChecker;
		std::string m_Name;
		const T* const mp_InitialValuePtr;
		T *mp_Value = nullptr;
		T *mp_InitialValue = nullptr;
		NamedCallback_t *mp_NamedCallback = nullptr;
		
		bool UpdateChangeCount(const size_t newChangeCount, const bool incrementChangeCount)
		{
			bool allowUpdate = false;
			if(!m_ChangeCountInitialized)
			{
				m_ChangeCount = newChangeCount;
				m_ChangeCountInitialized = true;
				ESP_LOGD("UpdateChangeCount", "\"%s\": Change Count Initialized: \"%i\"", GetName().c_str(), m_ChangeCount);
			}
			if(incrementChangeCount)
			{
				m_ChangeCount += 1;
				allowUpdate = true;
				ESP_LOGD("UpdateChangeCount", "\"%s\": Change Count Incremented: \"%i\"", GetName().c_str(), m_ChangeCount);
			}
			return allowUpdate;
		}

		bool IsChangeCountSmaller(size_t changeCount)
		{
			return (changeCount < m_ChangeCount) && (m_ChangeCount - changeCount <= (SIZE_MAX / 2));
		}

		virtual UpdateStatus_t UpdateStore(const T *newValues, const size_t newChangeCount)
		{
			UpdateStatus_t updateStatus;
			if(xSemaphoreTakeRecursive(m_ValueSemaphore, portMAX_DELAY) == pdTRUE)
			{
				assert(newValues != nullptr);
				assert(mp_Value != nullptr);
				assert(COUNT > 0);
				ESP_LOGD( "UpdateStore"
						, "Name: \"%s\" Update Store with value: \"%s\" Change Count: \"%i\" New Change Count: \"%i\""
						, GetName().c_str()
						, ConvertValueToString(newValues, COUNT).c_str()
						, m_ChangeCount
						, newChangeCount );
				updateStatus.ValueChanged = (0 != memcmp(mp_Value, newValues, sizeof(T)*COUNT));
				updateStatus.ValidValue = ConfirmValueValidity(newValues, COUNT);
				updateStatus.UpdateAllowed = updateStatus.ValueChanged && updateStatus.ValidValue;
				updateStatus.UpdateSuccessful = UpdateChangeCount(newChangeCount, updateStatus.UpdateAllowed);
				ESP_LOGD( "UpdateStore", "\"%s\": UpdateAllowed: \"%i\" Store Updated: \"%i\"", GetName().c_str(), updateAllowed, storeUpdated);
				if(updateStatus.UpdateSuccessful)
				{
					ZeroOutMemory(mp_Value);
					memcpy(mp_Value, newValues, sizeof(T) * COUNT);
					updateStatus.UpdateSuccessful = ( memcmp(mp_Value, newValues, sizeof(T) * COUNT) == 0);
					if(updateStatus.UpdateSuccessful)
					{
						ESP_LOGD( "UpdateStore", "\"%s\": Update Store: Successful. Value: \"%s\" Change Count: \"%i\"", GetName().c_str(), GetValueAsString().c_str(), m_ChangeCount);
						this->CallNamedCallbacks(mp_Value);
					}
					else
					{
						ESP_LOGE( "UpdateStore", "\"%s\": Update Store: Not Successful. Value: \"%s\" Change Count: \"%i\"", GetName().c_str(), GetValueAsString().c_str(), m_ChangeCount);
					}
				}
				else
				{
					ESP_LOGD( "UpdateStore", "\"%s\": Update Store: Not Allowed. Change Count: \"%i\"", GetName().c_str(), m_ChangeCount);
				}
				xSemaphoreGiveRecursive(m_ValueSemaphore);
			}
			return updateStatus;
		}

		virtual bool ConfirmValueValidity(const T* values, size_t count) const
		{
			for(int i = 0; i < count; ++i)
			{
				String stringValue = StringEncoderDecoder<T>::EncodeToString(values[i]);
				if(false == this->m_ValidValueChecker.IsValidStringValue(stringValue))
				{
					ESP_LOGW("SetValue", "WARNING! \"%s\" Value Rejected: \"%s\".", this->GetName().c_str(), stringValue.c_str() );
					return false;
				}
			}
			return true;
		}
		void ZeroOutMemory(T* object)
		{
			for (size_t i = 0; i < COUNT; ++i)
			{
				object[i] = T();
			}
		}
	private:
		SetupCallerInterface *mp_SetupCallerInterface = nullptr;
		size_t m_Count = COUNT;
		size_t m_ChangeCount = 0;
		bool m_ChangeCountInitialized = false;
	protected:
		SemaphoreHandle_t m_ValueSemaphore;
};