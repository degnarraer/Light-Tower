#ifndef Helpers_H
#define Helpers_H
#include <DataTypes.h>
#include "Streaming.h"


class QueueManager
{
	public:
		QueueManager(String Title): m_Title(Title)
		{
			ESP_LOGV("Function Debug", "%s, ", __func__);
		}
		QueueManager(String Title, size_t DataItemCount): m_Title(Title)
													    , m_DataItemCount(DataItemCount)
		{
			ESP_LOGV("Function Debug", "%s, ", __func__);
		}
		virtual ~QueueManager()
		{
			ESP_LOGV("Function Debug", "%s, ", __func__);
			if(true == m_MemoryAllocated)FreeMemory();
		}
		virtual DataItemConfig_t* GetDataItemConfig() = 0;
		virtual size_t GetDataItemConfigCount() = 0;
		
		DataItem_t& GetQueueManagerDataItems() { return *m_DataItem; }
		size_t GetQueueManagerDataItemCount() { return m_DataItemCount; }
		
		void SetupQueueManager(size_t DataItemCount)
		{
			ESP_LOGV("Function Debug", "%s, ", __func__);
			m_DataItemCount = DataItemCount;
			SetupQueueManager();
		}
		void SetupQueueManager()
		{
			ESP_LOGV("Function Debug", "%s, ", __func__);
			ESP_LOGD("CommonUtils", "%s: Setup", m_Title);
			if(true == m_MemoryAllocated)FreeMemory();
			AllocateMemory();
		}
		size_t GetQueueByteCountForDataItem(String Name)
		{
			ESP_LOGV("Function Debug", "%s, ", __func__);
			if(NULL != m_DataItem)
			{
				for(int i = 0; i < m_DataItemCount; ++i)
				{
					if(true == Name.equals(m_DataItem[i].Name))
					{
						return m_DataItem[i].TotalByteCount;
					}
				}
				ESP_LOGW("CommonUtils", "WARNING! %s: Data Item Not Found.", Name);
			}
			else
			{
				ESP_LOGW("CommonUtils", "WARNING! %s: NULL Data Item.", Name);
			}
			return NULL;
		}
		QueueHandle_t GetQueueHandleRXForDataItem(String Name)
		{
			ESP_LOGV("Function Debug", "%s, ", __func__);
			if(NULL != m_DataItem)
			{
				for(int i = 0; i < m_DataItemCount; ++i)
				{
					if(true == Name.equals(m_DataItem[i].Name))
					{
						return m_DataItem[i].QueueHandle_RX;
					}
				}
				ESP_LOGW("CommonUtils", "WARNING! %s: Data Item Not Found.", Name);
			}
			else
			{
				ESP_LOGW("CommonUtils", "WARNING! %s: NULL Data Item.", Name);
			}
			return NULL;
		}

		QueueHandle_t GetQueueHandleTXForDataItem(String Name)
		{
			ESP_LOGV("Function Debug", "%s, ", __func__);
			if(NULL != m_DataItem)
			{
				for(int i = 0; i < m_DataItemCount; ++i)
				{
					if(true == Name.equals(m_DataItem[i].Name))
					{
						return m_DataItem[i].QueueHandle_TX;
					}
				}
				ESP_LOGW("CommonUtils", "WARNING! %s: Data Item Not Found.", Name);
			}
			else
			{
				ESP_LOGW("CommonUtils", "WARNING! %s: NULL Data Item.", Name);
			}
			return NULL;
		}
		
