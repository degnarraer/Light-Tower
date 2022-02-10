#ifndef Helpers_H
#define Helpers_H
#include <DataTypes.h>
#include "Streaming.h"


class QueueManager
{
	public:
		QueueManager(String Title): m_Title(Title)
		{
		}
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
		
		void SetupQueueManager(size_t DataItemCount)
		{
			m_DataItemCount = DataItemCount;
			SetupQueueManager();
		}
		void SetupQueueManager()
		{
			Serial << m_Title << "Setup\n";
			if(true == m_MemoryAllocated)FreeMemory();
			AllocateMemory();
		}
		size_t GetQueueByteCountForDataItem(String Name)
		{
			if(NULL != m_DataItem)
			{
				for(int i = 0; i < m_DataItemCount; ++i)
				{
					if(true == Name.equals(m_DataItem[i].Name))
					{
						return m_DataItem[i].TotalByteCount;
					}
				}
				Serial << "GetQueueByteCountForDataItem: " << Name << ": Data Item Not Found\n";
			}
			else
			{
				Serial << "GetQueueByteCountForDataItem: " << Name << ": NULL Data Item\n";
			}
			return NULL;
		}
		QueueHandle_t GetQueueHandleRXForDataItem(String Name)
		{
			if(NULL != m_DataItem)
			{
				for(int i = 0; i < m_DataItemCount; ++i)
				{
					if(true == Name.equals(m_DataItem[i].Name))
					{
						return m_DataItem[i].QueueHandle_RX;
					}
				}
				Serial << "GetQueueHandleRXForDataItem: " << Name << ": Data Item Not Found\n";
			}
			else
			{
				Serial << "GetQueueHandleRXForDataItem: " << Name << ": NULL Data Item\n";
			}
			return NULL;
		}

		QueueHandle_t GetQueueHandleTXForDataItem(String Name)
		{
			if(NULL != m_DataItem)
			{
				for(int i = 0; i < m_DataItemCount; ++i)
				{
					if(true == Name.equals(m_DataItem[i].Name))
					{
						return m_DataItem[i].QueueHandle_TX;
					}
				}
				Serial << "GetQueueHandleTXForDataItem: " << Name << ": Data Item Not Found\n";
			}
			else
			{
				Serial << "GetQueueHandleTXForDataItem: " << Name << ": NULL Data Item\n";
			}
			return NULL;
		}
		
		size_t GetByteCountForDataItem(String Name)
		{
			if(NULL != m_DataItem)
			{
				for(int i = 0; i < m_DataItemCount; ++i)
				{
					if(true == Name.equals(m_DataItem[i].Name))
					{
						return m_DataItem[i].TotalByteCount;
					}
				}
			}
			else
			{
				Serial << "GetByteCountForDataItem: NULL Data Item\n";
			}
			return NULL;
		}
		
		size_t GetCountForDataItem(String Name)
		{
			if(NULL != m_DataItem)
			{
				for(int i = 0; i < m_DataItemCount; ++i)
				{
					if(true == Name.equals(m_DataItem[i].Name))
					{
						return m_DataItem[i].Count;
					}
				}
			}
			else
			{
				Serial << "GetByteCountForDataItem: NULL Data Item\n";
			}
			return NULL;
		}
		
