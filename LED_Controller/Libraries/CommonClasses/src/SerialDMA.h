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

#include <HardwareSerial.h>
#include <driver/uart.h>
#include <freertos/queue.h>
#include <functional>
#include "DataTypes.h"

class SerialDMA
{
    private:
        uart_port_t uartNum;                                            // UART port (e.g., UART_NUM_1)
        QueueHandle_t rxQueue;                                          // Queue to handle UART events
        TaskHandle_t uartEventTaskHandle;
        QueueHandle_t txQueue;                                          // Queue for TX data
        TaskHandle_t txTaskHandle;
        static constexpr size_t TX_QUEUE_SIZE = 10;                     // Maximum number of messages in the queue
        static constexpr size_t BUF_SIZE = 2048;                        // DMA buffer size
        std::function<void(const std::string&, void*)> messageCallback; // Callback for new messages
        void* callbackArg;                                              // Argument passed to the callback
        BaseType_t threadPriority;

        void initUART(int rxPin, int txPin, int baudRate)
        {
            txQueue = xQueueCreate(TX_QUEUE_SIZE, sizeof(char*));       // Store pointers to buffers
            if (!txQueue)
            {
                ESP_LOGE("SerialDMA", "Failed to create TX queue");
            }

            uart_config_t uartConfig = {
                .baud_rate = baudRate,
                .data_bits = UART_DATA_8_BITS,
                .parity = UART_PARITY_ODD,
                .stop_bits = UART_STOP_BITS_2,
                .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
                .source_clk = UART_SCLK_APB,
            };
            uart_param_config(uartNum, &uartConfig);
            uart_set_pin(uartNum, txPin, rxPin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
            uart_driver_install(uartNum, BUF_SIZE, BUF_SIZE, 10, &rxQueue, 0);
            uart_enable_pattern_det_intr(uartNum, '\n', 1, 100, 10, 1);
            uart_pattern_queue_reset(uartNum, 10);
        }

        static void uartEventTask(void* pvParameters)
        {
            SerialDMA* instance = static_cast<SerialDMA*>(pvParameters);
            uart_event_t event;

            uint8_t data[BUF_SIZE];
            while (true)
            {
                if(xQueueReceive(instance->rxQueue, (void*)&event, SEMAPHORE_BLOCK) == pdTRUE)
                {
                    switch(event.type)
                    {
                        case UART_DATA:
                            ESP_LOGD("uartEventTask", "UART_DATA");
                        break;
                        case UART_BREAK:
                            ESP_LOGI("uartEventTask", "UART_BREAK");
                        break;
                        case UART_BUFFER_FULL:
                            ESP_LOGI("uartEventTask", "UART_BUFFER_FULL");
                            instance->flush();
                        break;
                        case UART_FIFO_OVF:
                            ESP_LOGI("uartEventTask", "UART_FIFO_OVF");
                            instance->flush();
                        break;
                        case UART_FRAME_ERR:
                            ESP_LOGE("uartEventTask", "UART_FRAME_ERR");
                            instance->flush();
                        break;
                        case UART_PARITY_ERR:
                            ESP_LOGE("uartEventTask", "UART_FRAME_ERR");
                            instance->flush();
                        break;
                        case UART_DATA_BREAK:
                            ESP_LOGI("uartEventTask", "UART_FRAME_ERR");
                            instance->flush();
                        break;
                        case UART_PATTERN_DET:
                            ESP_LOGI("uartEventTask", "UART_PATTERN_DET");
                            {
                                while (true)
                                {
                                    int pos = uart_pattern_pop_pos(instance->uartNum);
                                    if (pos == -1)
                                    {
                                        break;
                                    }

                                    int len = uart_read_bytes(instance->uartNum, data, pos + 1, 0);
                                    if (len > 0)
                                    {
                                        data[len] = '\0';
                                        if (instance->messageCallback)
                                        {
                                            ESP_LOGD("uartEventTask: UART_PATTERN_DET", "%s", (const char*)data);
                                            instance->messageCallback(std::string((const char*)data), instance->callbackArg);
                                        }
                                    }
                                }
                            }
                        break;
                        case UART_EVENT_MAX:
                        default:
                            ESP_LOGE("uartEventTask", "UART_PATTERN_DET");
                        break;
                    }
                }
            }
        }

    public:
        SerialDMA( int rxPin
                 , int txPin
                 , int baudRate
                 , std::function<void(const std::string&, void*)> callback
                 , void* arg
                 , uart_port_t port
                 , BaseType_t priority )
                 : uartNum(port)
                 , messageCallback(callback)
                 , callbackArg(arg)
                 , threadPriority(priority)
        {
            initUART(rxPin, txPin, baudRate);
        }

        ~SerialDMA()
        {
            uart_driver_delete(uartNum);
            if(uartEventTaskHandle)
            {
                vTaskDelete(uartEventTaskHandle);
            }
            if(txTaskHandle)
            {
                vTaskDelete(txTaskHandle);
            }
        }

        void begin()
        {
            ESP_LOGI("begin", "Serial DMA Begin.");
            xTaskCreate(uartEventTask, "UART Rx Task", 4096, this, threadPriority, &uartEventTaskHandle);
            xTaskCreate(txTask, "UART TX Task", 4096, this, threadPriority, &txTaskHandle);
        }

        void flush()
        {
            ESP_LOGD("flush", "Serial DMA flush.");
            uart_flush(uartNum);

            uint8_t data[BUF_SIZE];
            size_t bufferedLen = 0;
            while (uart_get_buffered_data_len(uartNum, &bufferedLen) == ESP_OK && bufferedLen > 0)
            {
                uart_read_bytes(uartNum, data, BUF_SIZE, 0);
                uart_get_buffered_data_len(uartNum, &bufferedLen);
            }
        }

        void write(const std::string& data)
        {
            std::string dataWithNewline = data + '\n';
            char* buffer = (char*)malloc(dataWithNewline.length() + 1);
            if (buffer)
            {
                memcpy(buffer, dataWithNewline.c_str(), dataWithNewline.length() + 1);
                if (xQueueSend(txQueue, &buffer, 0) != pdTRUE)
                {
                    ESP_LOGE("write", "TX queue full, dropping message");
                    free(buffer);
                }
            }
            else
            {
                ESP_LOGE("write", "Failed to allocate memory for TX buffer");
            }
        }

        void write(const uint8_t* data, size_t length)
        {
            uint8_t* bufferWithNewline = (uint8_t*)malloc(length + 1);
            if (bufferWithNewline) {
                memcpy(bufferWithNewline, data, length);
                bufferWithNewline[length] = '\n';
                if (xQueueSend(txQueue, &bufferWithNewline, 0) != pdTRUE)
                {
                    ESP_LOGE("write", "TX queue full, dropping message");
                    free(bufferWithNewline);
                }
            }
            else
            {
                ESP_LOGE("write", "Failed to allocate memory for TX buffer");
            }
        }

        static void txTask(void* pvParameters)
        {
            SerialDMA* instance = static_cast<SerialDMA*>(pvParameters);
            uint8_t* buffer = nullptr;
            while (true)
            {
                if (xQueueReceive(instance->txQueue, &buffer, portMAX_DELAY) == pdTRUE)
                {
                    uart_write_bytes(instance->uartNum, (const char*)buffer, strlen((const char*)buffer));
                    free(buffer);
                }
            }
        }
};
