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

#include "DataItem/DataItem.h"
#include "DataItem/PreferencesWrapper.h"
#define PREFERENCE_TIMEOUT 10000UL

template <typename T, size_t COUNT>
class DataItemWithPreferences: public DataItem<T, COUNT>
{
	public:
		DataItemWithPreferences( const String name
							   , const T* initialValue
							   , const RxTxType_t rxTxType
							   , const UpdateStoreType_t updateStoreType
							   , const uint16_t rate
							   , IPreferences *preferencesInterface
							   , SerialPortMessageManager *serialPortMessageManager
							   , NamedCallback_t *namedCallback )
							   : DataItem<T, COUNT>( name
							   					   , initialValue
							   					   , rxTxType
							   					   , updateStoreType
							   					   , rate
							   					   , serialPortMessageManager
							   					   , namedCallback )
							   , mp_preferencesInterface(preferencesInterface)
							   , mp_PreferenceManager(nullptr)
		{
		}		   
		DataItemWithPreferences( const String name
							   , const T& initialValue
							   , const RxTxType_t rxTxType
							   , const UpdateStoreType_t updateStoreType
							   , const uint16_t rate
							   , IPreferences *preferencesInterface
							   , SerialPortMessageManager *serialPortMessageManager
							   , NamedCallback_t *namedCallback
							   , SetupCallerInterface *setupCallerInterface )
							   : DataItem<T, COUNT>( name
												   , initialValue
												   , rxTxType
												   , updateStoreType
												   , rate
												   , serialPortMessageManager
												   , namedCallback
												   , setupCallerInterface )
							   , mp_preferencesInterface(preferencesInterface)
							   , mp_PreferenceManager(nullptr)
		{
		}

		DataItemWithPreferences( const String name
							   , const T* initialValue
							   , const RxTxType_t rxTxType
							   , const UpdateStoreType_t updateStoreType
							   , const uint16_t rate
							   , IPreferences *preferencesInterface
							   , SerialPortMessageManager *serialPortMessageManager
							   , NamedCallback_t *namedCallback
							   , SetupCallerInterface *setupCallerInterface
							   , const ValidStringValues_t *validStringValues )
							   : DataItem<T, COUNT>( name
							   					   , initialValue
							   					   , rxTxType
							   					   , updateStoreType
							   					   , rate
							   					   , serialPortMessageManager
							   					   , namedCallback
												   , validStringValues
												   , setupCallerInterface )
							   , mp_preferencesInterface(preferencesInterface)
							   , mp_PreferenceManager(nullptr)
		{
		}
							   
		DataItemWithPreferences( const String name
							   , const T& initialValue
							   , const RxTxType_t rxTxType
							   , const UpdateStoreType_t updateStoreType
							   , const uint16_t rate
							   , IPreferences *preferencesInterface
							   , SerialPortMessageManager *serialPortMessageManager
							   , NamedCallback_t *namedCallback
							   , SetupCallerInterface *setupCallerInterface
							   , const ValidStringValues_t *validStringValues )
							   : DataItem<T, COUNT>( name
												   , initialValue
												   , rxTxType
												   , updateStoreType
												   , rate
												   , serialPortMessageManager
												   , namedCallback
												   , setupCallerInterface
												   , validStringValues )
							   , mp_preferencesInterface(preferencesInterface)
							   , mp_PreferenceManager(nullptr)
		{
		}

		virtual ~DataItemWithPreferences()
		{
			ESP_LOGI("DataItemWithPreferences::~DataItemWithPreferences()", "\"%s\": Freeing Memory", this->m_Name.c_str());
			if(mp_PreferenceManager)
			{
				delete mp_PreferenceManager;
				mp_PreferenceManager = nullptr;
			}
		}

		void Setup()
		{
			DataItem<T, COUNT>::Setup();
			mp_PreferenceManager = new PreferenceManager( mp_preferencesInterface
							   					  		, this->m_Name
							   				 	  		, this->GetInitialValueAsString()
												  		, PREFERENCE_TIMEOUT
							   				 	  		, this->StaticSetValueFromString
							   				 	  		, this );
			mp_PreferenceManager->InitializeAndLoadPreference();
		}
		
		virtual bool SetValueFromString(const String& stringValue) override
		{
			bool result = DataItem<T, COUNT>::SetValueFromString(stringValue);
			if(result)
			{
				mp_PreferenceManager->Update_Preference( PreferenceManager::PreferenceUpdateType::Save
									   				   , this->GetValueAsString() );
			}
			return result;
		}
	protected:

		bool DataItem_TX_Now()
		{
			bool result = DataItem<T, COUNT>::DataItem_TX_Now();
			if(result)
			{
				mp_PreferenceManager->Update_Preference( PreferenceManager::PreferenceUpdateType::Save
									   				   , this->GetValueAsString() );
			}
			return result;
		}

		virtual bool NewRxValueReceived(void* Object, size_t Count) override
		{
			bool result = DataItem<T, COUNT>::NewRxValueReceived(Object, Count);
			if(result)
			{
				mp_PreferenceManager->Update_Preference( PreferenceManager::PreferenceUpdateType::Save
									   				   , this->GetValueAsString() );
			}
			return result;
		}
	private:
		IPreferences *mp_preferencesInterface = nullptr;
		PreferenceManager *mp_PreferenceManager = nullptr;
};