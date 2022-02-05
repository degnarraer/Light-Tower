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

Sound_Processor::Sound_Processor(String Title, SerialDataLink &SerialDataLink): NamedItem(Title)
                                                                              , QueueManager(Title + "_QueueManager", m_ConfigCount)
                                                                              , m_SerialDataLink(SerialDataLink)
{
}
Sound_Processor::~Sound_Processor()
{
  FreeMemory();
}
void Sound_Processor::SetupSoundProcessor(size_t ChannelInputByteCount, int SampleRate, int Large_FFT_Length, int Small_FFT_Length)
{
  //Down Sample ratio must be Integer
  assert(0 == I2S_SAMPLE_RATE % DOWN_SAMPLED_RATE);
  m_ChannelInputByteCount = ChannelInputByteCount;
  m_ChannelInputSampleCount = m_ChannelInputByteCount / sizeof(int32_t);
  m_BytesToRead = sizeof(int32_t) * m_Large_FFT_Length;
  m_SampleRate = SampleRate;
  m_Large_FFT_Length = Large_FFT_Length;
  m_Small_FFT_Length = Small_FFT_Length;
  m_AudioBinLimit = GetBinForFrequency(DOWN_SAMPLED_RATE/2, m_Small_FFT_Length, DOWN_SAMPLED_RATE);
  Serial << "FFT Config: " << m_ChannelInputByteCount << " | " << m_SampleRate << " | " << m_Large_FFT_Length << "\n";
  if(true == m_MemoryIsAllocated)FreeMemory();
  AllocateMemory();
  SetupQueueManager();
}
void Sound_Processor::AllocateMemory()
{
  Serial << GetTitle() << ": Allocating Memory.\n";
  Right_Channel_fir_lp_0_to_3k.setFilterCoeffs(filter_taps_0_to_3k);
  Left_Channel_fir_lp_0_to_3k.setFilterCoeffs(filter_taps_0_to_3k);
  
  m_BandOutputByteCount = sizeof(float) * NUMBER_OF_BANDS;
  m_Large_FFT_Right_Data = (int16_t*)malloc(sizeof(int16_t) * m_Large_FFT_Length);
  m_Large_FFT_Left_Data = (int16_t*)malloc(sizeof(int16_t) * m_Large_FFT_Length);
  m_Small_FFT_Right_Data = (int16_t*)malloc(sizeof(int16_t) * m_Small_FFT_Length);
  m_Small_FFT_Left_Data = (int16_t*)malloc(sizeof(int16_t) * m_Small_FFT_Length);
  m_Right_Band_Values = (float*)malloc(m_BandOutputByteCount);
  m_Left_Band_Values = (float*)malloc(m_BandOutputByteCount);
  
  m_RightChannel_Filtered_0k_to_3k = (int32_t*)malloc(m_ChannelInputByteCount);
  m_LeftChannel_Filtered_0k_to_3k = (int32_t*)malloc(m_ChannelInputByteCount);
  m_DataBuffer1 = (int16_t*)malloc(m_ChannelInputByteCount);
  m_DataBuffer2 = (int16_t*)malloc(m_ChannelInputByteCount);
  m_DataBuffer3 = (int32_t*)malloc(m_ChannelInputByteCount);
  m_DataBuffer4 = (int32_t*)malloc(m_ChannelInputByteCount);
  m_DataBuffer5 = (int16_t*)malloc(m_ChannelInputByteCount);
  m_DataBuffer6 = (int16_t*)malloc(m_ChannelInputByteCount);
  
  m_MemoryIsAllocated = true;
}

void Sound_Processor::FreeMemory()
{
  Serial << GetTitle() << ": Freeing Memory.\n";
  delete m_RightChannel_Filtered_0k_to_3k;
  delete m_LeftChannel_Filtered_0k_to_3k;
  delete m_Large_FFT_Right_Data;
  delete m_Large_FFT_Left_Data;
  delete m_Small_FFT_Right_Data;
  delete m_Small_FFT_Left_Data;
  delete m_Right_Band_Values;
  delete m_Left_Band_Values;
  delete m_DataBuffer1;
  delete m_DataBuffer2;
  delete m_DataBuffer3;
  delete m_DataBuffer4;
  delete m_DataBuffer5;
  delete m_DataBuffer6;
  m_MemoryIsAllocated = false;
}
void Sound_Processor::ProcessEventQueue()
{
  ProcessRightChannelSoundData();
  ProcessLeftChannelSoundData();
}

