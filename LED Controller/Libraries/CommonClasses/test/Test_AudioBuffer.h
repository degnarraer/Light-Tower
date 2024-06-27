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
#include "AudioBuffer.h"

using ::testing::_;
using ::testing::NotNull;
using ::testing::NiceMock;
using namespace testing;

const static size_t bufferSize = 10;

// Test Fixture for DataItemFunctionCallTests
class ContinuousAudioBufferTests : public Test
{
    protected:
        ContinuousAudioBuffer<bufferSize> *audioBuffer;
        void SetUp() override
        {
            audioBuffer = new ContinuousAudioBuffer<bufferSize>();
            audioBuffer->Initialize();
        }
        void TearDown() override 
        {
            delete audioBuffer;
        }
};

TEST_F(ContinuousAudioBufferTests, Allocation_DeAllocation)
{
}

TEST_F(ContinuousAudioBufferTests, Starting_State)
{
    EXPECT_EQ(true, audioBuffer->IsEmpty());
    EXPECT_EQ(false, audioBuffer->IsFull());
    EXPECT_EQ(bufferSize, audioBuffer->Available());
}

TEST_F(ContinuousAudioBufferTests, Pushing_To_Full_Popping_To_Empty)
{
    Frame_t resultingBuffer[bufferSize];
    EXPECT_EQ(true, audioBuffer->IsEmpty());
    EXPECT_EQ(false, audioBuffer->IsFull());
    for(size_t i = 0; i < bufferSize+1; ++i)
    {
        Frame_t newFrame = {i,i};
        if(i < bufferSize-1)
        {
            EXPECT_EQ(bufferSize-i, audioBuffer->Available());
            EXPECT_EQ(true, audioBuffer->Push(newFrame));
            EXPECT_EQ(false, audioBuffer->IsEmpty());
            EXPECT_EQ(false, audioBuffer->IsFull());
        }
        else if(i == bufferSize-1)
        {
            EXPECT_EQ(bufferSize-i, audioBuffer->Available());
            EXPECT_EQ(true, audioBuffer->Push(newFrame));
            EXPECT_EQ(false, audioBuffer->IsEmpty());
            EXPECT_EQ(true, audioBuffer->IsFull());
        }
        else if(i > bufferSize-1)
        {
            EXPECT_EQ(0, audioBuffer->Available());
            EXPECT_EQ(false, audioBuffer->Push(newFrame));
            EXPECT_EQ(false, audioBuffer->IsEmpty());
            EXPECT_EQ(true, audioBuffer->IsFull());
        }
    }
    EXPECT_EQ(bufferSize, audioBuffer->Pop(resultingBuffer, bufferSize));
    EXPECT_EQ(resultingBuffer[0], (Frame_t{10, 10}));
    EXPECT_EQ(resultingBuffer[1], (Frame_t{9, 9}));
    EXPECT_EQ(resultingBuffer[2], (Frame_t{8, 8}));
    EXPECT_EQ(resultingBuffer[3], (Frame_t{7, 7}));
    EXPECT_EQ(resultingBuffer[4], (Frame_t{6, 6}));
    EXPECT_EQ(resultingBuffer[5], (Frame_t{5, 5}));
    EXPECT_EQ(resultingBuffer[6], (Frame_t{4, 4}));
    EXPECT_EQ(resultingBuffer[7], (Frame_t{3, 3}));
    EXPECT_EQ(resultingBuffer[8], (Frame_t{2, 2}));
    EXPECT_EQ(resultingBuffer[9], (Frame_t{1, 1}));
    EXPECT_EQ(bufferSize, audioBuffer->Available());
    EXPECT_EQ(true, audioBuffer->IsEmpty());
    EXPECT_EQ(false, audioBuffer->IsFull());
}

