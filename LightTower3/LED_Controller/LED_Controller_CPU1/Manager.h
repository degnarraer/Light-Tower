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

#ifndef I2S_EventHander_H
#define I2S_EventHander_H

#include <I2S_Device.h>
#include <DataTypes.h>
#include <Helpers.h>

#include "Sound_Processor.h"
#include "Serial_Datalink_Config.h"
#include "Bluetooth_Device.h"

enum InputType_t
{
  InputType_Microphone,
  InputType_Bluetooth
};

enum DAC_Data_Format_t
{
  DAC_Data_Format_Default,
  DAC_Data_Format_LSB16,
  DAC_Data_Format_LSB20,
  DAC_Data_Format_LSB24,
};

enum Mute_State_t
{
  Mute_State_Un_Muted = 0,
  Mute_State_Muted,
};


class Manager: public NamedItem
             , public I2S_Device_Callback
             , public Bluetooth_Sink_Callback
             , public CommonUtils
{
  public:
    Manager( String Title
           , Sound_Processor &FFTCalculator
           , SerialDataLink &SerialDataLink
           , Bluetooth_Sink &BT
           , I2S_Device &Mic_In
           , I2S_Device &Mic_Out );
    virtual ~Manager();
    void Setup();
    void RunTask();
    void ProcessEventQueue();
    void SetInputType(InputType_t Type);
    void SetDACMuteState(Mute_State_t MuteState);
    void SetDACDataFormat(DAC_Data_Format_t DAC_Data_Format);
    
    //Bluetooth_Callback
    //I2S_Device_Callback
    void DataBufferModifyRX(String DeviceTitle, uint8_t& DataBuffer, size_t Count);
    void RightChannelDataBufferModifyRX(String DeviceTitle, uint8_t& DataBuffer, size_t Count);
    void LeftChannelDataBufferModifyRX(String DeviceTitle, uint8_t& DataBuffer, size_t Count);

  private:
    Sound_Processor &m_Sound_Processor;
    SerialDataLink &m_SerialDataLink;
    Bluetooth_Sink &m_BT;
    I2S_Device &m_Mic_In;
    I2S_Device &m_Mic_Out;
    InputType_t m_InputType;
    Mute_State_t m_MuteState = Mute_State_Un_Muted;
    DAC_Data_Format_t m_DAC_Data_Format;

    void ProcessDataBufferQueue();
    void ProcessRightChannelDataBufferQueue();
    void ProcessLeftChannelDataBufferQueue();
    void ProcessRightFFTDataBufferQueue();
    void ProcessLeftFFTDataBufferQueue();
};

#endif
