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
                , Bluetooth_Sink &BT_In
                , I2S_Device &Mic_In
                , I2S_Device &I2S_Out ): NamedItem(Title)
                                       , m_SerialDataLink(SerialDataLink)
                                       , m_BT_In(BT_In)
                                       , m_Mic_In(Mic_In)
                                       , m_I2S_Out(I2S_Out)
{
}
Manager::~Manager()
{
}

void Manager::Setup()
{
  if(true == EVENT_HANDLER_DEBUG) Serial << "Setup i2s Event Handler\n";
  m_Mic_In.Setup();
  m_I2S_Out.Setup();
  m_BT_In.Setup();
  m_Mic_In.ResgisterForDataBufferRXCallback(this);
  //m_BT_In.ResgisterForDataBufferRXCallback(this);
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

void Manager::ProcessEventQueue()
{
  switch(m_InputType)
  {
    case InputType_Microphone:
      m_Mic_In.ProcessEventQueue();
      m_I2S_Out.ProcessEventQueue();
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
      m_BT_In.StopDevice();
      m_Mic_In.StartDevice();
      m_I2S_Out.StartDevice();
      SetDACMuteState(Mute_State_Muted);
    break;
    case InputType_Bluetooth:
      m_Mic_In.StopDevice();
      m_BT_In.StartDevice();
      m_I2S_Out.StopDevice();
      SetDACMuteState(Mute_State_Un_Muted);
    break;
    default:
      m_Mic_In.StopDevice();
      m_BT_In.StopDevice();
      m_I2S_Out.StopDevice();
      SetDACMuteState(Mute_State_Muted);
    break;
  }
}

//I2S_Device_Callback
//Bluetooth_Callback
void Manager::DataBufferModifyRX(String DeviceTitle, uint8_t* DataBuffer, size_t ByteCount, size_t SampleCount)
{
  if(DeviceTitle == m_Mic_In.GetTitle() || DeviceTitle == m_BT_In.GetTitle())
  {
    for(int i = 0; i < SampleCount; ++i)
    {
      if(true == PRINT_DATA_DEBUG)
      {
        Serial.println(((int32_t*)DataBuffer)[i]);
      }
      if(true == PRINT_DATA_DEBUG) Serial.println(m_Mic_In.GetDataBufferValue(DataBuffer, i));
    }
    if(DeviceTitle == m_Mic_In.GetTitle() || DeviceTitle == m_BT_In.GetTitle())
    {
      assert(m_I2S_Out.GetBytesToRead() == ByteCount);
      assert(m_I2S_Out.GetSampleCount() == SampleCount);
      m_I2S_Out.SetSoundBufferData(DataBuffer, ByteCount);
    }
  }
}
void Manager::RightChannelDataBufferModifyRX(String DeviceTitle, uint8_t* DataBuffer, size_t ByteCount, size_t SampleCount)
{
}
void Manager::LeftChannelDataBufferModifyRX(String DeviceTitle, uint8_t* DataBuffer, size_t ByteCount, size_t SampleCount)
{
}
