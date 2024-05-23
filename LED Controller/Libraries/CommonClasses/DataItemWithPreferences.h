#pragma once
#include "DataItem.h"
#include "PreferencesWrapper.h"

template <typename T, size_t COUNT>
class DataItemWithPreferences: public DataItem<T, COUNT>
							 , public PreferencesWrapper<T, COUNT>
{
	public:
		DataItemWithPreferences( const String name
							   , const T* initialValue
							   , const RxTxType_t rxTxType
							   , const UpdateStoreType_t updateStoreType
							   , const uint16_t rate
							   , Preferences *preferences
							   , SerialPortMessageManager &serialPortMessageManager
							   , NamedCallback_t *namedCallback )
							   : DataItem<T, COUNT>( name
							   					   , initialValue
							   					   , rxTxType
							   					   , updateStoreType
							   					   , rate
							   					   , serialPortMessageManager
							   					   , namedCallback )
							   , PreferencesWrapper<T, COUNT>(preferences)

		{
		}
							   
		DataItemWithPreferences( const String name
							   , const T& initialValue
							   , const RxTxType_t rxTxType
							   , const UpdateStoreType_t updateStoreType
							   , const uint16_t rate
							   , Preferences *preferences
							   , SerialPortMessageManager &serialPortMessageManager
							   , NamedCallback_t *namedCallback )
							   : DataItem<T, COUNT>( name
												   , initialValue
												   , rxTxType
												   , updateStoreType
												   , rate
												   , serialPortMessageManager
												   , namedCallback )
							   , PreferencesWrapper<T, COUNT>(preferences)
		{
		}
		virtual ~DataItemWithPreferences()
		{
			ESP_LOGI("DataItemWithPreferences::~DataItemWithPreferences()", "\"%s\": Freeing Memory", this->GetName().c_str());
		}
		virtual void Setup() override
		{
			DataItem<T, COUNT>::Setup();
			this->InitializeNVM( this->GetName().c_str()
							   , this->GetInitialValueAsString().c_str()
							   , NULL );
			this->CreatePreferencesTimer(this->GetName().c_str(), this->GetValueAsString().c_str(), this->GetInitialValueAsString().c_str());
		}
	protected:
		bool DataItem_TX_Now() override
		{
			bool result = DataItem<T, COUNT>::DataItem_TX_Now();
			if(result)
			{
				this->Update_Preference( PreferencesWrapper<T, COUNT>::PreferenceUpdateType::Save
									   , this->GetName().c_str()
									   , this->GetValueAsString().c_str()
									   , this->GetInitialValueAsString().c_str()
									   , NULL );
			}
			return result;
		}
		virtual bool NewRXValueReceived(void* Object, size_t Count) override
		{
			bool result = DataItem<T, COUNT>::NewRXValueReceived(Object, Count);
			if(result)
			{
				this->Update_Preference( PreferencesWrapper<T, COUNT>::PreferenceUpdateType::Save
									   , LocalDataItem<T, COUNT>::GetName().c_str()
									   , LocalDataItem<T, COUNT>::GetValueAsString().c_str()
									   , LocalDataItem<T, COUNT>::GetInitialValueAsString().c_str()
									   , NULL );
			}
			return result;
		}
};

class LocalStringDataItemWithPreferences: public LocalStringDataItem
							 			, public PreferencesWrapper<char, DATAITEM_STRING_LENGTH>
{
	public:
		LocalStringDataItemWithPreferences( const String name
					 	   , const char* initialValue
						   , Preferences *preferences
						   , SetupCallerInterface *setupCallerInterface
					 	   , NamedCallback_t *namedCallback )
						   : LocalStringDataItem( name, initialValue, namedCallback)
						   , PreferencesWrapper<char, DATAITEM_STRING_LENGTH>(preferences)
		{
			setupCallerInterface->RegisterForSetupCall(this);
		}
		
		LocalStringDataItemWithPreferences( const String name
										  , const char& initialValue
						   				  , Preferences *preferences
						   				  , SetupCallerInterface *setupCallerInterface
					 	   				  , NamedCallback_t *namedCallback )
						   				  : LocalStringDataItem( name, initialValue, namedCallback)
						   				  , PreferencesWrapper<char, DATAITEM_STRING_LENGTH>(preferences)
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
			PreferencesWrapper<char, DATAITEM_STRING_LENGTH>::InitializeNVM( this->GetName().c_str()
							   											   , this->GetInitialValueAsString().c_str()
							   											   , NULL );
			this->CreatePreferencesTimer(this->GetName().c_str(), this->GetValueAsString().c_str(), this->GetInitialValueAsString().c_str());
		}
		
		bool SetValue(const char* Value, size_t Count)
		{
			bool result = LocalStringDataItem::SetValue(Value, Count);
			if(result)
			{
				this->Update_Preference( PreferencesWrapper<char, DATAITEM_STRING_LENGTH>::PreferenceUpdateType::Save
									   , this->GetName().c_str()
									   , this->GetValueAsString()
									   , this->GetInitialValueAsString().c_str()
									   , NULL );
			}
			return result;
		}
};

class StringDataItemWithPreferences: public PreferencesWrapper<char, DATAITEM_STRING_LENGTH>
								   , public StringDataItem
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
								     : PreferencesWrapper<char, DATAITEM_STRING_LENGTH>(preferences)
									 , StringDataItem( name
													 , initialValue
													 , rxTxType
													 , updateStoreType
													 , rate
													 , serialPortMessageManager
													 , namedCallback )
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
								     : PreferencesWrapper<char, DATAITEM_STRING_LENGTH>(preferences)
									 , StringDataItem( name
													 , initialValue
													 , rxTxType
													 , updateStoreType
													 , rate
													 , serialPortMessageManager
													 , namedCallback )		
		{
			
		}
		
		virtual ~StringDataItemWithPreferences()
		{
			ESP_LOGI("StringDataItemWithPreferences::~StringDataItemWithPreferences()", "\"%s\": Freeing Memory", m_Name.c_str());
		}
	private:
		virtual void Setup() override
		{
			StringDataItem::Setup();
			PreferencesWrapper<char, DATAITEM_STRING_LENGTH>::InitializeNVM( this->GetName().c_str()
																		   , this->GetInitialValueAsString().c_str()
																		   , NULL );
			this->CreatePreferencesTimer(this->GetName().c_str(), this->GetValueAsString().c_str(), this->GetInitialValueAsString().c_str());
		}

		virtual bool DataItem_TX_Now() override
		{
			bool result = StringDataItem::DataItem_TX_Now();
			if(result)
			{
				this->Update_Preference( PreferenceUpdateType::Save
									   , this->GetName().c_str()
									   , this->GetValueAsString().c_str()
									   , this->GetInitialValueAsString().c_str()
									   , NULL );
			}
			return result;
		}

		virtual bool NewRXValueReceived(void* Object, size_t Count) override
		{
			bool result = StringDataItem::NewRXValueReceived(Object, Count);
			if(result) 
			{
				this->Update_Preference( PreferenceUpdateType::Save
									   , this->GetName().c_str()
									   , this->GetValueAsString().c_str()
									   , this->GetInitialValueAsString().c_str()
									   , NULL );
			}
			return result;
		}
};