void Sound_Processor::ProcessRightChannelSoundData()
{
  if( NULL != GetQueueHandleRXForDataItem("R_RAW32_IN") &&
      NULL != GetQueueHandleRXForDataItem("R_PSD_IN") )
  {
    QueueHandle_t Queues1[1] = { GetQueueHandleRXForDataItem("R_PSD_IN") };                       
    MoveDataFromQueueToQueues( "SP2a"
                             , GetQueueHandleRXForDataItem("R_RAW32_IN")
                             , Queues1
                             , sizeof(Queues1)/sizeof(Queues1[0])
                             , m_ChannelInputByteCount
                             , false
                             , false );
  }
  
  if( NULL != GetQueueHandleRXForDataItem("R_RAW16_IN") &&
      NULL != GetQueueHandleRXForDataItem("R_FFT_IN") &&
      NULL != GetQueueHandleRXForDataItem("R_MAXBIN_IN") )
  {
    QueueHandle_t Queues2[2] = { GetQueueHandleRXForDataItem("R_FFT_IN")
                              , GetQueueHandleRXForDataItem("R_MAXBIN_IN") };
                              
    MoveDataFromQueueToQueues( "SP2b"
                             , GetQueueHandleRXForDataItem("R_RAW16_IN")
                             , Queues2
                             , sizeof(Queues2)/sizeof(Queues2[0])
                             , m_ChannelInputByteCount
                             , false
                             , false );
  }
}

void Sound_Processor::ProcessLeftChannelSoundData()
{
  if( NULL != GetQueueHandleRXForDataItem("L_RAW32_IN") &&
      NULL != GetQueueHandleRXForDataItem("L_PSD_IN") )
  {
    QueueHandle_t Queues1[1] = { GetQueueHandleRXForDataItem("L_PSD_IN") };                       
    MoveDataFromQueueToQueues( "SP2a"
                             , GetQueueHandleRXForDataItem("L_RAW32_IN")
                             , Queues1
                             , sizeof(Queues1)/sizeof(Queues1[0])
                             , m_ChannelInputByteCount
                             , false
                             , false );
  }
  
  if( NULL != GetQueueHandleRXForDataItem("L_RAW16_IN") &&
      NULL != GetQueueHandleRXForDataItem("L_FFT_IN") &&
      NULL != GetQueueHandleRXForDataItem("L_MAXBIN_IN") )
  {
    QueueHandle_t Queues2[2] = { GetQueueHandleRXForDataItem("L_FFT_IN")
                              , GetQueueHandleRXForDataItem("L_MAXBIN_IN") };
                              
    MoveDataFromQueueToQueues( "SP2b"
                             , GetQueueHandleRXForDataItem("L_RAW16_IN")
                             , Queues2
                             , sizeof(Queues2)/sizeof(Queues2[0])
                             , m_ChannelInputByteCount
                             , false
                             , false );
  }
}

void Sound_Processor::ProcessFFT()
{
  ProcessRightChannelFFT();
  ProcessLeftChannelFFT();
}

