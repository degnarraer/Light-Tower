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

#define DAC_MUTE_PIN 22
#define DAC_SF0_PIN 32
#define DAC_SF1_PIN 33

Manager::Manager( String Title
                , Sound_Processor &FFTCalculator
                , SerialDataLink &SerialDataLink
                , Bluetooth_Sink &BT
                , I2S_Device &Mic_In
                , I2S_Device &Mic_Out ): NamedItem(Title)
                                       , m_Sound_Processor(FFTCalculator)
                                       , m_SerialDataLink(SerialDataLink)
                                       , m_BT(BT)
                                       , m_Mic_In(Mic_In)
                                       , m_Mic_Out(Mic_Out)
{
}
Manager::~Manager()
{
}

void Manager::Setup()
{
  if(true == EVENT_HANDLER_DEBUG) Serial << "Setup i2s Event Handler\n";
  m_Mic_In.ResgisterForDataBufferRXCallback(this);
  m_BT.ResgisterForDataBufferRXCallback(this);
  
  pinMode(DAC_SF0_PIN, OUTPUT);
  pinMode(DAC_SF1_PIN, OUTPUT);
  pinMode(DAC_MUTE_PIN, OUTPUT);
  
  //SetInputType(InputType_Bluetooth);
  SetInputType(InputType_Microphone);
}

void Manager::SetDACMuteState(Mute_State_t MuteState)
{
  switch(MuteState)
  {
    case Mute_State_Un_Muted:
      digitalWrite(DAC_MUTE_PIN, LOW);
      m_MuteState = Mute_State_Un_Muted;
      break;
    case Mute_State_Muted:
      digitalWrite(DAC_MUTE_PIN, HIGH);
      m_MuteState = Mute_State_Muted;
      break;
    default:
      break;
  }
}

