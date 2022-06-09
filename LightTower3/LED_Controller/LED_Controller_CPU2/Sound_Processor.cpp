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
  //ESP_LOGV("Function Debug", "%s, ", __func__);
}
Sound_Processor::~Sound_Processor()
{
  //ESP_LOGV("Function Debug", "%s, ", __func__);
  FreeMemory();
}
void Sound_Processor::SetupSoundProcessor()
{
  //ESP_LOGV("Function Debug", "%s, ", __func__);
  m_AudioBinLimit = GetBinForFrequency(MAX_VISUALIZATION_FREQUENCY);
  if(true == m_MemoryIsAllocated)FreeMemory();
  AllocateMemory();
  SetupQueueManager();
}
void Sound_Processor::AllocateMemory()
{
  //ESP_LOGV("Function Debug", "%s, ", __func__);
  m_MemoryIsAllocated = true;
}

void Sound_Processor::FreeMemory()
{
  //ESP_LOGV("Function Debug", "%s, ", __func__);
  m_MemoryIsAllocated = false;
}

void Sound_Processor::Sound_32Bit_44100Hz_Right_Channel_FFT()
{
  //ESP_LOGV("Function Debug", "%s, ", __func__);
  QueueHandle_t QueueIn = GetQueueHandleRXForDataItem("R_FFT_IN");
  QueueHandle_t Bands_QueueOut = m_SerialDataLink.GetQueueHandleTXForDataItem("R_BANDS");
  QueueHandle_t MaxBin_QueueOut = m_SerialDataLink.GetQueueHandleTXForDataItem("R_MAXBAND");
  QueueHandle_t MajorFreq_QueueOut = m_SerialDataLink.GetQueueHandleTXForDataItem("R_MAJOR_FREQ");
  if(NULL != QueueIn && NULL != Bands_QueueOut && NULL != MaxBin_QueueOut && NULL != MajorFreq_QueueOut)
  {
    int32_t *InputDataBuffer = (int32_t*)GetDataBufferForDataItem("R_FFT_IN");
    float *Bands_DataBuffer = (float*)m_SerialDataLink.GetDataBufferForDataItem("R_BANDS");
    MaxBandSoundData_t *MaxBandDataBuffer = (MaxBandSoundData_t *)m_SerialDataLink.GetDataBufferForDataItem("R_MAXBAND");
    size_t InputSampleCount = GetSampleCountForDataItem("R_FFT_IN");
    size_t InputDataBufferByteCount = GetTotalByteCountForDataItem("R_FFT_IN");
    size_t Bands_SampleCount = m_SerialDataLink.GetSampleCountForDataItem("R_BANDS");
    size_t Bands_DataBufferByteCount = m_SerialDataLink.GetTotalByteCountForDataItem("R_BANDS");
    size_t MaxBand_SampleCount = m_SerialDataLink.GetSampleCountForDataItem("R_MAXBAND");
    size_t MaxBand_DataBufferByteCount = m_SerialDataLink.GetTotalByteCountForDataItem("R_MAXBAND");
    size_t MajorFreq_SampleCount = m_SerialDataLink.GetSampleCountForDataItem("R_MAJOR_FREQ");
    size_t MajorFreq_DataBufferByteCount = m_SerialDataLink.GetTotalByteCountForDataItem("R_MAJOR_FREQ");
    assert(NUMBER_OF_BANDS == Bands_SampleCount);
    assert(1 == MaxBand_SampleCount);
    assert(sizeof(MaxBandSoundData_t) == MaxBand_DataBufferByteCount);
    assert(1 == MajorFreq_SampleCount);
    assert(sizeof(float) == MajorFreq_DataBufferByteCount);
    size_t MessagesWaiting = uxQueueMessagesWaiting(QueueIn);
    bool FFT_Calculated = false;
    for(int16_t i = 0; i < MessagesWaiting; ++i)
    {
      if ( xQueueReceive(QueueIn, InputDataBuffer, 0) == pdTRUE )
      {
        for(int16_t j = 0; j < InputSampleCount; ++j)
        {
          if(true == m_R_FFT.PushValueAndCalculateNormalizedFFT(InputDataBuffer[j], m_FFT_Gain))
          {
            FFT_Calculated = true;
            float MaxBandMagnitude = 0;
            int16_t MaxBandIndex = 0;
            memset(Bands_DataBuffer, 0, Bands_DataBufferByteCount);
            AssignToBands(Bands_DataBuffer, &m_R_FFT, FFT_SIZE);
            for(int16_t k = 0; k < Bands_SampleCount; ++k)
            {
              if(Bands_DataBuffer[k] > MaxBandMagnitude)
              {
                MaxBandMagnitude = Bands_DataBuffer[k];
                MaxBandIndex = k;
              }
            }
            MaxBandDataBuffer->MaxBandNormalizedPower = MaxBandMagnitude;
            MaxBandDataBuffer->MaxBandIndex = MaxBandIndex;
            MaxBandDataBuffer->TotalBands = Bands_SampleCount;
            static bool R_MaxBand_Push_Successful = true;
            PushValueToQueue(MaxBandDataBuffer, MaxBin_QueueOut, false, "Right Max Band: R_MAXBAND", R_MaxBand_Push_Successful);
            
            static bool R_Bands_Push_Successful = true;
            PushValueToQueue(Bands_DataBuffer, Bands_QueueOut, false, "Right Bands: R_BANDS", R_Bands_Push_Successful);
            
            static bool R_MajorFreq_Push_Successful = true;
            PushValueToQueue(m_R_FFT.GetMajorPeakPointer(), MajorFreq_QueueOut, false, "Right Major Frequency: R_MAJOR_FREQ", R_MajorFreq_Push_Successful);
            //xQueueReset(QueueIn);
          }
        }
        if(true == FFT_Calculated) break;
      }
    }
  }
}

