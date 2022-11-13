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
#ifndef AMPLITUDE_CALCULATOR_H
#define AMPLITUDE_CALCULATOR_H
#include <DataTypes.h>
#include "Streaming.h"

class Amplitude_Calculator
{
  public:
    Amplitude_Calculator( int32_t RequiredSampleCount, BitLength_t BitLength )
                        : m_RequiredSampleCount(RequiredSampleCount)
                        , m_BitLength(BitLength)
    {
    }
    virtual ~Amplitude_Calculator()
    {
    }
    ProcessedSoundData_t GetProcessedSoundData()
    {
      assert(true == m_SolutionReady);
      return m_ProcessedSoundDataOutput;
    }
    ProcessedSoundData_t* GetProcessedSoundDataPointer()
    {
      assert(true == m_SolutionReady);
      return &m_ProcessedSoundDataOutput;
    }
    bool PushValueAndCalculateSoundData(int32_t value)
    {
      m_SolutionReady = false;
      if(value < m_ProcessedSoundData.Minimum)
      {
        m_ProcessedSoundData.Minimum = value;
      }
      if(value > m_ProcessedSoundData.Maximum)
      {
        m_ProcessedSoundData.Maximum = value;
      }
      ++m_PushCount;
      if(m_PushCount >= m_RequiredSampleCount)
      {
        m_PushCount = 0;
        int32_t peakToPeak = (m_ProcessedSoundData.Maximum - m_ProcessedSoundData.Minimum);
        m_ProcessedSoundData.NormalizedPower = (float)peakToPeak / (float)GetBitMax();
        m_ProcessedSoundDataOutput = m_ProcessedSoundData;
        m_SolutionReady = true;
        m_ProcessedSoundData.Minimum = GetMax();
        m_ProcessedSoundData.Maximum = GetMin();
      }
      return m_SolutionReady;
    }
  private:
    int32_t GetMax()
    {
      switch(m_BitLength)
      {
        case BitLength_32:
          return INT32_MAX;
        break;
        case BitLength_16:
          return INT16_MAX;
        break;
        case BitLength_8:
          return INT8_MAX;
        break;
        default:
          return INT32_MAX;
        break;
      }
    }
    int32_t GetMin()
    {
      switch(m_BitLength)
      {
        case BitLength_32:
          return INT32_MIN;
        break;
        case BitLength_16:
          return INT16_MIN;
        break;
        case BitLength_8:
          return INT8_MIN;
        break;
        default:
          return INT32_MIN;
        break;
      }
    }
    float GetBitMax()
    {
      switch(m_BitLength)
      {
        case BitLength_32:
          return m_32BitLength;
        break;
        case BitLength_16:
          return m_16BitLength;
        break;
        case BitLength_8:
          return m_8BitLength;
        break;
        default:
          return m_32BitLength;
        break;
      }
    }
    ProcessedSoundData_t m_ProcessedSoundData;
    ProcessedSoundData_t m_ProcessedSoundDataOutput;
    int32_t m_RequiredSampleCount = 0;
    int32_t m_PushCount = 0;
    bool m_SolutionReady = false;
    BitLength_t m_BitLength;
    uint32_t m_32BitLength = pow(2,32);
    uint32_t m_16BitLength = pow(2,16);
    uint32_t m_8BitLength = pow(2,8);
};
         
#endif