TEST_F(ContinuousAudioBufferTests, Pushing_Array_To_Full_Popping_To_Empty)
{
    const size_t inputBufferSize = 11;
    Frame_t inputBuffer[inputBufferSize] = { (Frame_t{0, 0})
                                           , (Frame_t{1, 1})
                                           , (Frame_t{2, 2})
                                           , (Frame_t{3, 3})
                                           , (Frame_t{4, 4})
                                           , (Frame_t{5, 5})
                                           , (Frame_t{6, 6})
                                           , (Frame_t{7, 7})
                                           , (Frame_t{8, 8})
                                           , (Frame_t{9, 9})
                                           , (Frame_t{10, 10}) };
    Frame_t resultingBuffer[bufferSize];
    EXPECT_EQ(true, audioBuffer->IsEmpty());
    EXPECT_EQ(false, audioBuffer->IsFull());
    EXPECT_EQ(bufferSize, audioBuffer->Push(inputBuffer, inputBufferSize));
    EXPECT_EQ(bufferSize, audioBuffer->Pop(resultingBuffer, bufferSize));
    EXPECT_EQ(resultingBuffer[0], (Frame_t{10, 10}));
    EXPECT_EQ(resultingBuffer[1], (Frame_t{9, 9}));
    EXPECT_EQ(resultingBuffer[2], (Frame_t{8, 8}));
    EXPECT_EQ(resultingBuffer[3], (Frame_t{7, 7}));
    EXPECT_EQ(resultingBuffer[4], (Frame_t{6, 6}));
    EXPECT_EQ(resultingBuffer[5], (Frame_t{5, 5}));
    EXPECT_EQ(resultingBuffer[6], (Frame_t{4, 4}));
    EXPECT_EQ(resultingBuffer[7], (Frame_t{3, 3}));
    EXPECT_EQ(resultingBuffer[8], (Frame_t{2, 2}));
    EXPECT_EQ(resultingBuffer[9], (Frame_t{1, 1}));
    EXPECT_EQ(bufferSize, audioBuffer->Available());
    EXPECT_EQ(true, audioBuffer->IsEmpty());
    EXPECT_EQ(false, audioBuffer->IsFull());
}


TEST_F(ContinuousAudioBufferTests, Pushing_To_Full_Shifting_To_Empty)
{
    Frame_t resultingBuffer[bufferSize];
    EXPECT_EQ(true, audioBuffer->IsEmpty());
    EXPECT_EQ(false, audioBuffer->IsFull());
    for(size_t i = 0; i < bufferSize+1; ++i)
    {
        Frame_t newFrame = {i,i};
        if(i < bufferSize-1)
        {
            EXPECT_EQ(bufferSize-i, audioBuffer->Available());
            EXPECT_EQ(true, audioBuffer->Push(newFrame));
            EXPECT_EQ(false, audioBuffer->IsEmpty());
            EXPECT_EQ(false, audioBuffer->IsFull());
        }
        else if(i == bufferSize-1)
        {
            EXPECT_EQ(bufferSize-i, audioBuffer->Available());
            EXPECT_EQ(true, audioBuffer->Push(newFrame));
            EXPECT_EQ(false, audioBuffer->IsEmpty());
            EXPECT_EQ(true, audioBuffer->IsFull());
        }
        else if(i > bufferSize-1)
        {
            EXPECT_EQ(0, audioBuffer->Available());
            EXPECT_EQ(false, audioBuffer->Push(newFrame));
            EXPECT_EQ(false, audioBuffer->IsEmpty());
            EXPECT_EQ(true, audioBuffer->IsFull());
        }
    }
    EXPECT_EQ(bufferSize, audioBuffer->Shift(resultingBuffer, bufferSize));
    EXPECT_EQ(resultingBuffer[0], (Frame_t{1, 1}));
    EXPECT_EQ(resultingBuffer[1], (Frame_t{2, 2}));
    EXPECT_EQ(resultingBuffer[2], (Frame_t{3, 3}));
    EXPECT_EQ(resultingBuffer[3], (Frame_t{4, 4}));
    EXPECT_EQ(resultingBuffer[4], (Frame_t{5, 5}));
    EXPECT_EQ(resultingBuffer[5], (Frame_t{6, 6}));
    EXPECT_EQ(resultingBuffer[6], (Frame_t{7, 7}));
    EXPECT_EQ(resultingBuffer[7], (Frame_t{8, 8}));
    EXPECT_EQ(resultingBuffer[8], (Frame_t{9, 9}));
    EXPECT_EQ(resultingBuffer[9], (Frame_t{10, 10}));
    EXPECT_EQ(bufferSize, audioBuffer->Available());
    EXPECT_EQ(true, audioBuffer->IsEmpty());
    EXPECT_EQ(false, audioBuffer->IsFull());
}