		size_t GetByteCountForDataItem(String Name)
		{
			ESP_LOGV("Function Debug", "%s, ", __func__);
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
				ESP_LOGW("CommonUtils", "WARNING! NULL Data Item.");
			}
			return NULL;
		}
		
		size_t GetSampleCountForDataItem(String Name)
		{
			ESP_LOGV("Function Debug", "%s, ", __func__);
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
				ESP_LOGW("CommonUtils", "WARNING! NULL Data Item.");
			}
			return NULL;
		}
		
		void* GetDataBufferForDataItem(String Name)
		{
			ESP_LOGV("Function Debug", "%s, ", __func__);
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
				ESP_LOGW("CommonUtils", "WARNING! NULL Data Item.");
			}
			return NULL;
		}
		
		void PushValueToTXQueue(void* Value, String Name, bool WaitForOpenSlot)
		{
			ESP_LOGV("Function Debug", "%s, ", __func__);
			QueueHandle_t Queue = GetQueueHandleTXForDataItem(Name);
			if(NULL != Queue)
			{
				if(uxQueueSpacesAvailable(Queue) > 0 || true == WaitForOpenSlot)
				{
					if(xQueueSend(Queue, Value, portMAX_DELAY) != pdTRUE)
					{
						ESP_LOGE("CommonUtils", "Error! Error Setting Queue.");
					} 
				}
			}
			else
			{
				ESP_LOGE("CommonUtils", "ERROR! NULL Queue.");
			}
		}
		
		bool GetValueFromRXQueue(void* Value, String Name, size_t ByteCount, bool ReadUntilEmpty, bool DebugMessage)
		{
			ESP_LOGV("Function Debug", "%s, ", __func__);
			bool result = false;
			QueueHandle_t Queue = GetQueueHandleRXForDataItem(Name);
			if(NULL != Queue)
			{
				size_t QueueCount = uxQueueMessagesWaiting(Queue);
				ESP_LOGV("CommonUtils", "Queue Count: %i", QueueCount);
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
							ESP_LOGV("CommonUtils", "Error Receiving Queue!");
						}
					}
					delete DataBuffer;
				}
			}
			else
			{
				ESP_LOGE("CommonUtils", "ERROR! NULL Queue.");
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
			ESP_LOGV("Function Debug", "%s, ", __func__);
			size_t ConfigBytes = sizeof(DataItem_t) * m_DataItemCount;
			ESP_LOGI("CommonUtils", "%s: Allocating %i DataItem's for a total of %i bytes of Memory", m_Title, m_DataItemCount, ConfigBytes);
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
				ESP_LOGI("CommonUtils", "%s: Try Configuring DataItem %i of %i", m_Title, i+1, m_DataItemCount);
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
			ESP_LOGV("Function Debug", "%s, ", __func__);
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
			ESP_LOGV("Function Debug", "%s, ", __func__);
			ESP_LOGV("Helpers", "Creating %i Queue(s), Named: %s of size: %i for a total of %i", QueueCount, Name, ByteCount, ByteCount*QueueCount);
			Queue = xQueueCreate(QueueCount, ByteCount );
			if(Queue == NULL)
			{
				ESP_LOGE("Helpers", "ERROR! Error creating the Queue.");
			}
		}
		size_t GetSizeOfDataType(DataType_t DataType)
		{
			ESP_LOGV("Function Debug", "%s, ", __func__);
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
				case DataType_Double:
					return sizeof(double);
				break;
				case DataType_ProcessedSoundData_t:
					return sizeof(ProcessedSoundData_t);
				break;
				case DataType_MaxBandSoundData_t:
					return sizeof(MaxBandSoundData_t);
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
			ESP_LOGV("Function Debug", "%s, ", __func__);
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
		  ESP_LOGV("Function Debug", "%s, ", __func__);
		  if(NULL != TakeFromQueue && NULL != GiveToQueue)
		  {
			size_t QueueCount = uxQueueMessagesWaiting(TakeFromQueue);
			ESP_LOGV("Helpers", "MoveDataFromQueueToQueue: Queue Messages Waiting: %i Byte Count: %i", QueueCount, ByteCount);
			uint8_t* DataBuffer = (uint8_t*)malloc(ByteCount);
			for (uint8_t i = 0; i < QueueCount; ++i)
			{
			  memset(DataBuffer, 0, ByteCount);
			  if ( xQueueReceive(TakeFromQueue, DataBuffer, 0) == pdTRUE )
			  {
				if(uxQueueSpacesAvailable(GiveToQueue) > 0 || true == WaitForOpenSlot)
				{
					if(xQueueSend(GiveToQueue, DataBuffer, portMAX_DELAY) != pdTRUE)
					{
						ESP_LOGV("Helpers", "Error Setting Queue");
					}
					ESP_LOGV("Helpers", "Added Data to Queue");
				}
				else
				{
					ESP_LOGW("Helpers", "WARNING! Queue Full.");
				}	
			  }
			  else
			  {
			    ESP_LOGE("Helpers", "ERROR! Error Receiving Queue.");
			  }
			}
			delete DataBuffer;
		  }
		  else
		  {
			ESP_LOGW("CommonUtils", "NULL Queue");
		  }
		}
		
		void MoveDataFromQueueToQueues(String DebugTitle, QueueHandle_t TakeFromQueue, QueueHandle_t* GiveToQueues, size_t GiveToQueueCount, size_t ByteCount, bool WaitForOpenSlot, bool DebugMessage)
		{
		  ESP_LOGV("Function Debug", "%s, ", __func__);
		  if(NULL != TakeFromQueue)
		  {
			size_t QueueCount = uxQueueMessagesWaiting(TakeFromQueue);
			ESP_LOGV("Helpers", "Queue Messages Waiting: %i Receiver Queue Count: %i Byte Count: %i", QueueCount, GiveToQueueCount, ByteCount);
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
							ESP_LOGV("Helpers", "Adding Data to Queue");
							if(true == WaitForOpenSlot || uxQueueSpacesAvailable(GiveToQueue) > 0)
							{
								if(xQueueSend(GiveToQueue, DataBuffer, portMAX_DELAY) != pdTRUE)
								{
									ESP_LOGV("Helpers", "Error Setting Queue");
								}
							}
							else
							{
								ESP_LOGW("CommonUtils", "%s: Queue Full!", DebugTitle);
							}
						}
						else
						{
							ESP_LOGW("CommonUtils", "%s: NULL Queue!", DebugTitle);
						}
					}						
				}
				else
				{
					ESP_LOGW("CommonUtils", "%s: Error Receiving Queue!", DebugTitle);
				}
			}
			delete DataBuffer;
		  }
		  else
		  {
			ESP_LOGW("CommonUtils", "%s: NULL Queue!", DebugTitle);
		  }
		}

		void CreateQueue(QueueHandle_t &Queue, size_t ByteCount, size_t QueueCount, bool DebugMessage)
		{
			ESP_LOGV("Function Debug", "%s, ", __func__);
			ESP_LOGV("Helpers", "Creating Queue of size: %i", ByteCount);
			Queue = xQueueCreate(QueueCount, ByteCount );
			if(Queue == NULL)
			{
				ESP_LOGE("CommonUtils", "ERROR! Error creating Queue.");
			}
		}
		
		void PushValueToQueue(void* Value, QueueHandle_t Queue, bool WaitForOpenSlot, bool DebugMessage)
		{
			ESP_LOGV("Function Debug", "%s, ", __func__);
			if(NULL != Queue)
			{
				if(uxQueueSpacesAvailable(Queue) > 0 || true == WaitForOpenSlot)
				{
					if(xQueueSend(Queue, Value, portMAX_DELAY) != pdTRUE)
					{
						ESP_LOGE("CommonUtils", "ERROR! Error Pushing Value to Queue.");
					}
				}
				else
				{
					ESP_LOGW("CommonUtils", "WARNING! Queue Full.");
				}
			}
			else
			{
				ESP_LOGW("CommonUtils", "WARNING! NULL Queue!");
			}
		}
		
		void GetValueFromQueue(void* Value, QueueHandle_t Queue, size_t ByteCount, bool ReadUntilEmpty, bool DebugMessage)
		{
			if(NULL != Queue)
			{
				size_t QueueCount = uxQueueMessagesWaiting(Queue);
				ESP_LOGV("CommonUtils", "Queue Count: %i", QueueCount);
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
							ESP_LOGE("CommonUtils", "ERROR! Error Receiving Queue.");
						}
					}
					delete DataBuffer;
				}
			}
			else
			{
				ESP_LOGE("CommonUtils", "ERROR! NULL Queue.");
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
				case DataType_Double:
					return sizeof(double);
				break;
				case DataType_ProcessedSoundData_t:
					return sizeof(ProcessedSoundData_t);
				break;
				case DataType_MaxBandSoundData_t:
					return sizeof(MaxBandSoundData_t);
				break;
				default:
					return 0;
				break;
			}
		}
};

#endif