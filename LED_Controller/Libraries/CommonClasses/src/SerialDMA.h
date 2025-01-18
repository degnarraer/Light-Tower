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

class SerialDMA
{
    private:
        uart_port_t uartNum;                        // UART port (e.g., UART_NUM_1)
        QueueHandle_t uartQueue;                    // Queue to handle UART events
        static constexpr size_t BUF_SIZE = 1024;    // DMA buffer size
        std::function<void(const std::string&, void*)> messageCallback; // Callback for new messages
        void* callbackArg; // Argument passed to the callback

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
            uart_enable_pattern_det_intr(uartNum, '\n', 1, 10000, 10, 10);
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
                if (xQueueReceive(instance->uartQueue, (void*)&event, portMAX_DELAY))
                {
                    if (event.type == UART_PATTERN_DET)
                    {
                        // Determine the location of the pattern
                        int pos = uart_pattern_pop_pos(instance->uartNum);
                        if (pos != -1)
                        {
                            // Read message up to the pattern position
                            int len = uart_read_bytes(instance->uartNum, data, pos + 1, portMAX_DELAY);
                            data[len] = '\0'; // Null-terminate the message
                            if (instance->messageCallback)
                            {
                                ESP_LOGI("uartEventTask: UART_PATTERN_DET", "%s", (const char*)data);
                                instance->messageCallback(std::string((const char*)data), instance->callbackArg);
                            }
                        }
                        // Reset pattern queue after processing
                        uart_pattern_queue_reset(instance->uartNum, 10);
                    }
                    else if (event.type == UART_DATA)
                    {
                        // Handle timeout or buffer overflow
                        int length = uart_read_bytes(instance->uartNum, data, BUF_SIZE, 10 / portTICK_PERIOD_MS);
                        if (length > 0)
                        {
                            data[length] = '\0'; // Null-terminate the data
                            if (instance->messageCallback)
                            {
                                ESP_LOGI("uartEventTask: UART_DATA", "%s", (const char*)data);
                                instance->messageCallback(std::string((const char*)data), instance->callbackArg);
                            }
                        }
                    }
                }
            }
        }

    public:
        // Constructor: Use custom RX/TX pins
        SerialDMA(int rxPin, int txPin, int baudRate, std::function<void(const std::string&, void*)> callback, void* arg = nullptr, uart_port_t port = UART_NUM_1)
            : uartNum(port), messageCallback(callback), callbackArg(arg)
        {
            initUART(rxPin, txPin, baudRate);
        }

        ~SerialDMA()
        {
            uart_driver_delete(uartNum);
        }

        void begin()
        {
            // Create a FreeRTOS task to handle UART events
            ESP_LOGI("begin", "Serial DMA Begin.");
            xTaskCreate(uartEventTask, "UART Event Task", 5000, this, 12, nullptr);
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
            while (uart_get_buffered_data_len(uartNum, &bufferedLen) == ESP_OK && bufferedLen > 0) {
                uart_read_bytes(uartNum, data, BUF_SIZE, 0); // Non-blocking read to clear buffer
                uart_get_buffered_data_len(uartNum, &bufferedLen); // Update the buffered length
            }
        }

        // Method to send data over UART
        void write(const std::string& data)
        {
            ESP_LOGI("write", "%s", (const char*)data.c_str());
            uart_write_bytes(uartNum, data.c_str(), data.length());
        }

        // Alternative method to send data with a raw buffer
        void write(const uint8_t* data, size_t length)
        {
            ESP_LOG_BUFFER_HEXDUMP("write", data, length, ESP_LOG_INFO);
            uart_write_bytes(uartNum, (const char*)data, length);
        }
};
