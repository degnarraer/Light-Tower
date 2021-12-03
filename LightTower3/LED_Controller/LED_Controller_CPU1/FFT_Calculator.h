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

#ifndef I2S_FFT_CALCULATOR_H
#define I2S_FFT_CALCULATOR_H

#define NUMBER_OF_BANDS 32

#define FFT_CALCULATOR_DEBUG false
#define FFT_CALCULATOR_LOOPS_DEBUG false
#define FFT_CALCULATOR_INPUTDATA_DEBUG false
#define FFT_CALCULATOR_OUTPUTDATA_DEBUG false


#include <Arduino.h>
#include <Adafruit_ZeroFFT.h>
#include "Streaming.h"
#include <DataTypes.h>

class FFT_Calculator: public NamedItem
{
  public:
    FFT_Calculator(String Title);
    virtual ~FFT_Calculator();
    void Setup(size_t InputByteCount, int SampleRate, int FFT_Length);
    void ProcessEventQueue();

    //Input Data Queue
    QueueHandle_t GetFFTRightDataInputQueue() { return m_FFT_Right_Data_Input_Buffer_queue; }
    QueueHandle_t GetFFTLeftDataInputQueue() { return m_FFT_Left_Data_Input_Buffer_queue; }

    
    //Output Data Queue
    QueueHandle_t GetFFTRightBandDataOutputQueue() { return m_FFT_Right_BandData_Output_Buffer_queue; }
    size_t GetFFTRightBandDataBufferSize() { return m_BandOutputByteCount; }
    QueueHandle_t GetFFTLeftBandDataOutputQueue() { return m_FFT_Left_BandData_Output_Buffer_queue; }
    size_t GetFFTLeftBandDataBufferSize() { return m_BandOutputByteCount; }
    
  private:
    size_t m_InputByteCount = 0;
    int m_SampleRate = 0;
    int m_FFT_Length = 0;
    QueueHandle_t m_FFT_Right_Data_Input_Buffer_queue = NULL;
    QueueHandle_t m_FFT_Left_Data_Input_Buffer_queue = NULL;
    size_t m_BytesToRead = 0;

    QueueHandle_t m_FFT_Right_BandData_Output_Buffer_queue = NULL;
    QueueHandle_t m_FFT_Left_BandData_Output_Buffer_queue = NULL;
    size_t m_BandOutputByteCount = 0;

    int16_t* m_FFT_Right_Data;
    int16_t* m_FFT_Left_Data;
    int16_t* m_Right_Band_Values;
    int16_t* m_Left_Band_Values;

    int32_t* m_FFT_Right_Buffer_Data;
    int32_t* m_FFT_Left_Buffer_Data;
    int m_FFT_Right_Buffer_Index = 0;
    int m_FFT_Left_Buffer_Index = 0;

    void ProcessFFTQueue(int messageCount, QueueHandle_t& Queue, int32_t* InputDataBuffer, int& BufferIndex, int16_t* FFTBuffer, int16_t* BandDataBuffer);
    void ProcessRightFFTQueue(int messageCount);
    void ProcessLeftFFTQueue(int messageCount);
    float GetFreqForBin(unsigned int bin);
};



#endif