TEST_F(ContinuousAudioBufferTests, Unshift_To_Full_Popping_To_Empty)
{
    
    Frame_t resultingBuffer[bufferSize];
    EXPECT_EQ(true, audioBuffer->IsEmpty());
    EXPECT_EQ(false, audioBuffer->IsFull());
    for(size_t i = 0; i < bufferSize+1; ++i)
    {
        Frame_t newFrame = {i,i};
        if(i < bufferSize-1)
        {
            EXPECT_EQ(bufferSize-i, audioBuffer->Available());
            EXPECT_EQ(true, audioBuffer->Unshift(newFrame));
            EXPECT_EQ(false, audioBuffer->IsEmpty());
            EXPECT_EQ(false, audioBuffer->IsFull());
        }
        else if(i == bufferSize-1)
        {
            EXPECT_EQ(bufferSize-i, audioBuffer->Available());
            EXPECT_EQ(true, audioBuffer->Unshift(newFrame));
            EXPECT_EQ(false, audioBuffer->IsEmpty());
            EXPECT_EQ(true, audioBuffer->IsFull());
        }
        else if(i > bufferSize-1)
        {
            EXPECT_EQ(0, audioBuffer->Available());
            EXPECT_EQ(false, audioBuffer->Unshift(newFrame));
            EXPECT_EQ(false, audioBuffer->IsEmpty());
            EXPECT_EQ(true, audioBuffer->IsFull());
        }
    }
    EXPECT_EQ(bufferSize, audioBuffer->Pop(resultingBuffer, bufferSize));
    EXPECT_EQ(resultingBuffer[0], (Frame_t{1, 1}));
    EXPECT_EQ(resultingBuffer[1], (Frame_t{2, 2}));
    EXPECT_EQ(resultingBuffer[2], (Frame_t{3, 3}));
    EXPECT_EQ(resultingBuffer[3], (Frame_t{4, 4}));
    EXPECT_EQ(resultingBuffer[4], (Frame_t{5, 5}));
    EXPECT_EQ(resultingBuffer[5], (Frame_t{6, 6}));
    EXPECT_EQ(resultingBuffer[6], (Frame_t{7, 7}));
    EXPECT_EQ(resultingBuffer[7], (Frame_t{8, 8}));
    EXPECT_EQ(resultingBuffer[8], (Frame_t{9, 9}));
    EXPECT_EQ(resultingBuffer[9], (Frame_t{10, 10}));
    EXPECT_EQ(bufferSize, audioBuffer->Available());
    EXPECT_EQ(true, audioBuffer->IsEmpty());
    EXPECT_EQ(false, audioBuffer->IsFull());
}

