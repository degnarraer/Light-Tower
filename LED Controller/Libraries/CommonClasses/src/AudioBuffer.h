/*
    Light Tower by Rob Shockency
    Copyright (C) 2021 Rob Shockency degnarraer@yahoo.com

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version of the License, or
    (at your option) any later version. 3

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef AudioBuffer_H
#define AudioBuffer_H
#include "Datatypes.h"
#include "CircularBuffer.h" 
#include "circle_buf.h"
#include "Streaming.h"

template <uint32_t COUNT>
class AudioBuffer
{
  public:
    AudioBuffer(){}
    virtual ~AudioBuffer()
    {
      FreeMemory();
    }
    void Initialize()
	{
		AllocateMemory();
		pthread_mutexattr_t Attr;
		pthread_mutexattr_init(&Attr);
		pthread_mutexattr_settype(&Attr, PTHREAD_MUTEX_RECURSIVE);	  
		if(0 != pthread_mutex_init(&m_Lock, &Attr))
		{   
			ESP_LOGE("TestClass", "ERROR! Failed to Create Lock.");
		}
	}

	void AllocateMemory()
	{
        ESP_LOGD("AllocateMemory", "Allocating memory");
		size_t CircleBuffSize = sizeof(bfs::CircleBuf<Frame_t, COUNT>);
		void *CircularBuffer_Raw = (bfs::CircleBuf<Frame_t, COUNT>*)malloc(CircleBuffSize);
		if (CircularBuffer_Raw == nullptr) 
		{
            ESP_LOGE("AllocateMemory", "ERROR! Memory allocation failed.");
            return;
        }
		m_CircularAudioBuffer = new(CircularBuffer_Raw) bfs::CircleBuf<Frame_t, COUNT>;
	}

	void FreeMemory()
	{
		free(m_CircularAudioBuffer);
	}
	size_t GetFrameCapacity()
	{
		size_t Capacity = 0;
		pthread_mutex_lock(&m_Lock);
		Capacity = m_CircularAudioBuffer->capacity();
		pthread_mutex_unlock(&m_Lock);
		return Capacity;
	}

	bool ClearAudioBuffer()
	{
		bool Success = false;
		pthread_mutex_lock(&m_Lock);
		m_CircularAudioBuffer->Clear();
		Success = true;
		pthread_mutex_unlock(&m_Lock);
		return Success;
	}

	size_t GetFrameCount()
	{
		size_t size = 0;
		pthread_mutex_lock(&m_Lock);
		size = m_CircularAudioBuffer->size();
		pthread_mutex_unlock(&m_Lock);
		return size;
	}

	size_t GetFreeFrameCount()
	{
		return GetFrameCapacity() - GetFrameCount();
	}

	size_t WriteAudioFrames( Frame_t *FrameBuffer, size_t FrameCount )
	{
		size_t FramesWritten = 0;
		pthread_mutex_lock(&m_Lock);
		FramesWritten = m_CircularAudioBuffer->Write(FrameBuffer, FrameCount);
		pthread_mutex_unlock(&m_Lock);
		return FramesWritten;
	}

	bool WriteAudioFrame( Frame_t Frame )
	{
		bool Success = false;
		pthread_mutex_lock(&m_Lock);
		Success = m_CircularAudioBuffer->Write(Frame);
		pthread_mutex_unlock(&m_Lock);
		return Success;
	}

	size_t ReadAudioFrames(Frame_t *FrameBuffer, size_t FrameCount)
	{
		size_t FramesRead = 0;
		pthread_mutex_lock(&m_Lock);
		FramesRead = m_CircularAudioBuffer->Read(FrameBuffer, FrameCount);
		pthread_mutex_unlock(&m_Lock);
		return FramesRead;
	}

	bfs::optional<Frame_t> ReadAudioFrame()
	{
		bfs::optional<Frame_t> FrameRead;
		pthread_mutex_lock(&m_Lock);
		FrameRead = m_CircularAudioBuffer->Read();
		pthread_mutex_unlock(&m_Lock);
		return FrameRead;
	}
	
	private:
		bfs::CircleBuf<Frame_t, COUNT> *m_CircularAudioBuffer = nullptr;
		pthread_mutex_t m_Lock;
};


template <uint32_t COUNT>
class ContinuousAudioBuffer
{
  public:
    ContinuousAudioBuffer(){}
    virtual ~ContinuousAudioBuffer()
    {
      FreeMemory();
    }
    void Initialize()
	{ 
		AllocateMemory();  
		pthread_mutexattr_t Attr;
		pthread_mutexattr_init(&Attr);
		pthread_mutexattr_settype(&Attr, PTHREAD_MUTEX_RECURSIVE);	  
		if(0 != pthread_mutex_init(&m_Lock, &Attr))
		{
			ESP_LOGE("Initialize", "ERROR! Failed to Create Lock.");
		}
	}

	void AllocateMemory()
    {
        ESP_LOGD("AllocateMemory", "Allocating memory");
        size_t CircleBuffSize = sizeof(CircularBuffer<Frame_t, COUNT>);
        void* CircularBuffer_Raw = malloc (CircleBuffSize );
        if (CircularBuffer_Raw == nullptr)
		{
            ESP_LOGE("AllocateMemory", "ERROR! Memory allocation failed.");
            return;
        }
        m_CircularAudioBuffer = new(CircularBuffer_Raw) CircularBuffer<Frame_t, COUNT>;
    }

	void FreeMemory()
	{
		if (m_CircularAudioBuffer != nullptr) {
            m_CircularAudioBuffer->~CircularBuffer<Frame_t, COUNT>();
            free(m_CircularAudioBuffer);
            m_CircularAudioBuffer = nullptr;
        }
	}
	
	uint32_t GetAudioFrames(Frame_t *Buffer, uint32_t Count)
	{
		uint32_t ReturnCount = 0;
		pthread_mutex_lock(&m_Lock);
		for(int i = 0; i < Count && i < m_CircularAudioBuffer->size(); ++i)
		{
			Buffer[i] = ((Frame_t*)m_CircularAudioBuffer)[i];
			++ReturnCount;
		}
		pthread_mutex_unlock(&m_Lock);
		return ReturnCount;
	}
	
	bool Push(Frame_t Frame)
	{
		bool result = false;
		pthread_mutex_lock(&m_Lock);
		result = m_CircularAudioBuffer->push(Frame);
		if(result)
		{
			ESP_LOGD("Push", "Pushed %i|%i", Frame.channel1, Frame.channel2);
		}
		else
		{
			ESP_LOGD("Push", "Pushed and overwrote %i|%i", Frame.channel1, Frame.channel2);
		}
		pthread_mutex_unlock(&m_Lock);
		return result;
	}

	size_t Push(Frame_t *Frame, size_t Count)
	{
		size_t Result = 0;
		pthread_mutex_lock(&m_Lock);
		for(int i = 0; i < Count; ++i)
		{
			if(m_CircularAudioBuffer->push(Frame[i]))
			{
				ESP_LOGD("Push", "Pushed %i|%i", Frame[i].channel1, Frame[i].channel2);
				++Result;
			}
			else
			{
				ESP_LOGD("Push", "Pushed and overwrote %i|%i", Frame[i].channel1, Frame[i].channel2);
			}
		}
		pthread_mutex_unlock(&m_Lock);
		return Result;
	}

	Frame_t Pop()
	{
		Frame_t Result;
		pthread_mutex_lock(&m_Lock);
		Result = m_CircularAudioBuffer->pop();
		pthread_mutex_unlock(&m_Lock);
		return Result;
	}

	size_t Pop(Frame_t *Frame, size_t Count)
	{
		size_t Result = 0;
		pthread_mutex_lock(&m_Lock);
		for(int i = 0; i < Count; ++i)
		{
			if(0 < COUNT - m_CircularAudioBuffer->available())
			{
				Frame[i] = m_CircularAudioBuffer->pop();
				ESP_LOGD("Pop", "Popped %i|%i", Frame[i].channel1, Frame[i].channel2);
				++Result;
			}
		}
		pthread_mutex_unlock(&m_Lock);
		return Result;
	}

	bool Unshift(Frame_t Frame)
	{
		bool result = false;
		pthread_mutex_lock(&m_Lock);
		result = m_CircularAudioBuffer->unshift(Frame);
		if(result)
		{
			ESP_LOGD("Push", "Unshifted %i|%i", Frame.channel1, Frame.channel2);
		}
		else
		{
			ESP_LOGD("Push", "Unshifted and overwrote %i|%i", Frame.channel1, Frame.channel2);
		}
		pthread_mutex_unlock(&m_Lock);
		return result;
	}

	size_t Unshift(Frame_t *Frame, size_t Count)
	{
		size_t Result = 0;
		pthread_mutex_lock(&m_Lock);
		for(int i = 0; i < Count; ++i)
		{
			if( m_CircularAudioBuffer->unshift(Frame[i]) )
			{
				ESP_LOGD("Unshift", "Unshifted %i|%i", Frame[i].channel1, Frame[i].channel2);
				++Result;
			}
			else
			{
				ESP_LOGD("Unshift", "Unshifted and Overwrote %i|%i", Frame[i].channel1, Frame[i].channel2);
			}
		}
		pthread_mutex_unlock(&m_Lock);
		return Result;
	}

	Frame_t Shift()
	{
		Frame_t Result;
		pthread_mutex_lock(&m_Lock);
		Result = m_CircularAudioBuffer->shift();
		pthread_mutex_unlock(&m_Lock);
		return Result;
	}

	size_t Shift(Frame_t *Frame, size_t Count)
	{
		size_t Result = 0;
		pthread_mutex_lock(&m_Lock);
		for(int i = 0; i < Count; ++i)
		{
			if( 0 < COUNT - m_CircularAudioBuffer->available() )
			{
				Frame[i] = m_CircularAudioBuffer->shift();
				++Result;
			}
		}
		pthread_mutex_unlock(&m_Lock);
		return Result;
	}

	bool IsEmpty()
	{
		bool Result = false;
		pthread_mutex_lock(&m_Lock);
		Result = m_CircularAudioBuffer->isEmpty();
		pthread_mutex_unlock(&m_Lock);
		return Result;
	}

	bool IsFull()
	{
		bool Result = false;
		pthread_mutex_lock(&m_Lock);
		Result = m_CircularAudioBuffer->isFull();
		pthread_mutex_unlock(&m_Lock);
		return Result;
	}

	size_t Size()
	{
		size_t Result = 0;
		pthread_mutex_lock(&m_Lock);
		Result = m_CircularAudioBuffer->size();
		pthread_mutex_unlock(&m_Lock);
		return Result;
	}

	size_t Available()
	{
		size_t Result = 0;
		pthread_mutex_lock(&m_Lock);
		Result = m_CircularAudioBuffer->available();
		pthread_mutex_unlock(&m_Lock);
		return Result;
	}

	void Clear()
	{
		pthread_mutex_lock(&m_Lock);
		m_CircularAudioBuffer->clear();
		pthread_mutex_unlock(&m_Lock);
	}
	  private:
		CircularBuffer<Frame_t, COUNT> *m_CircularAudioBuffer = nullptr;
		pthread_mutex_t m_Lock;
};

#endif