		void* GetDataBufferForDataItem(String Name)
		{
			if(NULL != m_DataItem)
			{
				for(int i = 0; i < m_DataItemCount; ++i)
				{
					if(true == Name.equals(m_DataItem[i].Name))
					{
						return m_DataItem[i].DataBuffer;
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
					if(xQueueSend(Queue, Value, 0) != pdTRUE){Serial.println("Error Setting Queue");} 
				}
			}
			else
			{
				Serial << "PushValueToTXQueue: NULL Queue\n";
			}
		}
		
		bool GetValueFromRXQueue(void* Value, String Name, size_t ByteCount, bool ReadUntilEmpty, bool DebugMessage)
		{
			bool result = false;
			QueueHandle_t Queue = GetQueueHandleRXForDataItem(Name);
			if(NULL != Queue)
			{
				size_t QueueCount = uxQueueMessagesWaiting(Queue);
				if(true == DebugMessage) Serial << "Queue Count: " << QueueCount << "\n";
				if(QueueCount > 0)
				{
					void* DataBuffer = (void*)malloc(ByteCount);
					if(false == ReadUntilEmpty) QueueCount = 1;
					for(int i = 0; i < QueueCount; ++i)
					{
						if ( xQueueReceive(Queue, DataBuffer, 0) == pdTRUE )
						{
							memcpy(Value, DataBuffer, ByteCount);
							result = true;
						}
						else
						{
							Serial << "Error Receiving Queue!\n";
						}
					}
					delete DataBuffer;
				}
			}
			else
			{
				Serial << "GetValueFromRXQueue: NULL Queue\n";
			}
			return result;
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
				void* DataBuffer;
				size_t bytes = 0;
				
				bytes = GetSizeOfDataType(ConfigFile[i].DataType) * ConfigFile[i].Count;
				DataBuffer = malloc(bytes);
				switch(ConfigFile[i].TransceiverConfig)
				{
					case Transciever_None:
					break;
					case Transciever_TX:
						CreateManagedQueue(ConfigFile[i].Name, m_DataItem[i].QueueHandle_TX, bytes, ConfigFile[i].QueueCount, true);
					break;
					case Transciever_RX:
						CreateManagedQueue(ConfigFile[i].Name, m_DataItem[i].QueueHandle_RX, bytes, ConfigFile[i].QueueCount, true);
					break;
					case Transciever_TXRX:
						CreateManagedQueue(ConfigFile[i].Name, m_DataItem[i].QueueHandle_RX, bytes, ConfigFile[i].QueueCount, true);
						CreateManagedQueue(ConfigFile[i].Name, m_DataItem[i].QueueHandle_TX, bytes, ConfigFile[i].QueueCount, true);
					break;
				}
				Serial << m_Title << ": Try Configuring DataItem " << i+1 << " of " << m_DataItemCount << "\n"; 
				m_DataItem[i].Name = ConfigFile[i].Name;
				m_DataItem[i].DataType = ConfigFile[i].DataType;
				m_DataItem[i].Count = ConfigFile[i].Count;
				m_DataItem[i].TotalByteCount = bytes;
				m_DataItem[i].TransceiverConfig = ConfigFile[i].TransceiverConfig;
				m_DataItem[i].DataBuffer = DataBuffer;
				m_MemoryAllocated = true;
			}
		}
		void FreeMemory()
		{
			for(int i = 0; i < m_DataItemCount; ++i)
			{
				delete m_DataItem[i].DataBuffer;
				
				switch(m_DataItem[i].TransceiverConfig)
				{
					case Transciever_None:
					break;
					case Transciever_TX:
						vQueueDelete(m_DataItem[i].QueueHandle_TX);
					break;
					case Transciever_RX:
						vQueueDelete(m_DataItem[i].QueueHandle_RX);
					break;
					case Transciever_TXRX:
						vQueueDelete(m_DataItem[i].QueueHandle_RX);
						vQueueDelete(m_DataItem[i].QueueHandle_TX);
					break;
				}
			}
			delete m_DataItem;
			m_MemoryAllocated = false;
		}
		
