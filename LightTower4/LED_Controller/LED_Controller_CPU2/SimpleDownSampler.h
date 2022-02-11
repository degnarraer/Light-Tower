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
#ifndef SIMPLEDOWNSAMPLER_H
#define SIMPLEDOWNSAMPLER_H


#include <FIR.h>
#include <Helpers.h>
#include "float.h"

class SimpleDownSampler
{
  public:
    SimpleDownSampler()
    {
      m_Fir_lp_0_to_3k.setFilterCoeffs(filter_taps_0_to_3k);
      m_UpsampleBuffer = (int16_t*)malloc(m_LCM);
    }
    virtual ~SimpleDownSampler()
    {
      delete m_UpsampleBuffer;  
    }
    void FilterAndDownSample(int16_t *InputDataBuffer, size_t SampleCount, int16_t *OutputDataBuffer, size_t OutputSampleCount, size_t &ResultingSampleCount)
    {
      ResultingSampleCount = 0;
      int16_t Result;
      for(int i = 0; i < SampleCount; ++i)
      {
        Result = m_Fir_lp_0_to_3k.processReading( InputDataBuffer[i] );
        ++m_Count;
        if(0 == m_Count % m_DownSampleCount)
        {
          OutputDataBuffer[OutputSampleCount] = Result;
          ++ResultingSampleCount;
        }
      }
    }
  private:
    size_t m_DownSampleCount = I2S_SAMPLE_RATE / DOWN_SAMPLED_RATE;
    size_t m_LCM = LCM(I2S_SAMPLE_RATE, DOWN_SAMPLED_RATE);
    int16_t *m_UpsampleBuffer;
    int32_t m_Count = 0;
    int32_t LCM(int32_t A, int32_t B)
    {
      int32_t max_num = 0;
      while(true)    
      {    
        if(max_num % A == 0 && max_num % B == 0)  
        {   
            return max_num;  
        }  
        ++max_num;
      }  
    }
    /*
    FIR filter designed with
    http://t-filter.engineerjs.com/
    sampling frequency: 44100 Hz

    fixed point precision: 16 bits
    
    * 0 Hz - 3000 Hz
      gain = 1
      desired ripple = 5 dB
      actual ripple = n/a
    
    * 4000 Hz - 22050 Hz
      gain = 0
      desired attenuation = -40 dB
      actual attenuation = n/a
    
    */
    static const int32_t FILTER_TAP_NUM_0_to_3k = 49;
    float filter_taps_0_to_3k[FILTER_TAP_NUM_0_to_3k] = {
      -132,
      100,
      200,
      354,
      535,
      712,
      847,
      901,
      845,
      662,
      361,
      -24,
      -435,
      -797,
      -1025,
      -1045,
      -807,
      -292,
      473,
      1420,
      2448,
      3431,
      4247,
      4784,
      4972,
      4784,
      4247,
      3431,
      2448,
      1420,
      473,
      -292,
      -807,
      -1045,
      -1025,
      -797,
      -435,
      -24,
      361,
      662,
      845,
      901,
      847,
      712,
      535,
      354,
      200,
      100,
      -132
    };
    FIR<float, FILTER_TAP_NUM_0_to_3k> m_Fir_lp_0_to_3k;
};




#endif
