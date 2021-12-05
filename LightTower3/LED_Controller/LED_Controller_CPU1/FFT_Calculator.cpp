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

#include "FFT_Calculator.h"

FFT_Calculator::FFT_Calculator(String Title): NamedItem(Title)
{
}
FFT_Calculator::~FFT_Calculator()
{
}
void FFT_Calculator::Setup(size_t InputByteCount, int SampleRate, int FFT_Length)
{
  m_InputByteCount = InputByteCount;
  m_SampleRate = SampleRate;
  m_FFT_Length = FFT_Length;

  Serial << "FFT Config: " << m_InputByteCount << " | " << m_SampleRate << " | " << m_FFT_Length << "\n";

  Serial << GetTitle() << ": Allocating Memory.\n";
  m_BandOutputByteCount = sizeof(int16_t) * NUMBER_OF_BANDS;
  m_FFT_Right_Buffer_Data = (int32_t*)malloc(m_InputByteCount);
  m_FFT_Left_Buffer_Data = (int32_t*)malloc(m_InputByteCount);
  m_FFT_Right_Data = (int16_t*)malloc(sizeof(int16_t) * m_FFT_Length);
  m_FFT_Left_Data = (int16_t*)malloc(sizeof(int16_t) * m_FFT_Length);
  m_Right_Band_Values = (int16_t*)malloc(m_BandOutputByteCount);
  m_Left_Band_Values = (int16_t*)malloc(m_BandOutputByteCount);
  m_BytesToRead = sizeof(int32_t) * m_FFT_Length;

  
  CreateQueue(m_FFT_Right_Data_Input_Buffer_queue, m_InputByteCount, 10, true);
  CreateQueue(m_FFT_Left_Data_Input_Buffer_queue, m_InputByteCount, 10, true);
  CreateQueue(m_FFT_Right_BandData_Output_Buffer_queue, m_BandOutputByteCount, 10, true);
  CreateQueue(m_FFT_Left_BandData_Output_Buffer_queue, m_BandOutputByteCount, 10, true);
}

void FFT_Calculator::ProcessEventQueue()
{
  if(NULL != m_FFT_Right_Data_Input_Buffer_queue)
  {
    uint8_t fftRightDataBufferMsgCount = uxQueueMessagesWaiting(m_FFT_Right_Data_Input_Buffer_queue);
    if(true == FFT_CALCULATOR_DEBUG) Serial << "FFT Right Data Buffer Queue: " << fftRightDataBufferMsgCount << "\n";
    ProcessRightFFTQueue(fftRightDataBufferMsgCount);
  }
  
  if(NULL != m_FFT_Left_Data_Input_Buffer_queue)
  {
    uint8_t fftLeftDataBufferMsgCount = uxQueueMessagesWaiting(m_FFT_Left_Data_Input_Buffer_queue);
    if(true == FFT_CALCULATOR_DEBUG) Serial << "FFT Left Data Buffer Queue: " << fftLeftDataBufferMsgCount << "\n";
    ProcessLeftFFTQueue(fftLeftDataBufferMsgCount);
  }
}