void Manager::SetDACDataFormat(DAC_Data_Format_t DAC_Data_Format)
{
  m_DAC_Data_Format = DAC_Data_Format;
  switch(m_DAC_Data_Format)
  {
    case DAC_Data_Format_Default:
    Serial << "Setting DAC Mode: Default\n";
    digitalWrite(DAC_SF0_PIN, LOW);
    digitalWrite(DAC_SF1_PIN, LOW);
    break;
    case DAC_Data_Format_LSB16:
    Serial << "Setting DAC Mode: LSB16\n";
    digitalWrite(DAC_SF0_PIN, LOW);
    digitalWrite(DAC_SF1_PIN, HIGH);   
    break;
    case DAC_Data_Format_LSB20:
    Serial << "Setting DAC Mode: LSB20\n";
    digitalWrite(DAC_SF0_PIN, HIGH);
    digitalWrite(DAC_SF1_PIN, LOW);    
    break;
    case DAC_Data_Format_LSB24:
    Serial << "Setting DAC Mode: LSB24\n";
    digitalWrite(DAC_SF0_PIN, HIGH);
    digitalWrite(DAC_SF1_PIN, HIGH);   
    break;
    default:
    digitalWrite(DAC_SF0_PIN, LOW);
    digitalWrite(DAC_SF1_PIN, LOW);    
    break;
  }
}
void Manager::RunTask()
{
  switch(m_InputType)
  {
    case InputType_Microphone:
      m_Mic_In.ProcessEventQueue();
      m_Mic_Out.ProcessEventQueue();
    break;
    case InputType_Bluetooth:
      m_BT.ProcessEventQueue();
    break;
    default:
    break;
  }
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

void Manager::SetInputType(InputType_t Type)
{
  m_InputType = Type;
  switch(m_InputType)
  {
    case InputType_Microphone:
      m_BT.StopDevice();
      m_Sound_Processor.Setup(m_Mic_In.GetChannelBytesToRead(), m_Mic_In.GetSampleRate(), 2048);
      m_Mic_In.StartDevice();
      m_Mic_Out.StartDevice();
      SetDACDataFormat(DAC_Data_Format_LSB24);
      SetDACMuteState(Mute_State_Un_Muted);
    break;
    case InputType_Bluetooth:
      m_Mic_Out.StopDevice();
      m_Mic_In.StopDevice();
      m_Sound_Processor.Setup(m_BT.GetChannelBytesToRead(), m_BT.GetSampleRate(), 2048);
      m_BT.StartDevice();
      SetDACDataFormat(DAC_Data_Format_Default);
      SetDACMuteState(Mute_State_Un_Muted);
    break;
    default:
      SetDACDataFormat(DAC_Data_Format_Default);
      SetDACMuteState(Mute_State_Un_Muted);
    break;
  }
}

//I2S_Device_Callback
//Bluetooth_Callback
void Manager::DataBufferModifyRX(String DeviceTitle, char* DataBuffer, size_t Count)
{
  if(DeviceTitle == m_Mic_In.GetTitle())
  {
    for(int i = 0; i < m_Mic_In.GetSampleCount(); ++i)
    {
      if(true == PRINT_DATA_DEBUG) Serial.println(m_Mic_In.GetDataBufferValue(DataBuffer, i));
    } 
  }
}
void Manager::RightChannelDataBufferModifyRX(String DeviceTitle, char* DataBuffer, size_t Count)
{
  if(DeviceTitle == m_Mic_In.GetTitle())
  {
    for(int i = 0; i < m_Mic_In.GetChannelSampleCount(); ++i)
    {
      if(true == PRINT_RIGHT_CHANNEL_DATA_DEBUG) Serial.println(m_Mic_In.GetDataBufferValue(DataBuffer, i));
    } 
  }
  
}
void Manager::LeftChannelDataBufferModifyRX(String DeviceTitle, char* DataBuffer, size_t Count)
{
  if(DeviceTitle == m_Mic_In.GetTitle())
  {
    for(int i = 0; i < m_Mic_In.GetChannelSampleCount(); ++i)
    {
      if(true == PRINT_LEFT_CHANNEL_DATA_DEBUG) Serial.println(m_Mic_In.GetDataBufferValue(DataBuffer, i));
    } 
  }
  
}

void Manager::ProcessDataBufferQueue()
{
  if(NULL != m_Mic_In.GetDataBufferQueue())
  {
    if(uxQueueMessagesWaiting(m_Mic_In.GetDataBufferQueue()) > 0)
    {
      if(true == EVENT_HANDLER_DEBUG) Serial << "Manager Mic Data Buffer Queue: " << uxQueueMessagesWaiting(m_Mic_In.GetDataBufferQueue()) << "\n";
      char* DataBuffer = (char*)malloc(m_Mic_In.GetBytesToRead());
      if ( xQueueReceive(m_Mic_In.GetDataBufferQueue(), DataBuffer, portMAX_DELAY) == pdTRUE )
      {
        m_Mic_Out.SetSoundBufferData(DataBuffer, m_Mic_Out.GetBytesToRead());
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
  switch(m_InputType)
  {
    case InputType_Microphone:
      MoveDataFromQueueToQueue( m_Mic_In.GetRightDataBufferQueue()
                              , m_Sound_Processor.GetFFTRightDataInputQueue()
                              , m_Mic_In.GetChannelBytesToRead()
                              , false
                              , false );
    break;
    case InputType_Bluetooth:
      MoveDataFromQueueToQueue( m_BT.GetRightDataBufferQueue()
                              , m_Sound_Processor.GetFFTRightDataInputQueue()
                              , m_BT.GetChannelBytesToRead()
                              , false
                              , false );
    break;
    default:
    break;
  }
}

void Manager::ProcessLeftChannelDataBufferQueue()
{
  switch(m_InputType)
  {
    case InputType_Microphone:
      MoveDataFromQueueToQueue( m_Mic_In.GetLeftDataBufferQueue()
                                       , m_Sound_Processor.GetFFTLeftDataInputQueue()
                                       , m_Mic_In.GetChannelBytesToRead()
                                       , false
                                       , false );
    break;
    case InputType_Bluetooth:
      MoveDataFromQueueToQueue( m_BT.GetLeftDataBufferQueue()
                                       , m_Sound_Processor.GetFFTLeftDataInputQueue()
                                       , m_BT.GetChannelBytesToRead()
                                       , false
                                       , false );
    break;
    default:
    break;
  }
}

void Manager::ProcessRightFFTDataBufferQueue()
{
  MoveDataFromQueueToQueue( m_Sound_Processor.GetFFTRightBandDataOutputQueue()
                          , m_SerialDataLink.GetQueueHandleTXForDataItem("FFT_R")
                          , m_Sound_Processor.GetFFTRightBandDataBufferSize()
                          , false
                          , false );

  MoveDataFromQueueToQueue( m_Sound_Processor.GetRightChannelNormalizedPowerOutputQueue()
                          , m_SerialDataLink.GetQueueHandleTXForDataItem("R_Pow")
                          , m_Sound_Processor.GetRightChannelNormalizedPowerSize()
                          , false
                          , false );
                          
  MoveDataFromQueueToQueue( m_Sound_Processor.GetRightChannelDBOutputQueue()
                          , m_SerialDataLink.GetQueueHandleTXForDataItem("R_DB")
                          , m_Sound_Processor.GetRightChannelDBSize()
                          , false
                          , false );
                                   
  MoveDataFromQueueToQueue( m_Sound_Processor.GetRightChannelPowerMinOutputQueue()
                          , m_SerialDataLink.GetQueueHandleTXForDataItem("R_Min")
                          , m_Sound_Processor.GetRightChannelPowerMinSize()
                          , false
                          , false );
                                   
  MoveDataFromQueueToQueue( m_Sound_Processor.GetRightChannelPowerMaxOutputQueue()
                          , m_SerialDataLink.GetQueueHandleTXForDataItem("R_Max")
                          , m_Sound_Processor.GetRightChannelPowerMaxSize()
                          , false
                          , false );
}

void Manager::ProcessLeftFFTDataBufferQueue()
{
  MoveDataFromQueueToQueue( m_Sound_Processor.GetFFTLeftBandDataOutputQueue()
                          , m_SerialDataLink.GetQueueHandleTXForDataItem("FFT_L")
                          , m_Sound_Processor.GetFFTLeftBandDataBufferSize()
                          , false
                          , false );
                                   
  MoveDataFromQueueToQueue( m_Sound_Processor.GetLeftChannelNormalizedPowerOutputQueue()
                          , m_SerialDataLink.GetQueueHandleTXForDataItem("L_Pow")
                          , m_Sound_Processor.GetLeftChannelNormalizedPowerSize()
                          , false
                          , false );
                                   
  MoveDataFromQueueToQueue( m_Sound_Processor.GetLeftChannelDBOutputQueue()
                          , m_SerialDataLink.GetQueueHandleTXForDataItem("L_DB")
                          , m_Sound_Processor.GetLeftChannelDBSize()
                          , false
                          , false );
                                   
  MoveDataFromQueueToQueue( m_Sound_Processor.GetLeftChannelPowerMinOutputQueue()
                          , m_SerialDataLink.GetQueueHandleTXForDataItem("L_Min")
                          , m_Sound_Processor.GetLeftChannelPowerMinSize()
                          , false
                          , false );
                                   
  MoveDataFromQueueToQueue( m_Sound_Processor.GetLeftChannelPowerMaxOutputQueue()
                          , m_SerialDataLink.GetQueueHandleTXForDataItem("L_Max")
                          , m_Sound_Processor.GetLeftChannelPowerMaxSize()
                          , false
                          , false );                                
}
