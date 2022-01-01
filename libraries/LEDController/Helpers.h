#ifndef Helpers_H
#define Helpers_H
#include <DataTypes.h>
#include "Streaming.h"


class QueueManager
{
	public:
		QueueManager(String Title, size_t DataItemCount): m_Title(Title)
													    , m_DataItemCount(DataItemCount)
		{
		}
		virtual ~QueueManager()
		{
			if(true == m_MemoryAllocated)FreeMemory();
		}
		virtual DataItemConfig_t* GetDataItemConfig() = 0;
		virtual size_t GetDataItemConfigCount() = 0;
		
		DataItem_t& GetQueueManagerDataItems() { return *m_DataItem; }
		size_t GetQueueManagerDataItemCount() { return m_DataItemCount; }
		
		void SetupQueueManager()
		{
			Serial << m_Title << "Setup\n";
			if(true == m_MemoryAllocated)FreeMemory();
			AllocateMemory();
		}
		QueueHandle_t GetQueueHandleRXForDataItem(String Name)
		{
			if(NULL != m_DataItem)
			{
				for(int i = 0; i < m_DataItemCount; ++i)
				{
					if(Name == m_DataItem[i].Name)
					{
						return m_DataItem[i].QueueHandle_RX;
					}
				}
				Serial << "GetQueueHandleRXForDataItem: Data Item Not Found\n";
			}
			else
			{
				Serial << "GetQueueHandleRXForDataItem: NULL Data Item\n";
			}
			return NULL;
		}

		QueueHandle_t GetQueueHandleTXForDataItem(String Name)
		{
			if(NULL != m_DataItem)
			{
				for(int i = 0; i < m_DataItemCount; ++i)
				{
					if(Name == m_DataItem[i].Name)
					{
						return m_DataItem[i].QueueHandle_TX;
					}
				}
			}
			else
			{
				Serial << "GetQueueHandleTXForDataItem: NULL Data Item\n";
			}
			return NULL;
		}
		
		size_t GetByteCountForDataItem(String Name)
		{
			if(NULL != m_DataItem)
			{
				for(int i = 0; i < m_DataItemCount; ++i)
				{
					if(Name == m_DataItem[i].Name)
					{
						switch(m_DataItem[i].DataType)
						{
							case DataType_Int16_t:
								return sizeof(int16_t) * m_DataItem[i].Count;
							break;
							case DataType_Int32_t:
								return sizeof(int32_t) * m_DataItem[i].Count;
							break;
							case DataType_Uint16_t:
								return sizeof(uint16_t) * m_DataItem[i].Count;
							break;
							case DataType_Uint32_t:
								return sizeof(uint32_t) * m_DataItem[i].Count;
							break;
							case DataType_String:
								return sizeof(String) * m_DataItem[i].Count;
							break;
							case DataType_Float:
								return sizeof(float) * m_DataItem[i].Count;
							break;
							case DataType_ProcessedSoundData_t:
								return sizeof(ProcessedSoundData_t) * m_DataItem[i].Count;
							break;
							default:
								return 0;
							break;
						}
					}
				}
			}
			else
			{
				Serial << "GetByteCountForDataItem: NULL Data Item\n";
			}
			return NULL;
		}
		
		void PushValueToTXQueue(void* Value, String Name, bool WaitForOpenSlot)
		{
			QueueHandle_t Queue = GetQueueHandleTXForDataItem(Name);
			if(NULL != Queue)
			{
				if(uxQueueSpacesAvailable(Queue) > 0 || true == WaitForOpenSlot)
				{
					if(xQueueSend(Queue, Value, portMAX_DELAY) != pdTRUE){Serial.println("Error Setting Queue");} 
				}
			}
			else
			{
				Serial << "PushValueToTXQueue: NULL Queue\n";
			}
		}
		
		bool GetValueFromRXQueue(void* Value, String Name, size_t ByteCount, bool ReadUntilEmpty, bool DebugMessage)
		{
			QueueHandle_t Queue = GetQueueHandleRXForDataItem(Name);
			if(NULL != Queue)
			{
				size_t QueueCount = uxQueueMessagesWaiting(Queue);
				if(true == DebugMessage) Serial << "Queue Count: " << QueueCount << "\n";
				if(QueueCount > 0)
				{
					if(false == ReadUntilEmpty) QueueCount = 1;
					for(int i = 0; i < QueueCount; ++i)
					{
						void* DataBuffer = (void*)malloc(ByteCount);
						if ( xQueueReceive(Queue, DataBuffer, portMAX_DELAY) == pdTRUE )
						{
							memcpy(Value, DataBuffer, ByteCount);
							return true;
						}
						else
						{
							Serial << "Error Receiving Queue!\n";
							delete DataBuffer;
							return false;
						}
					}
				}
			}
			else
			{
				Serial << "GetValueFromRXQueue: NULL Queue\n";
			}
			return false;
		}
		