void FFT_Calculator::ProcessFFTQueue(int messageCount, QueueHandle_t& Queue, int32_t* InputDataBuffer, int& BufferIndex, int16_t* FFTBuffer, int16_t* BandDataBuffer)
{
  for(uint8_t i = 0; i < messageCount; ++i)
  {
    if ( xQueueReceive(Queue, InputDataBuffer, portMAX_DELAY) != pdTRUE ){ Serial.println("Error Getting Queue Data");}
    else
    {
      if(true == FFT_CALCULATOR_INPUTDATA_DEBUG) Serial << "Data R: ";
      for(int i = 0; i < m_InputByteCount; ++i)
      {
        if(true == FFT_CALCULATOR_LOOPS_DEBUG)Serial << "Loop Count: "<< i << " of " << m_InputByteCount << "\n";
        FFTBuffer[BufferIndex] = (InputDataBuffer[i] >> 8) & 0x0000FFFF;
        if(true == FFT_CALCULATOR_INPUTDATA_DEBUG) Serial << InputDataBuffer[i] << "|" << FFTBuffer[BufferIndex] << "\n";
        ++BufferIndex;
        
        if(BufferIndex >= m_FFT_Length)
        {
          BufferIndex = 0;
          ZeroFFT(FFTBuffer, m_FFT_Length);
          if(true == FFT_CALCULATOR_OUTPUTDATA_DEBUG)
          {
            Serial << "FFT R: ";
            for(int j = 0; j < m_FFT_Length / 2; ++j)
            {
              Serial << FFTBuffer[j] << " ";
            }
            Serial << "\n";
          }
          memset(BandDataBuffer, 0, sizeof(int16_t)*NUMBER_OF_BANDS);
          for(int i = 0; i < m_FFT_Length/2; ++i)
          {
            float freq = GetFreqForBin(i);
            int bandIndex = 0;
            
            if(freq > 0 && freq <= 20) bandIndex = 0;
            else if(freq > 20 && freq <= 25) bandIndex = 1;
            else if(freq > 25 && freq <= 31.5) bandIndex = 2;
            else if(freq > 31.5 && freq <= 40) bandIndex = 3;
            else if(freq > 40 && freq <= 50) bandIndex = 4;
            else if(freq > 50 && freq <= 63) bandIndex = 5;
            else if(freq > 63 && freq <= 80) bandIndex = 6;
            else if(freq > 80 && freq <= 100) bandIndex = 7;
            else if(freq > 100 && freq <= 125) bandIndex = 8;
            else if(freq > 125 && freq <= 160) bandIndex = 9;
            else if(freq > 160 && freq <= 200) bandIndex = 10;
            else if(freq > 200 && freq <= 250) bandIndex = 11;
            else if(freq > 250 && freq <= 315) bandIndex = 12;
            else if(freq > 315 && freq <= 400) bandIndex = 13;
            else if(freq > 400 && freq <= 500) bandIndex = 14;
            else if(freq > 500 && freq <= 630) bandIndex = 15;
            else if(freq > 630 && freq <= 800) bandIndex = 16;
            else if(freq > 800 && freq <= 1000) bandIndex = 17;
            else if(freq > 1000 && freq <= 1250) bandIndex = 18;
            else if(freq > 1250 && freq <= 1600) bandIndex = 19;
            else if(freq > 1600 && freq <= 2000) bandIndex = 20;
            else if(freq > 2000 && freq <= 2500) bandIndex = 21;
            else if(freq > 2500 && freq <= 3150) bandIndex = 22;
            else if(freq > 3150 && freq <= 4000) bandIndex = 23;
            else if(freq > 4000 && freq <= 5000) bandIndex = 24;
            else if(freq > 5000 && freq <= 6300) bandIndex = 25;
            else if(freq > 6300 && freq <= 8000) bandIndex = 26;
            else if(freq > 8000 && freq <= 10000) bandIndex = 27;
            else if(freq > 10000 && freq <= 12500) bandIndex = 28;
            else if(freq > 12500 && freq <= 16000) bandIndex = 29;
            else if(freq > 16000 && freq <= 20000) bandIndex = 30;
            else if(freq > 20000 && freq <= 40000) bandIndex = 31;
            
            BandDataBuffer[bandIndex] += FFTBuffer[i];
          }
        }
      } 
    }
  }
}

