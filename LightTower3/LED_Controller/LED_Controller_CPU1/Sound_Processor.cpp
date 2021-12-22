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

#include "Sound_Processor.h"

Sound_Processor::Sound_Processor(String Title): NamedItem(Title)
{
}
Sound_Processor::~Sound_Processor()
{
  FreeMemory();
}
void Sound_Processor::Setup(size_t InputByteCount, int SampleRate, int FFT_Length)
{
  m_InputByteCount = InputByteCount;
  m_SampleRate = SampleRate;
  m_FFT_Length = FFT_Length;
  m_InputSampleCount = InputByteCount / sizeof(int32_t);
  m_BandOutputByteCount = sizeof(int16_t) * NUMBER_OF_BANDS;
  m_BytesToRead = sizeof(int32_t) * m_FFT_Length;
  Serial << "FFT Config: " << m_InputByteCount << " | " << m_SampleRate << " | " << m_FFT_Length << "\n";
  if(true == m_MemoryIsAllocated)FreeMemory(); 
  AllocateMemory();
}
void Sound_Processor::AllocateMemory()
{
  Serial << GetTitle() << ": Allocating Memory.\n";
  
  m_FFT_Right_Data = (int16_t*)malloc(sizeof(int16_t) * m_FFT_Length);
  m_FFT_Left_Data = (int16_t*)malloc(sizeof(int16_t) * m_FFT_Length);
  m_Right_Band_Values = (int16_t*)malloc(m_BandOutputByteCount);
  m_Left_Band_Values = (int16_t*)malloc(m_BandOutputByteCount);
  
  CreateQueue(m_FFT_Right_Data_Input_Buffer_queue, m_InputByteCount, 10, true);
  CreateQueue(m_FFT_Left_Data_Input_Buffer_queue, m_InputByteCount, 10, true);
  
  CreateQueue(m_FFT_Right_BandData_Input_Buffer_Queue, m_BandOutputByteCount, 10, true);
  CreateQueue(m_FFT_Right_BandData_Output_Buffer_Queue, m_BandOutputByteCount, 10, true);
  
  CreateQueue(m_FFT_Left_BandData_Input_Buffer_Queue, m_BandOutputByteCount, 10, true);
  CreateQueue(m_FFT_Left_BandData_Output_Buffer_Queue, m_BandOutputByteCount, 10, true);
  
  CreateQueue(m_Right_ChanneL_Pow_Input_Buffer_Queue, m_InputByteCount, 10, true);
  CreateQueue(m_Right_Channel_Normalized_Power_Output_Buffer_Queue, sizeof(m_Right_ChanneL_Pow_Normalized), 10, true);
  CreateQueue(m_Right_Channel_DB_Output_Buffer_Queue, sizeof(m_Right_Channel_Db), 10, true);
  CreateQueue(m_Right_ChanneL_Pow_Min_Output_Buffer_Queue, sizeof(m_Right_Channel_Min), 10, true);
  CreateQueue(m_Right_ChanneL_Pow_Max_Output_Buffer_Queue, sizeof(m_Right_Channel_Max), 10, true);
  
  CreateQueue(m_Left_ChanneL_Pow_Input_Buffer_Queue, m_InputByteCount, 10, true);
  CreateQueue(m_Left_Channel_Normalized_Power_Output_Buffer_Queue, sizeof(m_Left_ChanneL_Pow_Normalized), 10, true);
  CreateQueue(m_Left_Channel_DB_Output_Buffer_Queue, sizeof(m_Left_Channel_Db), 10, true);
  CreateQueue(m_Left_ChanneL_Pow_Min_Output_Buffer_Queue, sizeof(m_Left_Channel_Min), 10, true);
  CreateQueue(m_Left_ChanneL_Pow_Max_Output_Buffer_Queue, sizeof(m_Left_Channel_Max), 10, true);
  m_MemoryIsAllocated = true;
}

