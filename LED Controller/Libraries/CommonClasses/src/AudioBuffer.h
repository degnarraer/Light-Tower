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
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

template <uint32_t COUNT>
class AudioBuffer
{
  public:
    AudioBuffer() 
    {
        m_Lock = xSemaphoreCreateMutex();
        if (m_Lock == nullptr)
        {
            ESP_LOGE("AudioBuffer", "ERROR! Failed to create semaphore.");
        }
    }

    virtual ~AudioBuffer()
    {
        FreeMemory();
        if (m_Lock != nullptr)
        {
            vSemaphoreDelete(m_Lock);
            m_Lock = nullptr;
        }
    }

    void Initialize()
    {
        AllocateMemory();
    }

    void AllocateMemory()
    {
        if (xSemaphoreTakeRecursive(m_Lock, pdMS_TO_TICKS(100)) == pdTRUE)
        {
            ESP_LOGD("AllocateMemory", "Allocating memory");
            m_CircularAudioBuffer = new(std::nothrow) bfs::CircleBuf<Frame_t, COUNT>;

            if (m_CircularAudioBuffer == nullptr)
            {
                ESP_LOGE("AllocateMemory", "ERROR! Memory allocation failed.");
            }
            xSemaphoreGiveRecursive(m_Lock);
        }
        else
        {
            ESP_LOGW("Semaphore Take Failure", "WARNING! Failed to take Semaphore");
        }
    }

    void FreeMemory()
    {
        if (xSemaphoreTakeRecursive(m_Lock, pdMS_TO_TICKS(100)) == pdTRUE)
        {
            if (m_CircularAudioBuffer != nullptr)
            {
                delete m_CircularAudioBuffer;
                m_CircularAudioBuffer = nullptr;
            }
            xSemaphoreGiveRecursive(m_Lock);
        }
        else
        {
            ESP_LOGW("Semaphore Take Failure", "WARNING! Failed to take Semaphore");
        }
    }

    size_t GetFrameCapacity()
    {
        size_t capacity = 0;
        if (xSemaphoreTakeRecursive(m_Lock, pdMS_TO_TICKS(100)) == pdTRUE)
        {
            capacity = m_CircularAudioBuffer->capacity();
            xSemaphoreGiveRecursive(m_Lock);
        }
        else
        {
            ESP_LOGW("Semaphore Take Failure", "WARNING! Failed to take Semaphore");
        }
        return capacity;
    }

    bool ClearAudioBuffer()
    {
        if (xSemaphoreTakeRecursive(m_Lock, pdMS_TO_TICKS(100)) == pdTRUE)
        {
            m_CircularAudioBuffer->Clear();
            xSemaphoreGiveRecursive(m_Lock);
            return true;
        }
        else
        {
            ESP_LOGW("Semaphore Take Failure", "WARNING! Failed to take Semaphore");
        }
        return false;
    }

    size_t GetFrameCount()
    {
        size_t count = 0;
        if (xSemaphoreTakeRecursive(m_Lock, pdMS_TO_TICKS(100)) == pdTRUE)
        {
            count = m_CircularAudioBuffer->size();
            xSemaphoreGiveRecursive(m_Lock);
        }
        else
        {
            ESP_LOGW("Semaphore Take Failure", "WARNING! Failed to take Semaphore");
        }
        return count;
    }

    size_t GetFreeFrameCount()
    {
        return GetFrameCapacity() - GetFrameCount();
    }

    size_t WriteAudioFrames(Frame_t* FrameBuffer, size_t FrameCount)
    {
        size_t written = 0;
        if (xSemaphoreTakeRecursive(m_Lock, pdMS_TO_TICKS(100)) == pdTRUE)
        {
            written = m_CircularAudioBuffer->Write(FrameBuffer, FrameCount);
            xSemaphoreGiveRecursive(m_Lock);
        }
        else
        {
            ESP_LOGW("Semaphore Take Failure", "WARNING! Failed to take Semaphore");
        }
        return written;
    }

