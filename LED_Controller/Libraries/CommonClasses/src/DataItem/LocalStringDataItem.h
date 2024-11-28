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

#include "DataItem/LocalDataItem.h"

#define DATAITEM_STRING_LENGTH 50

class LocalStringDataItem: public LocalDataItem<char, DATAITEM_STRING_LENGTH>
{
	public:
		LocalStringDataItem( const std::string name
					 	   , const std::string &initialValue
					 	   , NamedCallback_t *namedCallback
					 	   , SetupCallerInterface *setupCallerInterface )
						   : LocalDataItem<char, DATAITEM_STRING_LENGTH>( name, initialValue.c_str(), namedCallback, setupCallerInterface )
						   {
						   }

		virtual ~LocalStringDataItem() override
		{
			ESP_LOGI("LocalStringDataItem::~LocalStringDataItem()", "\"%s\": Freeing Memory", m_Name.c_str());
		}

		virtual size_t GetCount() const override
		{
			return LocalDataItem<char, DATAITEM_STRING_LENGTH>::GetCount();
		}

		virtual size_t GetChangeCount() const override
		{
			return LocalDataItem<char, DATAITEM_STRING_LENGTH>::GetChangeCount();
		}

		virtual bool GetInitialValueAsString(String &stringValue) const override
		{
			if(xSemaphoreTakeRecursive(this->m_ValueSemaphore, portMAX_DELAY) == pdTRUE)
			{
				if(mp_InitialValue)
				{
					stringValue = String(mp_InitialValue);
					ESP_LOGD("GetInitialValueAsString", "\"%s\": GetInitialValueAsString: \"%s\"", m_Name.c_str(), stringValue.c_str());
					xSemaphoreGiveRecursive(this->m_ValueSemaphore);
					return true;
				}
				else
				{
					ESP_LOGE("GetValueAsString", "ERROR! \"%s\": NULL Pointer.", m_Name.c_str());
					xSemaphoreGiveRecursive(this->m_ValueSemaphore);
					return false;
				}
			}
			else
			{
				ESP_LOGW("Semaphore Take Failure", "WARNING! Failed to take Semaphore");
			}
			return false;
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

		virtual bool GetValueAsString(String &stringValue) const override
		{
			if(xSemaphoreTakeRecursive(this->m_ValueSemaphore, portMAX_DELAY) == pdTRUE)
			{
				if(mp_Value)
				{
					stringValue = String(mp_Value);
					ESP_LOGD("GetValueAsString"
							, "\"%s\": GetValueAsString: %s"
							, m_Name.c_str()
							, stringValue.c_str());
					xSemaphoreGiveRecursive(this->m_ValueSemaphore);
					return true;
				}
				else
				{
					xSemaphoreGiveRecursive(this->m_ValueSemaphore);
					return false;
				}
			}
			else
			{
				ESP_LOGW("Semaphore Take Failure", "WARNING! Failed to take Semaphore");
			}
			return false;
		}

		virtual String GetValueAsString() const override
		{
			String value;
			if(!GetValueAsString(value))
			{
				value = "";
			}
			return value;
		}

		virtual UpdateStatus_t SetValueFromString(const String& stringValue) override
		{
			ESP_LOGD("LocalStringDataItem::SetValueFromString"
					, "Name: \"%s\" String Value: \"%s\""
					, m_Name.c_str()
					, stringValue.c_str());
			return SetValue(stringValue.c_str(), stringValue.length());
		}

		virtual UpdateStatus_t SetValue(const char* value, size_t count) override
		{
			UpdateStatus_t updateStatus;
			if(xSemaphoreTakeRecursive(this->m_ValueSemaphore, portMAX_DELAY) == pdTRUE)
			{
				assert(value != nullptr);
				assert(mp_Value != nullptr);
				assert(count <= DATAITEM_STRING_LENGTH);

				std::string newValue(value, count);
				ESP_LOGD("DataItem: SetValue", "\"%s\" Set Value: \"%s\"", m_Name.c_str(), newValue.c_str());
				
				updateStatus.ValueChanged = (strncmp(mp_Value, value, count) != 0);
				updateStatus.ValidValue = ConfirmValueValidity(value, count);
				updateStatus.UpdateAllowed = UpdateChangeCount(GetChangeCount(), (updateStatus.ValueChanged && updateStatus.ValidValue));
				if (updateStatus.UpdateAllowed)
				{   
					ZeroOutMemory(mp_Value);
					strncpy(mp_Value, value, count);
					updateStatus.UpdateSuccessful = (strncmp(mp_Value, value, count) == 0);
					if(updateStatus.UpdateSuccessful)
					{
						ESP_LOGI("LocalDataItem: SetValue", "\"%s\": Set Value to \"%s\".", GetName().c_str(), newValue.c_str());
						this->CallNamedCallbacks(mp_Value);
					}
					else
					{
						ESP_LOGE("LocalDataItem: SetValue", "ERROR! \"%s\": Setting value to \"%s\".", GetName().c_str(), newValue.c_str());
					}
				}
				xSemaphoreGiveRecursive(this->m_ValueSemaphore);
			}
			else
			{
				ESP_LOGW("Semaphore Take Failure", "WARNING! Failed to take Semaphore");
			}
			return updateStatus;
		}

		virtual bool ConfirmValueValidity(const char *values, size_t count) const override
		{
			return LocalDataItem<char, DATAITEM_STRING_LENGTH>::ConfirmValueValidity(values, count);
		}

		virtual String ConvertValueToString(const char* pvalue, size_t count) const override
		{
			return String(pvalue);
		}
};