void FFT_Calculator::ProcessRightFFTQueue(int messageCount)
{
  for(uint8_t i = 0; i < messageCount; ++i)
  {
    if ( xQueueReceive(m_FFT_Right_Data_Input_Buffer_queue, m_FFT_Right_Buffer_Data, portMAX_DELAY) != pdTRUE ){ Serial.println("Error Getting Queue Data");}
    else
    {
      if(true == FFT_CALCULATOR_INPUTDATA_DEBUG) Serial << "Data R: ";
      for(int i = 0; i < m_InputByteCount; ++i)
      {
        if(true == FFT_CALCULATOR_LOOPS_DEBUG)Serial << "Right Loop Count: "<< i << " of " << m_InputByteCount << "\n";
        m_FFT_Right_Data[m_FFT_Right_Buffer_Index] = (m_FFT_Right_Buffer_Data[i] >> 8) & 0x0000FFFF;
        if(true == FFT_CALCULATOR_INPUTDATA_DEBUG) Serial << m_FFT_Right_Buffer_Data[i] << "|" << m_FFT_Right_Data[m_FFT_Right_Buffer_Index] << "\n";
        ++m_FFT_Right_Buffer_Index;
        
        if(m_FFT_Right_Buffer_Index >= m_FFT_Length)
        {
          m_FFT_Right_Buffer_Index = 0;
          ZeroFFT(m_FFT_Right_Data, m_FFT_Length);
          if(true == FFT_CALCULATOR_OUTPUTDATA_DEBUG)
          {
            Serial << "FFT R: ";
            for(int j = 0; j < m_FFT_Length / 2; ++j)
            {
              Serial << m_FFT_Right_Data[j] << " ";
            }
            Serial << "\n";
          }
          memset(m_Right_Band_Values, 0, sizeof(int16_t)*NUMBER_OF_BANDS);
          for(int i = 0; i < m_FFT_Length/2; ++i)
          {
            float freq = GetFreqForBin(i);
            int bandIndex = 0;
            
            if(freq > 0 && freq <= 20) bandIndex = 0;
            else if(freq > 20 && freq <= 25) bandIndex = 1;
            else if(freq > 25 && freq <= 31.5) bandIndex = 2;
            else if(freq > 31.5 && freq <= 40) bandIndex = 3;
            else if(freq > 40 && freq <= 50) bandIndex = 4;
            else if(freq > 50 && freq <= 63) bandIndex = 5;
            else if(freq > 63 && freq <= 80) bandIndex = 6;
            else if(freq > 80 && freq <= 100) bandIndex = 7;
            else if(freq > 100 && freq <= 125) bandIndex = 8;
            else if(freq > 125 && freq <= 160) bandIndex = 9;
            else if(freq > 160 && freq <= 200) bandIndex = 10;
            else if(freq > 200 && freq <= 250) bandIndex = 11;
            else if(freq > 250 && freq <= 315) bandIndex = 12;
            else if(freq > 315 && freq <= 400) bandIndex = 13;
            else if(freq > 400 && freq <= 500) bandIndex = 14;
            else if(freq > 500 && freq <= 630) bandIndex = 15;
            else if(freq > 630 && freq <= 800) bandIndex = 16;
            else if(freq > 800 && freq <= 1000) bandIndex = 17;
            else if(freq > 1000 && freq <= 1250) bandIndex = 18;
            else if(freq > 1250 && freq <= 1600) bandIndex = 19;
            else if(freq > 1600 && freq <= 2000) bandIndex = 20;
            else if(freq > 2000 && freq <= 2500) bandIndex = 21;
            else if(freq > 2500 && freq <= 3150) bandIndex = 22;
            else if(freq > 3150 && freq <= 4000) bandIndex = 23;
            else if(freq > 4000 && freq <= 5000) bandIndex = 24;
            else if(freq > 5000 && freq <= 6300) bandIndex = 25;
            else if(freq > 6300 && freq <= 8000) bandIndex = 26;
            else if(freq > 8000 && freq <= 10000) bandIndex = 27;
            else if(freq > 10000 && freq <= 12500) bandIndex = 28;
            else if(freq > 12500 && freq <= 16000) bandIndex = 29;
            else if(freq > 16000 && freq <= 20000) bandIndex = 30;
            else if(freq > 20000 && freq <= 40000) bandIndex = 31;
            
            m_Right_Band_Values[bandIndex] += m_FFT_Right_Data[i];
          }
        }
      } 
    }
    if(xQueueSend(m_FFT_Right_BandData_Output_Buffer_queue, m_Right_Band_Values, portMAX_DELAY) != pdTRUE){Serial.println("Error Setting Queue");}
  }
}

