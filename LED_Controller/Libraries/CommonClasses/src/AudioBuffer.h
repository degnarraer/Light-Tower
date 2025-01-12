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
        if (xSemaphoreTakeRecursive(m_Lock, SEMAPHORE_SHORT_BLOCK) == pdTRUE)
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
        if (xSemaphoreTakeRecursive(m_Lock, SEMAPHORE_SHORT_BLOCK) == pdTRUE)
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
        if (xSemaphoreTakeRecursive(m_Lock, SEMAPHORE_SHORT_BLOCK) == pdTRUE)
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
        if (xSemaphoreTakeRecursive(m_Lock, SEMAPHORE_SHORT_BLOCK) == pdTRUE)
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
        if (xSemaphoreTakeRecursive(m_Lock, SEMAPHORE_SHORT_BLOCK) == pdTRUE)
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

    bool IsEmpty()
    {
        bool isEmpty = false;
        if (xSemaphoreTakeRecursive(m_Lock, SEMAPHORE_SHORT_BLOCK) == pdTRUE)
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

    bool IsFull()
    {
        bool isFull = false;
        if (xSemaphoreTakeRecursive(m_Lock, SEMAPHORE_SHORT_BLOCK) == pdTRUE)
        {
            isFull = m_CircularAudioBuffer->isFull();
            xSemaphoreGiveRecursive(m_Lock);
        }
        else
        {
            ESP_LOGW("Semaphore Take Failure", "WARNING! Failed to take Semaphore");
        }
        return isFull;
    }

    size_t WriteAudioFrames(Frame_t* FrameBuffer, size_t FrameCount)
    {
        size_t written = 0;
        if (xSemaphoreTakeRecursive(m_Lock, SEMAPHORE_SHORT_BLOCK) == pdTRUE)
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
        if (xSemaphoreTakeRecursive(m_Lock, SEMAPHORE_SHORT_BLOCK) == pdTRUE)
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
        if (xSemaphoreTakeRecursive(m_Lock, SEMAPHORE_SHORT_BLOCK) == pdTRUE)
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
        if (xSemaphoreTakeRecursive(m_Lock, SEMAPHORE_SHORT_BLOCK) == pdTRUE)
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

    void Setup()
    {
        AllocateMemory();
    }

    void AllocateMemory()
    {
        if (xSemaphoreTakeRecursive(m_Lock, SEMAPHORE_SHORT_BLOCK) == pdTRUE)
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
        if (xSemaphoreTakeRecursive(m_Lock, SEMAPHORE_SHORT_BLOCK) == pdTRUE)
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
        if (xSemaphoreTakeRecursive(m_Lock, SEMAPHORE_SHORT_BLOCK) == pdTRUE)
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
        if (xSemaphoreTakeRecursive(m_Lock, SEMAPHORE_SHORT_BLOCK) == pdTRUE)
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

    Frame_t Shift()
    {
        Frame_t frame;
        if (xSemaphoreTakeRecursive(m_Lock, SEMAPHORE_SHORT_BLOCK) == pdTRUE)
        {
            frame = m_CircularAudioBuffer->shift();
            xSemaphoreGiveRecursive(m_Lock);
        }
        else
        {
            ESP_LOGW("Semaphore Take Failure", "WARNING! Failed to take Semaphore");
        }
        return frame;
    }

    size_t Shift(Frame_t* Frames, size_t Count)
    {
        size_t shifted = 0;
        if (xSemaphoreTakeRecursive(m_Lock, SEMAPHORE_SHORT_BLOCK) == pdTRUE)
        {
            for (size_t i = 0; i < Count; ++i)
            {
                if (m_CircularAudioBuffer->size() > 0)
                {
                    Frames[i] = m_CircularAudioBuffer->shift();
                    ++shifted;
                }
            }
            xSemaphoreGiveRecursive(m_Lock);
        }
        else
        {
            ESP_LOGW("Semaphore Take Failure", "WARNING! Failed to take Semaphore");
        }
        return shifted;
    }

    bool Unshift(Frame_t Frame)
    {
        bool result = false;
        if (xSemaphoreTakeRecursive(m_Lock, SEMAPHORE_SHORT_BLOCK) == pdTRUE)
        {
            result = m_CircularAudioBuffer->unshift(Frame);
            xSemaphoreGiveRecursive(m_Lock);
        }
        else
        {
            ESP_LOGW("Semaphore Take Failure", "WARNING! Failed to take Semaphore");
        }
        return result;
    }

    size_t Unshift(Frame_t* Frames, size_t Count)
    {
        size_t pushed = 0;
        if (xSemaphoreTakeRecursive(m_Lock, SEMAPHORE_SHORT_BLOCK) == pdTRUE)
        {
            for (size_t i = 0; i < Count; ++i)
            {
                if (m_CircularAudioBuffer->unshift(Frames[i]))
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
        if (xSemaphoreTakeRecursive(m_Lock, SEMAPHORE_SHORT_BLOCK) == pdTRUE)
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
        if (xSemaphoreTakeRecursive(m_Lock, SEMAPHORE_SHORT_BLOCK) == pdTRUE)
        {
            for (size_t i = 0; i < Count; ++i)
            {
                if (m_CircularAudioBuffer->size() > 0)
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

    size_t GetFrameCapacity()
    {
        size_t frameCapacity = 0;
        if (xSemaphoreTakeRecursive(m_Lock, SEMAPHORE_SHORT_BLOCK) == pdTRUE)
        {
            frameCapacity = m_CircularAudioBuffer->capacity;
            xSemaphoreGiveRecursive(m_Lock);
        }
        else
        {
            ESP_LOGW("Semaphore Take Failure", "WARNING! Failed to take Semaphore");
        }
        return frameCapacity;
    }

    bool ClearAudioBuffer()
    {
        if (xSemaphoreTakeRecursive(m_Lock, SEMAPHORE_SHORT_BLOCK) == pdTRUE)
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
        if (xSemaphoreTakeRecursive(m_Lock, SEMAPHORE_SHORT_BLOCK) == pdTRUE)
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

    bool IsEmpty()
    {
        bool isEmpty = false;
        if (xSemaphoreTakeRecursive(m_Lock, SEMAPHORE_SHORT_BLOCK) == pdTRUE)
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

    bool IsFull()
    {
        bool isFull = false;
        if (xSemaphoreTakeRecursive(m_Lock, SEMAPHORE_SHORT_BLOCK) == pdTRUE)
        {
            isFull = m_CircularAudioBuffer->isFull();
            xSemaphoreGiveRecursive(m_Lock);
        }
        else
        {
            ESP_LOGW("Semaphore Take Failure", "WARNING! Failed to take Semaphore");
        }
        return isFull;
    }

    size_t Size()
    {
        size_t size = 0;
        if (xSemaphoreTakeRecursive(m_Lock, SEMAPHORE_SHORT_BLOCK) == pdTRUE)
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
        uint32_t elementsToRead = 0;

        if (xSemaphoreTakeRecursive(m_Lock, SEMAPHORE_SHORT_BLOCK) == pdTRUE)
        {
            uint32_t bufferSize = m_CircularAudioBuffer->size();
            
            if (bufferSize == 0)
            {
                xSemaphoreGiveRecursive(m_Lock); // Release lock if no data
                return 0;
            }

            elementsToRead = std::min(Count, bufferSize);
            
            for(int i = 0; i < elementsToRead; ++i)
            {
                Buffer[i] = m_CircularAudioBuffer->shift();
            }

            if (xSemaphoreGiveRecursive(m_Lock) != pdTRUE)
            {
                ESP_LOGE("Semaphore Release Failure", "Failed to release semaphore");
            }
        }
        else
        {
            ESP_LOGW("Semaphore Take Failure", "Task %s failed to acquire semaphore", pcTaskGetName(NULL));
            return 0;  // Return 0 if semaphore acquisition fails
        }

        return elementsToRead;
    }

    size_t WriteAudioFrames(const Frame_t *Buffer, uint32_t Count)
    {
        uint32_t ElementsWritten = 0;

        if (xSemaphoreTakeRecursive(m_Lock, SEMAPHORE_SHORT_BLOCK) == pdTRUE)
        {
            uint32_t bufferSize = m_CircularAudioBuffer->capacity; // Get the buffer's maximum size
            uint32_t currentSize = m_CircularAudioBuffer->size();   // Get the current number of elements in the buffer

            // Determine if we need to overwrite old data
            uint32_t spaceAvailable = bufferSize - currentSize;
            uint32_t elementsToWrite = std::min(Count, bufferSize);

            // If Count exceeds space available, the buffer will overwrite the oldest data
            if (Count > spaceAvailable)
            {
                // Shift the start index by the number of elements that will be overwritten
                for (uint32_t i = 0; i < Count - spaceAvailable; ++i)
                {
                    m_CircularAudioBuffer->shift(); // Discard oldest elements
                }
            }

            // Write elements to the circular buffer
            for (uint32_t i = 0; i < elementsToWrite; ++i)
            {
                m_CircularAudioBuffer->push(Buffer[i]);
            }

            ElementsWritten = elementsToWrite;

            if (xSemaphoreGiveRecursive(m_Lock) != pdTRUE)
            {
                ESP_LOGE("Semaphore Release Failure", "Failed to release semaphore after writing");
            }
        }
        else
        {
            ESP_LOGW("Semaphore Take Failure", "Task %s failed to acquire semaphore", pcTaskGetName(NULL));
            return 0;
        }

        return ElementsWritten;
    }

  private:
    CircularBuffer<Frame_t, COUNT>* m_CircularAudioBuffer = nullptr;
    SemaphoreHandle_t m_Lock;
};

#endif

