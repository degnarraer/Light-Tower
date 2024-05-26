
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

#include "DataItem/LocalStringDataItem.h"
#include "DataItem/StringDataItem.h"
#include "DataItem/PreferencesWrapper.h"

class LocalStringDataItemWithPreferences: public LocalStringDataItem
							 			, public PreferencesWrapper<DATAITEM_STRING_LENGTH>
{
	public:
		LocalStringDataItemWithPreferences( const String name
					 	   , const char* initialValue
						   , Preferences *preferences
						   , SetupCallerInterface *setupCallerInterface
					 	   , NamedCallback_t *namedCallback )
						   : LocalStringDataItem( name, initialValue, namedCallback)
						   , PreferencesWrapper<DATAITEM_STRING_LENGTH>(preferences)
		{
			setupCallerInterface->RegisterForSetupCall(this);
		}
		
		LocalStringDataItemWithPreferences( const String name
										  , const char& initialValue
						   				  , Preferences *preferences
						   				  , SetupCallerInterface *setupCallerInterface
					 	   				  , NamedCallback_t *namedCallback )
						   				  : LocalStringDataItem( name, initialValue, namedCallback)
						   				  , PreferencesWrapper<DATAITEM_STRING_LENGTH>(preferences)
		{
			setupCallerInterface->RegisterForSetupCall(this);
		}

		virtual ~LocalStringDataItemWithPreferences()
		{
			ESP_LOGI("LocalStringDataItemWithPreferences::~LocalStringDataItemWithPreferences()", "\"%s\": Freeing Memory", m_Name.c_str());
		}

		virtual void Setup() override
		{
			LocalStringDataItem::Setup();
			PreferencesWrapper<DATAITEM_STRING_LENGTH>::InitializeNVM( m_Name.c_str()
							   											   , GetInitialValueAsString().c_str()
							   											   , NULL );
			CreatePreferencesTimer(m_Name.c_str(), GetValueAsString().c_str(), GetInitialValueAsString().c_str());
		}
		
		bool SetValue(const char* Value, size_t Count)
		{
			bool result = LocalStringDataItem::SetValue(Value, Count);
			if(result)
			{
				this->Update_Preference( PreferencesWrapper<DATAITEM_STRING_LENGTH>::PreferenceUpdateType::Save
									   , m_Name.c_str()
									   , GetValueAsString()
									   , GetInitialValueAsString().c_str()
									   , NULL );
			}
			return result;
		}
};

class StringDataItemWithPreferences: public StringDataItem
								   , public PreferencesWrapper<DATAITEM_STRING_LENGTH>
{
	public:
		StringDataItemWithPreferences( const String name
								     , const char* initialValue
								     , const RxTxType_t rxTxType
								     , const UpdateStoreType_t updateStoreType
								     , const uint16_t rate
								     , Preferences *preferences
								     , SerialPortMessageManager &serialPortMessageManager
									 , NamedCallback_t *namedCallback )
								     : StringDataItem( name
													 , initialValue
													 , rxTxType
													 , updateStoreType
													 , rate
													 , serialPortMessageManager
													 , namedCallback )
									 , PreferencesWrapper<DATAITEM_STRING_LENGTH>(preferences)
		{
			
		}
		StringDataItemWithPreferences( const String name
								     , const char& initialValue
								     , const RxTxType_t rxTxType
								     , const UpdateStoreType_t updateStoreType
								     , const uint16_t rate
								     , Preferences *preferences
								     , SerialPortMessageManager &serialPortMessageManager 
									 , NamedCallback_t *namedCallback)
									 : StringDataItem( name
													 , initialValue
													 , rxTxType
													 , updateStoreType
													 , rate
													 , serialPortMessageManager
													 , namedCallback )
									 , PreferencesWrapper<DATAITEM_STRING_LENGTH>(preferences)	
		{
			
		}
		
		virtual ~StringDataItemWithPreferences()
		{
			ESP_LOGI("StringDataItemWithPreferences::~StringDataItemWithPreferences()", "\"%s\": Freeing Memory", m_Name.c_str());
		}
	private:
		void Setup()
		{
			StringDataItem::Setup();
			PreferencesWrapper<DATAITEM_STRING_LENGTH>::InitializeNVM( m_Name.c_str()
																	 , GetInitialValueAsString().c_str()
																	 , NULL );
			CreatePreferencesTimer(m_Name.c_str(), GetValueAsString().c_str(), GetInitialValueAsString().c_str());
		}

		bool DataItem_TX_Now()
		{
			bool result = StringDataItem::DataItem_TX_Now();
			if(result)
			{
				this->Update_Preference( PreferenceUpdateType::Save
									   , m_Name.c_str()
									   , GetValueAsString().c_str()
									   , GetInitialValueAsString().c_str()
									   , NULL );
			}
			return result;
		}

		bool NewRXValueReceived(void* Object, size_t Count)
		{
			bool result = StringDataItem::NewRXValueReceived(Object, Count);
			if(result) 
			{
				this->Update_Preference( PreferenceUpdateType::Save
									   , m_Name.c_str()
									   , GetValueAsString().c_str()
									   , GetInitialValueAsString().c_str()
									   , NULL );
			}
			return result;
		}
};