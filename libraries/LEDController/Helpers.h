#ifndef Helpers_H
#define Helpers_H
#include <DataTypes.h>
#include "Streaming.h"

class CommonUtils
{
	public:
		template <class T>
		void MoveDataFromQueueToQueue(QueueHandle_t TakeFromQueue, QueueHandle_t GiveToQueue, size_t ByteCount, bool WaitForOpenSlot, bool DebugMessage)
		{
		  if(NULL != TakeFromQueue && NULL != GiveToQueue)
		  {
			size_t QueueCount = uxQueueMessagesWaiting(TakeFromQueue);
			if(true == DebugMessage) Serial << "Queue Count: " << QueueCount << "\n";
			for (uint8_t i = 0; i < QueueCount; ++i)
			{
			  T* DataBuffer = (T*)malloc(ByteCount);
			  if ( xQueueReceive(TakeFromQueue, DataBuffer, portMAX_DELAY) == pdTRUE )
			  {
				if(true == DebugMessage)Serial << "Adding Data to Queue\n";
				if(true == WaitForOpenSlot || uxQueueSpacesAvailable(GiveToQueue) > 0)
				{
					if(xQueueSend(GiveToQueue, DataBuffer, portMAX_DELAY) != pdTRUE){Serial.println("Error Setting Queue");}
				}			
			  }
			  else
			  {
				Serial << "Error Receiving Queue!";
			  }
			  delete DataBuffer;
			}
		  }
		}
		template <class T>
		void MoveDataFromQueueToQueues(QueueHandle_t TakeFromQueue, QueueHandle_t* GiveToQueues, size_t GiveToQueueCount, size_t ByteCount, bool WaitForOpenSlot, bool DebugMessage)
		{
		  if(NULL != TakeFromQueue)
		  {
			size_t QueueCount = uxQueueMessagesWaiting(TakeFromQueue);
			if(true == DebugMessage) Serial << "Queue Count: " << QueueCount << "\n";
			for (uint8_t i = 0; i < QueueCount; ++i)
			{
				T* DataBuffer = (T*)malloc(ByteCount);
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
					}						
				}
				else
				{
					Serial << "Error Receiving Queue!";
				}
				delete DataBuffer;
			}
		  }
		}

		void CreateQueue(QueueHandle_t &Queue, size_t ByteCount, size_t QueueCount, bool DebugMessage)
		{
			if(true == DebugMessage) Serial << "Creating Queue.\n";
			Queue = xQueueCreate(QueueCount, ByteCount );
			if(Queue == NULL){Serial.println("Error creating the Queue");}
		}
		
		void PushValueToQueue(void* Value, QueueHandle_t Queue, bool WaitForOpenSlot)
		{
			if(uxQueueSpacesAvailable(Queue) > 0 || true == WaitForOpenSlot)
			{
				if(xQueueSend(Queue, &Value, portMAX_DELAY) != pdTRUE){Serial.println("Error Setting Queue");} 
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
							return true;
						}
						delete DataBuffer;
					}
				}
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
};

#endif