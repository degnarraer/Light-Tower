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

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <thread>
#include <chrono>
#include <vector>
#include "FFT_Computer.h"

using ::testing::_;
using ::testing::NotNull;
using ::testing::NiceMock;
using namespace testing;

class FFT_Computer_Tests : public Test
{
    protected:
        void SetUp() override
        {
            ESP_LOGD("SetUp", "SetUp");
        }
        
        void TearDown() override 
        {
            ESP_LOGD("TearDown", "TearDown");
            if(mp_FFT_Computer)
            {
                ESP_LOGD("TestFFT", "Delete FFT Computer");
                delete mp_FFT_Computer;
                mp_FFT_Computer = nullptr;
            }
        }
        void TestFFT(int fftSize, int hopSize, float f_s, float f_signal, float magnitude, bool normalizeMagnitudes, DataWidth_t dataWidth)
        {
            m_IsThreaded = false;
            RunTest(fftSize, hopSize, f_s, f_signal, magnitude, normalizeMagnitudes, dataWidth, THREAD_PRIORITY_IDLE, tskNO_AFFINITY);
        }
        void TestFFT(int fftSize, int hopSize, float f_s, float f_signal, float magnitude, bool normalizeMagnitudes, DataWidth_t dataWidth, UBaseType_t uxPriority, BaseType_t xCoreID)
        {
            m_IsThreaded = true;
            RunTest(fftSize, hopSize, f_s, f_signal, magnitude, normalizeMagnitudes, dataWidth, uxPriority, xCoreID);
        }
        

    protected:
        FFT_Computer* mp_FFT_Computer = nullptr;
        
        static void Static_FFT_Results_Callback(float *leftMagnitudes, float* sortedFrequenciesLeft, float* rightMagnitudes, float* sortedFrequenciesRight, size_t count, void* args)
        {
            FFT_Computer_Tests* aTest = static_cast<FFT_Computer_Tests*>(args);
            aTest->FFT_Results_Callback(leftMagnitudes, sortedFrequenciesLeft, rightMagnitudes, sortedFrequenciesRight, count);
        }

    private:
        std::vector<float> mp_Sine_Table;
        std::vector<Frame_t> mp_Frames;
        int m_FFT_Size = 0;
        int m_Hop_Size = 0;
        bool m_IsThreaded = false;
        float m_max_Freq_Result_Left = 0.0f;
        float m_max_Freq_Result_Right = 0.0f;

        void FFT_Results_Callback(float *leftMagnitudes, float* sortedFrequenciesLeft, float* rightMagnitudes, float* sortedFrequenciesRight, size_t count)
        {
            m_max_Freq_Result_Left = sortedFrequenciesLeft[0];
            m_max_Freq_Result_Right = sortedFrequenciesRight[0];
        }

        void RunTest(int fftSize, int hopSize, float f_s, float f_signal, float magnitude, bool normalizeMagnitudes, DataWidth_t dataWidth, UBaseType_t uxPriority, BaseType_t xCoreID)
        {
            ESP_LOGD("TestFFT", "TestFFT");
            m_FFT_Size = fftSize;
            m_Hop_Size = hopSize;

            if(m_IsThreaded)
            {
                ESP_LOGD("TestFFT", "Threaded");
                mp_FFT_Computer = new FFT_Computer(fftSize, hopSize, f_s, normalizeMagnitudes, dataWidth, uxPriority, xCoreID);
            }
            else
            {
                ESP_LOGD("TestFFT", "Unthreaded");
                mp_FFT_Computer = new FFT_Computer(fftSize, hopSize, f_s, normalizeMagnitudes, dataWidth);
            }

            mp_FFT_Computer->Setup(Static_FFT_Results_Callback, this);
            CreateInputSignals(f_s, f_signal, magnitude);
            mp_FFT_Computer->PushFrames(mp_Frames.data(), m_FFT_Size);

            float allowed_Freq_Error = f_s / fftSize;
            float freq_Error_Left = f_signal - m_max_Freq_Result_Left;
            float freq_Error_Right = f_signal - m_max_Freq_Result_Right;
            ESP_LOGD("TestFFT", "Error Left: \"%f\" Error Right: \"%f\"", freq_Error_Left, freq_Error_Right);
            EXPECT_TRUE(freq_Error_Left <= allowed_Freq_Error);
            EXPECT_TRUE(freq_Error_Right <= allowed_Freq_Error);
        }

