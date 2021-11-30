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
#define I2S_BUFFER_COUNT 10
#define I2S_BUFFER_SIZE 200

#include "i2s_Manager.h"

I2S_Manager::I2S_Manager(String Title, FFT_Calculator &fftCalculator): m_Title(Title)
                                                                     , m_FFT_Calculator(fftCalculator){}
I2S_Manager::~I2S_Manager()
{
  delete m_Mic;
  delete m_Speaker;
}

void I2S_Manager::Setup()
{
  if(true == EVENT_HANDLER_DEBUG) Serial << "Setup i2s Event Handler\n";
  m_Mic = new I2S_Device( "Microphone"
                        , I2S_NUM_0
                        , i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX)
                        , 44100
                        , I2S_BITS_PER_SAMPLE_32BIT
                        , I2S_CHANNEL_FMT_RIGHT_LEFT
                        , i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB)
                        , I2S_CHANNEL_STEREO
                        , I2S_BUFFER_COUNT
                        , I2S_BUFFER_SIZE
                        , 12
                        , 13
                        , 14
                        , I2S_PIN_NO_CHANGE
                        , 32 );
                        
    m_Speaker = new I2S_Device( "Speaker"
                              , I2S_NUM_1
                              , i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_TX)
                              , 44100
                              , I2S_BITS_PER_SAMPLE_32BIT
                              , I2S_CHANNEL_FMT_RIGHT_LEFT
                              , i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB)
                              , I2S_CHANNEL_STEREO
                              , I2S_BUFFER_COUNT
                              , I2S_BUFFER_SIZE
                              , 26
                              , 25
                              , I2S_PIN_NO_CHANGE
                              , 33
                              , I2S_PIN_NO_CHANGE );

  m_Mic->Setup();
  m_Speaker->Setup();
  m_FFT_Calculator.Setup(m_Mic->GetChannelBytesToRead(), m_Mic->GetSampleRate(), 4096);
  m_Mic->StartDevice();
  m_Speaker->StartDevice();
}

void I2S_Manager::RunTask()
{
  m_Mic->ProcessEventQueue();
  m_Speaker->ProcessEventQueue();
  ProcessEventQueue();
}

void I2S_Manager::ProcessEventQueue()
{
  if(NULL != m_Mic->GetDataBufferQueue())
  {
    uint8_t i2sMicBufferMsgCount = uxQueueMessagesWaiting(m_Mic->GetDataBufferQueue());
    if(true == EVENT_HANDLER_DEBUG) Serial << "I2S_Manager Mic Data Buffer Queue: " << i2sMicBufferMsgCount << "\n";
    
    for (uint8_t i = 0; i < i2sMicBufferMsgCount; ++i)
    {
      int32_t* DataBuffer = (int32_t*)malloc(m_Mic->GetBytesToRead());
      if ( xQueueReceive(m_Mic->GetDataBufferQueue(), DataBuffer, portMAX_DELAY) == pdTRUE )
      {
        m_Speaker->SetSoundBufferData(DataBuffer);
      }
      else
      {
        Serial << "Error Receiving Queue!";
      }
      delete DataBuffer;
    }
  }
  
  if(NULL != m_Mic->GetRightDataBufferQueue())
  {
    uint8_t i2sMicRightBufferMsgCount = uxQueueMessagesWaiting(m_Mic->GetRightDataBufferQueue());
    if(true == EVENT_HANDLER_DEBUG) Serial << "I2S_Manager Mic Right Data Buffer Queue: " << i2sMicRightBufferMsgCount << "\n";
    
    for (uint8_t i = 0; i < i2sMicRightBufferMsgCount; ++i)
    {
      int32_t* DataBuffer = (int32_t*)malloc(m_Mic->GetChannelBytesToRead());
      if ( xQueueReceive(m_Mic->GetRightDataBufferQueue(), DataBuffer, portMAX_DELAY) == pdTRUE )
      {
        if(true == EVENT_HANDLER_DEBUG)Serial << "I2S_Manager Adding to FFT Right Data Queue\n";
        if(xQueueSend(m_FFT_Calculator.GetFFTRightDataQueue(), DataBuffer, portMAX_DELAY) != pdTRUE){Serial.println("Error Setting Queue");}
      }
      else
      {
        Serial << "Error Receiving Queue!";
      }
      delete DataBuffer;
    }
  }
  
  if(NULL != m_Mic->GetLeftDataBufferQueue())
  {
    uint8_t i2sMicLeftBufferMsgCount = uxQueueMessagesWaiting(m_Mic->GetLeftDataBufferQueue());
    if(true == EVENT_HANDLER_DEBUG) Serial << "I2S_Manager Mic Left Data Buffer Queue: " << i2sMicLeftBufferMsgCount << "\n";
    
    for (uint8_t i = 0; i < i2sMicLeftBufferMsgCount; ++i)
    {
      int32_t* DataBuffer = (int32_t*)malloc(m_Mic->GetChannelBytesToRead());
      if ( xQueueReceive(m_Mic->GetLeftDataBufferQueue(), DataBuffer, portMAX_DELAY) == pdTRUE )
      {
        if(true == EVENT_HANDLER_DEBUG)Serial << "I2S_Manager Adding to FFT Left Data Queue\n";
        if(xQueueSend(m_FFT_Calculator.GetFFTLeftDataQueue(), DataBuffer, portMAX_DELAY) != pdTRUE){Serial.println("Error Setting Queue");}
      }
      else
      {
        Serial << "Error Receiving Queue!";
      }
      delete DataBuffer;
    }
  }

}