void Sound_Processor::FreeMemory()
{
  Serial << GetTitle() << ": Freeing Memory.\n";
  
  delete m_FFT_Right_Data;
  delete m_FFT_Left_Data;
  delete m_Right_Band_Values;
  delete m_Left_Band_Values;
  vQueueDelete(m_FFT_Right_Data_Input_Buffer_queue);
  vQueueDelete(m_FFT_Left_Data_Input_Buffer_queue);
  
  vQueueDelete(m_FFT_Right_BandData_Input_Buffer_Queue);
  vQueueDelete(m_FFT_Right_BandData_Output_Buffer_Queue);
  
  vQueueDelete(m_FFT_Left_BandData_Input_Buffer_Queue);
  vQueueDelete(m_FFT_Left_BandData_Output_Buffer_Queue);
  
  vQueueDelete(m_Right_ChanneL_Pow_Input_Buffer_Queue);
  vQueueDelete(m_Right_Channel_Normalized_Power_Output_Buffer_Queue);
  vQueueDelete(m_Right_Channel_DB_Output_Buffer_Queue);
  vQueueDelete(m_Right_ChanneL_Pow_Min_Output_Buffer_Queue);
  vQueueDelete(m_Right_ChanneL_Pow_Max_Output_Buffer_Queue);
  
  vQueueDelete(m_Left_ChanneL_Pow_Input_Buffer_Queue);
  vQueueDelete(m_Left_Channel_Normalized_Power_Output_Buffer_Queue);
  vQueueDelete(m_Left_Channel_DB_Output_Buffer_Queue);
  vQueueDelete(m_Left_ChanneL_Pow_Min_Output_Buffer_Queue);
  vQueueDelete(m_Left_ChanneL_Pow_Max_Output_Buffer_Queue);
  m_MemoryIsAllocated = false;
}
void Sound_Processor::ProcessEventQueue()
{
  ProcessRightChannelSoundData();
  ProcessLeftChannelSoundData();
}

void Sound_Processor::ProcessFFTEventQueue()
{
  ProcessRightChannelFFT();
  ProcessLeftChannelFFT();
}

void Sound_Processor::ProcessSoundPowerEventQueue()
{
  ProcessRightChannelPower();
  ProcessLeftChannelPower();
}

void Sound_Processor::ProcessRightChannelSoundData()
{
  if( NULL != m_FFT_Right_Data_Input_Buffer_queue &&
      NULL != m_FFT_Right_BandData_Input_Buffer_Queue &&
      NULL != m_Right_ChanneL_Pow_Input_Buffer_Queue )
  {
    QueueHandle_t Queues[2] = { m_FFT_Right_BandData_Input_Buffer_Queue, m_Right_ChanneL_Pow_Input_Buffer_Queue };
    MoveDataFromQueueToQueues<int32_t>( m_FFT_Right_Data_Input_Buffer_queue
                                      , Queues
                                      , 2
                                      , m_InputByteCount
                                      , false
                                      , false );
  }
}

