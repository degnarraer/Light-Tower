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
        
        static void Static_FFT_Results_Callback(const FFT_Bin_Data_Set_t &FFT_Bin_Data, void* args)
        {
            FFT_Computer_Tests* aTest = static_cast<FFT_Computer_Tests*>(args);
            aTest->FFT_Results_Callback(FFT_Bin_Data);
        }

    private:
        std::vector<float> mp_Sine_Table;
        std::vector<Frame_t> mp_Frames;
        int m_FFT_Size = 0;
        int m_Hop_Size = 0;
        bool m_IsThreaded = false;
        FFT_Bin_Data_t m_max_Freq_Result_Left;
        FFT_Bin_Data_t m_max_Freq_Result_Right;

        void FFT_Results_Callback(const FFT_Bin_Data_Set_t &FFT_Bin_Data)
        {
            ESP_LOGD("FFT_Results_Callback", "FFT_Results_Callback");
            m_max_Freq_Result_Left = (*FFT_Bin_Data.Left_Channel)[FFT_Bin_Data.MaxLeftBin];
            m_max_Freq_Result_Right = (*FFT_Bin_Data.Right_Channel)[FFT_Bin_Data.MaxRightBin];
            delete FFT_Bin_Data.Left_Channel;
            delete FFT_Bin_Data.Right_Channel;
        }

        void RunTest(int fftSize, int hopSize, float f_s, float f_signal, float magnitude, bool normalizeMagnitudes, DataWidth_t dataWidth, UBaseType_t uxPriority, BaseType_t xCoreID)
        {
            ESP_LOGD("TestFFT", "TestFFT");
            m_FFT_Size = fftSize;
            m_Hop_Size = hopSize;
            if(mp_FFT_Computer)
            {
                delete mp_FFT_Computer;
                mp_FFT_Computer = nullptr;
            }
            if(m_IsThreaded)
            {
                ESP_LOGD("TestFFT", "Threaded");
                mp_FFT_Computer = new FFT_Computer(fftSize, hopSize, f_s, dataWidth, uxPriority, xCoreID);
            }
            else
            {
                ESP_LOGD("TestFFT", "Unthreaded");
                mp_FFT_Computer = new FFT_Computer(fftSize, hopSize, f_s, dataWidth);
            }

            mp_FFT_Computer->Setup(Static_FFT_Results_Callback, this);
            CreateInputSignals(f_s, f_signal, magnitude);
            mp_FFT_Computer->PushFrames(mp_Frames.data(), m_FFT_Size);

            float allowed_Freq_Error = f_s / fftSize;
            float freq_Error_Left = f_signal - m_max_Freq_Result_Left.Frequency;
            float freq_Error_Right = f_signal - m_max_Freq_Result_Right.Frequency;
            ESP_LOGD( "TestFFT", "\nInput Freq: \t\"%.1f\", \nAllowed Error: \t\"%.1f\" \nMax Freq Left: \t\"%.1f\", \nError Left: \t\"%.1f\" \nMax Freq Right: \"%.1f\" \nError Right: \t\"%.1f\""
                    , f_signal
                    , allowed_Freq_Error
                    , m_max_Freq_Result_Left.Frequency
                    , freq_Error_Left
                    , m_max_Freq_Result_Right.Frequency
                    , freq_Error_Right);
            EXPECT_TRUE(freq_Error_Left <= allowed_Freq_Error);
            EXPECT_TRUE(freq_Error_Right <= allowed_Freq_Error);
        }

        void CreateInputSignals(float f_s, float f_signal, float magnitude)
        {
            ESP_LOGD("CreateInputSignals", "CreateInputSignals");
            Serial.print("Input Signal: ");
            mp_Sine_Table.resize(m_FFT_Size);
            mp_Frames.resize(m_FFT_Size);
            for (int i = 0; i < m_FFT_Size; ++i)
            {
                float angle = -2.0f * M_PI * f_signal * i / f_s;
                float sinAngle = magnitude*sinf(angle);
                mp_Sine_Table[i] = sinAngle;
                mp_Frames[i].channel1 = sinAngle;
                mp_Frames[i].channel2 = sinAngle;
            }
        }
};


