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
  m_DataManager.DeRegisterForEventNotification(this, m_DataManager.MicrophoneRightDataReady);
  m_DataManager.DeRegisterForEventNotification(this, m_DataManager.MicrophoneLeftDataReady);
}
void FFT_Calculator::Setup()
{
  if(true == DEBUG_FFT_CALCULATOR) Serial << "Setup FFT Calculator\n";
  m_DataManager.RegisterForEventNotification(this, m_DataManager.MicrophoneRightDataReady);
  m_DataManager.RegisterForEventNotification(this, m_DataManager.MicrophoneLeftDataReady);
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

  if(m_DataManager.MicrophoneDataReady == context)
  {
  }
  else if(m_DataManager.MicrophoneRightDataReady == context)
  {
    for(int i = 0; i < m_DataManager.GetSampleCount(); ++i)
    {
      if(true == DEBUG_FFT_CALCULATOR_LOOPS)Serial << "Right Loop Count: "<< i << " of " << m_DataManager.GetSampleCount() << "\n";
      m_FFT_Right_Buffer_Data[m_FFT_Right_Buffer_Index] = m_DataManager.GetRightChannelSoundBufferData(i);
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
  else if(m_DataManager.MicrophoneLeftDataReady == context)
  {
    for(int i = 0; i < m_DataManager.GetSampleCount(); ++i)
    {
      if(true == DEBUG_FFT_CALCULATOR_LOOPS)Serial << "Left Loop Count: "<< i << " of " << m_DataManager.GetSampleCount() << "\n";
      m_FFT_Left_Buffer_Data[m_FFT_Left_Buffer_Index] = m_DataManager.GetLeftChannelSoundBufferData(i);
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
