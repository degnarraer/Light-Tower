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
#define I2S_BUFFER_SIZE 100

#include "Manager.h"

Manager::Manager(String Title): NamedItem(Title)
{
}
Manager::~Manager()
{
  delete m_ESP32;
}

void Manager::Setup()
{
  if(true == EVENT_HANDLER_DEBUG) Serial << "Setup i2s Event Handler\n";
  m_ESP32 = new I2S_Device( "ESP32"
                        , I2S_NUM_0
                        , i2s_mode_t(I2S_MODE_SLAVE | I2S_MODE_RX)
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
                        , I2S_PIN_NO_CHANGE );


  m_ESP32->Setup();
  m_ESP32->StartDevice();
}

void Manager::RunTask()
{
  m_ESP32->ProcessEventQueue();
  ProcessEventQueue();
}

void Manager::ProcessEventQueue()
{
  if(NULL != m_ESP32->GetDataBufferQueue())
  {
    if(uxQueueMessagesWaiting(m_ESP32->GetDataBufferQueue()) > 0)
    {
      if(true == EVENT_HANDLER_DEBUG) Serial << "Manager ESP32 Data Buffer Queue: " << uxQueueMessagesWaiting(m_ESP32->GetDataBufferQueue()) << "\n";
      char* DataBuffer = (char*)malloc(m_ESP32->GetBytesToRead());
      if ( xQueueReceive(m_ESP32->GetDataBufferQueue(), DataBuffer, portMAX_DELAY) == pdTRUE )
      {
        if(true == PRINT_DATA_DEBUG)
        {
          for(int i = 0; i < m_ESP32->GetSampleCount(); ++i)
          {
            Serial << m_ESP32->GetDataBufferValue(DataBuffer, i) << "\n";
          }
        }
      }
      else
      {
        Serial << "Error Receiving Queue!";
      }
      delete DataBuffer;
    }
  }
  
  if(NULL != m_ESP32->GetRightDataBufferQueue())
  {
    if(uxQueueMessagesWaiting(m_ESP32->GetRightDataBufferQueue()) > 0)
    {
      if(true == EVENT_HANDLER_DEBUG) Serial << "Manager ESP32 Right Data Buffer Queue: " << uxQueueMessagesWaiting(m_ESP32->GetRightDataBufferQueue()) << "\n";
      char* DataBuffer = (char*)malloc(m_ESP32->GetChannelBytesToRead());
      if ( xQueueReceive(m_ESP32->GetRightDataBufferQueue(), DataBuffer, portMAX_DELAY) == pdTRUE )
      {
        if(true == PRINT_RIGHT_CHANNEL_DATA_DEBUG)
        {
          for(int i = 0; i < m_ESP32->GetSampleCount(); ++i)
          {
            Serial << m_ESP32->GetDataBufferValue(DataBuffer, i) << "\n";
          }
        }
      }
      else
      {
        Serial << "Error Receiving Queue!";
      }
      delete DataBuffer;
    }
  }
  
  if(NULL != m_ESP32->GetLeftDataBufferQueue())
  {
    if(uxQueueMessagesWaiting(m_ESP32->GetLeftDataBufferQueue()) > 0)
    {
      if(true == EVENT_HANDLER_DEBUG) Serial << "Manager ESP32 Left Data Buffer Queue: " << uxQueueMessagesWaiting(m_ESP32->GetLeftDataBufferQueue()) << "\n";
      char* DataBuffer = (char*)malloc(m_ESP32->GetChannelBytesToRead());
      if ( xQueueReceive(m_ESP32->GetLeftDataBufferQueue(), DataBuffer, portMAX_DELAY) == pdTRUE )
      {
        if(true == PRINT_LEFT_CHANNEL_DATA_DEBUG)
        {
          for(int i = 0; i < m_ESP32->GetSampleCount(); ++i)
          {
            Serial << m_ESP32->GetDataBufferValue(DataBuffer, i) << "\n";
          }
        }
      }
      else
      {
        Serial << "Error Receiving Queue!";
      }
      delete DataBuffer;
    }
  }
}