void Sound_Processor::Sound_32Bit_44100Hz_Left_Channel_FFT()
{
  //ESP_LOGV("Function Debug", "%s, ", __func__);
  QueueHandle_t QueueIn = GetQueueHandleRXForDataItem("L_FFT_IN");
  QueueHandle_t Bands_QueueOut = m_SerialDataLink.GetQueueHandleTXForDataItem("L_BANDS");
  QueueHandle_t MaxBin_QueueOut = m_SerialDataLink.GetQueueHandleTXForDataItem("L_MAXBAND");
  QueueHandle_t MajorFreq_QueueOut = m_SerialDataLink.GetQueueHandleTXForDataItem("L_MAJOR_FREQ");
  if(NULL != QueueIn && NULL != Bands_QueueOut && NULL != MaxBin_QueueOut && NULL != MajorFreq_QueueOut)
  {
    int32_t *InputDataBuffer = (int32_t*)GetDataBufferForDataItem("L_FFT_IN");
    float *Bands_DataBuffer = (float*)m_SerialDataLink.GetDataBufferForDataItem("L_BANDS");
    MaxBandSoundData_t *MaxBandDataBuffer = (MaxBandSoundData_t *)m_SerialDataLink.GetDataBufferForDataItem("L_MAXBAND");
    size_t InputSampleCount = GetSampleCountForDataItem("L_FFT_IN");
    size_t InputDataBufferByteCount = GetTotalByteCountForDataItem("L_FFT_IN");
    size_t Bands_SampleCount = m_SerialDataLink.GetSampleCountForDataItem("L_BANDS");
    size_t Bands_DataBufferByteCount = m_SerialDataLink.GetTotalByteCountForDataItem("L_BANDS");
    size_t MaxBand_SampleCount = m_SerialDataLink.GetSampleCountForDataItem("L_MAXBAND");
    size_t MaxBand_DataBufferByteCount = m_SerialDataLink.GetTotalByteCountForDataItem("L_MAXBAND");
    size_t MajorFreq_SampleCount = m_SerialDataLink.GetSampleCountForDataItem("L_MAJOR_FREQ");
    size_t MajorFreq_DataBufferByteCount = m_SerialDataLink.GetTotalByteCountForDataItem("L_MAJOR_FREQ");
    assert(NUMBER_OF_BANDS == Bands_SampleCount);
    assert(1 == MaxBand_SampleCount);
    assert(sizeof(MaxBandSoundData_t) == MaxBand_DataBufferByteCount);
    assert(1 == MajorFreq_SampleCount);
    assert(sizeof(float) == MajorFreq_DataBufferByteCount);
    size_t MessagesWaiting = uxQueueMessagesWaiting(QueueIn);
    bool FFT_Calculated = false;
    for(int16_t i = 0; i < MessagesWaiting; ++i)
    {
      if ( xQueueReceive(QueueIn, InputDataBuffer, 0) == pdTRUE )
      {
        for(int16_t j = 0; j < InputSampleCount; ++j)
        {
          if(true == m_L_FFT.PushValueAndCalculateNormalizedFFT(InputDataBuffer[j], m_FFT_Gain))
          {
            FFT_Calculated = true;
            float MaxBandMagnitude = 0;
            int16_t MaxBandIndex = 0;
            memset(Bands_DataBuffer, 0, Bands_DataBufferByteCount);
            AssignToBands(Bands_DataBuffer, &m_L_FFT, FFT_SIZE);
            for(int16_t k = 0; k < Bands_SampleCount; ++k)
            {
              if(Bands_DataBuffer[k] > MaxBandMagnitude)
              {
                MaxBandMagnitude = Bands_DataBuffer[k];
                MaxBandIndex = k;
              }
            }
            MaxBandDataBuffer->MaxBandNormalizedPower = MaxBandMagnitude;
            MaxBandDataBuffer->MaxBandIndex = MaxBandIndex;
            MaxBandDataBuffer->TotalBands = Bands_SampleCount;
            static bool L_MaxBand_Push_Successful = true;
            PushValueToQueue(MaxBandDataBuffer, MaxBin_QueueOut, false, "Left Max Band: L_MAXBAND", L_MaxBand_Push_Successful);
            
            static bool L_Bands_Push_Successful = true;
            PushValueToQueue(Bands_DataBuffer, Bands_QueueOut, false, "Left Bands: L_BANDS", L_Bands_Push_Successful);
            
            static bool L_MajorFreq_Push_Successful = true;
            PushValueToQueue(m_L_FFT.GetMajorPeakPointer(), MajorFreq_QueueOut, false, "Left Major Frequency: L_MAJOR_FREQ", L_MajorFreq_Push_Successful);
            //xQueueReset(QueueIn);
          }
        }
        if(true == FFT_Calculated) break;
      }
    }
  }
}

