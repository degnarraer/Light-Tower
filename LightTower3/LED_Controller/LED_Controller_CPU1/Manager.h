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

#define EVENT_HANDLER_DEBUG false
#define PRINT_DATA_DEBUG false
#define SAWTOOTH_OUTPUT_DATA_DEBUG false

#include <I2S_Device.h>
#include <DataTypes.h>
#include <Helpers.h>

#include "FFT_Calculator.h"
#include "Serial_Datalink_Config.h"
#include "Bluetooth_Device.h"

enum InputType_t
{
  InputType_Microphone,
  InputType_Bluetooth
};
enum OutputType_t
{
  OutputType_DAC,
  OutputType_Bluetooth
};

class Manager: public NamedItem
             , public I2S_Device_Callback
             , public CommonUtils
{
  public:
    Manager( String Title
           , FFT_Calculator &FFTCalculator
           , SerialDataLink &SerialDataLink
           , Bluetooth_Sink &BT
           , I2S_Device &Mic
           , I2S_Device &Speaker );
    virtual ~Manager();
    void Setup();
    void RunTask();
    void ProcessEventQueue();
    void SetInputType(InputType_t Type)
    {
      m_InputType = Type;
      switch(m_InputType)
      {
        case InputType_Microphone:
          m_BT.StopDevice();
          m_FFT_Calculator.Setup(m_Mic.GetBytesToRead(), m_Mic.GetSampleRate(), 2048);
          m_Mic.StartDevice();
          m_Speaker.StartDevice();
        break;
        case InputType_Bluetooth:
          m_Speaker.StopDevice();
          m_Mic.StopDevice();
          m_FFT_Calculator.Setup(m_BT.GetBytesToRead(), m_BT.GetSampleRate(), 2048);
          m_BT.StartDevice();
        break;
        default:
        break;
      }
    }

    //I2S_Device_Callback
    void DataBufferModifyRX(String DeviceTitle, int32_t* DataBuffer, size_t Count)
    {
      if(DeviceTitle == m_Mic.GetTitle())
      {
        for(int i = 0; i < Count; ++i)
        {
           if(true == SAWTOOTH_OUTPUT_DATA_DEBUG)
           {
            DataBuffer[i] = i;
           }
           DataBuffer[i] = DataBuffer[i] * 0.001;  // SET VOLUME HERE
           if(true == PRINT_DATA_DEBUG) Serial.println(DataBuffer[i]);
        } 
      }
    }
    void RightChannelDataBufferModifyRX(String DeviceTitle, int32_t* DataBuffer, size_t Count){}
    void LeftChannelDataBufferModifyRX(String DeviceTitle, int32_t* DataBuffer, size_t Count){}

  private:
    FFT_Calculator &m_FFT_Calculator;
    SerialDataLink &m_SerialDataLink;
    Bluetooth_Sink &m_BT;
    I2S_Device &m_Mic;
    I2S_Device &m_Speaker;
    InputType_t m_InputType;
    OutputType_t m_OutputType;

    void ProcessDataBufferQueue();
    void ProcessRightChannelDataBufferQueue();
    void ProcessLeftChannelDataBufferQueue();
    void ProcessRightFFTDataBufferQueue();
    void ProcessLeftFFTDataBufferQueue();
};

#endif
