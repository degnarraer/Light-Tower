
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
#define PREFERENCE_TIMEOUT 5000UL

class LocalStringDataItemWithPreferences: public LocalStringDataItem
							 			, public PreferenceManager
{
	public:
		LocalStringDataItemWithPreferences( const std::string name
					 	   				  , const std::string &initialValue
						   				  , IPreferences *preferencesInterface
					 	   				  , NamedCallback_t *namedCallback
						   				  , SetupCallerInterface *setupCallerInterface )
						   				  : LocalStringDataItem( name, initialValue, namedCallback, setupCallerInterface)
										  , PreferenceManager( preferencesInterface
											  				 , String(this->m_Name.c_str())
											  				 , String(initialValue.c_str())
												  			 , PREFERENCE_TIMEOUT
											  				 , this->StaticSetValueFromString
											  				 , this )
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
			PreferenceManager::InitializeAndLoadPreference();
		}

		virtual bool SetValueFromString(const String& stringValue) override
		{
			assert(stringValue.length() <= DATAITEM_STRING_LENGTH);
			ESP_LOGD("LocalStringDataItemWithPreferences::SetValueFromString"
					, "Name: \"%s\" String Value: \"%s\""
					, m_Name.c_str()
					, stringValue.c_str());
			return SetValue(stringValue.c_str(), stringValue.length());
		}
		
		virtual bool SetValue(const char* value, size_t count) override
		{
			bool result = LocalStringDataItem::SetValue(value, count);
			if(result)
			{
				this->Update_Preference( PreferenceManager::PreferenceUpdateType::Save
									   , GetValueAsString() );
			}
			return result;
		}
};

class StringDataItemWithPreferences: public StringDataItem
								   , public PreferenceManager
{
	public:
		StringDataItemWithPreferences( const std::string name
								     , const std::string &initialValue
								     , const RxTxType_t rxTxType
								     , const uint16_t rate
								     , IPreferences *preferencesInterface
								     , SerialPortMessageManager *serialPortMessageManager
									 , NamedCallback_t *namedCallback
									 , SetupCallerInterface *setupCallerInterface )
								     : StringDataItem( name
													 , initialValue
													 , rxTxType
													 , rate
													 , serialPortMessageManager
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
		
		virtual ~StringDataItemWithPreferences()
		{
			ESP_LOGI("StringDataItemWithPreferences::~StringDataItemWithPreferences()", "\"%s\": Freeing Memory", m_Name.c_str());
		}
	private:
		void Setup()
		{
			StringDataItem::Setup();
			PreferenceManager::InitializeAndLoadPreference();
		}
};