void Sound_Processor::Sound_32Bit_44100Hz_Calculate_Right_Channel_Power()
{
  //ESP_LOGV("Function Debug", "%s, ", __func__);
  QueueHandle_t QueueIn = GetQueueHandleRXForDataItem("R_PSD_IN");
  QueueHandle_t QueueOut = m_SerialDataLink.GetQueueHandleTXForDataItem("R_PSD");
  if( NULL != QueueIn && NULL != QueueOut )
  {
    assert(1 == m_SerialDataLink.GetSampleCountForDataItem("R_PSD"));
    int32_t *InputDataBuffer = (int32_t*)GetDataBufferForDataItem("R_PSD_IN");
    size_t InputSampleCount = GetSampleCountForDataItem("R_PSD_IN");
    size_t InputByteCount = GetSampleCountForDataItem("R_PSD_IN");
    size_t MessageCount = uxQueueMessagesWaiting(QueueIn);
    for(int i = 0; i < MessageCount; ++i)
    {
      if ( xQueueReceive(QueueIn, InputDataBuffer, 0) == pdTRUE )
      {
        for(int j = 0; j < InputSampleCount; ++j)
        {
          if(true == m_RightSoundData.PushValueAndCalculateSoundData(InputDataBuffer[j]))
          {
            static bool R_PSD_Push_Successful = true;
            PushValueToQueue(m_RightSoundData.GetProcessedSoundDataPointer(), QueueOut, false, "Right Processed Sound Data: R_PSD", R_PSD_Push_Successful);
            //xQueueReset(QueueIn);
          }
        }
      }
    }
  }
}

