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

FFT_Calculator::FFT_Calculator(String Title, DataManager &DataManager): Task(Title, DataManager)
{
  
}
FFT_Calculator::~FFT_Calculator()
{
  m_DataManager.DeRegisterForEventNotification(this, "Right_Channel_Sound_Buffer_Data");
  m_DataManager.DeRegisterForEventNotification(this, "Left_Channel_Sound_Buffer_Data");
}
void FFT_Calculator::Setup()
{
  if(true == DEBUG_FFT_CALCULATOR) Serial << "Setup FFT Calculator\n";
  m_DataManager.RegisterForEventNotification(this, "Right_Channel_Sound_Buffer_Data");
  m_DataManager.RegisterForEventNotification(this, "Left_Channel_Sound_Buffer_Data");
}
bool FFT_Calculator::CanRunMyTask()
{
  return false;
}
void FFT_Calculator::RunMyTask()
{
}

void FFT_Calculator::EventSystemNotification(String context)
{

  //DataManager
  if(String("Sound_Buffer_Data") == context)
  {
  }
  else if(String("Right_Channel_Sound_Buffer_Data") == context)
  {
    for(int i = 0; i < I2S_BUFFER_SIZE; ++i)
    {
      if(true == DEBUG_FFT_CALCULATOR_LOOPS)Serial << "Right Loop Count: "<< i << " of " << I2S_BUFFER_SIZE << "\n";
      m_FFT_Right_Buffer_Data[m_FFT_Right_Buffer_Index] = m_DataManager.GetValueAtIndex<int32_t>("Right_Channel_Sound_Buffer_Data", I2S_BUFFER_SIZE, i);
      ++m_FFT_Right_Buffer_Index;
      if(m_FFT_Right_Buffer_Index >= FFT_LENGTH)
      {
        m_FFT_Right_Buffer_Index = 0;
        for(int j = 0; j < FFT_LENGTH; ++j)
        {
          m_FFT_Right_Data[j] = (m_FFT_Right_Buffer_Data[j] >> 8) & 0x0000FFFF;
        }
        if(true == DEBUG_FFT_CALCULATOR_INPUTDATA)
        {
          Serial << "Data R: ";
          for(int j = 0; j < FFT_LENGTH; ++j)
          {
            Serial << m_FFT_Right_Buffer_Data[j] << "|" << m_FFT_Right_Data[j] << " ";
          }
          Serial << "\n";
        }
        ZeroFFT(m_FFT_Right_Data, FFT_LENGTH);
        if(true == DEBUG_FFT_CALCULATOR_OUTPUTDATA)
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
  else if(String("Left_Channel_Sound_Buffer_Data") == context)
  {
    for(int i = 0; i < I2S_BUFFER_SIZE; ++i)
    {
      if(true == DEBUG_FFT_CALCULATOR_LOOPS)Serial << "Left Loop Count: "<< i << " of " << I2S_BUFFER_SIZE << "\n";
      m_FFT_Left_Buffer_Data[m_FFT_Left_Buffer_Index] = m_DataManager.GetValueAtIndex<int32_t>("Left_Channel_Sound_Buffer_Data", I2S_BUFFER_SIZE, i);
      ++m_FFT_Left_Buffer_Index;
      if(m_FFT_Left_Buffer_Index >= FFT_LENGTH)
      {
        m_FFT_Left_Buffer_Index = 0;
        for(int j = 0; j < FFT_LENGTH; ++j)
        {
          m_FFT_Left_Data[j] = (m_FFT_Left_Buffer_Data[j] >> 8) & 0x0000FFFF;
        }
        if(true == DEBUG_FFT_CALCULATOR_INPUTDATA)
        {
          Serial << "Data L: ";
          for(int j = 0; j < FFT_LENGTH; ++j)
          {
            Serial << m_FFT_Left_Buffer_Data[j] << "|" << m_FFT_Left_Data[j] << " ";
          }
          Serial << "\n";
        }
        ZeroFFT(m_FFT_Left_Data, FFT_LENGTH);
        if(true == DEBUG_FFT_CALCULATOR_OUTPUTDATA)
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
