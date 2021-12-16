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

		void CreateQueue(QueueHandle_t &Queue, size_t ByteCount, size_t QueueCount, bool DebugMessage)
		{
			if(true == DebugMessage) Serial << "Creating Queue.\n";
			Queue = xQueueCreate(QueueCount, ByteCount );
			if(Queue == NULL){Serial.println("Error creating the Queue");}
		}
		
		int32_t GetDataBufferValue(char* DataBuffer, size_t BytesPerSample, size_t index)
		{
			switch(BytesPerSample)
			{
			  case 1:
				return ((int8_t*)DataBuffer)[index];
			  break;
			  case 2:
				return ((int16_t*)DataBuffer)[index];
			  break;
			  case 3:
			  break;
			  case 4:
				return ((int32_t*)DataBuffer)[index];
			  break;
			}
		}
		void SetDataBufferValue(char* DataBuffer, size_t BytesPerSample, size_t index, int32_t value)
		{
			switch(BytesPerSample)
			{
			  case 1:
				((int8_t*)DataBuffer)[index] = (int8_t)value;
			  break;
			  case 2:
				((int16_t*)DataBuffer)[index] = (int16_t)value;
			  break;
			  case 3:
			  break;
			  case 4:
				((int32_t*)DataBuffer)[index] = (int32_t)value;
			  break;
			}
		}
};

#endif