TEST_F(ContinuousAudioBufferTests, Unshifting_Array_To_Full_Popping_To_Empty)
{
    const size_t inputBufferSize = 11;
    Frame_t inputBuffer[inputBufferSize] = { (Frame_t{0, 0})
                                           , (Frame_t{1, 1})
                                           , (Frame_t{2, 2})
                                           , (Frame_t{3, 3})
                                           , (Frame_t{4, 4})
                                           , (Frame_t{5, 5})
                                           , (Frame_t{6, 6})
                                           , (Frame_t{7, 7})
                                           , (Frame_t{8, 8})
                                           , (Frame_t{9, 9})
                                           , (Frame_t{10, 10}) };
    Frame_t resultingBuffer[bufferSize];
    EXPECT_EQ(true, audioBuffer->IsEmpty());
    EXPECT_EQ(false, audioBuffer->IsFull());
    EXPECT_EQ(bufferSize, audioBuffer->Unshift(inputBuffer, inputBufferSize));
    EXPECT_EQ(bufferSize, audioBuffer->Pop(resultingBuffer, bufferSize));
    EXPECT_EQ(resultingBuffer[0], (Frame_t{1, 1}));
    EXPECT_EQ(resultingBuffer[1], (Frame_t{2, 2}));
    EXPECT_EQ(resultingBuffer[2], (Frame_t{3, 3}));
    EXPECT_EQ(resultingBuffer[3], (Frame_t{4, 4}));
    EXPECT_EQ(resultingBuffer[4], (Frame_t{5, 5}));
    EXPECT_EQ(resultingBuffer[5], (Frame_t{6, 6}));
    EXPECT_EQ(resultingBuffer[6], (Frame_t{7, 7}));
    EXPECT_EQ(resultingBuffer[7], (Frame_t{8, 8}));
    EXPECT_EQ(resultingBuffer[8], (Frame_t{9, 9}));
    EXPECT_EQ(resultingBuffer[9], (Frame_t{10, 10}));
    EXPECT_EQ(bufferSize, audioBuffer->Available());
    EXPECT_EQ(true, audioBuffer->IsEmpty());
    EXPECT_EQ(false, audioBuffer->IsFull());
}

TEST_F(ContinuousAudioBufferTests, Unshift_To_Full_Shift_To_Empty)
{
    
    Frame_t resultingBuffer[bufferSize];
    EXPECT_EQ(true, audioBuffer->IsEmpty());
    EXPECT_EQ(false, audioBuffer->IsFull());
    for(size_t i = 0; i < bufferSize+1; ++i)
    {
        Frame_t newFrame = {i,i};
        if(i < bufferSize-1)
        {
            EXPECT_EQ(bufferSize-i, audioBuffer->Available());
            EXPECT_EQ(true, audioBuffer->Unshift(newFrame));
            EXPECT_EQ(false, audioBuffer->IsEmpty());
            EXPECT_EQ(false, audioBuffer->IsFull());
        }
        else if(i == bufferSize-1)
        {
            EXPECT_EQ(bufferSize-i, audioBuffer->Available());
            EXPECT_EQ(true, audioBuffer->Unshift(newFrame));
            EXPECT_EQ(false, audioBuffer->IsEmpty());
            EXPECT_EQ(true, audioBuffer->IsFull());
        }
        else if(i > bufferSize-1)
        {
            EXPECT_EQ(0, audioBuffer->Available());
            EXPECT_EQ(false, audioBuffer->Unshift(newFrame));
            EXPECT_EQ(false, audioBuffer->IsEmpty());
            EXPECT_EQ(true, audioBuffer->IsFull());
        }
    }
    EXPECT_EQ(bufferSize, audioBuffer->Shift(resultingBuffer, bufferSize));
    EXPECT_EQ(resultingBuffer[0], (Frame_t{10, 10}));
    EXPECT_EQ(resultingBuffer[1], (Frame_t{9, 9}));
    EXPECT_EQ(resultingBuffer[2], (Frame_t{8, 8}));
    EXPECT_EQ(resultingBuffer[3], (Frame_t{7, 7}));
    EXPECT_EQ(resultingBuffer[4], (Frame_t{6, 6}));
    EXPECT_EQ(resultingBuffer[5], (Frame_t{5, 5}));
    EXPECT_EQ(resultingBuffer[6], (Frame_t{4, 4}));
    EXPECT_EQ(resultingBuffer[7], (Frame_t{3, 3}));
    EXPECT_EQ(resultingBuffer[8], (Frame_t{2, 2}));
    EXPECT_EQ(resultingBuffer[9], (Frame_t{1, 1}));
    EXPECT_EQ(true, audioBuffer->IsEmpty());
    EXPECT_EQ(false, audioBuffer->IsFull());
}

