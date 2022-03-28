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

class Amplitude_Calculator
{
  public:
    Amplitude_Calculator(int32_t RequiredSampleCount): m_RequiredSampleCount(RequiredSampleCount)
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
        m_ProcessedSoundData.NormalizedPower = (float)peakToPeak / (float)m_32BitMax;
        m_ProcessedSoundDataOutput = m_ProcessedSoundData;
        m_SolutionReady = true;
        m_ProcessedSoundData.Minimum = INT32_MAX;
        m_ProcessedSoundData.Maximum = INT32_MIN;
      }
    }
  private:
    ProcessedSoundData_t m_ProcessedSoundData;
    ProcessedSoundData_t m_ProcessedSoundDataOutput;
    int32_t m_RequiredSampleCount = 0;
    int32_t m_PushCount = 0;
    bool m_SolutionReady = false;
    uint32_t m_32BitMax = pow(2,32);
};
         
#endif