	private:
		DataItem_t* m_DataItem;
		size_t m_DataItemCount;
		String m_Title = "";
		bool m_MemoryAllocated = false;
		void AllocateMemory()
		{
			size_t ConfigBytes = sizeof(DataItem_t) * m_DataItemCount;
			Serial << m_Title << ": Allocating " << m_DataItemCount << " DataItem's for a total of " << ConfigBytes << " bytes of Memory\n";
			m_DataItemCount = GetDataItemConfigCount();
			DataItemConfig_t* ConfigFile = GetDataItemConfig();
			m_DataItem = new DataItem_t[m_DataItemCount];
			for(int i = 0; i < m_DataItemCount; ++i)
			{
				void* Object;
				size_t bytes = 0;
				
				bytes = GetSizeOfDataType(ConfigFile[i].DataType) * ConfigFile[i].Count;
				Object = malloc(bytes);
				CreateManagedQueue(ConfigFile[i].Name, m_DataItem[i].QueueHandle_RX, bytes, 10, true);
				CreateManagedQueue(ConfigFile[i].Name, m_DataItem[i].QueueHandle_TX, bytes, 10, true);
				Serial << m_Title << ": Try Configuring DataItem " << i+1 << " of " << m_DataItemCount << "\n"; 
				m_DataItem[i].Name = ConfigFile[i].Name;
				m_DataItem[i].DataType = ConfigFile[i].DataType;
				m_DataItem[i].Count = ConfigFile[i].Count;
				m_DataItem[i].QueueByteCount = bytes;
				m_DataItem[i].TransceiverConfig = ConfigFile[i].TransceiverConfig;
				m_DataItem[i].Object = Object;
				m_MemoryAllocated = true;
			}
		}
		void FreeMemory()
		{
			for(int i = 0; i < m_DataItemCount; ++i)
			{
				delete m_DataItem[i].Object;
			}
			delete m_DataItem;
			m_MemoryAllocated = false;
		}
		
		void CreateManagedQueue(String Name, QueueHandle_t &Queue, size_t ByteCount, size_t QueueCount, bool DebugMessage)
		{
			if(true == DebugMessage) Serial << "Creating Queue: " << Name << " of size: " << ByteCount << "\n";
			Queue = xQueueCreate(QueueCount, ByteCount );
			if(Queue == NULL){Serial.println("Error creating the Queue");}
		}
		size_t GetSizeOfDataType(DataType_t DataType)
		{
			switch(DataType)
			{
				case DataType_Int16_t:
					return sizeof(int16_t);
				break;
				case DataType_Int32_t:
					return sizeof(int32_t);
				break;
				case DataType_Uint16_t:
					return sizeof(uint16_t);
				break;
				case DataType_Uint32_t:
					return sizeof(uint32_t);
				break;
				case DataType_String:
					return sizeof(String);
				break;
				case DataType_Float:
					return sizeof(float);
				break;
				case DataType_ProcessedSoundData_t:
					return sizeof(ProcessedSoundData_t);
				break;
				default:
					return 0;
				break;
			}
		}
};

class CommonUtils
{
	public:
		void MoveDataFromQueueToQueue(String DebugTitle, QueueHandle_t TakeFromQueue, QueueHandle_t GiveToQueue, size_t ByteCount, bool WaitForOpenSlot, bool DebugMessage)
		{
		  if(NULL != TakeFromQueue && NULL != GiveToQueue)
		  {
			size_t QueueCount = uxQueueMessagesWaiting(TakeFromQueue);
			if(true == DebugMessage) Serial << "Queue Messages Waiting: " << QueueCount << " Byte Count: " << ByteCount << "\n";
			for (uint8_t i = 0; i < QueueCount; ++i)
			{
			  uint8_t* DataBuffer = (uint8_t*)malloc(ByteCount);
			  if ( xQueueReceive(TakeFromQueue, DataBuffer, portMAX_DELAY) == pdTRUE )
			  {
				if(true == WaitForOpenSlot || uxQueueSpacesAvailable(GiveToQueue) > 0)
				{
					if(xQueueSend(GiveToQueue, DataBuffer, portMAX_DELAY) != pdTRUE){Serial.println("Error Setting Queue");}
					if(true == DebugMessage)Serial << "Added Data to Queue\n";
				}
				else
				{
					if(true == DebugMessage)Serial << "Queue Full\n";
				}	
			  }
			  else
			  {
				Serial << "Error Receiving Queue!";
			  }
			  delete DataBuffer;
			}
		  }
		  else
		  {
		     Serial << "MoveDataFromQueueToQueue: " << DebugTitle << " NULL Queue\n";
		  }
		}
		
