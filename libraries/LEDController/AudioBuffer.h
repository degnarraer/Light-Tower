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

template <uint32_t T>
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
	  if(0 != pthread_mutex_init(&m_Lock, NULL))
	  {
		 ESP_LOGE("TestClass", "Failed to Create Lock");
	  }
	}

	void AllocateMemory()
	{
		size_t CircleBuffSize = sizeof(CircularBuffer<Frame_t, T>);
		void *CircularBuffer_Raw = (CircularBuffer<Frame_t, T>*)heap_caps_malloc(CircleBuffSize, MALLOC_CAP_SPIRAM);
		m_CircularAudioBuffer = new(CircularBuffer_Raw) CircularBuffer<Frame_t, T>;
	}

	void FreeMemory()
	{
	 heap_caps_free(m_CircularAudioBuffer);
	}

	bool Push(Frame_t Frame)
	{
		bool Result = false;
		if(0 == pthread_mutex_lock(&m_Lock))
		{
			Result = m_CircularAudioBuffer->push(Frame);
			pthread_mutex_unlock(&m_Lock);
		}
		return Result;
	}

	Frame_t Pop()
	{
		Frame_t Result;
		if(0 == pthread_mutex_lock(&m_Lock))
		{
			Result = m_CircularAudioBuffer->pop();
			pthread_mutex_unlock(&m_Lock);
		}
		return Result;
	}

	bool Unshift(Frame_t Frame)
	{
		bool Result = false;
		if(0 == pthread_mutex_lock(&m_Lock))
		{
			Result = m_CircularAudioBuffer->unshift(Frame);
			pthread_mutex_unlock(&m_Lock);
		}
		return Result;
	}

	Frame_t Shift()
	{
		Frame_t Result;
		if(0 == pthread_mutex_lock(&m_Lock))
		{
			Result = m_CircularAudioBuffer->shift();
			pthread_mutex_unlock(&m_Lock);
		}
		return Result;
	}

	bool IsEmpty()
	{
		bool Result = false;
		if(0 == pthread_mutex_lock(&m_Lock))
		{
			Result = m_CircularAudioBuffer->isEmpty();
			pthread_mutex_unlock(&m_Lock);
		}
		return Result;
	}

	bool IsFull()
	{
		bool Result = false;
		if(0 == pthread_mutex_lock(&m_Lock))
		{
			Result = m_CircularAudioBuffer->isFull();
			pthread_mutex_unlock(&m_Lock);
		}
		return Result;
	}

	size_t Size()
	{
		size_t Result = 0;
		if(0 == pthread_mutex_lock(&m_Lock))
		{
			Result = m_CircularAudioBuffer->size();
			pthread_mutex_unlock(&m_Lock);
		}
		return Result;
	}

	size_t Available()
	{
		size_t Result = 0;
		if(0 == pthread_mutex_lock(&m_Lock))
		{
			Result = m_CircularAudioBuffer->available();
			pthread_mutex_unlock(&m_Lock);
		}
		return Result;
	}

	void Clear()
	{
		if(0 == pthread_mutex_lock(&m_Lock))
		{
			m_CircularAudioBuffer->clear();
			pthread_mutex_unlock(&m_Lock);
		}
	}
	  private:
		CircularBuffer<Frame_t, T> *m_CircularAudioBuffer;
		pthread_mutex_t m_Lock;
	};

#endif