void Sound_Processor::ProcessRightChannelFFT()
{
  if(uxQueueMessagesWaiting(m_FFT_Right_Data_Input_Buffer_queue) > 0)
  {
    int32_t* DataBuffer = (int32_t*)malloc(m_InputByteCount);
    if(true == SOUND_PROCESSOR_QUEUE_DEBUG) Serial << "FFT Right Data Buffer Queue Count: " << uxQueueMessagesWaiting(m_FFT_Right_Data_Input_Buffer_queue) << "\n";
    if ( xQueueReceive(m_FFT_Right_BandData_Input_Buffer_Queue, DataBuffer, portMAX_DELAY) != pdTRUE ){ Serial.println("Error Getting Queue Data");}
    else
    {
      if(true == SOUND_PROCESSOR_INPUTDATA_DEBUG) Serial << "Data R: ";
      for(int i = 0; i < m_InputByteCount; ++i)
      {
        if(true == SOUND_PROCESSOR_LOOPS_DEBUG)Serial << "Right Loop Count: "<< i << " of " << m_InputByteCount << "\n";
        m_FFT_Right_Data[m_FFT_Right_Buffer_Index] = (DataBuffer[i] >> 8) & 0x0000FFFF;
        if(true == SOUND_PROCESSOR_INPUTDATA_DEBUG) Serial << DataBuffer[i] << "|" << m_FFT_Right_Data[m_FFT_Right_Buffer_Index] << "\n";
        ++m_FFT_Right_Buffer_Index;
        
        if(m_FFT_Right_Buffer_Index >= m_FFT_Length)
        {
          m_FFT_Right_Buffer_Index = 0;
          ZeroFFT(m_FFT_Right_Data, m_FFT_Length);
          if(true == SOUND_PROCESSOR_OUTPUTDATA_DEBUG)
          {
            Serial << "FFT R: ";
            for(int j = 0; j < m_FFT_Length / 2; ++j)
            {
              Serial << m_FFT_Right_Data[j] << " ";
            }
            Serial << "\n";
          }
          memset(m_Right_Band_Values, 0, sizeof(int16_t)*NUMBER_OF_BANDS);
          for(int i = 0; i < m_FFT_Length/2; ++i)
          {
            float freq = GetFreqForBin(i);
            int bandIndex = 0;
            
            if(freq > 0 && freq <= 20) bandIndex = 0;
            else if(freq > 20 && freq <= 25) bandIndex = 1;
            else if(freq > 25 && freq <= 31.5) bandIndex = 2;
            else if(freq > 31.5 && freq <= 40) bandIndex = 3;
            else if(freq > 40 && freq <= 50) bandIndex = 4;
            else if(freq > 50 && freq <= 63) bandIndex = 5;
            else if(freq > 63 && freq <= 80) bandIndex = 6;
            else if(freq > 80 && freq <= 100) bandIndex = 7;
            else if(freq > 100 && freq <= 125) bandIndex = 8;
            else if(freq > 125 && freq <= 160) bandIndex = 9;
            else if(freq > 160 && freq <= 200) bandIndex = 10;
            else if(freq > 200 && freq <= 250) bandIndex = 11;
            else if(freq > 250 && freq <= 315) bandIndex = 12;
            else if(freq > 315 && freq <= 400) bandIndex = 13;
            else if(freq > 400 && freq <= 500) bandIndex = 14;
            else if(freq > 500 && freq <= 630) bandIndex = 15;
            else if(freq > 630 && freq <= 800) bandIndex = 16;
            else if(freq > 800 && freq <= 1000) bandIndex = 17;
            else if(freq > 1000 && freq <= 1250) bandIndex = 18;
            else if(freq > 1250 && freq <= 1600) bandIndex = 19;
            else if(freq > 1600 && freq <= 2000) bandIndex = 20;
            else if(freq > 2000 && freq <= 2500) bandIndex = 21;
            else if(freq > 2500 && freq <= 3150) bandIndex = 22;
            else if(freq > 3150 && freq <= 4000) bandIndex = 23;
            else if(freq > 4000 && freq <= 5000) bandIndex = 24;
            else if(freq > 5000 && freq <= 6300) bandIndex = 25;
            else if(freq > 6300 && freq <= 8000) bandIndex = 26;
            else if(freq > 8000 && freq <= 10000) bandIndex = 27;
            else if(freq > 10000 && freq <= 12500) bandIndex = 28;
            else if(freq > 12500 && freq <= 16000) bandIndex = 29;
            else if(freq > 16000 && freq <= 20000) bandIndex = 30;
            else if(freq > 20000 && freq <= 40000) bandIndex = 31;
            
            m_Right_Band_Values[bandIndex] += m_FFT_Right_Data[i];
          }
        }
      }
    }
    PushValueToQueue(&m_Right_Band_Values, m_FFT_Right_BandData_Output_Buffer_Queue, false);
    delete DataBuffer;
  }
}

void Sound_Processor::ProcessRightChannelPower()
{
  if(uxQueueMessagesWaiting(m_Right_ChanneL_Pow_Input_Buffer_Queue) > 0)
  {
    int32_t* DataBuffer = (int32_t*)malloc(m_InputByteCount);
    if(true == SOUND_PROCESSOR_QUEUE_DEBUG) Serial << "Right Channel Power Input Buffer Queue Count: " << uxQueueMessagesWaiting(m_Right_ChanneL_Pow_Input_Buffer_Queue) << "\n";
    if ( xQueueReceive(m_Right_ChanneL_Pow_Input_Buffer_Queue, DataBuffer, portMAX_DELAY) != pdTRUE ){ Serial.println("Error Getting Queue Data");}
    else
    {
      float peakToPeak = 0;
      int minValue = INT16_MAX;
      int maxValue = -INT16_MAX;
      for(int i = 0; i < m_InputSampleCount; ++i)
      {
        if(DataBuffer[i] < minValue)
        {
          minValue = DataBuffer[i];
        }
        if(DataBuffer[i] > maxValue)
        {
          maxValue = DataBuffer[i];
        }
      }
      peakToPeak = maxValue - minValue;
      m_Right_ChanneL_Pow_Normalized = (float)peakToPeak / (float)pow(2,24); //This needs to know bit size
      if(peakToPeak > 0)
      {
        m_Right_Channel_Db = 20*log10(peakToPeak/100.0);
      }
      else
      {
        m_Right_Channel_Db = 0;
      }
      m_Right_Channel_Min = minValue;
      m_Right_Channel_Max = maxValue;
      PushValueToQueue(&m_Right_ChanneL_Pow_Normalized, m_Right_Channel_Normalized_Power_Output_Buffer_Queue, false);
      PushValueToQueue(&m_Right_Channel_Db, m_Right_Channel_DB_Output_Buffer_Queue, false);
      PushValueToQueue(&m_Right_Channel_Min, m_Right_ChanneL_Pow_Min_Output_Buffer_Queue, false);
      PushValueToQueue(&m_Right_Channel_Max, m_Right_ChanneL_Pow_Max_Output_Buffer_Queue, false);
    }
    delete DataBuffer;
  }
}

