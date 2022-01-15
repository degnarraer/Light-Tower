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
  
  SetInputType(InputType_Bluetooth);
  //SetInputType(InputType_Microphone);
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
    break;
    default:
    break;
  }
  ProcessEventQueue();
}

void Manager::SetInputType(InputType_t Type)
{
  m_InputType = Type;
  switch(m_InputType)
  {
    case InputType_Microphone:
      m_BT.StopDevice();
      m_Mic_In.StartDevice();
      m_Mic_Out.StartDevice();
      m_Sound_Processor.SetupSoundProcessor(m_Mic_In.GetChannelBytesToRead(), m_Mic_In.GetSampleRate(), FFT_LARGE_SIZE, FFT_SMALL_SIZE);
      SetDACDataFormat(DAC_Data_Format_LSB24);
      SetDACMuteState(Mute_State_Muted);
    break;
    case InputType_Bluetooth:
      m_Mic_Out.StopDevice();
      m_Mic_In.StopDevice();
      m_BT.StartDevice();
      m_Sound_Processor.SetupSoundProcessor(m_BT.GetChannelBytesToRead(), m_BT.GetSampleRate(), FFT_LARGE_SIZE, FFT_SMALL_SIZE);
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
void Manager::DataBufferModifyRX(String DeviceTitle, uint8_t& DataBuffer, size_t Count)
{
  if(DeviceTitle == m_Mic_In.GetTitle())
  {
    for(int i = 0; i < m_Mic_In.GetSampleCount(); ++i)
    {
      int32_t raw = ((int32_t*)DataBuffer)[i];
      ((int32_t*)DataBuffer)[i] = raw * ANALOG_GAIN; // SET VOLUME HERE
      if(true == PRINT_BYTE_MANIPULATION_DEBUG)
      {
        int32_t raw = ((int32_t*)DataBuffer)[i];
        Serial.print("Result: ");
        Serial.println(raw, HEX);
      }
      if(true == PRINT_DATA_DEBUG) Serial.println(m_Mic_In.GetDataBufferValue(&DataBuffer, i));
    }
  }
}
void Manager::RightChannelDataBufferModifyRX(String DeviceTitle, uint8_t& DataBuffer, size_t Count)
{
  if(DeviceTitle == m_Mic_In.GetTitle())
  {
    if(true == PRINT_RIGHT_CHANNEL_DATA_DEBUG)
    {
      for(int i = 0; i < m_Mic_In.GetChannelSampleCount(); ++i)
      {
        Serial.println(m_Mic_In.GetDataBufferValue(&DataBuffer, i));
      }
    } 
  }
  
}
void Manager::LeftChannelDataBufferModifyRX(String DeviceTitle, uint8_t& DataBuffer, size_t Count)
{
  if(DeviceTitle == m_Mic_In.GetTitle())
  {
    if(true == PRINT_LEFT_CHANNEL_DATA_DEBUG)
    {
      for(int i = 0; i < m_Mic_In.GetChannelSampleCount(); ++i)
      {
        Serial.println(m_Mic_In.GetDataBufferValue(&DataBuffer, i));
      }
    } 
  }
  
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
  switch(m_InputType)
  {
    case InputType_Microphone:
      if(NULL != m_Mic_In.GetDataBufferQueue())
      {
        size_t MessageCount = uxQueueMessagesWaiting(m_Mic_In.GetDataBufferQueue());
        for(int i = 0; i < MessageCount; ++i)
        {
          if(true == EVENT_HANDLER_DEBUG) Serial << "Manager Mic Data Buffer Queue: " << uxQueueMessagesWaiting(m_Mic_In.GetDataBufferQueue()) << "\n";
          uint8_t* DataBuffer = (uint8_t*)malloc(m_Mic_In.GetBytesToRead());
          if ( xQueueReceive(m_Mic_In.GetDataBufferQueue(), DataBuffer, 0) == pdTRUE )
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
    break;
    case InputType_Bluetooth:
    break;
  }
}

void Manager::ProcessRightChannelDataBufferQueue()
{
  switch(m_InputType)
  {
    case InputType_Microphone:
      MoveDataFromQueueToQueue( "MANAGER1"
                              , m_Mic_In.GetRightDataBufferQueue()
                              , m_Sound_Processor.GetQueueHandleRXForDataItem("R_RAW_IN")
                              , m_Mic_In.GetChannelBytesToRead()
                              , false
                              , false );
    break;
    case InputType_Bluetooth:
      MoveDataFromQueueToQueue( "MANAGER2"
                              , m_BT.GetQueueHandleRXForDataItem("R_BT")
                              , m_Sound_Processor.GetQueueHandleRXForDataItem("R_RAW_IN")
                              , m_BT.GetByteCountForDataItem("R_BT")
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
      MoveDataFromQueueToQueue( "MANAGER3"
                              , m_Mic_In.GetLeftDataBufferQueue()
                              , m_Sound_Processor.GetQueueHandleRXForDataItem("L_RAW_IN")
                              , m_Mic_In.GetChannelBytesToRead()
                              , false
                              , false );
    break;
    case InputType_Bluetooth:
      MoveDataFromQueueToQueue( "MANAGER4"
                              , m_BT.GetQueueHandleRXForDataItem("L_BT")
                              , m_Sound_Processor.GetQueueHandleRXForDataItem("L_RAW_IN")
                              , m_BT.GetByteCountForDataItem("L_BT")
                              , false
                              , false );
    break;
    default:
    break;
  }
}

void Manager::ProcessRightFFTDataBufferQueue()
{
  MoveDataFromQueueToQueue( "MANAGER5"
                          , m_Sound_Processor.GetQueueHandleTXForDataItem("R_FFT")
                          , m_SerialDataLink.GetQueueHandleTXForDataItem("R_FFT")
                          , m_Sound_Processor.GetByteCountForDataItem("R_FFT")
                          , false
                          , false );
                          
  MoveDataFromQueueToQueue( "MANAGER6"
                          , m_Sound_Processor.GetQueueHandleTXForDataItem("R_PSD")
                          , m_SerialDataLink.GetQueueHandleTXForDataItem("R_PSD")
                          , m_Sound_Processor.GetByteCountForDataItem("R_PSD")
                          , false
                          , false );
                          
  MoveDataFromQueueToQueue( "MANAGER7"
                          , m_Sound_Processor.GetQueueHandleTXForDataItem("R_MAXBIN")
                          , m_SerialDataLink.GetQueueHandleTXForDataItem("R_MAXBIN")
                          , m_Sound_Processor.GetByteCountForDataItem("R_MAXBIN")
                          , false
                          , false );
}

void Manager::ProcessLeftFFTDataBufferQueue()
{
  MoveDataFromQueueToQueue( "MANAGER8"
                          , m_Sound_Processor.GetQueueHandleTXForDataItem("L_FFT")
                          , m_SerialDataLink.GetQueueHandleTXForDataItem("L_FFT")
                          , m_Sound_Processor.GetByteCountForDataItem("L_FFT")
                          , false
                          , false );

  MoveDataFromQueueToQueue( "MANAGER9"
                          , m_Sound_Processor.GetQueueHandleTXForDataItem("L_PSD")
                          , m_SerialDataLink.GetQueueHandleTXForDataItem("L_PSD")
                          , m_Sound_Processor.GetByteCountForDataItem("L_PSD")
                          , false
                          , false );
                          
  MoveDataFromQueueToQueue( "MANAGER10"
                          , m_Sound_Processor.GetQueueHandleTXForDataItem("L_MAXBIN")
                          , m_SerialDataLink.GetQueueHandleTXForDataItem("L_MAXBIN")
                          , m_Sound_Processor.GetByteCountForDataItem("L_MAXBIN")
                          , false
                          , false );
}
