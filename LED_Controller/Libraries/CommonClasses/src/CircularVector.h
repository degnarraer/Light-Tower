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

#pragma once

#include "DataTypes.h"

class ShocksRingBuffer {
public:
    ShocksRingBuffer(size_t size): bufferSize(size)
                                 , writeIndex(0)
                                 , readIndex(0)
    {
        p_buffer = (Frame_t*)ps_malloc(size);
        mutex = xSemaphoreCreateMutex();
    }

    ~ShocksRingBuffer() {
        if(p_buffer)
        {
            free(p_buffer);
            p_buffer = nullptr;
        }
        if(mutex)
        {
            vSemaphoreDelete(mutex);
            mutex = nullptr;
        }
    }

    // Push new frame into the buffer (overwrites if full)
    void push(Frame_t frame, TickType_t waitTime)
    {
        if (xSemaphoreTake(mutex, waitTime) == pdTRUE)
        {
            p_buffer[writeIndex] = frame;
            writeIndex = (writeIndex + 1) % bufferSize;
            if (writeIndex == readIndex)
            {
                readIndex = (readIndex + 1) % bufferSize;
            }
            xSemaphoreGive(mutex);
        }
    }

    size_t push(Frame_t* frames, size_t count, TickType_t waitTime)
    {
        size_t pushed = 0;
        if (frames && xSemaphoreTake(mutex, waitTime) == pdTRUE)
        {
            for (size_t i = 0; i < count; ++i)
            {
                p_buffer[writeIndex] = frames[i];
                writeIndex = (writeIndex + 1) % bufferSize;
                if (writeIndex == readIndex)
                {
                    readIndex = (readIndex + 1) % bufferSize;
                }
                pushed++;
            }
            xSemaphoreGive(mutex);
        }
        return pushed;
    }

    size_t get(std::vector<Frame_t>& frames, size_t count, TickType_t waitTime) {
        size_t returned = 0;
        if (xSemaphoreTake(mutex, waitTime) == pdTRUE) {
            size_t available = (writeIndex >= readIndex) 
                ? (writeIndex - readIndex) 
                : (bufferSize - readIndex + writeIndex);

            size_t framesToReturn = std::min(count, available);
            frames.resize(framesToReturn);
            size_t tempIndex = readIndex; // Use a temporary index for reading
            for (size_t i = 0; i < framesToReturn; ++i) {
                frames[i] = p_buffer[tempIndex];
                tempIndex = (tempIndex + 1) % bufferSize;
            }
            returned = framesToReturn;
            xSemaphoreGive(mutex);
        }
        return returned;
    }

    size_t get(Frame_t* frames, size_t count, TickType_t waitTime) {
        size_t returned = 0;
        if (xSemaphoreTake(mutex, waitTime) == pdTRUE) {
            size_t available = (writeIndex >= readIndex) 
                ? (writeIndex - readIndex) 
                : (bufferSize - readIndex + writeIndex);

            size_t framesToReturn = std::min(count, available);
            size_t tempIndex = readIndex; // Use a temporary index for reading
            for (size_t i = 0; i < framesToReturn; ++i) {
                frames[i] = p_buffer[tempIndex];
                tempIndex = (tempIndex + 1) % bufferSize;
            }
            returned = framesToReturn;
            xSemaphoreGive(mutex);
        }
        return returned;
    }

private:
    Frame_t* p_buffer = nullptr;
    size_t bufferSize;
    size_t writeIndex;
    size_t readIndex;
    SemaphoreHandle_t mutex = nullptr;
};