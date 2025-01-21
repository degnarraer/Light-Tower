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
			ESP_LOGI("DataItem<T, COUNT>::Setup()", "\"%s\": LocalDataItem Freeing Memory", m_Name);
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
			if(xSemaphoreTakeRecursive(m_ValueSemaphore, SEMAPHORE_BLOCK) == pdTRUE)
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
			else
			{
				ESP_LOGW("Semaphore Take Failure", "WARNING! Failed to take Semaphore");
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
			if(xSemaphoreTakeRecursive(m_ValueSemaphore, SEMAPHORE_BLOCK) == pdTRUE)
			{
				ESP_LOGD("DataItem<T, COUNT>::Setup()", "\"%s\": Allocating Memory", m_Name);
				if(mp_NamedCallback) this->RegisterNamedCallback(mp_NamedCallback);
				mp_Value = (T*)ps_malloc(sizeof(T)*COUNT);
				mp_InitialValue = (T*)ps_malloc(sizeof(T)*COUNT);
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
								, m_Name
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
								, m_Name
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
			else
			{
				ESP_LOGW("Semaphore Take Failure", "WARNING! Failed to take Semaphore");
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
		virtual std::string GetName() const
		{
			return m_Name;
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
			if(xSemaphoreTakeRecursive(m_ValueSemaphore, SEMAPHORE_BLOCK) == pdTRUE)
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
			else
			{
				ESP_LOGW("Semaphore Take Failure", "WARNING! Failed to take Semaphore");
			}
		}

		T* GetValuePointer() const
		{
			if(xSemaphoreTakeRecursive(m_ValueSemaphore, SEMAPHORE_BLOCK) == pdTRUE)
			{
				if(!mp_Value)
				{
					ESP_LOGE("GetValueAsString", "ERROR! \"%s\": NULL Pointer.", m_Name.c_str());
				}
				xSemaphoreGiveRecursive(m_ValueSemaphore);
			}
			else
			{
				ESP_LOGW("Semaphore Take Failure", "WARNING! Failed to take Semaphore");
			}
			return mp_Value;
		}

		virtual T GetValue() const
		{
			if(xSemaphoreTakeRecursive(m_ValueSemaphore, SEMAPHORE_BLOCK) == pdTRUE)
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
			else
			{
				ESP_LOGW("Semaphore Take Failure", "WARNING! Failed to take Semaphore");
			}
			return T();
		}

		virtual bool GetInitialValueAsString(std::string &stringValue) const
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

		virtual std::string GetInitialValueAsString() const
		{
			std::string value;
			if(!GetInitialValueAsString(value))
			{
				ESP_LOGE("GetInitialValueAsString", "ERROR! \"%s\": Unable to Get String Value! Returning Empty String.", m_Name.c_str());
				value = "";
			}
			return value;
		}

		virtual bool GetValueAsString(std::string &stringValue) const
		{
			if(xSemaphoreTakeRecursive(m_ValueSemaphore, SEMAPHORE_BLOCK) == pdTRUE)
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
			else
			{
				ESP_LOGW("Semaphore Take Failure", "WARNING! Failed to take Semaphore");
			}
			return false;
		}

		virtual std::string GetValueAsString() const
		{
			std::string value;
			if(!GetValueAsString(value))
			{
				value = "";
			}
			return value;
		}

		static UpdateStatus_t StaticSetValueFromString(const std::string& stringValue, void* objectptr)
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

		virtual size_t ParseStringValueIntoValues(const std::string& stringValue, T* values) 
		{
			// Use string_view for efficient substring manipulation without allocations
			std::string_view input(stringValue);

			size_t substringCount = 0;
			size_t start = 0;

			while (start < input.size() && substringCount < COUNT) 
			{
				// Find the position of the next ENCODE_OBJECT_DIVIDER
				size_t end = input.find(ENCODE_OBJECT_DIVIDER, start);

				// If not found, end at the end of the string
				if (end == std::string_view::npos) {
					end = input.size();
				}

				// Extract the substring view
				std::string_view substring = input.substr(start, end - start);

				if (!substring.empty()) {
					// Validate the substring
					if (!m_ValidValueChecker.IsValidStringValue(std::string(substring))) {
						ESP_LOGW("SetValue", "WARNING! \"%s\" Value Rejected: \"%s\".", m_Name.c_str(), std::string(substring).c_str());
						return 0;
					}

					// Decode and store the value
					values[substringCount++] = StringEncoderDecoder<T>::DecodeFromString(std::string(substring));
				} else {
					ESP_LOGW("SetValue", "WARNING! Empty Value Rejected at position %zu.", start);
					return 0;
				}

				// Advance to the next position
				start = end + 1;
			}

			// Validate substring count
			if (substringCount != COUNT) {
				ESP_LOGE("SetValueFromString", "Expected %zu substrings but got %zu in string: \"%s\".",
						COUNT, substringCount, stringValue.c_str());
				return 0;
			}

			return substringCount;
		}



		virtual UpdateStatus_t SetValueFromString(const std::string& stringValue)
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

		virtual UpdateStatus_t SetValue(const T *newValues, size_t count)
		{
			ESP_LOGD( "LocalDataItem: SetValue"
					, "\"%s\" Set Value: \"%s\""
					, m_Name.c_str()
					, this->ConvertValueToString(newValues, count) );
			return UpdateStore(GetValuePointer(), newValues, GetChangeCount()+1);
		}

		virtual UpdateStatus_t SetValue(const T& value)
		{
			assert(COUNT == 1);
			assert(mp_Value != nullptr);	
			return SetValue(&value, 1);
		}

		bool EqualsValue(T *values, size_t count) const
		{
			if(xSemaphoreTakeRecursive(m_ValueSemaphore, SEMAPHORE_BLOCK) == pdTRUE)
			{
				if(COUNT == count)
				{
					bool result = (memcmp(mp_Value, values, sizeof(T)*count) == 0);
					xSemaphoreGiveRecursive(m_ValueSemaphore);
					return result;
				}
				xSemaphoreGiveRecursive(m_ValueSemaphore);
			}
			else
			{
				ESP_LOGW("Semaphore Take Failure", "WARNING! Failed to take Semaphore");
			}
			return false;
		}

		virtual std::string ConvertValueToString(const T* pvalue, size_t count) const
		{
			if (!pvalue || count == 0)
			{
				return ""; // Return an empty string if no values are provided
			}

			// Reserve space in the resulting string to reduce reallocations
			std::ostringstream resultStream;

			for (size_t i = 0; i < count; ++i)
			{
				// Encode the value and append it to the result stream
				std::string encodedString = StringEncoderDecoder<T>::EncodeToString(pvalue[i]);
				ESP_LOGD("ConvertValueToString", "Encoded String \"%s\"", encodedString.c_str());

				resultStream << encodedString;

				// Add a delimiter after each value except the last one
				if (i < count - 1)
				{
					resultStream << ENCODE_OBJECT_DIVIDER;
				}
			}

			// Convert the stream into a string
			return resultStream.str();
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

		virtual UpdateStatus_t UpdateStore(T* oldValues, const T* newValues, const size_t newChangeCount)
		{
			UpdateStatus_t updateStatus;

			if (xSemaphoreTakeRecursive(m_ValueSemaphore, SEMAPHORE_BLOCK) == pdTRUE)
			{
				assert(newValues != nullptr);
				assert(oldValues != nullptr);
				assert(COUNT > 0);

				const char* name = GetName().c_str();
				bool valueChanged = (memcmp(oldValues, newValues, sizeof(T) * COUNT) != 0);
				bool validValue = valueChanged ? ConfirmValueValidity(newValues, COUNT) : true;

				ESP_LOGD("UpdateStore", 
						"Name: \"%s\" Update Store with value: \"%s\" Change Count: \"%i\" New Change Count: \"%i\"", 
						name, ConvertValueToString(newValues, COUNT).c_str(), m_ChangeCount, newChangeCount);

				updateStatus.ValueChanged = valueChanged;
				updateStatus.ValidValue = validValue;
				updateStatus.UpdateAllowed = valueChanged && validValue;
				updateStatus.UpdateSuccessful = UpdateChangeCount(m_ChangeCount, updateStatus.UpdateAllowed);

				if (updateStatus.UpdateSuccessful)
				{
					// Use memcpy directly; no need to zero out memory beforehand.
					memcpy(oldValues, newValues, sizeof(T) * COUNT);

					if (memcmp(oldValues, newValues, sizeof(T) * COUNT) == 0)
					{
						this->CallNamedCallbacks(oldValues);
					}
					else
					{
						ESP_LOGE("UpdateStore", 
								"\"%s\": Update Store: Not Successful. Value: \"%s\" Change Count: \"%i\"", 
								name, GetValueAsString().c_str(), m_ChangeCount);
						updateStatus.UpdateSuccessful = false;
					}
				}
				else
				{
					ESP_LOGD("UpdateStore", 
							"\"%s\": Update Store: Not Allowed. Change Count: \"%i\"", 
							name, m_ChangeCount);
				}

				ESP_LOGD("UpdateStore", 
						"\"%s\": Update Status: \"%i|%i|%i|%i\"", 
						name, updateStatus.ValueChanged, updateStatus.ValidValue, 
						updateStatus.UpdateAllowed, updateStatus.UpdateSuccessful);

				xSemaphoreGiveRecursive(m_ValueSemaphore);
			}
			else
			{
				ESP_LOGW("Semaphore Take Failure", "WARNING! Failed to take Semaphore");
			}

			return updateStatus;
		}


		virtual bool ConfirmValueValidity(const T* values, size_t count) const
		{
			if(m_ValidValueChecker.IsConfigured())
			{
				for(int i = 0; i < count; ++i)
				{
					std::string stringValue = StringEncoderDecoder<T>::EncodeToString(values[i]);
					if(!this->m_ValidValueChecker.IsValidStringValue(stringValue))
					{
						ESP_LOGW("SetValue", "WARNING! \"%s\" Value Rejected: \"%s\".", this->GetName().c_str(), stringValue.c_str() );
						return false;
					}		
				}
			}
			return true;
		}
	private:
		SetupCallerInterface *mp_SetupCallerInterface = nullptr;
		size_t m_Count = COUNT;
		size_t m_ChangeCount = 0;
		bool m_ChangeCountInitialized = false;
	protected:
		SemaphoreHandle_t m_ValueSemaphore;
};