void Sound_Processor::ProcessRightChannelFFT()
{
  QueueHandle_t QueueIn = GetQueueHandleRXForDataItem("R_FFT_IN");
  QueueHandle_t QueueOut = m_SerialDataLink.GetQueueHandleTXForDataItem("R_FFT");
  if(NULL != QueueIn && NULL != QueueOut )
  {
    if( uxQueueSpacesAvailable(QueueOut) > 0 )
    {
      size_t MessagesWaiting = uxQueueMessagesWaiting(QueueIn);

      for(int i = 0; i < MessagesWaiting; ++i)
      {
        memset(m_DataBuffer1, 0, m_ChannelInputByteCount);
        if(true == SOUND_PROCESSOR_QUEUE_DEBUG) Serial << "FFT Right Data Buffer Queue Count: " << MessagesWaiting << "\n";
        if ( xQueueReceive(QueueIn, m_DataBuffer1, 0) == pdTRUE )
        {
          for(int j = 0; j < m_ChannelInputSampleCount; ++j)
          {
            if(true == SOUND_PROCESSOR_LOOPS_DEBUG)Serial << "Right Loop Count: "<< j << " of " << m_ChannelInputByteCount << "\n";
            m_Large_FFT_Right_Data[m_FFT_Large_Right_Buffer_Index] = m_DataBuffer1[j];
            if(true == SOUND_PROCESSOR_INPUTDATA_R_DEBUG) Serial << m_Large_FFT_Right_Data[m_FFT_Large_Right_Buffer_Index] << "\n";
            ++m_FFT_Large_Right_Buffer_Index;
            
            if(m_FFT_Large_Right_Buffer_Index >= m_Large_FFT_Length)
            {
              m_FFT_Large_Right_Buffer_Index = 0;
              if(true == SOUND_PROCESSOR_INPUT_COMPLETE_DATA_R_DEBUG)
              {
                for(int k = 0; k < m_Large_FFT_Length / 2; ++k)
                {
                  Serial << m_Large_FFT_Right_Data[k] << "\n";
                }
              }
              for(int k = 0; k < m_Large_FFT_Length / 2; ++k)
              {
                m_Large_FFT_Right_Data[k] *= m_FFT_Gain;
              }
              ZeroFFT(m_Large_FFT_Right_Data, m_Large_FFT_Length);
              if(true == SOUND_PROCESSOR_OUTPUTDATA_R_DEBUG)
              {
                for(int k = 0; k < m_Large_FFT_Length / 2; ++k)
                {
                  Serial << "Frequency: " << GetFreqForBin(k, m_Large_FFT_Length, I2S_SAMPLE_RATE) << " Magnitude: " << m_Large_FFT_Right_Data[k] << "\n";
                }
              }
              memset(m_Right_Band_Values, 0, sizeof(*m_Right_Band_Values)*NUMBER_OF_BANDS);
              AssignToBins(*m_Right_Band_Values, m_Large_FFT_Right_Data, m_Large_FFT_Length, I2S_SAMPLE_RATE);
              for(int k = 0; k < NUMBER_OF_BANDS; ++k)
              {
                m_Right_Band_Values[k] = m_Right_Band_Values[k] / m_16BitMax;
                if(m_Right_Band_Values[k] > 1.0) m_Right_Band_Values[k] = 1.0;
              }
              if(true == SOUND_PROCESSOR_OUTPUT_R_BANDDATA_DEBUG)
              {
                for(int k = 0; k < NUMBER_OF_BANDS; ++k)
                {
                    Serial << "Band: " << k << " Value: " << m_Right_Band_Values[k] << "\n";
                }
              }
              PushValueToQueue(m_Right_Band_Values, QueueOut, false, false); 
              xQueueReset(QueueIn);
            }
          }
        }
      }
    }
  }
}

