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
	}

	void AllocateMemory()
	{
		std::lock_guard<std::recursive_mutex> lock(m_Lock)
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
		std::lock_guard<std::recursive_mutex> lock(m_Lock)
		size_t Capacity = 0;
		Capacity = m_CircularAudioBuffer->capacity();
		return Capacity;
	}

	bool ClearAudioBuffer()
	{
		std::lock_guard<std::recursive_mutex> lock(m_Lock)
		bool Success = false;
		m_CircularAudioBuffer->Clear();
		Success = true;
		return Success;
	}

	size_t GetFrameCount()
	{
		std::lock_guard<std::recursive_mutex> lock(m_Lock)
		size_t size = 0;
		size = m_CircularAudioBuffer->size();
		return size;
	}

	size_t GetFreeFrameCount()
	{
		return GetFrameCapacity() - GetFrameCount();
	}

	size_t WriteAudioFrames( Frame_t *FrameBuffer, size_t FrameCount )
	{
		std::lock_guard<std::recursive_mutex> lock(m_Lock)
		size_t FramesWritten = 0;
		FramesWritten = m_CircularAudioBuffer->Write(FrameBuffer, FrameCount);
		return FramesWritten;
	}

	bool WriteAudioFrame( Frame_t Frame )
	{
		std::lock_guard<std::recursive_mutex> lock(m_Lock)
		bool Success = false;
		Success = m_CircularAudioBuffer->Write(Frame);
		return Success;
	}

	size_t ReadAudioFrames(Frame_t *FrameBuffer, size_t FrameCount)
	{
		std::lock_guard<std::recursive_mutex> lock(m_Lock)
		size_t FramesRead = 0;
		FramesRead = m_CircularAudioBuffer->Read(FrameBuffer, FrameCount);
		return FramesRead;
	}

	bfs::optional<Frame_t> ReadAudioFrame()
	{
		std::lock_guard<std::recursive_mutex> lock(m_Lock)
		bfs::optional<Frame_t> FrameRead;
		FrameRead = m_CircularAudioBuffer->Read();
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
	}

	void AllocateMemory()
    {
		std::lock_guard<std::recursive_mutex> lock(m_Lock)
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
		std::lock_guard<std::recursive_mutex> lock(m_Lock)
		if (m_CircularAudioBuffer != nullptr) {
            m_CircularAudioBuffer->~CircularBuffer<Frame_t, COUNT>();
            free(m_CircularAudioBuffer);
            m_CircularAudioBuffer = nullptr;
        }
	}
	
	uint32_t GetAudioFrames(Frame_t *Buffer, uint32_t Count)
	{
		std::lock_guard<std::recursive_mutex> lock(m_Lock)
		uint32_t ReturnCount = 0;
		for(int i = 0; i < Count && i < m_CircularAudioBuffer->size(); ++i)
		{
			Buffer[i] = ((Frame_t*)m_CircularAudioBuffer)[i];
			++ReturnCount;
		}
		return ReturnCount;
	}
	
	bool Push(Frame_t Frame)
	{
		std::lock_guard<std::recursive_mutex> lock(m_Lock)
		bool result = false;
		result = m_CircularAudioBuffer->push(Frame);
		if(result)
		{
			ESP_LOGD("Push", "Pushed %i|%i", Frame.channel1, Frame.channel2);
		}
		else
		{
			ESP_LOGD("Push", "Pushed and overwrote %i|%i", Frame.channel1, Frame.channel2);
		}
		return result;
	}

	size_t Push(Frame_t *Frame, size_t Count)
	{
		std::lock_guard<std::recursive_mutex> lock(m_Lock)
		size_t Result = 0;
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
		return Result;
	}

	Frame_t Pop()
	{
		std::lock_guard<std::recursive_mutex> lock(m_Lock)
		Frame_t Result;
		Result = m_CircularAudioBuffer->pop();
		return Result;
	}

	size_t Pop(Frame_t *Frame, size_t Count)
	{
		std::lock_guard<std::recursive_mutex> lock(m_Lock)
		size_t Result = 0;
		for(int i = 0; i < Count; ++i)
		{
			if(0 < COUNT - m_CircularAudioBuffer->available())
			{
				Frame[i] = m_CircularAudioBuffer->pop();
				ESP_LOGD("Pop", "Popped %i|%i", Frame[i].channel1, Frame[i].channel2);
				++Result;
			}
		}
		return Result;
	}

	bool Unshift(Frame_t Frame)
	{
		std::lock_guard<std::recursive_mutex> lock(m_Lock)
		bool result = false;
		result = m_CircularAudioBuffer->unshift(Frame);
		if(result)
		{
			ESP_LOGD("Push", "Unshifted %i|%i", Frame.channel1, Frame.channel2);
		}
		else
		{
			ESP_LOGD("Push", "Unshifted and overwrote %i|%i", Frame.channel1, Frame.channel2);
		}
		return result;
	}

	size_t Unshift(Frame_t *Frame, size_t Count)
	{
		std::lock_guard<std::recursive_mutex> lock(m_Lock)
		size_t Result = 0;
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
		return Result;
	}

	Frame_t Shift()
	{
		std::lock_guard<std::recursive_mutex> lock(m_Lock)
		Frame_t Result;
		Result = m_CircularAudioBuffer->shift();
		return Result;
	}

	size_t Shift(Frame_t *Frame, size_t Count)
	{
		std::lock_guard<std::recursive_mutex> lock(m_Lock)
		size_t Result = 0;
		for(int i = 0; i < Count; ++i)
		{
			if( 0 < COUNT - m_CircularAudioBuffer->available() )
			{
				Frame[i] = m_CircularAudioBuffer->shift();
				++Result;
			}
		}
		return Result;
	}

	bool IsEmpty()
	{
		std::lock_guard<std::recursive_mutex> lock(m_Lock)
		bool Result = false;
		Result = m_CircularAudioBuffer->isEmpty();
		return Result;
	}

	bool IsFull()
	{
		std::lock_guard<std::recursive_mutex> lock(m_Lock)
		bool Result = false;
		Result = m_CircularAudioBuffer->isFull();
		return Result;
	}

	size_t Size()
	{
		std::lock_guard<std::recursive_mutex> lock(m_Lock)
		size_t Result = 0;
		Result = m_CircularAudioBuffer->size();
		return Result;
	}

	size_t Available()
	{
		std::lock_guard<std::recursive_mutex> lock(m_Lock)
		size_t Result = 0;
		Result = m_CircularAudioBuffer->available();
		return Result;
	}

	void Clear()
	{
		std::lock_guard<std::recursive_mutex> lock(m_Lock)
		m_CircularAudioBuffer->clear();
	}
	  private:
		CircularBuffer<Frame_t, COUNT> *m_CircularAudioBuffer = nullptr;
		std::recursive_mutex m_Lock;
};

#endif
