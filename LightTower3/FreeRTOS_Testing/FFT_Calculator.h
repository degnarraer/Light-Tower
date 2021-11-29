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
#define FFT_LENGTH 2048

#define FFT_CALCULATOR_DEBUG false
#define FFT_CALCULATOR_LOOPS_DEBUG false
#define FFT_CALCULATOR_INPUTDATA_DEBUG false
#define FFT_CALCULATOR_OUTPUTDATA_DEBUG false


#include <Arduino.h>
#include <Adafruit_ZeroFFT.h>
#include "Streaming.h"

class FFT_Calculator
{
  public:
    FFT_Calculator(String Title);
    virtual ~FFT_Calculator();
    void Setup(size_t BufferCount);
    void ProcessEventQueue();
    QueueHandle_t GetFFTRightDataQueue() { return m_FFT_Right_Data_Buffer_queue; }
    QueueHandle_t GetFFTLeftDataQueue() { return m_FFT_Left_Data_Buffer_queue; }
    
  private:
    size_t m_BufferCount = 0;
    String m_Title;
    QueueHandle_t m_FFT_Right_Data_Buffer_queue = NULL;
    QueueHandle_t m_FFT_Left_Data_Buffer_queue = NULL;
    int m_BytesToRead = 0;

    int16_t m_FFT_Right_Data[FFT_LENGTH];
    int16_t m_FFT_Left_Data[FFT_LENGTH];
    int32_t* m_FFT_Right_Buffer_Data;
    int32_t* m_FFT_Left_Buffer_Data;
    int m_FFT_Right_Buffer_Index = 0;
    int m_FFT_Left_Buffer_Index = 0;

    void ProcessRightFFTQueue(int messageCount);
    void ProcessLeftFFTQueue(int messageCount);
};



#endif
