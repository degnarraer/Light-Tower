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


class FFT_Calculator
{
  public:
    FFT_Calculator(int32_t FFT_Size, int32_t SampleRate): m_FFT_Size(FFT_Size)
                                                        , m_FFT_SampleRate(SampleRate)
    {
      mp_RealBuffer = (float*)malloc(sizeof(float)*m_FFT_Size);
      mp_ImaginaryBuffer = (float*)malloc(sizeof(float)*m_FFT_Size);
    }
    virtual ~FFT_Calculator()
    {
      delete mp_RealBuffer;
      delete mp_ImaginaryBuffer;
    }
    int32_t GetFreeSpace() { return m_FFT_Size - m_CurrentIndex; }
    float GetFFTBufferValue(int32_t index)
    {
      assert(true == m_SolutionReady);
      assert(index < m_FFT_Size);
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
    void SetGainValue(float Gain){m_Gain = Gain;}
    bool PushValueAndCalculateNormalizedFFT(int32_t value)
    {
      m_SolutionReady = false;
      mp_RealBuffer[m_CurrentIndex] = value;
      mp_ImaginaryBuffer[m_CurrentIndex] = 0.0;
      ++m_CurrentIndex;
      if(m_CurrentIndex >= m_FFT_Size)
      {
        ArduinoFFT<float>myFFT = ArduinoFFT<float>(mp_RealBuffer, mp_ImaginaryBuffer, m_FFT_Size, m_FFT_SampleRate);
        m_CurrentIndex = 0;
        m_MaxFFTBinValue = 0;
        m_MaxFFTBinIndex = 0;
        myFFT.windowing(FFTWindow::Hamming, FFTDirection::Forward, true);
        myFFT.compute(FFTDirection::Forward);
        myFFT.complexToMagnitude();
        m_MajorPeak = myFFT.majorPeak();
        for(int16_t i = 0; i < (m_FFT_Size >> 1); ++i)
        {
          mp_RealBuffer[i] = ( ( (2 * mp_RealBuffer[i]) / (float)m_FFT_Size ) * m_Gain ) / m_32BitMax;
          if(mp_RealBuffer[i] > 1.0) mp_RealBuffer[i] = 1.0;
          if(mp_RealBuffer[i] > m_MaxFFTBinValue)
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
    uint32_t m_32BitMax = pow(2,32);
};


#endif