		void CreateManagedQueue(String Name, QueueHandle_t &Queue, size_t ByteCount, size_t QueueCount, bool DebugMessage)
		{
			if(true == DebugMessage) Serial << "Creating " << QueueCount << " Queue(s): " << Name << " of size: " << ByteCount << " for a total of " << ByteCount*QueueCount <<"\n";
			Queue = xQueueCreate(QueueCount, ByteCount );
			if(Queue == NULL){Serial.println("Error creating the Queue");}
		}
		size_t GetSizeOfDataType(DataType_t DataType)
		{
			switch(DataType)
			{
				case DataType_Int8_t:
					return sizeof(int8_t);
				break;
				case DataType_Int16_t:
					return sizeof(int16_t);
				break;
				case DataType_Int32_t:
					return sizeof(int32_t);
				break;
				case DataType_Uint8_t:
					return sizeof(uint8_t);
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
				case DataType_MaxBinSoundData_t:
					return sizeof(MaxBinSoundData_t);
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
		template <class T>
		T ScaleWithLimits(T& Input, T Scalar, T LowerLimit, T UpperLimit)
		{
			double Result = (double)Input * (double) Scalar;
			if(Result > UpperLimit)
			{
				Result = UpperLimit;
			}
			else if(Result < LowerLimit)
			{
				Result = LowerLimit;
			}
			return (T)Result;
		}
			
		void MoveDataFromQueueToQueue(String DebugTitle, QueueHandle_t TakeFromQueue, QueueHandle_t GiveToQueue, size_t ByteCount, bool WaitForOpenSlot, bool DebugMessage)
		{
		  if(NULL != TakeFromQueue && NULL != GiveToQueue)
		  {
			size_t QueueCount = uxQueueMessagesWaiting(TakeFromQueue);
			if(true == DebugMessage) Serial << "Queue Messages Waiting: " << QueueCount << " Byte Count: " << ByteCount << "\n";
			uint8_t* DataBuffer = (uint8_t*)malloc(ByteCount);
			for (uint8_t i = 0; i < QueueCount; ++i)
			{
			  memset(DataBuffer, 0, ByteCount);
			  if ( xQueueReceive(TakeFromQueue, DataBuffer, 0) == pdTRUE )
			  {
				if(true == WaitForOpenSlot || uxQueueSpacesAvailable(GiveToQueue) > 0)
				{
					if(xQueueSend(GiveToQueue, DataBuffer, 0) != pdTRUE){Serial.println("Error Setting Queue");}
					if(true == DebugMessage)Serial << "Added Data to Queue\n";
				}
				else
				{
					if(true == DebugMessage)Serial << DebugTitle << ": Queue Full\n";
				}	
			  }
			  else
			  {
				Serial << "Error Receiving Queue!";
			  }
			}
			delete DataBuffer;
		  }
		  else
		  {
		     if(true == DebugMessage)Serial << "MoveDataFromQueueToQueue: " << DebugTitle << " NULL Queue\n";
		  }
		}
		
		void MoveDataFromQueueToQueues(String DebugTitle, QueueHandle_t TakeFromQueue, QueueHandle_t* GiveToQueues, size_t GiveToQueueCount, size_t ByteCount, bool WaitForOpenSlot, bool DebugMessage)
		{
		  if(NULL != TakeFromQueue)
		  {
			size_t QueueCount = uxQueueMessagesWaiting(TakeFromQueue);
			if(true == DebugMessage) Serial << "Queue Messages Waiting: " << QueueCount << " Receiver Queue Count: " << GiveToQueueCount << " Byte Count: " << ByteCount << "\n";
			uint8_t* DataBuffer = (uint8_t*)malloc(ByteCount);
			for (uint8_t i = 0; i < QueueCount; ++i)
			{
				if ( xQueueReceive(TakeFromQueue, DataBuffer, 0) == pdTRUE )
				{
					for(int j = 0; j < GiveToQueueCount; ++j)
					{	
						QueueHandle_t GiveToQueue = GiveToQueues[j];
						if(NULL != GiveToQueue)
						{
							if(true == DebugMessage)Serial << "Adding Data to Queue\n";
							if(true == WaitForOpenSlot || uxQueueSpacesAvailable(GiveToQueue) > 0)
							{
								if(xQueueSend(GiveToQueue, DataBuffer, 0) != pdTRUE){Serial.println("Error Setting Queue");}
							}
							else
							{
								if(true == DebugMessage)Serial << "MoveDataFromQueueToQueues: " << DebugTitle << ": Queue Full\n";
							}
						}
						else
						{
							Serial << "MoveDataFromQueueToQueues: " << DebugTitle << ": NULL Queue\n";
						}
					}						
				}
				else
				{
					Serial << "Error Receiving Queue!";
				}
			}
			delete DataBuffer;
		  }
		  else
		  {
		     if(true == DebugMessage)Serial << "MoveDataFromQueueToQueues: " << DebugTitle << " NULL Queue\n";
		  }
		}

		void CreateQueue(QueueHandle_t &Queue, size_t ByteCount, size_t QueueCount, bool DebugMessage)
		{
			if(true == DebugMessage) Serial << "Creating Queue of size: " << ByteCount << "\n";
			Queue = xQueueCreate(QueueCount, ByteCount );
			if(Queue == NULL){Serial.println("Error creating the Queue");}
		}
		
		void PushValueToQueue(void* Value, QueueHandle_t Queue, bool WaitForOpenSlot, bool DebugMessage)
		{
			if(NULL != Queue)
			{
				if(true == WaitForOpenSlot || uxQueueSpacesAvailable(Queue) > 0)
				{
					if(xQueueSend(Queue, Value, 0) != pdTRUE){Serial.println("Error Setting Queue");}
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
		
		void GetValueFromQueue(void* Value, QueueHandle_t Queue, size_t ByteCount, bool ReadUntilEmpty, bool DebugMessage)
		{
			if(NULL != Queue)
			{
				size_t QueueCount = uxQueueMessagesWaiting(Queue);
				if(true == DebugMessage) Serial << "Queue Count: " << QueueCount << "\n";
				if(QueueCount > 0)
				{
					if(false == ReadUntilEmpty) QueueCount = 1;
					void* DataBuffer = (void*)malloc(ByteCount);
					for(int i = 0; i < QueueCount; ++i)
					{
						if ( xQueueReceive(Queue, DataBuffer, 0) == pdTRUE )
						{
							memcpy(Value, DataBuffer, ByteCount);
						}
						else
						{
							Serial << "Error Receiving Queue!\n";
						}
					}
					delete DataBuffer;
				}
			}
			else
			{
				Serial << "GetValueFromQueue: NULL Queue\n";
			}
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
				case DataType_MaxBinSoundData_t:
					return sizeof(MaxBinSoundData_t);
				break;
				default:
					return 0;
				break;
			}
		}
};

#endif