void Sound_Processor::ProcessLeftChannelFFT()
{
  QueueHandle_t QueueIn = GetQueueHandleRXForDataItem("L_FFT_IN");
  QueueHandle_t QueueOut = m_SerialDataLink.GetQueueHandleTXForDataItem("L_FFT");
  if( NULL != QueueIn && NULL != QueueOut )
  {
    if( uxQueueSpacesAvailable(QueueOut) > 0)
    {
      size_t MessagesWaiting = uxQueueMessagesWaiting(QueueIn);
      for(int i = 0; i < MessagesWaiting; ++i)
      {
        memset(m_DataBuffer2, 0, m_ChannelInputByteCount);
        if(true == SOUND_PROCESSOR_QUEUE_DEBUG) Serial << "FFT Left Data Buffer Queue Count: " << MessagesWaiting << "\n";
        if ( xQueueReceive(QueueIn, m_DataBuffer2, 0) == pdTRUE )
        {
          for(int j = 0; j < m_ChannelInputSampleCount; ++j)
          {
            if(true == SOUND_PROCESSOR_LOOPS_DEBUG)Serial << "Left Loop Count: "<< j << " of " << m_ChannelInputByteCount << "\n";
            m_Large_FFT_Left_Data[m_FFT_Large_Left_Buffer_Index] = m_DataBuffer2[j];
            if(true == SOUND_PROCESSOR_INPUTDATA_L_DEBUG) Serial << m_Large_FFT_Left_Data[m_FFT_Large_Left_Buffer_Index] << "\n";
            ++m_FFT_Large_Left_Buffer_Index;
            
            if(m_FFT_Large_Left_Buffer_Index >= m_Large_FFT_Length)
            {
              m_FFT_Large_Left_Buffer_Index = 0;
              if(true == SOUND_PROCESSOR_INPUT_COMPLETE_DATA_L_DEBUG)
              {
                for(int k = 0; k < m_Large_FFT_Length / 2; ++k)
                {
                  Serial << m_Large_FFT_Left_Data[k] << "\n";
                }
              }
              for(int k = 0; k < m_Large_FFT_Length / 2; ++k)
              {
                m_Large_FFT_Left_Data[k] *= m_FFT_Gain;
              }
              ZeroFFT(m_Large_FFT_Left_Data, m_Large_FFT_Length);
              if(true == SOUND_PROCESSOR_OUTPUTDATA_L_DEBUG)
              {
                for(int k = 0; k < m_Large_FFT_Length / 2; ++k)
                {
                  Serial << "Frequency: " << GetFreqForBin(k, m_Large_FFT_Length, I2S_SAMPLE_RATE) << " Magnitude: " << m_Large_FFT_Left_Data[k] << "\n";
                }
              }
              memset(m_Left_Band_Values, 0, sizeof(float)*NUMBER_OF_BANDS);
              AssignToBins(*m_Left_Band_Values, m_Large_FFT_Left_Data, m_Large_FFT_Length, I2S_SAMPLE_RATE);
              for(int k = 0; k < NUMBER_OF_BANDS; ++k)
              {
                m_Left_Band_Values[k] = m_Left_Band_Values[k] / m_16BitMax; 
                if(m_Left_Band_Values[k] > 1.0) m_Left_Band_Values[k] = 1.0;
              }
              if(true == SOUND_PROCESSOR_OUTPUT_L_BANDDATA_DEBUG)
              {
                for(int k = 0; k < NUMBER_OF_BANDS; ++k)
                {
                    Serial << "Band: " << k << " Value: " << m_Left_Band_Values[k] << "\n";
                }
              }
              PushValueToQueue(m_Left_Band_Values, QueueOut, false, false); 
              xQueueReset(QueueIn);
            }
          }
        }
      }
    }
  }
}
void Sound_Processor::ProcessSoundPower()
{
  ProcessRightChannelPower();
  ProcessLeftChannelPower();
}

void Sound_Processor::ProcessRightChannelPower()
{
  QueueHandle_t QueueIn = GetQueueHandleRXForDataItem("R_PSD_IN");
  QueueHandle_t QueueOut = m_SerialDataLink.GetQueueHandleTXForDataItem("R_PSD");
  if( NULL != QueueIn && NULL != QueueOut )
  {
    size_t MessageCount = uxQueueMessagesWaiting(QueueIn);
    for(int i = 0; i < MessageCount; ++i)
    {
      memset(m_DataBuffer3, 0, m_ChannelInputByteCount);
      if(true == SOUND_PROCESSOR_QUEUE_DEBUG) Serial << "Right Channel Power Input Buffer Queue Count: " << MessageCount << "\n";
      if ( xQueueReceive(QueueIn, m_DataBuffer3, 0) == pdTRUE )
      {
        int32_t peakToPeak = 0;
        int32_t minValue = INT32_MAX;
        int32_t maxValue = -INT32_MAX;
        for(int j = 0; j < m_ChannelInputSampleCount; ++j)
        {
          if(m_DataBuffer3[j] < minValue)
          {
            minValue = m_DataBuffer3[j];
          }
          if(m_DataBuffer3[j] > maxValue)
          {
            maxValue = m_DataBuffer3[j];
          }
        }
        peakToPeak = (maxValue - minValue) * m_Gain;
        m_Right_Channel_Processed_Sound_Data.NormalizedPower = (float)peakToPeak / (float)m_24BitMax;
        if(peakToPeak > 0)
        {
          m_Right_Channel_Processed_Sound_Data.PowerDB = m_IMNP441_1PA_Offset + 20*log10((float)peakToPeak / m_IMNP441_1PA_Value);
        }
        else
        {
          m_Right_Channel_Processed_Sound_Data.PowerDB = 0;
        }
        m_Right_Channel_Processed_Sound_Data.Minimum = minValue;
        m_Right_Channel_Processed_Sound_Data.Maximum = maxValue;
        PushValueToQueue(&m_Right_Channel_Processed_Sound_Data, QueueOut, false, false);
      }
    }
  }
}