void FFT_Calculator::ProcessLeftFFTQueue(int messageCount)
{
  for(uint8_t i = 0; i < messageCount; ++i)
  {
    if ( xQueueReceive(m_FFT_Left_Data_Input_Buffer_queue, m_FFT_Left_Buffer_Data, portMAX_DELAY) != pdTRUE ){ Serial.println("Error Getting Queue Data");}
    else
    {
      if(true == FFT_CALCULATOR_INPUTDATA_DEBUG) Serial << "Data L: ";
      for(int i = 0; i < m_InputByteCount; ++i)
      {
        if(true == FFT_CALCULATOR_LOOPS_DEBUG)Serial << "Left Loop Count: "<< i << " of " << m_InputByteCount << "\n";
        m_FFT_Left_Data[m_FFT_Left_Buffer_Index] = (m_FFT_Left_Buffer_Data[i] >> 8) & 0x0000FFFF;
        if(true == FFT_CALCULATOR_INPUTDATA_DEBUG) Serial << m_FFT_Left_Buffer_Data[i] << "|" << m_FFT_Left_Data[m_FFT_Left_Buffer_Index] << "\n";
        ++m_FFT_Left_Buffer_Index;
        
        if(m_FFT_Left_Buffer_Index >= m_FFT_Length)
        {
          m_FFT_Left_Buffer_Index = 0;
          ZeroFFT(m_FFT_Left_Data, m_FFT_Length);
          if(true == FFT_CALCULATOR_OUTPUTDATA_DEBUG)
          {
            Serial << "FFT L: ";
            for(int j = 0; j < m_FFT_Length / 2; ++j)
            {
              Serial << m_FFT_Left_Data[j] << " ";
            }
            Serial << "\n";
          }
          
          memset(m_Left_Band_Values, 0, sizeof(int16_t)*NUMBER_OF_BANDS);
          for(int i = 0; i < m_FFT_Length/2; ++i)
          {
            
            float freq = GetFreqForBin(i);
            int bandIndex = 0;
            
            if(freq > 0 && freq <= 20) bandIndex = 0;
            else if(freq > 20 && freq <= 25) bandIndex = 1;
            else if(freq > 25 && freq <= 31.5) bandIndex = 2;
            else if(freq > 31.5 && freq <= 40) bandIndex = 3;
            else if(freq > 40 && freq <= 50) bandIndex = 4;
            else if(freq > 50 && freq <= 63) bandIndex = 5;
            else if(freq > 63 && freq <= 80) bandIndex = 6;
            else if(freq > 80 && freq <= 100) bandIndex = 7;
            else if(freq > 100 && freq <= 125) bandIndex = 8;
            else if(freq > 125 && freq <= 160) bandIndex = 9;
            else if(freq > 160 && freq <= 200) bandIndex = 10;
            else if(freq > 200 && freq <= 250) bandIndex = 11;
            else if(freq > 250 && freq <= 315) bandIndex = 12;
            else if(freq > 315 && freq <= 400) bandIndex = 13;
            else if(freq > 400 && freq <= 500) bandIndex = 14;
            else if(freq > 500 && freq <= 630) bandIndex = 15;
            else if(freq > 630 && freq <= 800) bandIndex = 16;
            else if(freq > 800 && freq <= 1000) bandIndex = 17;
            else if(freq > 1000 && freq <= 1250) bandIndex = 18;
            else if(freq > 1250 && freq <= 1600) bandIndex = 19;
            else if(freq > 1600 && freq <= 2000) bandIndex = 20;
            else if(freq > 2000 && freq <= 2500) bandIndex = 21;
            else if(freq > 2500 && freq <= 3150) bandIndex = 22;
            else if(freq > 3150 && freq <= 4000) bandIndex = 23;
            else if(freq > 4000 && freq <= 5000) bandIndex = 24;
            else if(freq > 5000 && freq <= 6300) bandIndex = 25;
            else if(freq > 6300 && freq <= 8000) bandIndex = 26;
            else if(freq > 8000 && freq <= 10000) bandIndex = 27;
            else if(freq > 10000 && freq <= 12500) bandIndex = 28;
            else if(freq > 12500 && freq <= 16000) bandIndex = 29;
            else if(freq > 16000 && freq <= 20000) bandIndex = 30;
            else if(freq > 20000 && freq <= 40000) bandIndex = 31;
  
            m_Left_Band_Values[bandIndex] += m_FFT_Left_Data[i];
          }
        }
      } 
    }
    if(xQueueSend(m_FFT_Left_BandData_Output_Buffer_queue, m_Left_Band_Values, portMAX_DELAY) != pdTRUE){Serial.println("Error Setting Queue");}
  }
}
float FFT_Calculator::GetFreqForBin(unsigned int bin)
{
  if(bin > m_FFT_Length/2) bin = m_FFT_Length/2;
  if(bin < 0) bin = 0;
  return FFT_BIN(bin, m_SampleRate, m_FFT_Length);
}
