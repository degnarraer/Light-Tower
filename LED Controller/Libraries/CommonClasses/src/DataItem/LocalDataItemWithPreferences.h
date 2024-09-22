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
#define PREFERENCE_TIMEOUT 10000UL

template <typename T, size_t COUNT>
class LocalDataItemWithPreferences: public LocalDataItem<T, COUNT>
{
	public:
		LocalDataItemWithPreferences( const String name
									, const T* initialValue
									, IPreferences *preferencesInterface
									, NamedCallback_t *namedCallback )
									: LocalDataItem<T, COUNT>( name
															 , initialValue
															 , namedCallback )
									, mp_preferencesInterface(preferencesInterface)
									, mp_PreferenceManager(nullptr)
		{
		}		   
		LocalDataItemWithPreferences( const String name
									, const T& initialValue
									, IPreferences *preferencesInterface
									, NamedCallback_t *namedCallback
									, SetupCallerInterface *setupCallerInterface )
									: LocalDataItem<T, COUNT>( name
															 , initialValue
															 , namedCallback
															 , setupCallerInterface )
									, mp_preferencesInterface(preferencesInterface)
									, mp_PreferenceManager(nullptr)
		{
		}

		LocalDataItemWithPreferences( const String name
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
									, mp_preferencesInterface(preferencesInterface)
									, mp_PreferenceManager(nullptr)
		{
		}
							   
		LocalDataItemWithPreferences( const String name
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
									, mp_preferencesInterface(preferencesInterface)
									, mp_PreferenceManager(nullptr)
		{
		}

		virtual ~LocalDataItemWithPreferences()
		{
			ESP_LOGI("~LocalDataItemWithPreferences()", "\"%s\": Freeing Memory", this->m_Name.c_str());
			if(mp_PreferenceManager)
			{
				delete mp_PreferenceManager;
				mp_PreferenceManager = nullptr;
			}
		}

		virtual void Setup() override
		{
			LocalDataItem<T, COUNT>::Setup();
			mp_PreferenceManager = new PreferenceManager( mp_preferencesInterface
							   					  		, this->m_Name
							   				 	  		, this->GetInitialValueAsString()
												  		, PREFERENCE_TIMEOUT
							   				 	  		, this->StaticSetValueFromString
							   				 	  		, this );
			mp_PreferenceManager->InitializeAndLoadPreference();
		}
		
		virtual size_t GetCount() const override
		{
			return LocalDataItem<T, COUNT>::GetCount();
		}

		virtual size_t GetChangeCount() const override
		{
			return LocalDataItem<T, COUNT>::GetChangeCount();
		}

		virtual bool SetValue(const T* values, size_t count) override
		{
			ESP_LOGI("SetValue", "SetValue for Local Data Item With Preferences 1");
			bool result = LocalDataItem<T, COUNT>::SetValue(values, count);
			if(result)
			{
				
				ESP_LOGI("SetValue", "SetValue for Local Data Item With Preferences 2");
				mp_PreferenceManager->Update_Preference( PreferenceManager::PreferenceUpdateType::Save
									   				   , this->GetValueAsString() );
			}
			return result;
		}
		
		virtual bool SetValue(const T& value) override
		{
			return this->SetValue(&value, 1);
		}

		virtual bool SetValueFromString(const String& stringValue) override
		{
			bool result = LocalDataItem<T, COUNT>::SetValueFromString(stringValue);
			if(result)
			{
				mp_PreferenceManager->Update_Preference( PreferenceManager::PreferenceUpdateType::Save
									   				   , this->GetValueAsString() );
			}
			return result;
		}

		virtual bool ConfirmValueValidity(const T* values, size_t count) const override
		{
			return LocalDataItem<T, COUNT>::ConfirmValueValidity(values, count);
		}

	private:
		IPreferences *mp_preferencesInterface = nullptr;
		PreferenceManager *mp_PreferenceManager = nullptr;
};