void Sound_Processor::ProcessLeftChannelPower()
{
  QueueHandle_t QueueIn = GetQueueHandleRXForDataItem("L_PSD_IN");
  QueueHandle_t QueueOut = m_SerialDataLink.GetQueueHandleTXForDataItem("L_PSD");
  if(NULL != QueueIn && NULL != QueueOut )
  {
    size_t MessageCount = uxQueueMessagesWaiting(QueueIn);
    for(int i = 0; i < MessageCount; ++i)
    {
      memset(m_DataBuffer4, 0, m_ChannelInputByteCount);
      if(true == SOUND_PROCESSOR_QUEUE_DEBUG) Serial << "Left Channel Power Input Buffer Queue Count: " << uxQueueMessagesWaiting(QueueIn) << "\n";
      if ( xQueueReceive(QueueIn, m_DataBuffer4, 0) == pdTRUE )
      {
        int32_t peakToPeak = 0;
        int32_t minValue = INT32_MAX;
        int32_t maxValue = -INT32_MAX;
        for(int j = 0; j < m_ChannelInputSampleCount; ++j)
        {
          if(m_DataBuffer4[j] < minValue)
          {
            minValue = m_DataBuffer4[j];
          }
          if(m_DataBuffer4[j] > maxValue)
          {
            maxValue = m_DataBuffer4[j];
          }
        }
        peakToPeak = (maxValue - minValue) * m_Gain;
        m_Left_Channel_Processed_Sound_Data.NormalizedPower = (float)peakToPeak / (float)m_24BitMax;
        if(m_Left_Channel_Processed_Sound_Data.NormalizedPower > 1.0) m_Left_Channel_Processed_Sound_Data.NormalizedPower = 1.0;
        if(m_Left_Channel_Processed_Sound_Data.NormalizedPower < 0.0) m_Left_Channel_Processed_Sound_Data.NormalizedPower = 0.0;
        if(peakToPeak > 0)
        {
          m_Left_Channel_Processed_Sound_Data.PowerDB = 20*log10((float)peakToPeak / (float)m_24BitMax);
        }
        else
        {
          m_Left_Channel_Processed_Sound_Data.PowerDB = 0;
        }
        m_Left_Channel_Processed_Sound_Data.Minimum = minValue;
        m_Left_Channel_Processed_Sound_Data.Maximum = maxValue;
        PushValueToQueue(&m_Left_Channel_Processed_Sound_Data, QueueOut, false, false);   
      }
    }
  }
}

void Sound_Processor::ProcessMaxBand()
{
  ProcessRightChannelMaxBand();
  ProcessLeftChannelMaxBand();
}