        void CreateInputSignals(float f_s, float f_signal, float magnitude)
        {
            ESP_LOGD("CreateInputSignals", "CreateInputSignals");

            // Ensure m_FFT_Size is correctly set before proceeding
            Serial.print("Input Signal: ");
            if (mp_Sine_Table.size() != m_FFT_Size)
            {
                // Resize the vectors to match the FFT size
                mp_Sine_Table.resize(m_FFT_Size);
                mp_Frames.resize(m_FFT_Size);

                // Calculate the sine wave at the specified signal frequency
                for (int i = 0; i < m_FFT_Size; ++i)
                {
                    // Calculate the phase angle for the sine wave based on the sample frequency (f_s) and signal frequency (f_signal)
                    float angle = -2.0f * M_PI * f_signal * i / f_s;  // Adjusted for f_signal and f_s

                    // Generate the sine wave value for this sample
                    float sinAngle = magnitude*sinf(angle);

                    // Store the sine wave value in the table and in the frames
                    mp_Sine_Table[i] = sinAngle;
                    mp_Frames[i].channel1 = sinAngle;
                    mp_Frames[i].channel2 = sinAngle;
                }
            }
        }
};


TEST_F(FFT_Computer_Tests, Allocation_DeAllocation)
{
    size_t freeMemoryBeforeAllocation = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    ESP_LOGD("FFT_Computer_Tests", "Free heap before allocation: %u bytes", freeMemoryBeforeAllocation);
    ESP_LOGD("FFT_Computer_Tests", "Starting Allocation_DeAllocation");
    mp_FFT_Computer = new FFT_Computer(128, 128, 44100, false, DataWidth_16);
    mp_FFT_Computer->Setup(Static_FFT_Results_Callback, this);
    size_t freeMemoryAfterAllocation = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    ESP_LOGD("FFT_Computer_Tests", "Free heap after allocation: %u bytes", freeMemoryAfterAllocation);
    delete mp_FFT_Computer;
    mp_FFT_Computer = nullptr;
    size_t freeMemoryAfterDeallocation = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    ESP_LOGD("FFT_Computer_Tests", "Free heap after deallocation: %u bytes", freeMemoryAfterDeallocation);
    EXPECT_TRUE(freeMemoryAfterAllocation == freeMemoryBeforeAllocation);
    EXPECT_TRUE(freeMemoryBeforeAllocation == freeMemoryAfterDeallocation);
}

TEST_F(FFT_Computer_Tests, FFT_Calculation_Testing)
{
    TestFFT(128, 128, 44100, 1000, 100, true, DataWidth_16);
    TestFFT(128, 128, 44100, 2000, 100, true, DataWidth_16);
    TestFFT(128, 128, 44100, 3000, 100, true, DataWidth_16);
    TestFFT(128, 128, 44100, 4000, 100, true, DataWidth_16);
    TestFFT(128, 128, 44100, 5000, 100, true, DataWidth_16);
    TestFFT(128, 128, 44100, 6000, 100, true, DataWidth_16);
    TestFFT(128, 128, 44100, 7000, 100, true, DataWidth_16);
    TestFFT(128, 128, 44100, 8000, 100, true, DataWidth_16);
    TestFFT(128, 128, 44100, 9000, 100, true, DataWidth_16);
    TestFFT(128, 128, 44100, 10000, 100, true, DataWidth_16);
    TestFFT(128, 128, 44100, 11000, 100, true, DataWidth_16);
    TestFFT(128, 128, 44100, 12000, 100, true, DataWidth_16);
    TestFFT(128, 128, 44100, 13000, 100, true, DataWidth_16);
    TestFFT(128, 128, 44100, 14000, 100, true, DataWidth_16);
    TestFFT(128, 128, 44100, 15000, 100, true, DataWidth_16);
    TestFFT(128, 128, 44100, 16000, 100, true, DataWidth_16);
    TestFFT(128, 128, 44100, 17000, 100, true, DataWidth_16);
    TestFFT(128, 128, 44100, 18000, 100, true, DataWidth_16);
    TestFFT(128, 128, 44100, 19000, 100, true, DataWidth_16);
    TestFFT(128, 128, 44100, 20000, 100, true, DataWidth_16);
    TestFFT(128, 128, 44100, 21000, 100, true, DataWidth_16);
    TestFFT(128, 128, 44100, 22000, 100, true, DataWidth_16);
}

