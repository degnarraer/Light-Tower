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

#include <Arduino.h>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include "DataTypes.h"

template <typename T>
class CircularVector 
{
    public:
        CircularVector(size_t size) : m_size(size), m_start(0), m_end(0), m_full(false) {
            m_buffer.resize(size, T());
            m_full = true;
        }
        
        void fill(const T& value)
        {
            std::fill(m_buffer.begin(), m_buffer.end(), value);
        }

        void push(const T& value)
        {
            if (m_full) {
                m_start = (m_start + 1) % m_size;
            }
            m_buffer[m_end] = value;
            m_end = (m_end + 1) % m_size;

            if (m_end == m_start) {
                m_full = true;
            }
        }

        T& get(size_t index)
        {
            if (index >= size()) {
                throw std::out_of_range("Index out of bounds");
            }
            return m_buffer[(m_start + index) % m_size];
        }

        const T& get(size_t index) const
        {
            if (index >= size()) {
                throw std::out_of_range("Index out of bounds");
            }
            return m_buffer[(m_start + index) % m_size];
        }

        size_t size() const
        {
            if (m_full) {
                return m_size;
            }
            if (m_end >= m_start) {
                return m_end - m_start;
            }
            return m_size + m_end - m_start;
        }

        bool isEmpty() const
        {
            return (!m_full && (m_start == m_end));
        }

    private:
        std::vector<T> m_buffer;  // Using vector to store the elements
        size_t m_size;
        size_t m_start;           // Index of the oldest element
        size_t m_end;             // Index of the next available position
        bool m_full;              // Whether the buffer is full
};


class ShocksRingBuffer {
public:
    ShocksRingBuffer(size_t size) 
        : buffer(new Frame_t[size]), bufferSize(size), writeIndex(0), readIndex(0) {
        vSemaphoreCreateBinary(mutex);
    }

    ~ShocksRingBuffer() {
        delete buffer;
        vSemaphoreDelete(mutex);
    }

    // Push new frame into the buffer (overwrites if full)
    void push(Frame_t frame) {
        if (xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE) {
            buffer[writeIndex] = frame;
            writeIndex = (writeIndex + 1) % bufferSize;
            if (writeIndex == readIndex) {
                readIndex = (readIndex + 1) % bufferSize;
            }
            xSemaphoreGive(mutex);
        }
    }

    size_t push(Frame_t* frames, size_t count) {
        size_t pushed = 0;
        if (xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE) {
            for (size_t i = 0; i < count; ++i) {
                buffer[writeIndex] = frames[i];
                writeIndex = (writeIndex + 1) % bufferSize;

                if (writeIndex == readIndex) {
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
            size_t available = (writeIndex >= readIndex) ? (writeIndex - readIndex) : (bufferSize - readIndex + writeIndex);
            size_t framesToReturn = (available >= count) ? count : available;
            frames.resize(framesToReturn);
            for (size_t i = 0; i < framesToReturn; ++i) {
                frames[i] = buffer[(readIndex + i) % bufferSize];
            }
            readIndex = (readIndex + framesToReturn) % bufferSize;
            returned = framesToReturn;
            xSemaphoreGive(mutex);
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