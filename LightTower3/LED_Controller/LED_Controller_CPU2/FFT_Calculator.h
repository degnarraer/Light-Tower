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
#ifndef FFT_CALCULATOR_H
#define FFT_CALCULATOR_H

#include "arduinoFFT.h"
#include <DataTypes.h>
#include "Streaming.h"

class FFT_Calculator
{
  public:
    FFT_Calculator(int32_t FFT_Size, int32_t SampleRate, BitLength_t BitLength ): m_FFT_Size(FFT_Size)
                                                                                , m_FFT_SampleRate(SampleRate)
    {
      mp_RealBuffer = (float*)malloc(sizeof(float)*m_FFT_Size);
      mp_ImaginaryBuffer = (float*)malloc(sizeof(float)*m_FFT_Size);
      m_MyFFT = new ArduinoFFT<float>(mp_RealBuffer, mp_ImaginaryBuffer, m_FFT_Size, m_FFT_SampleRate);
      switch(BitLength)
      {
        case BitLength_32:
          m_BitLengthMaxValue = pow(2,32);
        break;
        case BitLength_16:
          m_BitLengthMaxValue = pow(2,16);
        break;
        case BitLength_8:
          m_BitLengthMaxValue = pow(2,8);
        break;
        default:
          m_BitLengthMaxValue = pow(2,32);
        break;
      }
    }
    virtual ~FFT_Calculator()
    {
      free(mp_RealBuffer);
      free(mp_ImaginaryBuffer);
      free(m_MyFFT);
    }
    int32_t GetFreeSpace() { return m_FFT_Size - m_CurrentIndex; }
    float GetFFTBufferValue(int32_t index)
    {
      assert(true == m_SolutionReady);
      assert(index < m_FFT_Size/2);
      return mp_RealBuffer[index];
    }
    float GetFFTMaxValue(){return m_MaxFFTBinValue;}
    int32_t GetFFTMaxValueBin()
    {
      assert(true == m_SolutionReady);
      return m_MaxFFTBinIndex;
    }
    float GetMajorPeak()
    {
      assert(true == m_SolutionReady);
      return m_MajorPeak;
    }
    float* GetMajorPeakPointer()
    {
      assert(true == m_SolutionReady);
      return &m_MajorPeak;
    }
    size_t GetRequiredValueCount()
    {
      return m_FFT_Size - m_CurrentIndex;
    }
    bool PushValuesAndCalculateNormalizedFFT(int32_t *value, size_t Count, float Gain)
    {
      bool result = false;
      for(int i = 0; i < Count; ++i)
      {
        result = PushValueAndCalculateNormalizedFFT(value[i], Gain);
      }
      return result;
    }
    bool PushValueAndCalculateNormalizedFFT(int32_t value, float Gain)
    {
      m_SolutionReady = false;
      m_MaxFFTBinValue = 0;
      m_MaxFFTBinIndex = 0;
      mp_RealBuffer[m_CurrentIndex] = value;
      mp_ImaginaryBuffer[m_CurrentIndex] = 0.0;
      ++m_CurrentIndex;
      if(m_CurrentIndex >= m_FFT_Size)
      {
        m_CurrentIndex = 0;
        m_MaxFFTBinValue = 0;
        m_MaxFFTBinIndex = 0;
        m_MyFFT->windowing(FFTWindow::Hamming, FFTDirection::Forward, true);
        m_MyFFT->compute(FFTDirection::Forward);
        m_MyFFT->complexToMagnitude();
        m_MajorPeak = m_MyFFT->majorPeak();
        for(int i = 0; i < m_FFT_Size; ++i)
        {
          mp_RealBuffer[i] = ( ( (2 * mp_RealBuffer[i]) / (float)m_FFT_Size ) * Gain ) / m_BitLengthMaxValue;
          if(mp_RealBuffer[i] > 1.0)
          {
            mp_RealBuffer[i] = 1.0;
          }
          if(i < m_FFT_Size/2 && mp_RealBuffer[i] > m_MaxFFTBinValue)
          {
            m_MaxFFTBinValue = mp_RealBuffer[i];
            m_MaxFFTBinIndex = i;
          }
        }
        m_SolutionReady = true;
      }
      return m_SolutionReady;
    }
  private:
    int32_t m_CurrentIndex = 0;
    int32_t m_FFT_Size = 0;
    int32_t m_FFT_SampleRate = 0;
    float *mp_RealBuffer;
    float *mp_ImaginaryBuffer;
    float m_Gain = 1.0;
    float m_MaxFFTBinValue = 0;
    int32_t m_MaxFFTBinIndex = 0;
    float m_MajorPeak = 0;
    bool m_SolutionReady = false;
    float m_BitLengthMaxValue = 1.0;
    ArduinoFFT<float>*m_MyFFT;
};


#endif
