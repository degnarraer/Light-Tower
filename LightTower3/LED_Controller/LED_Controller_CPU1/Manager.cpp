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
void Manager::ProcessEventQueue()
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
void Manager::DataBufferModifyRX(String DeviceTitle, uint8_t* DataBuffer, size_t ByteCount, size_t SampleCount)
{
  if(DeviceTitle == m_Mic_In.GetTitle() || DeviceTitle == m_BT.GetTitle())
  {
    for(int i = 0; i < SampleCount; ++i)
    {
      int32_t raw = ((int32_t*)DataBuffer)[i];
      ((int32_t*)DataBuffer)[i] = raw * 10; // Set Volume Here
      if(true == PRINT_DATA_DEBUG)
      {
        Serial.println(((int32_t*)DataBuffer)[i]);
      }
      if(true == PRINT_DATA_DEBUG) Serial.println(m_Mic_In.GetDataBufferValue(DataBuffer, i));
    }
    if(DeviceTitle == m_Mic_In.GetTitle())
    {
      assert(m_Mic_Out.GetBytesToRead() == ByteCount);
      assert(m_Mic_Out.GetSampleCount() == SampleCount);
      m_Mic_Out.SetSoundBufferData(DataBuffer, ByteCount);
    }
  }
}
void Manager::RightChannelDataBufferModifyRX(String DeviceTitle, uint8_t* DataBuffer, size_t ByteCount, size_t SampleCount)
{
  if(DeviceTitle == m_Mic_In.GetTitle() || DeviceTitle == m_BT.GetTitle())
  {
    QueueHandle_t Queue32 = m_Sound_Processor.GetQueueHandleRXForDataItem("R_RAW32_IN");
    size_t Queue32ByteCount = m_Sound_Processor.GetQueueByteCountForDataItem("R_RAW32_IN");
    if(NULL != Queue32)
    {
      assert(Queue32ByteCount == ByteCount);
      PushValueToQueue(DataBuffer, Queue32, false, false);
    } 
  }
}
void Manager::LeftChannelDataBufferModifyRX(String DeviceTitle, uint8_t* DataBuffer, size_t ByteCount, size_t SampleCount)
{
  if(DeviceTitle == m_Mic_In.GetTitle() || DeviceTitle == m_BT.GetTitle())
  {
    QueueHandle_t Queue32 = m_Sound_Processor.GetQueueHandleRXForDataItem("L_RAW32_IN");
    size_t Queue32ByteCount = m_Sound_Processor.GetQueueByteCountForDataItem("L_RAW32_IN");
    if(NULL != Queue32)
    {
      assert(Queue32ByteCount == ByteCount);
      PushValueToQueue(DataBuffer, Queue32, false, false);
    } 
  }
}
