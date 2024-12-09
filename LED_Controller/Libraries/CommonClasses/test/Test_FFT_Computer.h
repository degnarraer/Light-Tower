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
#include "FFT_Computer.h"

using ::testing::_;
using ::testing::NotNull;
using ::testing::NiceMock;
using namespace testing;

// Test Fixture for DataItemFunctionCallTests
class FFT_Computer_Tests : public Test
{
    protected:
        void SetUp() override
        {
        }
        void TearDown() override 
        {
        }
        void TestFFT(int fftSize, int hopSize, UBaseType_t uxPriority, BaseType_t xCoreID, bool normalizeMagnitudes, int dataWidth, bool isThreaded)
        {
            m_FFT_Size = fftSize;
            m_Hop_Size = hopSize;
            m_IsThreaded = isThreaded;
            if(m_IsThreaded)
            {
                m_FFT_Computer = new FFT_Computer(fftSize, hopSize, normalizeMagnitudes, dataWidth, uxPriority, xCoreID);
            }
            else
            {
                m_FFT_Computer = new FFT_Computer(fftSize, hopSize, normalizeMagnitudes, dataWidth);
            }
            m_FFT_Computer->Setup(Static_MagnitudesCallback, this);
            CreateInputSignals();
            m_FFT_Computer->PushFrames(m_Frames, m_FFT_Size);
        }
    private:
        float* m_Sine_Table = nullptr;
        Frame_t* m_Frames = nullptr;
        int m_Table_Size = 0;
        int m_FFT_Size = 0;
        int m_Hop_Size = 0;
        bool m_IsThreaded = false;
        FFT_Computer* m_FFT_Computer;
        
        static void Static_MagnitudesCallback(float *leftMagnitudes, float* rightMagnitudes, size_t count, void* args)
        {

        }
        void MagnitudesCallback(float *leftMagnitudes, float* rightMagnitudes, size_t count, void* args)
        {

        }
        void CreateInputSignals()
        {
            if (m_Table_Size != m_FFT_Size) 
            {
                // Free old tables if they exist
                if(m_Sine_Table)
                {
                    delete[] m_Sine_Table;
                    m_Sine_Table = nullptr;
                }

                // Allocate new tables
                m_Table_Size = m_FFT_Size;
                m_Sine_Table = new float[m_Table_Size];

                // Fill the tables
                for (int i = 0; i < m_Table_Size; ++i)
                {
                    float angle = -2.0f * M_PI * i / m_FFT_Size;
                    float sinAngle = sinf(angle);
                    m_Sine_Table[i] = sinAngle;
                    m_Frames[i].channel1 = sinAngle;
                    m_Frames[i].channel2 = sinAngle;
                }
            }
        }
};

TEST_F(FFT_Computer_Tests, Allocation_DeAllocation)
{
    TestFFT(128, 128, configMAX_PRIORITIES - 1, 0, false, BitLength_16, false);
    EXPECT_TRUE(true);
}