		void MoveDataFromQueueToQueues(String DebugTitle, QueueHandle_t TakeFromQueue, QueueHandle_t* GiveToQueues, size_t GiveToQueueCount, size_t ByteCount, bool WaitForOpenSlot, bool DebugMessage)
		{
		  if(NULL != TakeFromQueue)
		  {
			size_t QueueCount = uxQueueMessagesWaiting(TakeFromQueue);
			if(true == DebugMessage) Serial << "Queue Messages Waiting: " << QueueCount << " Receiver Queue Count: " << GiveToQueueCount << " Byte Count: " << ByteCount << "\n";
			for (uint8_t i = 0; i < QueueCount; ++i)
			{
				uint8_t* DataBuffer = (uint8_t*)malloc(ByteCount);
				if ( xQueueReceive(TakeFromQueue, DataBuffer, portMAX_DELAY) == pdTRUE )
				{
					for(int j = 0; j < GiveToQueueCount; ++j)
					{	
						QueueHandle_t GiveToQueue = GiveToQueues[j];
						if(NULL != GiveToQueue)
						{
							if(true == DebugMessage)Serial << "Adding Data to Queue\n";
							if(true == WaitForOpenSlot || uxQueueSpacesAvailable(GiveToQueue) > 0)
							{
								if(xQueueSend(GiveToQueue, DataBuffer, portMAX_DELAY) != pdTRUE){Serial.println("Error Setting Queue");}
							}
						}
						else
						{
							Serial << "MoveDataFromQueueToQueues: " << DebugTitle << " NULL Queue\n";
						}
					}						
				}
				else
				{
					Serial << "Error Receiving Queue!";
				}
				delete DataBuffer;
			}
		  }
		  else
		  {
		     Serial << "MoveDataFromQueueToQueues: " << DebugTitle << " NULL Queue\n";
		  }
		}

		void CreateQueue(QueueHandle_t &Queue, size_t ByteCount, size_t QueueCount, bool DebugMessage)
		{
			if(true == DebugMessage) Serial << "Creating Queue with Queue Count: " << QueueCount << " Byte Count: " << ByteCount << "\n";
			Queue = xQueueCreate(QueueCount, ByteCount );
			if(Queue == NULL){Serial.println("Error creating the Queue");}
		}
		
		void PushValueToQueue(void* Value, QueueHandle_t Queue, bool WaitForOpenSlot, bool DebugMessage)
		{
			if(NULL != Queue)
			{
				if(true == WaitForOpenSlot || uxQueueSpacesAvailable(Queue) > 0)
				{
					if(xQueueSend(Queue, Value, portMAX_DELAY) != pdTRUE){Serial.println("Error Setting Queue");}
					else{ if(true == DebugMessage)Serial << "Value Pushed to Queue\n"; }
				}
				else
				{
					if(true == DebugMessage)Serial << "Queue Full\n";
				}
			}
			else
			{
				Serial << "PushValueToQueue: NULL Queue\n";
			}
		}
		
		bool GetValueFromQueue(void* Value, QueueHandle_t Queue, size_t ByteCount, bool ReadUntilEmpty, bool DebugMessage)
		{
			if(NULL != Queue)
			{
				size_t QueueCount = uxQueueMessagesWaiting(Queue);
				if(true == DebugMessage) Serial << "Queue Count: " << QueueCount << "\n";
				if(QueueCount > 0)
				{
					if(false == ReadUntilEmpty) QueueCount = 1;
					for(int i = 0; i < QueueCount; ++i)
					{
						void* DataBuffer = (void*)malloc(ByteCount);
						if ( xQueueReceive(Queue, DataBuffer, portMAX_DELAY) == pdTRUE )
						{
							memcpy(Value, DataBuffer, ByteCount);
							delete DataBuffer;
							return true;
						}
						else
						{
							Serial << "Error Receiving Queue!\n";
							delete DataBuffer;
							return false;
						}
					}
				}
			}
			else
			{
				Serial << "GetValueFromQueue: NULL Queue\n";
			}
			return false;
		}
		
		DataType_t GetDataTypeFromString(String DataType)
		{
			for(int i = 0; i < sizeof(DataTypeStrings) / sizeof(DataTypeStrings[0]); ++i)
			{
				if(DataType.equals(DataTypeStrings[i]))return (DataType_t)i;
			}
			return DataType_Undef;
		}
		size_t GetSizeOfDataType(DataType_t DataType)
		{
			switch(DataType)
			{
				case DataType_Int16_t:
					return sizeof(int16_t);
				break;
				case DataType_Int32_t:
					return sizeof(int32_t);
				break;
				case DataType_Uint16_t:
					return sizeof(uint16_t);
				break;
				case DataType_Uint32_t:
					return sizeof(uint32_t);
				break;
				case DataType_String:
					return sizeof(String);
				break;
				case DataType_Float:
					return sizeof(float);
				break;
				case DataType_ProcessedSoundData_t:
					return sizeof(ProcessedSoundData_t);
				break;
				default:
					return 0;
				break;
			}
		}
};

#endif