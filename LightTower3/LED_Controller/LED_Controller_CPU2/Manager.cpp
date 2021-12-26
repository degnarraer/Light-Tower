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

#include "Manager.h"

Manager::Manager( String Title
                , SerialDataLink &SerialDataLink
                , StatisticalEngine &StatisticalEngine
                , I2S_Device& I2S_In): NamedItem(Title)
                                     , m_SerialDataLink(SerialDataLink)
                                     , m_StatisticalEngine(StatisticalEngine)
                                     , m_I2S_In(I2S_In)
{
}
Manager::~Manager()
{
}

void Manager::Setup()
{
  if(true == EVENT_HANDLER_DEBUG) Serial << "Setup i2s Event Handler\n";
  m_I2S_In.StartDevice();
}

void Manager::RunTask()
{
  m_I2S_In.ProcessEventQueue();
  ProcessEventQueue();
}

void Manager::ProcessEventQueue()
{
  ProcessDataBufferQueue();
  ProcessRightChannelDataBufferQueue();
  ProcessLeftChannelDataBufferQueue();
  ProcessRightChannelSoundDataQueue();
  ProcessLeftChannelSoundDataQueue();
}

void Manager::ProcessDataBufferQueue()
{
  if(NULL != m_I2S_In.GetDataBufferQueue())
  {
    if(uxQueueMessagesWaiting(m_I2S_In.GetDataBufferQueue()) > 0)
    {
      if(true == EVENT_HANDLER_DEBUG) Serial << "Manager ESP32 Data Buffer Queue: " << uxQueueMessagesWaiting(m_I2S_In.GetDataBufferQueue()) << "\n";
      char* DataBuffer = (char*)malloc(m_I2S_In.GetBytesToRead());
      if ( xQueueReceive(m_I2S_In.GetDataBufferQueue(), DataBuffer, portMAX_DELAY) == pdTRUE )
      {
        if(true == PRINT_DATA_DEBUG)
        {
          for(int i = 0; i < m_I2S_In.GetSampleCount(); ++i)
          {
            Serial << m_I2S_In.GetDataBufferValue(DataBuffer, i) << "\n";
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
void Manager::ProcessRightChannelDataBufferQueue()
{
  if(NULL != m_I2S_In.GetRightDataBufferQueue())
  {
    if(uxQueueMessagesWaiting(m_I2S_In.GetRightDataBufferQueue()) > 0)
    {
      if(true == EVENT_HANDLER_DEBUG) Serial << "Manager ESP32 Right Data Buffer Queue: " << uxQueueMessagesWaiting(m_I2S_In.GetRightDataBufferQueue()) << "\n";
      char* DataBuffer = (char*)malloc(m_I2S_In.GetChannelBytesToRead());
      if ( xQueueReceive(m_I2S_In.GetRightDataBufferQueue(), DataBuffer, portMAX_DELAY) == pdTRUE )
      {
        if(true == PRINT_RIGHT_CHANNEL_DATA_DEBUG)
        {
          for(int i = 0; i < m_I2S_In.GetSampleCount(); ++i)
          {
            Serial << m_I2S_In.GetDataBufferValue(DataBuffer, i) << "\n";
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
void Manager::ProcessLeftChannelDataBufferQueue()
{
  if(NULL != m_I2S_In.GetLeftDataBufferQueue())
  {
    if(uxQueueMessagesWaiting(m_I2S_In.GetLeftDataBufferQueue()) > 0)
    {
      if(true == EVENT_HANDLER_DEBUG) Serial << "Manager ESP32 Left Data Buffer Queue: " << uxQueueMessagesWaiting(m_I2S_In.GetLeftDataBufferQueue()) << "\n";
      char* DataBuffer = (char*)malloc(m_I2S_In.GetChannelBytesToRead());
      if ( xQueueReceive(m_I2S_In.GetLeftDataBufferQueue(), DataBuffer, portMAX_DELAY) == pdTRUE )
      {
        if(true == PRINT_LEFT_CHANNEL_DATA_DEBUG)
        {
          for(int i = 0; i < m_I2S_In.GetSampleCount(); ++i)
          {
            Serial << m_I2S_In.GetDataBufferValue(DataBuffer, i) << "\n";
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
void Manager::ProcessRightChannelSoundDataQueue()
{
  MoveDataFromQueueToQueue<int16_t>( m_SerialDataLink.GetQueueHandleRXForDataItem("FFT_R")
                                   , m_StatisticalEngine.GetFFTRightBandDataInputQueue()
                                   , m_StatisticalEngine.GetFFTRightBandDataBufferSize()
                                   , false
                                   , false );

  MoveDataFromQueueToQueue<float>( m_SerialDataLink.GetQueueHandleRXForDataItem("R_Pow")
                                 , m_StatisticalEngine.GetRightChannelNormalizedPowerInputQueue()
                                 , m_StatisticalEngine.GetRightChannelNormalizedPowerSize()
                                 , false
                                 , false );
                          
  MoveDataFromQueueToQueue<float>( m_SerialDataLink.GetQueueHandleRXForDataItem("R_DB")
                                 , m_StatisticalEngine.GetRightChannelDBInputQueue()
                                 , m_StatisticalEngine.GetRightChannelDBSize()
                                 , false
                                 , false );
                                   
  MoveDataFromQueueToQueue<int32_t>( m_SerialDataLink.GetQueueHandleRXForDataItem("R_Min")
                                   , m_StatisticalEngine.GetRightChannelPowerMinInputQueue()
                                   , m_StatisticalEngine.GetRightChannelPowerMinSize()
                                   , false
                                   , false );
                                   
  MoveDataFromQueueToQueue<int32_t>( m_SerialDataLink.GetQueueHandleRXForDataItem("R_Max")
                                   , m_StatisticalEngine.GetRightChannelPowerMaxInputQueue()
                                   , m_StatisticalEngine.GetRightChannelPowerMaxSize()
                                   , false
                                   , false );
}
void Manager::ProcessLeftChannelSoundDataQueue()
{
  MoveDataFromQueueToQueue<int16_t>( m_SerialDataLink.GetQueueHandleRXForDataItem("FFT_L")
                                   , m_StatisticalEngine.GetFFTLeftBandDataInputQueue()
                                   , m_StatisticalEngine.GetFFTLeftBandDataBufferSize()
                                   , false
                                   , false );

  MoveDataFromQueueToQueue<float>( m_SerialDataLink.GetQueueHandleRXForDataItem("L_Pow")
                                 , m_StatisticalEngine.GetLeftChannelNormalizedPowerInputQueue()
                                 , m_StatisticalEngine.GetLeftChannelNormalizedPowerSize()
                                 , false
                                 , false );
                          
  MoveDataFromQueueToQueue<float>( m_SerialDataLink.GetQueueHandleRXForDataItem("L_DB")
                                 , m_StatisticalEngine.GetLeftChannelDBInputQueue()
                                 , m_StatisticalEngine.GetLeftChannelDBSize()
                                 , false
                                 , false );
                                   
  MoveDataFromQueueToQueue<int32_t>( m_SerialDataLink.GetQueueHandleRXForDataItem("L_Min")
                                   , m_StatisticalEngine.GetLeftChannelPowerMinInputQueue()
                                   , m_StatisticalEngine.GetLeftChannelPowerMinSize()
                                   , false
                                   , false );
                                   
  MoveDataFromQueueToQueue<int32_t>( m_SerialDataLink.GetQueueHandleRXForDataItem("L_Max")
                                   , m_StatisticalEngine.GetLeftChannelPowerMaxInputQueue()
                                   , m_StatisticalEngine.GetLeftChannelPowerMaxSize()
                                   , false
                                   , false );
}