void Sound_Processor::ProcessLeftChannelSoundData()
{
  if( NULL != m_FFT_Left_Data_Input_Buffer_queue &&
      NULL != m_FFT_Left_BandData_Input_Buffer_Queue &&
      NULL != m_Left_ChanneL_Pow_Input_Buffer_Queue )
  {
    QueueHandle_t Queues[2] = { m_FFT_Left_BandData_Input_Buffer_Queue, m_Left_ChanneL_Pow_Input_Buffer_Queue };
    MoveDataFromQueueToQueues<int32_t>( m_FFT_Left_Data_Input_Buffer_queue
                                      , Queues
                                      , 2
                                      , m_InputByteCount
                                      , false
                                      , false );
  }
}
void Sound_Processor::ProcessLeftChannelFFT()
{
  if(uxQueueMessagesWaiting(m_FFT_Left_Data_Input_Buffer_queue) > 0)
  {
    int32_t* DataBuffer = (int32_t*)malloc(m_InputByteCount);
    if(true == SOUND_PROCESSOR_QUEUE_DEBUG) Serial << "FFT Left Data Buffer Queue Count: " << uxQueueMessagesWaiting(m_FFT_Left_Data_Input_Buffer_queue) << "\n";
    if ( xQueueReceive(m_FFT_Left_Data_Input_Buffer_queue, DataBuffer, portMAX_DELAY) != pdTRUE ){ Serial.println("Error Getting Queue Data");}
    else
    {
      if(true == SOUND_PROCESSOR_INPUTDATA_DEBUG) Serial << "Data L: ";
      for(int i = 0; i < m_InputByteCount; ++i)
      {
        if(true == SOUND_PROCESSOR_LOOPS_DEBUG)Serial << "Left Loop Count: "<< i << " of " << m_InputByteCount << "\n";
        m_FFT_Left_Data[m_FFT_Left_Buffer_Index] = (DataBuffer[i] >> 8) & 0x0000FFFF;
        if(true == SOUND_PROCESSOR_INPUTDATA_DEBUG) Serial << DataBuffer[i] << "|" << m_FFT_Left_Data[m_FFT_Left_Buffer_Index] << "\n";
        ++m_FFT_Left_Buffer_Index;
        
        if(m_FFT_Left_Buffer_Index >= m_FFT_Length)
        {
          m_FFT_Left_Buffer_Index = 0;
          ZeroFFT(m_FFT_Left_Data, m_FFT_Length);
          if(true == SOUND_PROCESSOR_OUTPUTDATA_DEBUG)
          {
            Serial << "FFT L: ";
            for(int j = 0; j < m_FFT_Length / 2; ++j)
            {
              Serial << m_FFT_Left_Data[j] << " ";
            }
            Serial << "\n";
          }
          
          memset(m_Left_Band_Values, 0, sizeof(int16_t)*NUMBER_OF_BANDS);
          for(int i = 0; i < m_FFT_Length/2; ++i)
          {
            float freq = GetFreqForBin(i);
            int bandIndex = 0;
            
            if(freq > 0 && freq <= 20) bandIndex = 0;
            else if(freq > 20 && freq <= 25) bandIndex = 1;
            else if(freq > 25 && freq <= 31.5) bandIndex = 2;
            else if(freq > 31.5 && freq <= 40) bandIndex = 3;
            else if(freq > 40 && freq <= 50) bandIndex = 4;
            else if(freq > 50 && freq <= 63) bandIndex = 5;
            else if(freq > 63 && freq <= 80) bandIndex = 6;
            else if(freq > 80 && freq <= 100) bandIndex = 7;
            else if(freq > 100 && freq <= 125) bandIndex = 8;
            else if(freq > 125 && freq <= 160) bandIndex = 9;
            else if(freq > 160 && freq <= 200) bandIndex = 10;
            else if(freq > 200 && freq <= 250) bandIndex = 11;
            else if(freq > 250 && freq <= 315) bandIndex = 12;
            else if(freq > 315 && freq <= 400) bandIndex = 13;
            else if(freq > 400 && freq <= 500) bandIndex = 14;
            else if(freq > 500 && freq <= 630) bandIndex = 15;
            else if(freq > 630 && freq <= 800) bandIndex = 16;
            else if(freq > 800 && freq <= 1000) bandIndex = 17;
            else if(freq > 1000 && freq <= 1250) bandIndex = 18;
            else if(freq > 1250 && freq <= 1600) bandIndex = 19;
            else if(freq > 1600 && freq <= 2000) bandIndex = 20;
            else if(freq > 2000 && freq <= 2500) bandIndex = 21;
            else if(freq > 2500 && freq <= 3150) bandIndex = 22;
            else if(freq > 3150 && freq <= 4000) bandIndex = 23;
            else if(freq > 4000 && freq <= 5000) bandIndex = 24;
            else if(freq > 5000 && freq <= 6300) bandIndex = 25;
            else if(freq > 6300 && freq <= 8000) bandIndex = 26;
            else if(freq > 8000 && freq <= 10000) bandIndex = 27;
            else if(freq > 10000 && freq <= 12500) bandIndex = 28;
            else if(freq > 12500 && freq <= 16000) bandIndex = 29;
            else if(freq > 16000 && freq <= 20000) bandIndex = 30;
            else if(freq > 20000 && freq <= 40000) bandIndex = 31;
  
            m_Left_Band_Values[bandIndex] += m_FFT_Left_Data[i];
          }
        }
      } 
    }
    PushValueToQueue(&m_Left_Band_Values, m_FFT_Left_BandData_Output_Buffer_Queue, false);
  }
}

