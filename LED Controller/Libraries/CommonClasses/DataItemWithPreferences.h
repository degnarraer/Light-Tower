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
		
		virtual void Setup() override
		{
			DataItem<T, COUNT>::Setup();
			this->InitializeNVM(this->GetName().c_str(), this->mp_Value, this->mp_InitialValue);
			this->CreatePreferencesTimer(this->GetName().c_str(), this->mp_Value, this->mp_InitialValue);
		}
		virtual ~DataItemWithPreferences(){}
	protected:
		bool DataItem_TX_Now() override
		{
			bool result = DataItem<T, COUNT>::DataItem_TX_Now();
			if(result)
			{
				this->Update_Preference("Updated", this->GetName().c_str(), this->mp_Value, this->mp_InitialValue);
			}
			return result;
		}
		virtual bool NewRXValueReceived(void* Object, size_t Count) override
		{
			bool result = DataItem<T, COUNT>::NewRXValueReceived(Object, Count);
			if(result)
			{
				this->Update_Preference("Updated", this->GetName().c_str(), this->mp_Value, this->mp_InitialValue);
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
		
		virtual ~StringDataItemWithPreferences(){}
	private:
		virtual void Setup() override
		{
			StringDataItem::Setup();
			this->InitializeNVM(this->GetName().c_str(), this->mp_Value, this->mp_InitialValue);
			this->CreatePreferencesTimer(this->GetName().c_str(), this->mp_Value, this->mp_InitialValue);
		}

		virtual bool DataItem_TX_Now() override
		{
			bool result = StringDataItem::DataItem_TX_Now();
			if(result)
			{
				this->Update_Preference("Updated", this->GetName().c_str(), this->mp_Value, this->mp_InitialValue);
			}
			return result;
		}

		virtual bool NewRXValueReceived(void* Object, size_t Count) override
		{
			bool result = StringDataItem::NewRXValueReceived(Object, Count);
			if(result) 
			{
				this->Update_Preference("Updated", this->GetName().c_str(), this->mp_Value, this->mp_InitialValue);
			}
			return result;
		}
};