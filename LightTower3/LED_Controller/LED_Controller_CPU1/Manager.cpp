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
                , FFT_Calculator &FFTCalculator
                , SerialDataLink &SerialDataLink
                , Bluetooth_Sink &BT
                , I2S_Device &Mic
                , I2S_Device &Mic_Out ): NamedItem(Title)
                                       , m_FFT_Calculator(FFTCalculator)
                                       , m_SerialDataLink(SerialDataLink)
                                       , m_BT(BT)
                                       , m_Mic_In(Mic)
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
      m_FFT_Calculator.Setup(m_Mic_In.GetChannelBytesToRead(), m_Mic_In.GetSampleRate(), 4096);
      m_Mic_In.StartDevice();
      m_Mic_Out.StartDevice();
      SetDACDataFormat(DAC_Data_Format_LSB24);
      SetDACMuteState(Mute_State_Un_Muted);
    break;
    case InputType_Bluetooth:
      m_Mic_Out.StopDevice();
      m_Mic_In.StopDevice();
      m_FFT_Calculator.Setup(m_BT.GetChannelBytesToRead(), m_BT.GetSampleRate(), 4096);
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
    for(int i = 0; i < Count / m_Mic_In.GetBytesPerSample(); ++i)
    {
       if(true == PRINT_DATA_DEBUG) Serial.println(GetDataBufferValue(DataBuffer, m_Mic_In.GetBytesPerSample(), i));
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
        m_Mic_Out.SetSoundBufferData(DataBuffer);
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
      MoveDataFromQueueToQueue<int32_t>( m_Mic_In.GetRightDataBufferQueue()
                                       , m_FFT_Calculator.GetFFTRightDataInputQueue()
                                       , m_Mic_In.GetChannelBytesToRead()
                                       , false
                                       , false );
    break;
    case InputType_Bluetooth:
      MoveDataFromQueueToQueue<int32_t>( m_BT.GetRightDataBufferQueue()
                                       , m_FFT_Calculator.GetFFTRightDataInputQueue()
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
      MoveDataFromQueueToQueue<int32_t>( m_Mic_In.GetLeftDataBufferQueue()
                                       , m_FFT_Calculator.GetFFTLeftDataInputQueue()
                                       , m_Mic_In.GetChannelBytesToRead()
                                       , false
                                       , false );
    break;
    case InputType_Bluetooth:
      MoveDataFromQueueToQueue<int32_t>( m_BT.GetLeftDataBufferQueue()
                                       , m_FFT_Calculator.GetFFTLeftDataInputQueue()
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
