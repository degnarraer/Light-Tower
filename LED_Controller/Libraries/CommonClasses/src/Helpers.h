#ifndef Helpers_H
#define Helpers_H
#include "DataTypes.h"
#include "Streaming.h"

class LogWithRateLimit
{
public:
    LogWithRateLimit(unsigned long interval) : logInterval(interval), lastLogTime(0), occurrenceCount(0) {}

    // Log function that supports different log levels and tracks occurrences
    void Log(esp_log_level_t level, const char* tag, const char* message)
    {
        unsigned long currentTime = millis();

        if (currentTime - lastLogTime >= logInterval) // If enough time has passed
        {
            if (occurrenceCount > 1)
            {
                // Log the number of occurrences if more than 1
                char messageWithCount[256];
                snprintf(messageWithCount, sizeof(messageWithCount), "%s (Repeated %d times)", message, occurrenceCount);
                switch (level)
                {
                    case ESP_LOG_ERROR:
                        ESP_LOGE(tag, "%s", messageWithCount);
                        break;
                    case ESP_LOG_WARN:
                        ESP_LOGW(tag, "%s", messageWithCount);
                        break;
                    case ESP_LOG_INFO:
                        ESP_LOGI(tag, "%s", messageWithCount);
                        break;
                    case ESP_LOG_DEBUG:
                        ESP_LOGD(tag, "%s", messageWithCount);
                        break;
                    case ESP_LOG_VERBOSE:
                        ESP_LOGV(tag, "%s", messageWithCount);
                        break;
                    default:
                        ESP_LOGI(tag, "%s", messageWithCount); // Default to INFO if no valid level
                        break;
                }
            }
            else
            {
                // Log the message normally if occurrence count is 1
                switch (level)
                {
                    case ESP_LOG_ERROR:
                        ESP_LOGE(tag, "%s", message);
                        break;
                    case ESP_LOG_WARN:
                        ESP_LOGW(tag, "%s", message);
                        break;
                    case ESP_LOG_INFO:
                        ESP_LOGI(tag, "%s", message);
                        break;
                    case ESP_LOG_DEBUG:
                        ESP_LOGD(tag, "%s", message);
                        break;
                    case ESP_LOG_VERBOSE:
                        ESP_LOGV(tag, "%s", message);
                        break;
                    default:
                        ESP_LOGI(tag, "%s", message); // Default to INFO if no valid level
                        break;
                }
            }

            // Reset count and update the last log time
            occurrenceCount = 0;
            lastLogTime = currentTime;
        }
        else
        {
            // Increment occurrence count if within the rate limit period
            ++occurrenceCount;
        }
    }

private:
    unsigned long lastLogTime;
    unsigned long logInterval;
    int occurrenceCount; // Track the number of occurrences within the rate-limited period
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
};
class QueueController: public DataTypeFunctions
{
	public:
		