    bool WriteAudioFrame(Frame_t Frame)
    {
        bool result = false;
        if (xSemaphoreTakeRecursive(m_Lock, pdMS_TO_TICKS(100)) == pdTRUE)
        {
            result = m_CircularAudioBuffer->Write(Frame);
            xSemaphoreGiveRecursive(m_Lock);
        }
        else
        {
            ESP_LOGW("Semaphore Take Failure", "WARNING! Failed to take Semaphore");
        }
        return result;
    }

    size_t ReadAudioFrames(Frame_t* FrameBuffer, size_t FrameCount)
    {
        size_t read = 0;
        if (xSemaphoreTakeRecursive(m_Lock, pdMS_TO_TICKS(100)) == pdTRUE)
        {
            read = m_CircularAudioBuffer->Read(FrameBuffer, FrameCount);
            xSemaphoreGiveRecursive(m_Lock);
        }
        else
        {
            ESP_LOGW("Semaphore Take Failure", "WARNING! Failed to take Semaphore");
        }
        return read;
    }

    bfs::optional<Frame_t> ReadAudioFrame()
    {
        bfs::optional<Frame_t> frame;
        if (xSemaphoreTakeRecursive(m_Lock, pdMS_TO_TICKS(100)) == pdTRUE)
        {
            frame = m_CircularAudioBuffer->Read();
            xSemaphoreGiveRecursive(m_Lock);
        }
        else
        {
            ESP_LOGW("Semaphore Take Failure", "WARNING! Failed to take Semaphore");
        }
        return frame;
    }

  private:
    bfs::CircleBuf<Frame_t, COUNT>* m_CircularAudioBuffer = nullptr;
    SemaphoreHandle_t m_Lock;
};

template <uint32_t COUNT>
class ContinuousAudioBuffer
{
  public:
    ContinuousAudioBuffer() 
    {
        m_Lock = xSemaphoreCreateMutex();
        if (m_Lock == nullptr)
        {
            ESP_LOGE("ContinuousAudioBuffer", "ERROR! Failed to create semaphore.");
        }
    }

    virtual ~ContinuousAudioBuffer()
    {
        FreeMemory();
        if (m_Lock)
        {
            vSemaphoreDelete(m_Lock);
            m_Lock = nullptr;
        }
    }

    void Initialize()
    {
        AllocateMemory();
    }

    void AllocateMemory()
    {
        if (xSemaphoreTakeRecursive(m_Lock, pdMS_TO_TICKS(100)) == pdTRUE)
        {
            ESP_LOGD("AllocateMemory", "Allocating memory");
            m_CircularAudioBuffer = new(std::nothrow) CircularBuffer<Frame_t, COUNT>;
            if (m_CircularAudioBuffer == nullptr)
            {
                ESP_LOGE("AllocateMemory", "ERROR! Memory allocation failed.");
            }
            xSemaphoreGiveRecursive(m_Lock);
        }
        else
        {
            ESP_LOGW("Semaphore Take Failure", "WARNING! Failed to take Semaphore");
        }
    }

    void FreeMemory()
    {
        if (xSemaphoreTakeRecursive(m_Lock, pdMS_TO_TICKS(100)) == pdTRUE)
        {
            if (m_CircularAudioBuffer != nullptr)
            {
                delete m_CircularAudioBuffer;
                m_CircularAudioBuffer = nullptr;
            }
            xSemaphoreGiveRecursive(m_Lock);
        }
        else
        {
            ESP_LOGW("Semaphore Take Failure", "WARNING! Failed to take Semaphore");
        }
    }

    bool Push(Frame_t Frame)
    {
        bool result = false;
        if (xSemaphoreTakeRecursive(m_Lock, pdMS_TO_TICKS(100)) == pdTRUE)
        {
            result = m_CircularAudioBuffer->push(Frame);
            xSemaphoreGiveRecursive(m_Lock);
        }
        else
        {
            ESP_LOGW("Semaphore Take Failure", "WARNING! Failed to take Semaphore");
        }
        return result;
    }

