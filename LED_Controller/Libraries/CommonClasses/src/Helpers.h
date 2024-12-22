#ifndef Helpers_H
#define Helpers_H
#include "DataTypes.h"
#include "Streaming.h"
#include <Ticker.h>
#include <mutex>

class LogWithRateLimit
{
public:
    LogWithRateLimit(uint32_t interval, esp_log_level_t delayedLogLevel = ESP_LOG_INFO)
        : logInterval(interval), lastLogTime(0), delayedLogLevel(delayedLogLevel), occurrenceCount(0)
    {
    }

    void Log(esp_log_level_t level, const std::string& tag_in, const std::string& message_in)
    {
        std::lock_guard<std::mutex> lock(logMutex);  // Ensure thread-safety
        tag = tag_in;
        message = message_in;
        
        if (occurrenceCount == 0)
        {
            LogMessage(level, tag, message);
        }
		if(millis() - lastLogTime > logInterval)
		{
            ResetCount();
		}
		else
		{
        	++occurrenceCount;
		}
    }

private:
    unsigned long logInterval;
	unsigned long lastLogTime;
    esp_log_level_t delayedLogLevel;
    int occurrenceCount;
    std::string tag;
    std::string message;
    std::mutex logMutex;

    void LogMessage(esp_log_level_t level, const std::string& tag, const std::string& message)
    {
		lastLogTime = millis();
		switch (level)
        {
            case ESP_LOG_ERROR:
                ESP_LOGE(tag.c_str(), "%s", message.c_str());
                break;
            case ESP_LOG_WARN:
                ESP_LOGW(tag.c_str(), "%s", message.c_str());
                break;
            case ESP_LOG_INFO:
                ESP_LOGI(tag.c_str(), "%s", message.c_str());
                break;
            case ESP_LOG_DEBUG:
                ESP_LOGD(tag.c_str(), "%s", message.c_str());
                break;
            case ESP_LOG_VERBOSE:
                ESP_LOGV(tag.c_str(), "%s", message.c_str());
                break;
            default:
                ESP_LOGI(tag.c_str(), "%s", message.c_str());
                break;
        }
    }

    void ResetCount()
    {
        std::lock_guard<std::mutex> lock(logMutex);  // Ensure thread-safety
        if (occurrenceCount > 1)
        {
            std::string messageWithCount = message + " Delayed Log: Additional occurrences during interval: " + std::to_string(occurrenceCount - 1);
            LogMessage(delayedLogLevel, tag, messageWithCount);
        }
        occurrenceCount = 0;
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
		QueueManager(std::string Title): m_Title(Title){
		}
		QueueManager(std::string Title, size_t DataItemCount): m_Title(Title)
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
		DataItem_t *GetPointerToDataItemWithName(std::string Name)
		{
			if(NULL != m_DataItem)
			{
				for(int i = 0; i < m_DataItemCount; ++i)
				{
					if(Name == m_DataItem[i].Name)
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
		QueueHandle_t GetQueueHandleRXForDataItem(std::string Name){
			if(NULL != m_DataItem)
			{
				for(int i = 0; i < m_DataItemCount; ++i)
				{
					if(Name == m_DataItem[i].Name)
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
		QueueHandle_t GetQueueHandleTXForDataItem(std::string Name){
			if(NULL != m_DataItem)
			{
				for(int i = 0; i < m_DataItemCount; ++i)
				{
					if(Name == m_DataItem[i].Name)
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
		size_t GetQueueByteCountForDataItem(std::string Name){
			if(NULL != m_DataItem)
			{
				for(int i = 0; i < m_DataItemCount; ++i)
				{
					if(Name == m_DataItem[i].Name)
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
		
		size_t GetTotalByteCountForDataItem(std::string Name){
			if(NULL != m_DataItem)
			{
				for(int i = 0; i < m_DataItemCount; ++i)
				{
					if(Name == m_DataItem[i].Name)
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
		size_t GetSampleCountForDataItem(std::string Name){
			if(NULL != m_DataItem)
			{
				for(int i = 0; i < m_DataItemCount; ++i)
				{
					if(Name == m_DataItem[i].Name)
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
		bool GetValueFromRXQueue(void* Value, const std::string Name, bool ReadUntilEmpty, TickType_t TicksToWait, bool &DataPullHasErrored){
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
		bool GetValueFromTXQueue(void* Value, std::string Name, bool ReadUntilEmpty, TickType_t TicksToWait, bool &DataPullHasErrored){
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
		void PushValueToRXQueue(void* Value, std::string Name, TickType_t TicksToWait, bool &DataPushHasErrored){
			QueueHandle_t Queue = GetQueueHandleRXForDataItem(Name);
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
				ESP_LOGE("CommonUtils", "ERROR! NULL Queue for: %s.", Name.c_str());
			}
		}
		void PushValueToTXQueue(void* Value, std::string Name, TickType_t TicksToWait, bool &DataPushHasErrored){
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
		void PushValueToTXQueue(String &Value, std::string Name, TickType_t TicksToWait, bool &DataPushHasErrored){
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
		std::string m_Title = "";
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
		void CreateManagedQueue(std::string Name, QueueHandle_t &Queue, size_t ByteCount, size_t QueueCount, bool DebugMessage){
			ESP_LOGV("Helpers", "Creating %i Queue(s), Named: %s of size: %i for a total of %i", QueueCount, Name.c_str(), ByteCount, ByteCount*QueueCount);
			Queue = xQueueCreate(QueueCount, ByteCount );
			if(Queue == NULL)
			{
				ESP_LOGE("Helpers", "ERROR! Error creating the Queue.");
			}
		}
};

#endif