void Sound_Processor::ProcessLeftChannelPower()
{
  if(uxQueueMessagesWaiting(m_Left_ChanneL_Pow_Input_Buffer_Queue) > 0)
  {
    int32_t* DataBuffer = (int32_t*)malloc(m_InputByteCount);
    if(true == SOUND_PROCESSOR_QUEUE_DEBUG) Serial << "Left Channel Power Input Buffer Queue Count: " << uxQueueMessagesWaiting(m_Left_ChanneL_Pow_Input_Buffer_Queue) << "\n";
    if ( xQueueReceive(m_Left_ChanneL_Pow_Input_Buffer_Queue, DataBuffer, portMAX_DELAY) != pdTRUE ){ Serial.println("Error Getting Queue Data");}
    else
    {
      float peakToPeak = 0;
      int minValue = INT16_MAX;
      int maxValue = -INT16_MAX;
      for(int i = 0; i < m_InputSampleCount; ++i)
      {
        if(DataBuffer[i] < minValue)
        {
          minValue = DataBuffer[i];
        }
        if(DataBuffer[i] > maxValue)
        {
          maxValue = DataBuffer[i];
        }
      }
      peakToPeak = maxValue - minValue;
      m_Left_ChanneL_Pow_Normalized = (float)peakToPeak / (float)pow(2,24); //This needs to know bit size
      if(peakToPeak > 0)
      {
        m_Left_Channel_Db = 20*log10(peakToPeak/100.0);
      }
      else
      {
        m_Left_Channel_Db = 0;
      }
      m_Left_Channel_Min = minValue;
      m_Left_Channel_Max = maxValue;
      PushValueToQueue(&m_Left_ChanneL_Pow_Normalized, m_Left_Channel_Normalized_Power_Output_Buffer_Queue, false);
      PushValueToQueue(&m_Left_Channel_Db, m_Left_Channel_DB_Output_Buffer_Queue, false);
      PushValueToQueue(&m_Left_Channel_Min, m_Left_ChanneL_Pow_Min_Output_Buffer_Queue, false);
      PushValueToQueue(&m_Left_Channel_Max, m_Left_ChanneL_Pow_Max_Output_Buffer_Queue, false);
    }
    delete DataBuffer;
  }
}

float Sound_Processor::GetFreqForBin(unsigned int bin)
{
  if(bin > m_FFT_Length/2) bin = m_FFT_Length/2;
  if(bin < 0) bin = 0;
  return FFT_BIN(bin, m_SampleRate, m_FFT_Length);
}