    size_t Push(Frame_t* Frames, size_t Count)
    {
        size_t pushed = 0;
        if (xSemaphoreTakeRecursive(m_Lock, pdMS_TO_TICKS(100)) == pdTRUE)
        {
            for (size_t i = 0; i < Count; ++i)
            {
                if (m_CircularAudioBuffer->push(Frames[i]))
                {
                    ++pushed;
                }
            }
            xSemaphoreGiveRecursive(m_Lock);
        }
        else
        {
            ESP_LOGW("Semaphore Take Failure", "WARNING! Failed to take Semaphore");
        }
        return pushed;
    }

    Frame_t Pop()
    {
        Frame_t frame;
        if (xSemaphoreTakeRecursive(m_Lock, pdMS_TO_TICKS(100)) == pdTRUE)
        {
            frame = m_CircularAudioBuffer->pop();
            xSemaphoreGiveRecursive(m_Lock);
        }
        else
        {
            ESP_LOGW("Semaphore Take Failure", "WARNING! Failed to take Semaphore");
        }
        return frame;
    }

    size_t Pop(Frame_t* Frames, size_t Count)
    {
        size_t popped = 0;
        if (xSemaphoreTakeRecursive(m_Lock, pdMS_TO_TICKS(100)) == pdTRUE)
        {
            for (size_t i = 0; i < Count; ++i)
            {
                if (m_CircularAudioBuffer->available() > 0)
                {
                    Frames[i] = m_CircularAudioBuffer->pop();
                    ++popped;
                }
            }
            xSemaphoreGiveRecursive(m_Lock);
        }
        else
        {
            ESP_LOGW("Semaphore Take Failure", "WARNING! Failed to take Semaphore");
        }
        return popped;
    }

    bool IsEmpty()
    {
        bool isEmpty = false;
        if (xSemaphoreTakeRecursive(m_Lock, pdMS_TO_TICKS(100)) == pdTRUE)
        {
            isEmpty = m_CircularAudioBuffer->isEmpty();
            xSemaphoreGiveRecursive(m_Lock);
        }
        else
        {
            ESP_LOGW("Semaphore Take Failure", "WARNING! Failed to take Semaphore");
        }
        return isEmpty;
    }

    size_t Size()
    {
        size_t size = 0;
        if (xSemaphoreTakeRecursive(m_Lock, pdMS_TO_TICKS(100)) == pdTRUE)
        {
            size = m_CircularAudioBuffer->size();
            xSemaphoreGiveRecursive(m_Lock);
        }
        else
        {
            ESP_LOGW("Semaphore Take Failure", "WARNING! Failed to take Semaphore");
        }
        return size;
    }
	
	size_t ReadAudioFrames(Frame_t *Buffer, uint32_t Count)
	{
		uint32_t ElementsToRead = 0;
        if (xSemaphoreTakeRecursive(m_Lock, pdMS_TO_TICKS(100)) == pdTRUE)
        {
			ElementsToRead = std::min(Count, static_cast<uint32_t>(m_CircularAudioBuffer->size()));
			uint32_t StartIndex = m_CircularAudioBuffer->size() - ElementsToRead;
			for(int i = 0; i < ElementsToRead; ++i)
			{
				Buffer[i] = (*m_CircularAudioBuffer)[StartIndex + i];
			}
            xSemaphoreGiveRecursive(m_Lock);
		}
        else
        {
            ESP_LOGW("Semaphore Take Failure", "WARNING! Failed to take Semaphore");
        }
		return ElementsToRead;
	}

  private:
    CircularBuffer<Frame_t, COUNT>* m_CircularAudioBuffer = nullptr;
    SemaphoreHandle_t m_Lock;  // FreeRTOS mutex with priority inheritance
};

#endif

