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

FFT_Calculator::FFT_Calculator(String Title): m_Title(Title)
{
  
}
FFT_Calculator::~FFT_Calculator()
{
}
void FFT_Calculator::Setup(size_t BufferCount)
{
  m_BufferCount = BufferCount;
  m_FFT_Right_Buffer_Data = (int32_t*)malloc(m_BufferCount);
  m_FFT_Left_Buffer_Data = (int32_t*)malloc(m_BufferCount);
  m_BytesToRead = sizeof(int32_t) * FFT_LENGTH;
  
  Serial << m_Title << ": Creating Right FFT queue.\n";
  m_FFT_Right_Data_Buffer_queue = xQueueCreate(10, m_BufferCount );
  if(m_FFT_Right_Data_Buffer_queue == NULL){Serial.println("Error creating the Right Channel FFT queue");}
  
  Serial << m_Title << ": Creating Left FFT queue.\n";
  m_FFT_Left_Data_Buffer_queue = xQueueCreate(10, m_BufferCount );
  if(m_FFT_Left_Data_Buffer_queue == NULL){Serial.println("Error creating the Left Channel FFT queue");}
}

void FFT_Calculator::ProcessEventQueue()
{
  if(NULL != m_FFT_Right_Data_Buffer_queue)
  {
    uint8_t fftRightDataBufferMsgCount = uxQueueMessagesWaiting(m_FFT_Right_Data_Buffer_queue);
    if(true == FFT_CALCULATOR_DEBUG) Serial << "FFT Right Data Buffer Queue: " << fftRightDataBufferMsgCount << "\n";
    ProcessRightFFTQueue(fftRightDataBufferMsgCount);
  }
  
  if(NULL != m_FFT_Left_Data_Buffer_queue)
  {
    uint8_t fftLeftDataBufferMsgCount = uxQueueMessagesWaiting(m_FFT_Left_Data_Buffer_queue);
    if(true == FFT_CALCULATOR_DEBUG) Serial << "FFT Left Data Buffer Queue: " << fftLeftDataBufferMsgCount << "\n";
    ProcessLeftFFTQueue(fftLeftDataBufferMsgCount);
  }
}



void FFT_Calculator::ProcessRightFFTQueue(int messageCount)
{
  for(uint8_t i = 0; i < messageCount; ++i)
  {
    if ( xQueueReceive(m_FFT_Right_Data_Buffer_queue, m_FFT_Right_Buffer_Data, portMAX_DELAY) != pdTRUE ){ Serial.println("Error Getting Queue Data");}
    else
    {
      if(true == FFT_CALCULATOR_INPUTDATA_DEBUG) Serial << "Data R: ";
      for(int i = 0; i < m_BufferCount; ++i)
      {
        if(true == FFT_CALCULATOR_LOOPS_DEBUG)Serial << "Right Loop Count: "<< i << " of " << m_BufferCount << "\n";
        m_FFT_Right_Data[m_FFT_Right_Buffer_Index] = (m_FFT_Right_Buffer_Data[i] >> 8) & 0x0000FFFF;
        if(true == FFT_CALCULATOR_INPUTDATA_DEBUG) Serial << m_FFT_Right_Buffer_Data[i] << "|" << m_FFT_Right_Data[m_FFT_Right_Buffer_Index] << "\n";
        ++m_FFT_Right_Buffer_Index;
        
        if(m_FFT_Right_Buffer_Index >= FFT_LENGTH)
        {
          m_FFT_Right_Buffer_Index = 0;
          ZeroFFT(m_FFT_Right_Data, FFT_LENGTH);
          if(true == FFT_CALCULATOR_OUTPUTDATA_DEBUG)
          {
            Serial << "FFT R: ";
            for(int j = 0; j < FFT_LENGTH / 2; ++j)
            {
              Serial << m_FFT_Right_Data[j] << " ";
            }
            Serial << "\n";
          }
        }
      } 
    }
  }
}

void FFT_Calculator::ProcessLeftFFTQueue(int messageCount)
{
  for(uint8_t i = 0; i < messageCount; ++i)
  {
    if ( xQueueReceive(m_FFT_Left_Data_Buffer_queue, m_FFT_Left_Buffer_Data, portMAX_DELAY) != pdTRUE ){ Serial.println("Error Getting Queue Data");}
    else
    {
      if(true == FFT_CALCULATOR_INPUTDATA_DEBUG) Serial << "Data L: ";
      for(int i = 0; i < m_BufferCount; ++i)
      {
        if(true == FFT_CALCULATOR_LOOPS_DEBUG)Serial << "Left Loop Count: "<< i << " of " << m_BufferCount << "\n";
        m_FFT_Left_Data[m_FFT_Left_Buffer_Index] = (m_FFT_Left_Buffer_Data[i] >> 8) & 0x0000FFFF;
        if(true == FFT_CALCULATOR_INPUTDATA_DEBUG) Serial << m_FFT_Left_Buffer_Data[i] << "|" << m_FFT_Left_Data[m_FFT_Left_Buffer_Index] << "\n";
        ++m_FFT_Left_Buffer_Index;
        
        if(m_FFT_Left_Buffer_Index >= FFT_LENGTH)
        {
          m_FFT_Left_Buffer_Index = 0;
          ZeroFFT(m_FFT_Left_Data, FFT_LENGTH);
          if(true == FFT_CALCULATOR_OUTPUTDATA_DEBUG)
          {
            Serial << "FFT L: ";
            for(int j = 0; j < FFT_LENGTH / 2; ++j)
            {
              Serial << m_FFT_Left_Data[j] << " ";
            }
            Serial << "\n";
          }
        }
      } 
    }
  }
}
