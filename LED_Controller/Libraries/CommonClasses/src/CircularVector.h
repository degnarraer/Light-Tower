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
    ShocksRingBuffer(size_t size) 
        : buffer(new Frame_t[size]), bufferSize(size), writeIndex(0), readIndex(0) {
        mutex = xSemaphoreCreateRecursiveMutex();
    }

    ~ShocksRingBuffer() {
        delete[] buffer;
        vSemaphoreDelete(mutex);
    }

    // Push new frame into the buffer (overwrites if full)
    void push(Frame_t frame, TickType_t waitTime) {
        if (xSemaphoreTakeRecursive(mutex, waitTime) == pdTRUE) {
            buffer[writeIndex] = frame;
            writeIndex = (writeIndex + 1) % bufferSize;
            if (writeIndex == readIndex) {
                readIndex = (readIndex + 1) % bufferSize;
            }
            xSemaphoreGiveRecursive(mutex);
        }
    }

    size_t push(Frame_t* frames, size_t count, TickType_t waitTime) {
        size_t pushed = 0;
        if (frames && xSemaphoreTakeRecursive(mutex, waitTime) == pdTRUE) {
            for (size_t i = 0; i < count; ++i) {
                buffer[writeIndex] = frames[i];
                writeIndex = (writeIndex + 1) % bufferSize;
                if (writeIndex == readIndex) {
                    readIndex = (readIndex + 1) % bufferSize;
                }
                pushed++;
            }
            xSemaphoreGiveRecursive(mutex);
        }
        return pushed;
    }

    size_t get(std::vector<Frame_t>& frames, size_t count, TickType_t waitTime) {
        size_t returned = 0;
        if (xSemaphoreTakeRecursive(mutex, waitTime) == pdTRUE) {
            size_t available = (writeIndex >= readIndex) ? (writeIndex - readIndex) : (bufferSize - readIndex + writeIndex);
            size_t framesToReturn = (available >= count) ? count : available;
            frames.resize(framesToReturn);
            for (size_t i = 0; i < framesToReturn; ++i) {
                frames[i] = buffer[(readIndex + i) % bufferSize];
            }
            readIndex = (readIndex + framesToReturn) % bufferSize;
            returned = framesToReturn;
            xSemaphoreGiveRecursive(mutex);
        }
        return returned;
    }

private:
    Frame_t* buffer;
    size_t bufferSize;
    size_t writeIndex;
    size_t readIndex;
    SemaphoreHandle_t mutex;
};