void Sound_Processor::ProcessRightChannelMaxBand()
{
  QueueHandle_t QueueIn = GetQueueHandleRXForDataItem("R_MAXBIN_IN");
  QueueHandle_t QueueOut = m_SerialDataLink.GetQueueHandleTXForDataItem("R_MAXBIN");
  if( NULL != QueueIn && NULL != QueueOut )
  {
    size_t MessageCount = uxQueueMessagesWaiting(QueueIn);
    for(int i = 0; i < MessageCount; ++i)
    {
      static int REMOVE_THIS_COUNTER = 0;
      ++REMOVE_THIS_COUNTER;
      memset(m_DataBuffer5, 0, m_ChannelInputByteCount);
      if(true == SOUND_PROCESSOR_QUEUE_DEBUG) Serial << "Right Max Bin Buffer Queue Count: " << uxQueueMessagesWaiting(GetQueueHandleRXForDataItem("R_MAXBIN_IN")) << "\n";
      if ( xQueueReceive(QueueIn, m_DataBuffer5, 0) == pdTRUE )
      {
        for(int j = 0; j < m_ChannelInputSampleCount; ++j)
        {
          int16_t Value16t = m_DataBuffer5[j];
          Value16t = Right_Channel_fir_lp_0_to_3k.processReading( Value16t );
          Value16t = ScaleWithLimits<int16_t>(Value16t, m_FFT_Gain, INT16_MIN, INT16_MAX);
          ++m_FFT_Small_Right_DownSampleCount;
          if(0 == m_FFT_Small_Right_DownSampleCount % m_DownSampleRatio)
          {
            m_Small_FFT_Right_Data[m_FFT_Small_Right_Buffer_Index] = Value16t;
            ++m_FFT_Small_Right_Buffer_Index;
            if(m_FFT_Small_Right_Buffer_Index >= m_Small_FFT_Length)
            {
              m_FFT_Small_Right_Buffer_Index = 0;
              if(0 == REMOVE_THIS_COUNTER % 50 || true == SOUND_PROCESSOR_MAX_BAND_INPUT_COMPLETE_DATA_R_DEBUG)
              {
                for(int k = 0; k < m_Small_FFT_Length; ++k)
                {
                  Serial << m_Small_FFT_Right_Data[k] << "\n";
                }
              }
              ZeroFFT(m_Small_FFT_Right_Data, m_Small_FFT_Length);
              float maxFFTMagnitude = FLT_MIN;
              int16_t maxFFTValueIndex = 0;
              for(int k = 0; k < min(m_AudioBinLimit, int16_t(FFT_SMALL_SIZE / 2)); ++k)
              {
                float magnitude = sqrt( sq(m_Small_FFT_Right_Data[k]) + sq(m_Small_FFT_Right_Data[m_Small_FFT_Length - k - 1]) );
                if(magnitude > maxFFTMagnitude)
                {
                  maxFFTMagnitude = magnitude;
                  maxFFTValueIndex = k;
                }
              }
              m_Right_MaxBinSoundData.MaxBinNormalizedPower = ((float)maxFFTMagnitude * m_FFT_Out_Gain) / (float)m_16BitMax;
              if(m_Right_MaxBinSoundData.MaxBinNormalizedPower > 1.0) m_Right_MaxBinSoundData.MaxBinNormalizedPower = 1.0;
              m_Right_MaxBinSoundData.MaxBinIndex = maxFFTValueIndex;
              m_Right_MaxBinSoundData.TotalBins = min(m_AudioBinLimit, int16_t(FFT_SMALL_SIZE / 2));
              if(true == SOUND_PROCESSOR_OUTPUT_R_MAXBAND_DEBUG)Serial << "Right Max Frequency: " << GetFreqForBin(m_Right_MaxBinSoundData.MaxBinIndex, m_Small_FFT_Length, DOWN_SAMPLED_RATE) << " Power: " << m_Right_MaxBinSoundData.MaxBinNormalizedPower << "\n";
              PushValueToQueue(&m_Right_MaxBinSoundData, QueueOut, false, false); 
              xQueueReset(QueueIn);
            }
          }
        }
      }
    }
  }
}

