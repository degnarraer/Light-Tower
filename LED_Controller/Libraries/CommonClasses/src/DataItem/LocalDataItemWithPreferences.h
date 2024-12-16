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
#include "DataItem/PreferencesWrapper.h"
#define PREFERENCE_TIMEOUT 5000UL

template <typename T, size_t COUNT>
class LocalDataItemWithPreferences: public LocalDataItem<T, COUNT>
							 	  , public PreferenceManager
{
	public:
		LocalDataItemWithPreferences( const std::string name
									, const T* initialValue
									, IPreferences *preferencesInterface
									, NamedCallback_t *namedCallback )
									: LocalDataItem<T, COUNT>( name
															 , initialValue
															 , namedCallback )
									, PreferenceManager( preferencesInterface
									 				   , String(this->m_Name.c_str())
									 				   , this->GetInitialValueAsString()
													   , PREFERENCE_TIMEOUT
									 				   , this->StaticSetValueFromString
									 				   , this )
		{
		}		   
		LocalDataItemWithPreferences( const std::string name
									, const T& initialValue
									, IPreferences *preferencesInterface
									, NamedCallback_t *namedCallback
									, SetupCallerInterface *setupCallerInterface )
									: LocalDataItem<T, COUNT>( name
															 , initialValue
															 , namedCallback
															 , setupCallerInterface )
									, PreferenceManager( preferencesInterface
									 				   , String(this->m_Name.c_str())
									 				   , this->GetInitialValueAsString()
													   , PREFERENCE_TIMEOUT
									 				   , this->StaticSetValueFromString
									 				   , this )
		{
		}

		LocalDataItemWithPreferences( const std::string name
									, const T* initialValue
									, IPreferences *preferencesInterface
									, NamedCallback_t *namedCallback
									, SetupCallerInterface *setupCallerInterface
									, const ValidStringValues_t *validStringValues )
									: LocalDataItem<T, COUNT>( name
															 , initialValue
															 , namedCallback
															 , validStringValues
															 , setupCallerInterface )
									, PreferenceManager( preferencesInterface
									 				   , String(this->m_Name.c_str())
									 				   , this->GetInitialValueAsString()
													   , PREFERENCE_TIMEOUT
									 				   , this->StaticSetValueFromString
									 				   , this )
		{
		}
							   
		LocalDataItemWithPreferences( const std::string name
									, const T& initialValue
									, IPreferences *preferencesInterface
									, NamedCallback_t *namedCallback
									, SetupCallerInterface *setupCallerInterface
									, const ValidStringValues_t *validStringValues )
									: LocalDataItem<T, COUNT>( name
															 , initialValue
															 , namedCallback
															 , setupCallerInterface
															 , validStringValues )
									, PreferenceManager( preferencesInterface
									 				   , name
									 				   , this->GetInitialValueAsString()
													   , PREFERENCE_TIMEOUT
									 				   , this->StaticSetValueFromString
									 				   , this )
		{
		}

		virtual ~LocalDataItemWithPreferences()
		{
			ESP_LOGI("~LocalDataItemWithPreferences()", "\"%s\": Freeing Memory", this->m_Name.c_str());
		}

		virtual void Setup() override
		{
			LocalDataItem<T, COUNT>::Setup();
			PreferenceManager::InitializeAndLoadPreference();
		}
		
		virtual size_t GetCount() const override
		{
			return LocalDataItem<T, COUNT>::GetCount();
		}

		virtual size_t GetChangeCount() const override
		{
			return LocalDataItem<T, COUNT>::GetChangeCount();
		}

		virtual UpdateStatus_t SetValue(const T* values, size_t count) override
		{
			UpdateStatus_t result = LocalDataItem<T, COUNT>::SetValue(values, count);
			if(result.UpdateSuccessful)
			{
				this->Update_Preference( PreferenceManager::PreferenceUpdateType::Save
									   , this->GetValueAsString() );
			}
			return result;
		}
		
		virtual UpdateStatus_t SetValue(const T& value) override
		{
			return this->SetValue(&value, 1);
		}

		virtual UpdateStatus_t SetValueFromString(const std::string& stringValue) override
		{
			UpdateStatus_t result = LocalDataItem<T, COUNT>::SetValueFromString(stringValue);
			if(result.UpdateSuccessful)
			{
				this->Update_Preference( PreferenceManager::PreferenceUpdateType::Save
									   				   , this->GetValueAsString() );
			}
			return result;
		}

		virtual bool ConfirmValueValidity(const T* values, size_t count) const override
		{
			return LocalDataItem<T, COUNT>::ConfirmValueValidity(values, count);
		}
};