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
    FFT_Calculator(int32_t FFT_Size, int32_t SampleRate, float Max_Value): m_FFT_Size(FFT_Size)
                                                                         , m_FFT_SampleRate(SampleRate)
                                                                         , m_MaxValue(Max_Value)
    {
      mp_RealBuffer = (float*)malloc(m_FFT_Size);
      mp_ImaginaryBuffer = (float*)malloc(m_FFT_Size);
      mp_FFT = new ArduinoFFT<float>(mp_RealBuffer, mp_ImaginaryBuffer, m_FFT_Size, m_FFT_SampleRate);
    }
    virtual ~FFT_Calculator()
    {
      delete mp_RealBuffer;
      delete mp_ImaginaryBuffer;
      delete mp_FFT;
    }
    float GetFFTBufferValue(int32_t index)
    {
      assert(index < m_FFT_Size);
      return mp_RealBuffer[index];
    }
    float* GetFFTRealBuffer()
    {
      return mp_RealBuffer;
    }
    float GetFFTMaxValue(){return m_MaxFFTBinValue;}
    int32_t GetFFTMaxValueBin(){return m_MaxFFTBinIndex;}
    float GetMajorPeak(){return m_MajorPeak;}
    float* GetMajorPeakPointer(){return &m_MajorPeak;}
    void SetGainValue(float Gain){m_Gain = Gain;}
    bool PushValueAndCalculateNormalizedFFT(int32_t value)
    {
      bool SolutionReady = false;
      mp_RealBuffer[m_CurrentIndex] = value;
      ++m_CurrentIndex;

      if(m_CurrentIndex >= m_FFT_Size)
      {
        m_CurrentIndex = 0;
        mp_FFT->windowing(FFTWindow::Hamming, FFTDirection::Forward, true);
        mp_FFT->compute(FFTDirection::Forward);
        mp_FFT->complexToMagnitude();
        m_MajorPeak = mp_FFT->majorPeak();
        for(int16_t i=0; i < (m_FFT_Size >> 1); ++i)
        {
          mp_RealBuffer[i] = ( ( 2 * mp_RealBuffer[i] / m_FFT_Size ) / m_MaxValue ) * m_Gain;
          if(mp_RealBuffer[i] > 1.0) mp_RealBuffer[i] = 1.0;
          if(mp_RealBuffer[i] >= m_MaxFFTBinValue)
          {
            m_MaxFFTBinValue = mp_RealBuffer[i];
            m_MaxFFTBinIndex = i;
          }
        }
        SolutionReady = true;
      }
      return SolutionReady;
    }
  
  private:
    int32_t m_CurrentIndex = 0;
    int32_t m_FFT_Size = 0;
    int32_t m_FFT_SampleRate = 0;
    float *mp_RealBuffer;
    float *mp_ImaginaryBuffer;
    float m_MaxValue = 0;
    float m_Gain = 1.0;
    float m_MaxFFTBinValue = 0;
    int32_t m_MaxFFTBinIndex = 0;
    float m_MajorPeak = 0;
    ArduinoFFT<float> *mp_FFT;
};


#endif