		void PushValueToQueue(void* Value, QueueHandle_t Queue, const String &DebugTitle, TickType_t TicksToWait, bool &DataPushHasErrored){
			if(NULL != Queue)
			{
				if(xQueueSend(Queue, Value, TicksToWait) != pdTRUE)
				{
					if(false == DataPushHasErrored)
					{
						DataPushHasErrored = true;
						ESP_LOGD("CommonUtils", "ERROR! %s: Error Pushing Value to Queue.", DebugTitle.c_str());
					}
				}
			}
			else
			{
				if(false == DataPushHasErrored)
				{
					DataPushHasErrored = true;
					ESP_LOGD("CommonUtils", "Error! %s: NULL Queue!", DebugTitle.c_str());
				}
			}
		}	
		bool GetValueFromQueue(void* Value, QueueHandle_t Queue, String DebugTitle, bool ReadUntilEmpty, TickType_t TicksToWait, bool &DataPullHasErrored)
		{
			bool Result = false;
			if(NULL != Queue)
			{
				size_t QueueCount = uxQueueMessagesWaiting(Queue);
				if(QueueCount > 0)
				{
					ESP_LOGD("CommonUtils", "Queue Count: %i", QueueCount);
					for(int i = 0; i < QueueCount; ++i)
					{
						if ( xQueueReceive(Queue, Value, TicksToWait) == pdTRUE )
						{
							Result = true;
						}
						else
						{
							if(false == DataPullHasErrored)
							{
								DataPullHasErrored = true;
								ESP_LOGE("CommonUtils", "ERROR! %s: Error Receiving Value from Queue.", DebugTitle.c_str());
							}
						}
						if(false == ReadUntilEmpty)
						{
							return Result;
						}
					}
				}
			}
			else
			{
				ESP_LOGE("CommonUtils", "ERROR! NULL Queue.");
			}
			return Result;
		}	
		void MoveDataFromQueueToQueue( String DebugTitle
									 , QueueHandle_t TakeFromQueue
									 , size_t TakeFromQueueByteCount
									 , QueueHandle_t GiveToQueue
									 , size_t GiveToQueueByteCount
									 , TickType_t TicksToWait
									 , bool DebugMessage )
		{
			assert(TakeFromQueueByteCount == GiveToQueueByteCount);
			if(NULL != TakeFromQueue && NULL != GiveToQueue)
			{
				size_t QueueCount = uxQueueMessagesWaiting(TakeFromQueue);
				if(true == DebugMessage && QueueCount > 0)
				{
					ESP_LOGE("Helpers", "ERROR! %s: MoveDataFromQueueToQueue: Queue Messages Waiting: %i Byte Count: %i.", DebugTitle.c_str(), QueueCount, TakeFromQueueByteCount);
				}
				for (uint8_t i = 0; i < QueueCount; ++i)
				{
					uint8_t DataBuffer[TakeFromQueueByteCount];
					if ( xQueueReceive(TakeFromQueue, DataBuffer, TicksToWait) == pdTRUE )
					{
						if(xQueueSend(GiveToQueue, DataBuffer, TicksToWait) != pdTRUE)
						{
							if(true == DebugMessage) ESP_LOGE("Helpers", "ERROR! %s: Error Setting Queue Value.", DebugTitle.c_str());
						}
						else
						{
							if(true == DebugMessage) ESP_LOGV("Helpers", "%s: Added Data to Queue", DebugTitle.c_str());
						}
					}
					else
					{
						ESP_LOGD("Helpers", "ERROR! %s: Error Receiving Queue.", DebugTitle.c_str());
					}
				}
			}
		  else
		  {
			ESP_LOGE("CommonUtils", "%s: ERROR! NULL Queue.", DebugTitle.c_str());
		  }
		}
		void MoveDataFromQueueToQueues(String DebugTitle, QueueHandle_t TakeFromQueue, QueueHandle_t* GiveToQueues, size_t GiveToQueueCount, size_t ByteCount, TickType_t TicksToWait,  bool DebugMessage)
		{
		  if(NULL != TakeFromQueue)
		  {
			size_t QueueCount = uxQueueMessagesWaiting(TakeFromQueue);
			if(true == DebugMessage && QueueCount >0) ESP_LOGV("Helpers", "Queue Messages Waiting: %i Receiver Queue Count: %i Byte Count: %i", QueueCount, GiveToQueueCount, ByteCount);
			
			for (uint8_t i = 0; i < QueueCount; ++i)
			{
				uint8_t DataBuffer[ByteCount];
				if ( xQueueReceive(TakeFromQueue, DataBuffer, TicksToWait) == pdTRUE )
				{
					for(int j = 0; j < GiveToQueueCount; ++j)
					{	
						QueueHandle_t GiveToQueue = GiveToQueues[j];
						if(NULL != GiveToQueue)
						{
							if(true == DebugMessage) Serial << DebugTitle + ": Adding Data to Queue\n";
							if(xQueueSend(GiveToQueue, DataBuffer, TicksToWait) != pdTRUE)
							{
								if(true == DebugMessage) ESP_LOGD("Helpers", "ERROR! %s: Error Setting Queue Value", DebugTitle.c_str());
							}
							else
							{
								if(true == DebugMessage) Serial << DebugTitle.c_str() << ": Added Data to Queue.\n";
							}
							
						}
						else
						{
							if(true == DebugMessage) ESP_LOGD("CommonUtils", "ERROR! %s: NULL Queue.", DebugTitle.c_str());
						}
					}						
				}
				else
				{
					ESP_LOGD("CommonUtils", "ERROR! %s: Error Receiving Queue.", DebugTitle.c_str());
				}
			}
		  }
		  else
		  {
			ESP_LOGE("CommonUtils", "ERROR! %s: NULL Queue.", DebugTitle.c_str());
		  }
		}
};

