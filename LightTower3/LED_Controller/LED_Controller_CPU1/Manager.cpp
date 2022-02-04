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
  delete m_DataBuffer1;
}

void Manager::Setup()
{
  if(true == EVENT_HANDLER_DEBUG) Serial << "Setup i2s Event Handler\n";
  m_Mic_In.ResgisterForDataBufferRXCallback(this);
  m_BT.ResgisterForDataBufferRXCallback(this);
  m_DataBuffer1 = (uint8_t*)malloc(m_Mic_In.GetBytesToRead());
  
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
void Manager::DataBufferModifyRX(String DeviceTitle, uint8_t* DataBuffer, size_t Count)
{
  if(DeviceTitle == m_Mic_In.GetTitle())
  {
    for(int i = 0; i < m_Mic_In.GetSampleCount(); ++i)
    {
      int32_t raw = ((int32_t*)DataBuffer)[i];
      ((int32_t*)DataBuffer)[i] = raw * 10; // Set Volume Here
      if(true == PRINT_DATA_DEBUG)
      {
        int32_t raw = ((int32_t*)DataBuffer)[i];
        //Serial.print("Result: ");
        //Serial.println(raw, HEX);
        Serial.println(raw);
      }
      if(true == PRINT_DATA_DEBUG) Serial.println(m_Mic_In.GetDataBufferValue(DataBuffer, i));
    }
  }
}
void Manager::RightChannelDataBufferModifyRX(String DeviceTitle, uint8_t* DataBuffer, size_t Count)
{
  if(DeviceTitle == m_Mic_In.GetTitle())
  {
    if(true == PRINT_RIGHT_CHANNEL_DATA_DEBUG)
    {
      for(int i = 0; i < m_Mic_In.GetChannelSampleCount(); ++i)
      {
        Serial.println(m_Mic_In.GetDataBufferValue(DataBuffer, i));
      }
    } 
  }
  
}
void Manager::LeftChannelDataBufferModifyRX(String DeviceTitle, uint8_t* DataBuffer, size_t Count)
{
  if(DeviceTitle == m_Mic_In.GetTitle())
  {
    if(true == PRINT_LEFT_CHANNEL_DATA_DEBUG)
    {
      for(int i = 0; i < m_Mic_In.GetChannelSampleCount(); ++i)
      {
        Serial.println(m_Mic_In.GetDataBufferValue(DataBuffer, i));
      }
    } 
  }
  
}

void Manager::ProcessEventQueue()
{
  ProcessDataBufferQueue();
  ProcessRightChannelInputDataBufferQueue();
  ProcessLeftChannelInputDataBufferQueue();
  ProcessRightSoundProcessorDataBufferQueue();
  ProcessLeftSoundProcessorDataBufferQueue();
}

void Manager::ProcessDataBufferQueue()
{
  switch(m_InputType)
  {
    case InputType_Microphone:
    {
      QueueHandle_t Queue = m_Mic_In.GetQueueHandleRXForDataItem("I2S");
      if(NULL != Queue)
      {
        size_t MessageCount = uxQueueMessagesWaiting(Queue);
        for(int i = 0; i < MessageCount; ++i)
        {
          if(true == EVENT_HANDLER_DEBUG) Serial << "Manager Mic Data Buffer Queue: " << uxQueueMessagesWaiting(Queue) << "\n";
          memset(m_DataBuffer1, 0, m_Mic_In.GetBytesToRead());
          if ( xQueueReceive(Queue, m_DataBuffer1, 0) == pdTRUE )
          {
            m_Mic_Out.SetSoundBufferData(m_DataBuffer1, m_Mic_Out.GetBytesToRead());
          }
          else
          {
            Serial << "Error Receiving Queue!";
          }
        }
      }
    }
    break;
    case InputType_Bluetooth:
      //Handled via function in Bluetooth_Device.cpp
    break;
  }
}

void Manager::ProcessRightChannelInputDataBufferQueue()
{
  switch(m_InputType)
  {
    case InputType_Microphone:
    {
      QueueHandle_t TakeFromQueue32 = m_Mic_In.GetQueueHandleRXForDataItem("R_I2S");
      QueueHandle_t GiveToQueue32 = m_Sound_Processor.GetQueueHandleRXForDataItem("R_RAW32_IN");
      QueueHandle_t GiveToQueue16 = m_Sound_Processor.GetQueueHandleRXForDataItem("R_RAW16_IN");
      if(NULL != TakeFromQueue32 && NULL != GiveToQueue32 && NULL != GiveToQueue16)
      {
        size_t ByteCount = m_Mic_In.GetByteCountForDataItem("R_I2S");
        size_t SampleCount = m_Mic_In.GetChannelSampleCount();
        size_t QueueCount = uxQueueMessagesWaiting(TakeFromQueue32);
        if(false) Serial << "Queue Messages Waiting: " << QueueCount << " Byte Count: " << ByteCount << "\n";
        uint32_t* DataBuffer32 = (uint32_t*)malloc(ByteCount);
        uint16_t* DataBuffer16 = (uint16_t*)malloc(ByteCount);
        for (uint8_t i = 0; i < QueueCount; ++i)
        {
          memset(DataBuffer32, 0, ByteCount);
          memset(DataBuffer16, 0, ByteCount);
          if ( xQueueReceive(TakeFromQueue32, DataBuffer32, 0) == pdTRUE )
          { 
            PushValueToQueue(DataBuffer32, GiveToQueue32, false, false);
            for(int j = 0; j < SampleCount; ++j)
            {
              //Shift 32bit sound by 16 to get the highest 16 bits
              DataBuffer16[j] = (int16_t)((DataBuffer32[j] >> 16) & 0x0000FFFF);
            }
            PushValueToQueue(DataBuffer16, GiveToQueue16, false, false);
          }
          else
          { 
            Serial << "Error Receiving Queue!";
          }
        }
        delete DataBuffer32;
        delete DataBuffer16;
      }
      else
      {
         if(false)Serial << "ProcessRightChannelInputDataBufferQueue: NULL Queue\n";
      }
    }
    break;
    case InputType_Bluetooth:
    {
      QueueHandle_t TakeFromQueue32 = m_BT.GetQueueHandleRXForDataItem("R_BT");
      QueueHandle_t GiveToQueue32 = m_Sound_Processor.GetQueueHandleRXForDataItem("R_RAW32_IN");
      QueueHandle_t GiveToQueue16 = m_Sound_Processor.GetQueueHandleRXForDataItem("R_RAW16_IN");
      if(NULL != TakeFromQueue32 && NULL != GiveToQueue32 && NULL != GiveToQueue16)
      {
        size_t ByteCount = m_BT.GetByteCountForDataItem("R_BT");
        size_t SampleCount = m_Mic_In.GetChannelSampleCount();
        size_t QueueCount = uxQueueMessagesWaiting(TakeFromQueue32);
        if(false) Serial << "Queue Messages Waiting: " << QueueCount << " Byte Count: " << ByteCount << "\n";
        uint32_t* DataBuffer32 = (uint32_t*)malloc(ByteCount);
        uint16_t* DataBuffer16 = (uint16_t*)malloc(ByteCount);
        for (uint8_t i = 0; i < QueueCount; ++i)
        {
          memset(DataBuffer32, 0, ByteCount);
          memset(DataBuffer16, 0, ByteCount);
          if ( xQueueReceive(TakeFromQueue32, DataBuffer32, 0) == pdTRUE )
          { 
            PushValueToQueue(DataBuffer32, GiveToQueue32, false, false);
            for(int j = 0; j < SampleCount; ++j)
            {
              //Shift 32bit sound by 16 to get the highest 16 bits
              DataBuffer16[j] = (int16_t)((DataBuffer32[j] >> 16) & 0x0000FFFF);
            }
            PushValueToQueue(DataBuffer16, GiveToQueue16, false, false);
          }
          else
          { 
            Serial << "Error Receiving Queue!";
          }
        }
        delete DataBuffer32;
        delete DataBuffer16;
      }
      else
      {
         if(false)Serial << "ProcessRightChannelInputDataBufferQueue: NULL Queue\n";
      }
    }
    break;
    default:
    break;
  }
}

void Manager::ProcessLeftChannelInputDataBufferQueue()
{
  switch(m_InputType)
  {
    case InputType_Microphone:
    {
      QueueHandle_t TakeFromQueue32 = m_Mic_In.GetQueueHandleRXForDataItem("L_I2S");
      QueueHandle_t GiveToQueue32 = m_Sound_Processor.GetQueueHandleRXForDataItem("L_RAW32_IN");
      QueueHandle_t GiveToQueue16 = m_Sound_Processor.GetQueueHandleRXForDataItem("L_RAW16_IN");
      if(NULL != TakeFromQueue32 && NULL != GiveToQueue32 && NULL != GiveToQueue16)
      {
        size_t ByteCount = m_Mic_In.GetByteCountForDataItem("L_I2S");
        size_t SampleCount = m_Mic_In.GetChannelSampleCount();
        size_t QueueCount = uxQueueMessagesWaiting(TakeFromQueue32);
        if(false) Serial << "Queue Messages Waiting: " << QueueCount << " Byte Count: " << ByteCount << "\n";
        uint32_t* DataBuffer32 = (uint32_t*)malloc(ByteCount);
        uint16_t* DataBuffer16 = (uint16_t*)malloc(ByteCount);
        for (uint8_t i = 0; i < QueueCount; ++i)
        {
          memset(DataBuffer32, 0, ByteCount);
          memset(DataBuffer16, 0, ByteCount);
          if ( xQueueReceive(TakeFromQueue32, DataBuffer32, 0) == pdTRUE )
          { 
            PushValueToQueue(DataBuffer32, GiveToQueue32, false, false);
            for(int j = 0; j < SampleCount; ++j)
            {
              //Shift 32bit sound by 16 to get the highest 16 bits
              DataBuffer16[j] = (int16_t)((DataBuffer32[j] >> 16) & 0x0000FFFF);
            }
            PushValueToQueue(DataBuffer16, GiveToQueue16, false, false);
          }
          else
          { 
            Serial << "Error Receiving Queue!";
          }
        }
        delete DataBuffer32;
        delete DataBuffer16;
      }
      else
      {
         if(false)Serial << "ProcessRightChannelInputDataBufferQueue: NULL Queue\n";
      }
    }
    break;
    case InputType_Bluetooth:
    {
      
      QueueHandle_t TakeFromQueue32 = m_BT.GetQueueHandleRXForDataItem("L_BT");
      QueueHandle_t GiveToQueue32 = m_Sound_Processor.GetQueueHandleRXForDataItem("L_RAW32_IN");
      QueueHandle_t GiveToQueue16 = m_Sound_Processor.GetQueueHandleRXForDataItem("L_RAW16_IN");
      if(NULL != TakeFromQueue32 && NULL != GiveToQueue32 && NULL != GiveToQueue16)
      {
        size_t ByteCount = m_BT.GetByteCountForDataItem("L_BT");
        size_t SampleCount = m_Mic_In.GetChannelSampleCount();
        size_t QueueCount = uxQueueMessagesWaiting(TakeFromQueue32);
        if(false) Serial << "Queue Messages Waiting: " << QueueCount << " Byte Count: " << ByteCount << "\n";
        uint32_t* DataBuffer32 = (uint32_t*)malloc(ByteCount);
        uint16_t* DataBuffer16 = (uint16_t*)malloc(ByteCount);
        for (uint8_t i = 0; i < QueueCount; ++i)
        {
          memset(DataBuffer32, 0, ByteCount);
          memset(DataBuffer16, 0, ByteCount);
          if ( xQueueReceive(TakeFromQueue32, DataBuffer32, 0) == pdTRUE )
          { 
            PushValueToQueue(DataBuffer32, GiveToQueue32, false, false);
            for(int j = 0; j < SampleCount; ++j)
            {
              //Shift 32bit sound by 16 to get the highest 16 bits
              DataBuffer16[j] = (int16_t)((DataBuffer32[j] >> 16) & 0x0000FFFF);
            }
            PushValueToQueue(DataBuffer16, GiveToQueue16, false, false);
          }
          else
          { 
            Serial << "Error Receiving Queue!";
          }
        }
        delete DataBuffer32;
        delete DataBuffer16;
      }
      else
      {
         if(false)Serial << "ProcessRightChannelInputDataBufferQueue: NULL Queue\n";
      }
    }
    break;
    default:
    break;
  }
}

void Manager::ProcessRightSoundProcessorDataBufferQueue()
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

void Manager::ProcessLeftSoundProcessorDataBufferQueue()
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