void Sound_Processor::Sound_32Bit_44100Hz_Calculate_Left_Channel_Power()
{
  //ESP_LOGV("Function Debug", "%s, ", __func__);
  QueueHandle_t QueueIn = GetQueueHandleRXForDataItem("L_PSD_IN");
  QueueHandle_t QueueOut = m_SerialDataLink.GetQueueHandleTXForDataItem("L_PSD");
  if( NULL != QueueIn && NULL != QueueOut )
  {
    assert(1 == m_SerialDataLink.GetSampleCountForDataItem("L_PSD"));
    int32_t *InputDataBuffer = (int32_t*)GetDataBufferForDataItem("L_PSD_IN");
    size_t InputSampleCount = GetSampleCountForDataItem("L_PSD_IN");
    size_t InputByteCount = GetSampleCountForDataItem("L_PSD_IN");
    size_t MessageCount = uxQueueMessagesWaiting(QueueIn);
    for(int i = 0; i < MessageCount; ++i)
    {
      if ( xQueueReceive(QueueIn, InputDataBuffer, 0) == pdTRUE )
      {
        for(int j = 0; j < InputSampleCount; ++j)
        {
          if(true == m_LeftSoundData.PushValueAndCalculateSoundData(InputDataBuffer[j]))
          {
            static bool L_PSD_Push_Successful = true;
            PushValueToQueue(m_LeftSoundData.GetProcessedSoundDataPointer(), QueueOut, false, "Left Processed Sound Data: L_PSD", L_PSD_Push_Successful);
            //xQueueReset(QueueIn);
          }
        }
      }
    }
  }
}

void Sound_Processor::AssignToBands(float* Band_Data, FFT_Calculator* FFT_Calculator, int16_t FFT_Size)
{
  //ESP_LOGV("Function Debug", "%s, ", __func__);
  for(int i = 0; i < FFT_Size/2; ++i)
  {
    float magnitude = FFT_Calculator->GetFFTBufferValue(i);
    float freq = GetFreqForBin(i);
    int bandIndex = -1;
    if(freq > 0 && freq <= 43) bandIndex = 0;
    else if(freq > 43 && freq <= 86) bandIndex = 1;
    else if(freq > 86 && freq <= 129) bandIndex = 2;
    else if(freq > 129 && freq <= 172) bandIndex = 3;
    else if(freq > 172 && freq <= 215) bandIndex = 4;
    else if(freq > 215 && freq <= 258) bandIndex = 5;
    else if(freq > 258 && freq <= 301) bandIndex = 6;
    else if(freq > 301 && freq <= 344) bandIndex = 7;
    else if(freq > 344 && freq <= 388) bandIndex = 8;
    else if(freq > 388 && freq <= 431) bandIndex = 9;
    else if(freq > 431 && freq <= 474) bandIndex = 10;
    else if(freq > 474 && freq <= 517) bandIndex = 11;
    else if(freq > 517 && freq <= 560) bandIndex = 12;
    else if(freq > 560 && freq <= 603) bandIndex = 13;
    else if(freq > 603 && freq <= 646) bandIndex = 14;
    else if(freq > 646 && freq <= 689) bandIndex = 15;
    else if(freq > 689 && freq <= 800) bandIndex = 16;
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
    if(bandIndex >= 0 && freq < I2S_SAMPLE_RATE / 2) Band_Data[bandIndex] += magnitude;
  }
}

float Sound_Processor::GetFreqForBin(int Bin)
{
  //ESP_LOGV("Function Debug", "%s, ", __func__);
  return (float)(Bin * ((float)I2S_SAMPLE_RATE / (float)(FFT_SIZE)));
}
int Sound_Processor::GetBinForFrequency(float Frequency)
{
  //ESP_LOGV("Function Debug", "%s, ", __func__);
  return ((int)((float)Frequency / ((float)I2S_SAMPLE_RATE / (float)(FFT_SIZE))));
}
