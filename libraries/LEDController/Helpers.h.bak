#ifndef Helpers_H
#define Helpers_H
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
				if(true == WaitForOpenSlot)
				{
					if(xQueueSend(GiveToQueue, DataBuffer, portMAX_DELAY) != pdTRUE){Serial.println("Error Setting Queue");}
				}
				else
				{
					if(uxQueueSpacesAvailable(GiveToQueue) > 0)
					{
						if(xQueueSend(GiveToQueue, DataBuffer, portMAX_DELAY) != pdTRUE){Serial.println("Error Setting Queue");}
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
};

#endif