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
#include <memory>
#include <vector>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

class ShocksRingBuffer {
public:
    ShocksRingBuffer(size_t size)
        : bufferSize(size), writeIndex(0), readIndex(0),
          p_buffer(std::make_unique<Frame_t[]>(size)) {
        mutex = xSemaphoreCreateMutex();
        if (!mutex) {
            throw std::runtime_error("Failed to create mutex");
        }
    }

    ~ShocksRingBuffer() {
        if (mutex) {
            vSemaphoreDelete(mutex);
            mutex = nullptr;
        }
    }

    // Push single frame
    void push(Frame_t frame, TickType_t waitTime) {
        if (xSemaphoreTake(mutex, waitTime) == pdTRUE) {
            p_buffer[writeIndex] = frame;
            writeIndex = (writeIndex + 1) % bufferSize;
            if (writeIndex == readIndex) { // Overwrite logic
                readIndex = (readIndex + 1) % bufferSize;
            }
            xSemaphoreGive(mutex);
        }
    }

    // Push multiple frames
    size_t push(Frame_t* frames, size_t count, TickType_t waitTime) {
        size_t pushed = 0;
        if (frames && xSemaphoreTake(mutex, waitTime) == pdTRUE) {
            for (size_t i = 0; i < count; ++i) {
                p_buffer[writeIndex] = frames[i];
                writeIndex = (writeIndex + 1) % bufferSize;
                if (writeIndex == readIndex) { // Overwrite logic
                    readIndex = (readIndex + 1) % bufferSize;
                }
                pushed++;
            }
            xSemaphoreGive(mutex);
        }
        return pushed;
    }

    // Retrieve frames into a vector
    size_t get(std::vector<Frame_t>& frames, size_t count, TickType_t waitTime) {
        size_t returned = 0;
        if (xSemaphoreTake(mutex, waitTime) == pdTRUE) {
            size_t available = getAvailableFrames();
            size_t framesToReturn = std::min(count, available);
            frames.resize(framesToReturn);
            for (size_t i = 0; i < framesToReturn; ++i) {
                frames[i] = p_buffer[(readIndex + i) % bufferSize];
            }
            returned = framesToReturn;
            xSemaphoreGive(mutex);
        }
        return returned;
    }

    // Retrieve frames into a raw array
    size_t get(Frame_t* frames, size_t count, TickType_t waitTime) {
        size_t returned = 0;
        if (xSemaphoreTake(mutex, waitTime) == pdTRUE) {
            size_t available = getAvailableFrames();
            size_t framesToReturn = std::min(count, available);
            for (size_t i = 0; i < framesToReturn; ++i) {
                frames[i] = p_buffer[(readIndex + i) % bufferSize];
            }
            returned = framesToReturn;
            xSemaphoreGive(mutex);
        }
        return returned;
    }

private:
    size_t getAvailableFrames() const {
        return (writeIndex >= readIndex)
            ? (writeIndex - readIndex)
            : (bufferSize - readIndex + writeIndex);
    }

    std::unique_ptr<Frame_t[]> p_buffer;
    size_t bufferSize;
    size_t writeIndex;
    size_t readIndex;
    SemaphoreHandle_t mutex = nullptr;
};