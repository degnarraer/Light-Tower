
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
							 			, public PreferencesManager
{
	public:
		LocalStringDataItemWithPreferences( const String name
					 	   				  , const String &initialValue
						   				  , IPreferences *preferences
					 	   				  , NamedCallback_t *namedCallback
						   				  , SetupCallerInterface *setupCallerInterface )
						   				  : LocalStringDataItem( name, initialValue, namedCallback, setupCallerInterface)
						   				  , PreferencesManager(preferences)
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
			PreferencesManager::InitializeAndLoadPreference( m_Name
																	 			   , GetInitialValueAsString()
																	 			   , this->StaticSetValueFromString
																	 			   , this );
			CreatePreferencesTimer(m_Name, GetValueAsString(), GetInitialValueAsString());
		}

		virtual bool SetValueFromString(const String& stringValue) override
		{
			assert(stringValue.length() <= DATAITEM_STRING_LENGTH);
			ESP_LOGD("LocalStringDataItemWithPreferences::SetValueFromString"
					, "\"%s\": String Value: \"%s\""
					, m_Name.c_str()
					, stringValue.c_str());
			return SetValue(stringValue.c_str(), stringValue.length());
		}
		
		virtual bool SetValue(const char* value, size_t count) override
		{
			bool result = LocalStringDataItem::SetValue(value, count);
			if(result)
			{
				this->Update_Preference( PreferencesManager::PreferenceUpdateType::Save
									   , m_Name
									   , GetValueAsString()
									   , GetInitialValueAsString()
									   , this->StaticSetValueFromString
									   , this );
			}
			return result;
		}
};

class StringDataItemWithPreferences: public StringDataItem
								   , public PreferencesManager
{
	public:
		StringDataItemWithPreferences( const String name
								     , const String &initialValue
								     , const RxTxType_t rxTxType
								     , const UpdateStoreType_t updateStoreType
								     , const uint16_t rate
								     , IPreferences *preferences
								     , SerialPortMessageManager *serialPortMessageManager
									 , NamedCallback_t *namedCallback
									 , SetupCallerInterface *setupCallerInterface )
								     : StringDataItem( name
													 , initialValue
													 , rxTxType
													 , updateStoreType
													 , rate
													 , serialPortMessageManager
													 , namedCallback
													 , setupCallerInterface )
									 , PreferencesManager(preferences)
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
			PreferencesManager::InitializeAndLoadPreference( m_Name
																	 			   , GetInitialValueAsString()
																	 			   , this->StaticSetValueFromString
																	 			   , this );
			CreatePreferencesTimer(m_Name, GetValueAsString(), GetInitialValueAsString());
		}

		bool DataItem_TX_Now()
		{
			bool result = StringDataItem::DataItem_TX_Now();
			if(result)
			{
				this->Update_Preference( PreferenceUpdateType::Save
									   , m_Name
									   , GetValueAsString()
									   , GetInitialValueAsString()
									   , this->StaticSetValueFromString
									   , this );
			}
			return result;
		}

		virtual bool NewRxValueReceived(void* object, size_t count) override
		{
			bool result = StringDataItem::NewRxValueReceived(object, count);
			if(result) 
			{
				this->Update_Preference( PreferenceUpdateType::Save
									   , m_Name
									   , GetValueAsString()
									   , GetInitialValueAsString()
									   , this->StaticSetValueFromString
									   , this );
			}
			return result;
		}
};