void Sound_Processor::ProcessLeftChannelMaxBand()
{
  QueueHandle_t QueueIn = GetQueueHandleRXForDataItem("L_MAXBIN_IN");
  QueueHandle_t QueueOut = m_SerialDataLink.GetQueueHandleTXForDataItem("L_MAXBIN");
  if( NULL != QueueIn && NULL != QueueOut )
  {
    size_t MessageCount = uxQueueMessagesWaiting(GetQueueHandleRXForDataItem("L_MAXBIN_IN"));
    for(int i = 0; i < MessageCount; ++i)
    {
      memset(m_DataBuffer6, 0, m_ChannelInputByteCount);
      if(true == SOUND_PROCESSOR_QUEUE_DEBUG) Serial << "Left Max Bin Buffer Queue Count: " << uxQueueMessagesWaiting(QueueIn) << "\n";
      if ( xQueueReceive(QueueIn, m_DataBuffer6, 0) == pdTRUE )
      {
        for(int j = 0; j < m_ChannelInputSampleCount; ++j)
        {
          int16_t Value16t = m_DataBuffer6[j];
          Value16t = Left_Channel_fir_lp_0_to_3k.processReading( Value16t );
          Value16t = ScaleWithLimits<int16_t>(Value16t, m_FFT_Gain, INT16_MIN, INT16_MAX);
          ++m_FFT_Small_Left_DownSampleCount;
          if(0 == m_FFT_Small_Left_DownSampleCount % m_DownSampleRatio)
          {
            m_Small_FFT_Left_Data[m_FFT_Small_Left_Buffer_Index] = Value16t;
            ++m_FFT_Small_Left_Buffer_Index;
            if(m_FFT_Small_Left_Buffer_Index >= m_Small_FFT_Length)
            {
              m_FFT_Small_Left_Buffer_Index = 0;
              if(true == SOUND_PROCESSOR_MAX_BAND_INPUT_COMPLETE_DATA_L_DEBUG)
              {
                for(int k = 0; k < m_Small_FFT_Length; ++k)
                {
                  Serial << m_Small_FFT_Left_Data[k] << "\n";
                }
              }
              ZeroFFT(m_Small_FFT_Left_Data, m_Small_FFT_Length);
              float maxFFTMagnitude = FLT_MIN;
              int16_t maxFFTValueIndex = 0;
              for(int k = 0; k < min(m_AudioBinLimit, int16_t(FFT_SMALL_SIZE / 2)); ++k)
              {
                float magnitude = sqrt( sq(m_Small_FFT_Left_Data[k]) + sq(m_Small_FFT_Left_Data[m_Small_FFT_Length - k - 1]) );
                if(magnitude > maxFFTMagnitude)
                {
                  maxFFTMagnitude = magnitude;
                  maxFFTValueIndex = k;
                }
              }
              m_Left_MaxBinSoundData.MaxBinNormalizedPower = ((float) maxFFTMagnitude * m_FFT_Out_Gain) / (float) m_16BitMax;
              if(m_Left_MaxBinSoundData.MaxBinNormalizedPower > 1.0) m_Left_MaxBinSoundData.MaxBinNormalizedPower = 1.0;
              m_Left_MaxBinSoundData.MaxBinIndex = maxFFTValueIndex;
              m_Left_MaxBinSoundData.TotalBins = min(m_AudioBinLimit, int16_t(FFT_SMALL_SIZE / 2));
              if(true == SOUND_PROCESSOR_OUTPUT_L_MAXBAND_DEBUG)Serial << "Left Max Frequency: " << GetFreqForBin(m_Left_MaxBinSoundData.MaxBinIndex, m_Small_FFT_Length, DOWN_SAMPLED_RATE) << " Power: " << m_Left_MaxBinSoundData.MaxBinNormalizedPower << "\n";
              PushValueToQueue(&m_Left_MaxBinSoundData, QueueOut, false, false); 
              xQueueReset(QueueIn);
            }
          }
        }
      }
    }
  }
}

void Sound_Processor::AssignToBins(float& Band_Data, int16_t* FFT_Data, int16_t FFT_Length, int16_t SampleRate)
{
  for(int k = 0; k < FFT_Length/2; ++k)
  {
    float magnitude = sqrt( sq(FFT_Data[k]) + sq(FFT_Data[FFT_Length - k - 1]) );
    float freq = GetFreqForBin(k, FFT_Length, SampleRate);
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
    else if(freq > 20000 ) bandIndex = 31;
    if(bandIndex >= 0 && freq < m_SampleRate / 2) (&Band_Data)[bandIndex] += magnitude;
  }
}

float Sound_Processor::GetFreqForBin(unsigned int bin, int16_t FFT_Length, int16_t SampleRate)
{
  if(bin > FFT_Length/2) bin = FFT_Length/2;
  if(bin < 0) bin = 0;
  return FFT_BIN(bin, SampleRate, FFT_Length);
}
int16_t Sound_Processor::GetBinForFrequency(float Frequency, int16_t FFT_Length, int16_t SampleRate )
{
  for(int i = 0; i < FFT_Length/2; ++i)
  {
    int16_t CalculatedBin = FFT_BIN(i, SampleRate, FFT_Length);
    if(CalculatedBin > Frequency)
    {  
      if(CalculatedBin > 0) CalculatedBin = i - 1;
      return CalculatedBin;
    }
  }
  return FFT_Length/2;
}
