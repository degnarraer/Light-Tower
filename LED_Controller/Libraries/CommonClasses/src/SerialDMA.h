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
        QueueHandle_t uartQueue;                                        // Queue to handle UART events
        TaskHandle_t taskHandle;
        static constexpr size_t BUF_SIZE = 2048;                        // DMA buffer size
        std::function<void(const std::string&, void*)> messageCallback; // Callback for new messages
        void* callbackArg;                                              // Argument passed to the callback
        BaseType_t threadPriority;

        void initUART(int rxPin, int txPin, int baudRate)
        {
            // Configure UART parameters
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
            uart_driver_install(uartNum, BUF_SIZE, BUF_SIZE, 10, &uartQueue, 0);

            // Enable pattern detection for '\n' (ASCII 10)
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
                // Wait for a UART event
                while (xQueueReceive(instance->uartQueue, (void*)&event, SEMAPHORE_NO_BLOCK) == pdTRUE)
                {
                    switch(event.type)
                    {
                        case UART_DATA:
                        break;
                        case UART_BREAK:
                            ESP_LOGD("uartEventTask", "UART_BREAK");
                        break;
                        case UART_BUFFER_FULL:
                            ESP_LOGD("uartEventTask", "UART_BUFFER_FULL");
                            instance->flush();
                        break;
                        case UART_FIFO_OVF:
                            ESP_LOGD("uartEventTask", "UART_FIFO_OVF");
                            instance->flush();
                        break;
                        case UART_FRAME_ERR:
                            ESP_LOGD("uartEventTask", "UART_FRAME_ERR");
                            instance->flush();
                        break;
                        case UART_PARITY_ERR:
                            ESP_LOGD("uartEventTask", "UART_FRAME_ERR");
                            instance->flush();
                        break;
                        case UART_DATA_BREAK:
                            ESP_LOGD("uartEventTask", "UART_FRAME_ERR");
                            instance->flush();
                        break;
                        case UART_PATTERN_DET:
                            ESP_LOGD("uartEventTask", "UART_PATTERN_DET");
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
                                    vTaskDelay(pdMS_TO_TICKS(1));
                                }
                            }
                        break;
                        case UART_EVENT_MAX:
                        default:
                            ESP_LOGI("uartEventTask", "UART_PATTERN_DET");
                        break;
                    }
                }
                vTaskDelay(pdMS_TO_TICKS(10));
            }
        }

    public:
        // Constructor: Use custom RX/TX pins
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
            if(taskHandle)
            {
                vTaskDelete(taskHandle);
            }
        }

        void begin()
        {
            // Create a FreeRTOS task to handle UART events
            ESP_LOGI("begin", "Serial DMA Begin.");
            xTaskCreate(uartEventTask, "UART Event Task", 5000, this, threadPriority, &taskHandle);
        }

        // Flush the UART buffers (TX and RX)
        void flush()
        {
            ESP_LOGD("flush", "Serial DMA flush.");
            // Flush TX buffer
            uart_flush(uartNum);

            // Clear the RX buffer by reading all available data
            uint8_t data[BUF_SIZE];
            size_t bufferedLen = 0;
            while (uart_get_buffered_data_len(uartNum, &bufferedLen) == ESP_OK && bufferedLen > 0)
            {
                uart_read_bytes(uartNum, data, BUF_SIZE, 0); // Non-blocking read to clear buffer
                uart_get_buffered_data_len(uartNum, &bufferedLen); // Update the buffered length
            }
        }

        // Method to send data over UART
        void write(const std::string& data)
        {
            ESP_LOGD("write", "%s", (const char*)data.c_str());
            std::string dataWithNewline = data + '\n';
            uart_write_bytes(uartNum, dataWithNewline.c_str(), dataWithNewline.length());
        }

        // Alternative method to send data with a raw buffer
        void write(const uint8_t* data, size_t length)
        {
            // Create a new buffer to hold the original data + '\n'
            uint8_t* bufferWithNewline = (uint8_t*)malloc(length + 1);
            if (bufferWithNewline)
            {
                memcpy(bufferWithNewline, data, length);  // Copy the original data
                bufferWithNewline[length] = '\n';         // Append '\n'

                ESP_LOG_BUFFER_HEXDUMP("write", bufferWithNewline, length + 1, ESP_LOG_DEBUG);
                uart_write_bytes(uartNum, (const char*)bufferWithNewline, length + 1);

                free(bufferWithNewline);  // Free the allocated memory
            }
            else
            {
                ESP_LOGE("write", "Failed to allocate memory for bufferWithNewline");
            }
        }
};