class QueueManager: public CommonUtils
				  , public QueueController
{
	public:
		QueueManager(String Title): m_Title(Title){
		}
		QueueManager(String Title, size_t DataItemCount): m_Title(Title)
													    , m_DataItemCount(DataItemCount){
		}
		virtual ~QueueManager(){
			if(true == m_MemoryAllocated)FreeMemory();
		}
		virtual DataItemConfig_t* GetDataItemConfig() = 0;
		virtual size_t GetDataItemConfigCount() = 0;
		DataItem_t& GetQueueManagerDataItems() { return *m_DataItem; }
		size_t GetQueueManagerDataItemCount() { return m_DataItemCount; }
		void SetupQueueManager(size_t DataItemCount){
			m_DataItemCount = DataItemCount;
			SetupQueueManager();
		}
		void SetupQueueManager(){
			ESP_LOGD("CommonUtils", "%s: Setup", m_Title.c_str());
			if(true == m_MemoryAllocated)FreeMemory();
			AllocateMemory();
		}	
		void CreateQueue(QueueHandle_t &Queue, size_t ByteCount, size_t QueueCount, bool DebugMessage){
			ESP_LOGV("Helpers", "Creating Queue of size: %i", ByteCount);
			Queue = xQueueCreate(QueueCount, ByteCount );
			if(Queue == NULL)
			{
				ESP_LOGE("CommonUtils", "ERROR! Error creating Queue.");
			}
		}
		//Data Items
		DataItem_t *GetPointerToDataItemWithName(String Name)
		{
			if(NULL != m_DataItem)
			{
				for(int i = 0; i < m_DataItemCount; ++i)
				{
					if(true == Name.equals(m_DataItem[i].Name))
					{
						return &(m_DataItem[i]);
					}
				}
				ESP_LOGE("CommonUtils", "ERROR! %s: Data Item Not Found.", Name.c_str());
			}
			else
			{
				ESP_LOGE("CommonUtils", "ERROR! %s: NULL Data Item.", Name.c_str());
			}
			return NULL;
		}
		QueueHandle_t GetQueueHandleRXForDataItem(String Name){
			if(NULL != m_DataItem)
			{
				for(int i = 0; i < m_DataItemCount; ++i)
				{
					if(true == Name.equals(m_DataItem[i].Name))
					{
						return m_DataItem[i].QueueHandle_RX;
					}
				}
				ESP_LOGE("CommonUtils", "ERROR! %s: Data Item Not Found.", Name.c_str());
			}
			else
			{
				ESP_LOGE("CommonUtils", "ERROR! %s: NULL Data Item.", Name.c_str());
			}
			return NULL;
		}
		QueueHandle_t GetQueueHandleTXForDataItem(String Name){
			if(NULL != m_DataItem)
			{
				for(int i = 0; i < m_DataItemCount; ++i)
				{
					if(true == Name.equals(m_DataItem[i].Name))
					{
						return m_DataItem[i].QueueHandle_TX;
					}
				}
				ESP_LOGE("CommonUtils", "ERROR! %s: Data Item Not Found.", Name.c_str());
			}
			else
			{
				ESP_LOGE("CommonUtils", "ERROR! %s: NULL Data Item.", Name.c_str());
			}
			return NULL;
		}
		size_t GetQueueByteCountForDataItem(String Name){
			if(NULL != m_DataItem)
			{
				for(int i = 0; i < m_DataItemCount; ++i)
				{
					if(true == Name.equals(m_DataItem[i].Name))
					{
						return m_DataItem[i].TotalByteCount;
					}
				}
				ESP_LOGE("CommonUtils", "ERROR! %s: Data Item Not Found.", Name.c_str());
			}
			else
			{
				ESP_LOGE("CommonUtils", "ERROR! %s: NULL Data Item.", Name.c_str());
			}
			return 0;
		}
		
		size_t GetTotalByteCountForDataItem(String Name){
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
				ESP_LOGE("CommonUtils", "ERROR! NULL Data Item.");
			}
			return 0;
		}	
		size_t GetSampleCountForDataItem(String Name){
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
				ESP_LOGE("CommonUtils", "ERROR! NULL Data Item.");
			}
			return 0;
		}
	
		
		//Queues
		bool GetValueFromRXQueue(void* Value, const String Name, bool ReadUntilEmpty, TickType_t TicksToWait, bool &DataPullHasErrored){
			bool result = false;
			QueueHandle_t Queue = GetQueueHandleRXForDataItem(Name);
			if(NULL != Queue)
			{
				size_t QueueCount = uxQueueMessagesWaiting(Queue);
				ESP_LOGV("CommonUtils", "Queue Count: %i", QueueCount);
				if(QueueCount > 0)
				{
					if(false == ReadUntilEmpty) QueueCount = 1;
					for(int i = 0; i < QueueCount; ++i)
					{
						if ( xQueueReceive(Queue, Value, TicksToWait) == pdTRUE )
						{
							result = true;
						}
						else
						{	
							if(false == DataPullHasErrored)
							{
								DataPullHasErrored = true;
								ESP_LOGE("CommonUtils", "ERROR! did not receive Queue.");
							}
						}
					}
				}
			}
			else
			{
				ESP_LOGE("CommonUtils", "ERROR! NULL Queue for: %s", Name.c_str());
			}
			return result;
		}
		bool GetValueFromTXQueue(void* Value, String Name, bool ReadUntilEmpty, TickType_t TicksToWait, bool &DataPullHasErrored){
			bool Result = false;
			QueueHandle_t Queue = GetQueueHandleTXForDataItem(Name);
			if(NULL != Queue)
			{
				size_t QueueCount = uxQueueMessagesWaiting(Queue);
				ESP_LOGV("CommonUtils", "Queue Count: %i", QueueCount);
				if(QueueCount > 0)
				{
					if(false == ReadUntilEmpty) QueueCount = 1;
					for(int i = 0; i < QueueCount; ++i)
					{
						if ( xQueueReceive(Queue, Value, TicksToWait) == pdTRUE )
						{
							Result = true;
						}
						else
						{
							if(false == DataPullHasErrored)
							{
								DataPullHasErrored = true;
								ESP_LOGE("CommonUtils", "ERROR! Unable to receive queue.");
							}
						}
					}
				}
			}
			else
			{
				ESP_LOGE("CommonUtils", "ERROR! NULL Queue for: %s.", Name.c_str());
			}
			return Result;
		}
		void PushValueToRXQueue(void* Value, String Name, TickType_t TicksToWait, bool &DataPushHasErrored){
			QueueHandle_t Queue = GetQueueHandleRXForDataItem(Name);
			if(NULL != Queue)
			{
				if(true == Name.equals("Amplitude Gain"))  Serial << "Push RX\n";
				if(xQueueSend(Queue, Value, TicksToWait) != pdTRUE)
				{
					if(false == DataPushHasErrored)
					{
						DataPushHasErrored = true;
						ESP_LOGE("CommonUtils", "ERROR! %s: Error Setting Queue.", Name.c_str());
					}
				} 
			}
			else
			{
				ESP_LOGE("CommonUtils", "ERROR! NULL Queue for: %s.", Name.c_str());
			}
		}
		void PushValueToTXQueue(void* Value, String Name, TickType_t TicksToWait, bool &DataPushHasErrored){
			QueueHandle_t Queue = GetQueueHandleTXForDataItem(Name);
			if(NULL != Queue)
			{
				if(xQueueSend(Queue, Value, TicksToWait) != pdTRUE)
				{
					if(false == DataPushHasErrored)
					{
						DataPushHasErrored = true;
						ESP_LOGE("CommonUtils", "ERROR! %s: Error Setting Queue.", Name.c_str());
					}
				} 
			}
			else
			{
				ESP_LOGE("CommonUtils", "ERROR! NULL Queue.");
			}
		}
		void PushValueToTXQueue(String &Value, String Name, TickType_t TicksToWait, bool &DataPushHasErrored){
			QueueHandle_t Queue = GetQueueHandleTXForDataItem(Name);
			if(NULL != Queue)
			{
				if(xQueueSend(Queue, &Value, TicksToWait) != pdTRUE)
				{
					if(false == DataPushHasErrored)
					{
						DataPushHasErrored = true;
						ESP_LOGE("CommonUtils", "ERROR! %s: Error Setting Queue.", Name.c_str());
					}
				} 
			}
			else
			{
				ESP_LOGE("CommonUtils", "ERROR! NULL Queue for: %s.", Name.c_str());
			}
		}
		
	private:
		DataItem_t* m_DataItem = nullptr;
		String m_Title = "";
		size_t m_DataItemCount = 0;
		bool m_MemoryAllocated = false;
		void AllocateMemory(){
			m_DataItemCount = GetDataItemConfigCount();
			size_t ConfigBytes = sizeof(struct DataItem_t) * m_DataItemCount;
			DataItemConfig_t* ConfigFile = GetDataItemConfig();
			ESP_LOGE("CommonUtils", "ERROR! %s: Allocating %i DataItem's for a total of %i bytes of Memory.", m_Title.c_str(), m_DataItemCount, ConfigBytes);
			
			//Placement Allocation
			void *DataItem_t_raw = malloc(sizeof(DataItem_t) * m_DataItemCount);
			m_DataItem = new(DataItem_t_raw) DataItem_t[m_DataItemCount];

			assert(m_DataItem != NULL);
			for(int i = 0; i < m_DataItemCount; ++i)
			{
				size_t bytes = 0;
				bytes = GetSizeOfDataType(ConfigFile[i].DataType) * ConfigFile[i].Count;
				switch(ConfigFile[i].TransceiverConfig)
				{
					case Transciever_t::Transciever_None:
					break;
					case Transciever_t::Transciever_TX:
						CreateManagedQueue(ConfigFile[i].Name, m_DataItem[i].QueueHandle_TX, bytes, ConfigFile[i].QueueCount, true);
					break;
					case Transciever_t::Transciever_RX:
						CreateManagedQueue(ConfigFile[i].Name, m_DataItem[i].QueueHandle_RX, bytes, ConfigFile[i].QueueCount, true);
					break;
					case Transciever_t::Transciever_TXRX:
						CreateManagedQueue(ConfigFile[i].Name, m_DataItem[i].QueueHandle_RX, bytes, ConfigFile[i].QueueCount, true);
						CreateManagedQueue(ConfigFile[i].Name, m_DataItem[i].QueueHandle_TX, bytes, ConfigFile[i].QueueCount, true);
					break;
				}
				ESP_LOGD("CommonUtils", "%s: Try Configuring DataItem %i of %i", m_Title.c_str(), i+1, m_DataItemCount);
				m_DataItem[i].Name = ConfigFile[i].Name;
				m_DataItem[i].DataType = ConfigFile[i].DataType;
				m_DataItem[i].Count = ConfigFile[i].Count;
				m_DataItem[i].TotalByteCount = bytes;
				m_DataItem[i].TransceiverConfig = ConfigFile[i].TransceiverConfig;
			}
			m_MemoryAllocated = true;
		}
		void FreeMemory(){
			for(int i = 0; i < m_DataItemCount; ++i)
			{		
				switch(m_DataItem[i].TransceiverConfig)
				{
					case Transciever_t::Transciever_None:
					break;
					case Transciever_t::Transciever_TX:
						vQueueDelete(m_DataItem[i].QueueHandle_TX);
					break;
					case Transciever_t::Transciever_RX:
						vQueueDelete(m_DataItem[i].QueueHandle_RX);
					break;
					case Transciever_t::Transciever_TXRX:
						vQueueDelete(m_DataItem[i].QueueHandle_RX);
						vQueueDelete(m_DataItem[i].QueueHandle_TX);
					break;
				}
			}
			free(m_DataItem);
			m_MemoryAllocated = false;
		}		
		void CreateManagedQueue(String Name, QueueHandle_t &Queue, size_t ByteCount, size_t QueueCount, bool DebugMessage){
			ESP_LOGV("Helpers", "Creating %i Queue(s), Named: %s of size: %i for a total of %i", QueueCount, Name.c_str(), ByteCount, ByteCount*QueueCount);
			Queue = xQueueCreate(QueueCount, ByteCount );
			if(Queue == NULL)
			{
				ESP_LOGE("Helpers", "ERROR! Error creating the Queue.");
			}
		}
};

#endif