TEST_F(FFT_Computer_Tests, Allocation_DeAllocation)
{
    size_t freeMemoryBeforeAllocation = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    ESP_LOGD("Allocation_DeAllocation", "Memory before allocation: %i", freeMemoryBeforeAllocation);
    mp_FFT_Computer = new FFT_Computer(128, 128, 44100, DataWidth_16);
    mp_FFT_Computer->Setup(Static_FFT_Results_Callback, this);
    size_t freeMemoryAfterAllocation = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    ESP_LOGD("Allocation_DeAllocation", "Memory after allocation: %i", freeMemoryAfterAllocation);
    delete mp_FFT_Computer;
    mp_FFT_Computer = nullptr;
    size_t freeMemoryAfterDeallocation = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    ESP_LOGD("Allocation_DeAllocation", "Memory after deallocation: %i", freeMemoryAfterDeallocation);
    EXPECT_TRUE(freeMemoryAfterAllocation < freeMemoryBeforeAllocation);
    EXPECT_TRUE(freeMemoryBeforeAllocation == freeMemoryAfterDeallocation);
}

TEST_F(FFT_Computer_Tests, 128_Bin_FFT_Calculation_Testing)
{
    int f_s = 44100;
    int nyquist = f_s/2;
    int start_Freq = 1000;
    int test_step = 1000;
    int magnitude = 100;
    int fft_size = 128;
    int hop_size = 128;
    for(int input_freq = start_Freq; input_freq < nyquist; input_freq+=1000)
    {
        ESP_LOGD("128_Bin_FFT_Calculation_Testing", "Memory before test: %i", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
        ESP_LOGD("128_Bin_FFT_Calculation_Testing", "Stack Memory before test: %i", uxTaskGetStackHighWaterMark(NULL) * 4);
        ESP_LOGD("128_Bin_FFT_Calculation_Testing", "Testing Input Frequency: %s", std::to_string(input_freq).c_str());
        TestFFT(fft_size, hop_size, f_s, input_freq, magnitude, true, DataWidth_16);
    }
}

TEST_F(FFT_Computer_Tests, 256_Bin_FFT_Calculation_Testing)
{
    int f_s = 44100;
    int nyquist = f_s/2;
    int start_Freq = 1000;
    int test_step = 1000;
    int magnitude = 100;
    int fft_size = 256;
    int hop_size = 256;
    for(int input_freq = start_Freq; input_freq < nyquist; input_freq+=1000)
    {
        ESP_LOGD("256_Bin_FFT_Calculation_Testing", "Memory before test: %i", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
        ESP_LOGD("256_Bin_FFT_Calculation_Testing", "Stack Memory before test: %i", uxTaskGetStackHighWaterMark(NULL) * 4);
        ESP_LOGD("256_Bin_FFT_Calculation_Testing", "Testing Input Frequency: %s", std::to_string(input_freq).c_str());
        TestFFT(fft_size, hop_size, f_s, input_freq, magnitude, true, DataWidth_16);
    }
}

TEST_F(FFT_Computer_Tests, 512_Bin_FFT_Calculation_Testing)
{
    int f_s = 44100;
    int nyquist = f_s/2;
    int start_Freq = 1000;
    int test_step = 1000;
    int magnitude = 100;
    int fft_size = 512;
    int hop_size = 512;
    for(int input_freq = start_Freq; input_freq < nyquist; input_freq+=1000)
    {
        ESP_LOGD("512_Bin_FFT_Calculation_Testing", "Memory before test: %i", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
        ESP_LOGD("512_Bin_FFT_Calculation_Testing", "Stack Memory before test: %i", uxTaskGetStackHighWaterMark(NULL) * 4);
        ESP_LOGD("512_Bin_FFT_Calculation_Testing", "Testing Input Frequency: %s", std::to_string(input_freq).c_str());
        TestFFT(fft_size, hop_size, f_s, input_freq, magnitude, true, DataWidth_16);
    }
}

TEST_F(FFT_Computer_Tests, 1024_Bin_FFT_Calculation_Testing)
{
    int f_s = 44100;
    int nyquist = f_s/2;
    int start_Freq = 1000;
    int test_step = 1000;
    int magnitude = 100;
    int fft_size = 1024;
    int hop_size = 1024;
    for(int input_freq = start_Freq; input_freq < nyquist; input_freq+=1000)
    {
        ESP_LOGD("1024_Bin_FFT_Calculation_Testing", "Memory before test: %i", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
        ESP_LOGD("1024_Bin_FFT_Calculation_Testing", "Stack Memory before test: %i", uxTaskGetStackHighWaterMark(NULL) * 4);
        ESP_LOGD("1024_Bin_FFT_Calculation_Testing", "Testing Input Frequency: %s", std::to_string(input_freq).c_str());
        TestFFT(fft_size, hop_size, f_s, input_freq, magnitude, true, DataWidth_16);
    }
}

TEST_F(FFT_Computer_Tests, 2048_Bin_FFT_Calculation_Testing)
{
    int f_s = 44100;
    int nyquist = f_s/2;
    int start_Freq = 1000;
    int test_step = 1000;
    int magnitude = 100;
    int fft_size = 2048;
    int hop_size = 2048;
    for(int input_freq = start_Freq; input_freq < nyquist; input_freq+=1000)
    {
        ESP_LOGD("2048_Bin_FFT_Calculation_Testing", "Memory before test: %i", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
        ESP_LOGD("2048_Bin_FFT_Calculation_Testing", "Stack Memory before test: %i", uxTaskGetStackHighWaterMark(NULL) * 4);
        ESP_LOGD("2048_Bin_FFT_Calculation_Testing", "Testing Input Frequency: %s", std::to_string(input_freq).c_str());
        TestFFT(fft_size, hop_size, f_s, input_freq, magnitude, true, DataWidth_16);
    }
}

TEST_F(FFT_Computer_Tests, 4096_Bin_FFT_Calculation_Testing)
{
    int f_s = 44100;
    int nyquist = f_s/2;
    int start_Freq = 1000;
    int test_step = 1000;
    int magnitude = 100;
    int fft_size = 4096;
    int hop_size = 4096;
    for(int input_freq = start_Freq; input_freq < nyquist; input_freq+=1000)
    {
        ESP_LOGD("4096_Bin_FFT_Calculation_Testing", "Heap Memory before test: %i", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
        ESP_LOGD("4096_Bin_FFT_Calculation_Testing", "Stack Memory before test: %i", uxTaskGetStackHighWaterMark(NULL) * 4);
        ESP_LOGD("4096_Bin_FFT_Calculation_Testing", "Testing Input Frequency: %s", std::to_string(input_freq).c_str());
        TestFFT(fft_size, hop_size, f_s, input_freq, magnitude, true, DataWidth_16);
    }
}

TEST_F(FFT_Computer_Tests, 8192_Bin_FFT_Calculation_Testing)
{
    int f_s = 44100;
    int nyquist = f_s/2;
    int start_Freq = 1000;
    int test_step = 1000;
    int magnitude = 100;
    int fft_size = 8192;
    int hop_size = 8192;
    for(int input_freq = start_Freq; input_freq < nyquist; input_freq+=1000)
    {
        ESP_LOGD("8192_Bin_FFT_Calculation_Testing", "Memory before test: %i", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
        ESP_LOGD("8192_Bin_FFT_Calculation_Testing", "Stack Memory before test: %i", uxTaskGetStackHighWaterMark(NULL) * 4);
        ESP_LOGD("8192_Bin_FFT_Calculation_Testing", "Testing Input Frequency: %s", std::to_string(input_freq).c_str());
        TestFFT(fft_size, hop_size, f_s, input_freq, magnitude, true, DataWidth_16);
    }
}
