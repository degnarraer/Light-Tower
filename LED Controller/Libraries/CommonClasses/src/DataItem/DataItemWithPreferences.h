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
#define PREFERENCE_TIMEOUT 5000UL

template <typename T, size_t COUNT>
class DataItemWithPreferences: public DataItem<T, COUNT>
							 , public PreferenceManager
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
							   , PreferenceManager( preferencesInterface
							   					  , this->m_Name
							   				 	  , this->GetInitialValueAsString()
												  , PREFERENCE_TIMEOUT
							   				 	  , this->StaticSetValueFromString
							   				 	  , this )

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
							   , PreferenceManager( preferencesInterface
							   					  , this->m_Name
							   				 	  , this->GetInitialValueAsString()
												  , PREFERENCE_TIMEOUT
							   				 	  , this->StaticSetValueFromString
							   				 	  , this )
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
							   , PreferenceManager( preferencesInterface
							   					  , this->m_Name
							   				 	  , this->GetInitialValueAsString()
												  , PREFERENCE_TIMEOUT
							   				 	  , this->StaticSetValueFromString
							   				 	  , this )
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
							   , PreferenceManager( preferencesInterface
							   					  , this->m_Name
							   				 	  , this->GetInitialValueAsString()
												  , PREFERENCE_TIMEOUT
							   				 	  , this->StaticSetValueFromString
							   				 	  , this )
		{
		}

		virtual ~DataItemWithPreferences()
		{
			ESP_LOGI("DataItemWithPreferences::~DataItemWithPreferences()", "\"%s\": Freeing Memory", this->m_Name.c_str());
		}

		void Setup()
		{
			DataItem<T, COUNT>::Setup();
			this->InitializeAndLoadPreference();
		}
	protected:

		bool DataItem_TX_Now()
		{
			bool result = DataItem<T, COUNT>::DataItem_TX_Now();
			if(result)
			{
				this->Update_Preference( PreferenceManager::PreferenceUpdateType::Save
									   , this->GetValueAsString() );
			}
			return result;
		}

		virtual bool NewRxValueReceived(void* Object, size_t Count) override
		{
			bool result = DataItem<T, COUNT>::NewRxValueReceived(Object, Count);
			if(result)
			{
				this->Update_Preference( PreferenceManager::PreferenceUpdateType::Save
									   , this->GetValueAsString() );
			}
			return result;
		}
};