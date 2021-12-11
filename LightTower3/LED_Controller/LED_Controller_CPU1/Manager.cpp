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
                , FFT_Calculator &FFTCalculator
                , SerialDataLink &SerialDataLink
                , Bluetooth_Sink &BT
                , I2S_Device &Mic
                , I2S_Device &Speaker ): NamedItem(Title)
                                       , m_FFT_Calculator(FFTCalculator)
                                       , m_SerialDataLink(SerialDataLink)
                                       , m_BT(BT)
                                       , m_Mic(Mic)
                                       , m_Speaker(Speaker)
{
}
Manager::~Manager()
{
}

void Manager::Setup()
{
  if(true == EVENT_HANDLER_DEBUG) Serial << "Setup i2s Event Handler\n";
  
  //Setup Sound Inputs
  m_Mic.ResgisterForDataBufferRXCallback(this);

  //m_Speaker->StartDevice();
  //m_Mic.StartDevice();
  m_BT.StartDevice();
}

void Manager::RunTask()
{
  m_Mic.ProcessEventQueue();
  //m_Speaker.ProcessEventQueue();
  ProcessEventQueue();
}

void Manager::ProcessEventQueue()
{
  ProcessDataBufferQueue();
  ProcessRightChannelDataBufferQueue();
  ProcessLeftChannelDataBufferQueue();
  ProcessRightFFTDataBufferQueue();
  ProcessLeftFFTDataBufferQueue();
}

void Manager::ProcessDataBufferQueue()
{
  if(NULL != m_Mic.GetDataBufferQueue())
  {
    if(uxQueueMessagesWaiting(m_Mic.GetDataBufferQueue()) > 0)
    {
      if(true == EVENT_HANDLER_DEBUG) Serial << "Manager Mic Data Buffer Queue: " << uxQueueMessagesWaiting(m_Mic.GetDataBufferQueue()) << "\n";
      int32_t* DataBuffer = (int32_t*)malloc(m_Mic.GetBytesToRead());
      if ( xQueueReceive(m_Mic.GetDataBufferQueue(), DataBuffer, portMAX_DELAY) == pdTRUE )
      {
        //m_Speaker->SetSoundBufferData(DataBuffer);
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
  MoveDataFromQueueToQueue<int32_t>( m_Mic.GetRightDataBufferQueue()
                                   , m_FFT_Calculator.GetFFTRightDataInputQueue()
                                   , m_Mic.GetChannelBytesToRead()
                                   , false
                                   , false );
}

void Manager::ProcessLeftChannelDataBufferQueue()
{
  MoveDataFromQueueToQueue<int32_t>( m_Mic.GetLeftDataBufferQueue()
                                   , m_FFT_Calculator.GetFFTLeftDataInputQueue()
                                   , m_Mic.GetChannelBytesToRead()
                                   , false
                                   , false );
}

void Manager::ProcessRightFFTDataBufferQueue()
{
  MoveDataFromQueueToQueue<int16_t>( m_FFT_Calculator.GetFFTRightBandDataOutputQueue()
                                   , m_SerialDataLink.GetQueueHandleForDataItem("FFT_RBand_Data")
                                   , m_FFT_Calculator.GetFFTRightBandDataBufferSize()
                                   , false
                                   , false );
}

void Manager::ProcessLeftFFTDataBufferQueue()
{
  MoveDataFromQueueToQueue<int16_t>( m_FFT_Calculator.GetFFTLeftBandDataOutputQueue()
                                   , m_SerialDataLink.GetQueueHandleForDataItem("FFT_LBand_Data")
                                   , m_FFT_Calculator.GetFFTLeftBandDataBufferSize()
                                   , false
                                   , false );
}
