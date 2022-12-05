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

Sound_Processor::Sound_Processor( String Title
                                , SPIDataLinkMaster &SPIDataLinkMaster
                                , ContinuousAudioBuffer<AUDIO_BUFFER_SIZE> AudioBuffer)
                                : NamedItem(Title)
                                , QueueManager(Title, GetDataItemConfigCount())
                                , m_SPIDataLinkMaster(SPIDataLinkMaster)
                                , m_AudioBuffer(AudioBuffer)
{
}
Sound_Processor::~Sound_Processor()
{
  FreeMemory();
}
void Sound_Processor::SetupSoundProcessor()
{
  m_AudioBinLimit = GetBinForFrequency(MAX_VISUALIZATION_FREQUENCY);
  if(true == m_MemoryIsAllocated)FreeMemory();
  SetupQueueManager();
  AllocateMemory();
}
void Sound_Processor::AllocateMemory()
{
  m_MemoryIsAllocated = true;
}

void Sound_Processor::FreeMemory()
{
  m_MemoryIsAllocated = false;
}

void Sound_Processor::Calculate_FFTs()
{
  QueueHandle_t QueueIn = GetQueueHandleRXForDataItem("FFT_Frames");
  bool R_FFT_Calculated = false;
  bool L_FFT_Calculated = false;
  
  if(QueueIn != NULL && uxQueueMessagesWaiting(QueueIn) > 0)
  {
    ESP_LOGV("Sound_Processor", "Pushing FFT Data");
    size_t FFTFrameCount = GetSampleCountForDataItem("FFT_Frames");
    size_t FFTByteCount = GetTotalByteCountForDataItem("FFT_Frames");
    
    Frame_t FrameBuffer[FFTFrameCount];
    GetValueFromRXQueue(FrameBuffer, "FFT_Frames", FFTByteCount, false, false);
    for(int i = 0; i < FFTFrameCount; ++i)
    {
      if(true == m_R_FFT.PushValueAndCalculateNormalizedFFT(FrameBuffer[i].channel1, m_FFT_Gain))
      {
        Calculate_Right_Channel_FFT();
        R_FFT_Calculated = true;
      }
      if(true == m_L_FFT.PushValueAndCalculateNormalizedFFT(FrameBuffer[i].channel2, m_FFT_Gain))
      {
        Calculate_Left_Channel_FFT();
        L_FFT_Calculated = true;
      }
      assert(R_FFT_Calculated == L_FFT_Calculated);
    }
  }
}

void Sound_Processor::Calculate_Right_Channel_FFT()
{
  QueueHandle_t R_Bands_QueueOut = m_SPIDataLinkMaster.GetQueueHandleTXForDataItem("R_BANDS");
  QueueHandle_t R_MaxBin_QueueOut = m_SPIDataLinkMaster.GetQueueHandleTXForDataItem("R_MAXBAND");
  QueueHandle_t R_MajorFreq_QueueOut = m_SPIDataLinkMaster.GetQueueHandleTXForDataItem("R_MAJOR_FREQ");
  if(NULL != R_Bands_QueueOut && NULL != R_MaxBin_QueueOut && NULL != R_MajorFreq_QueueOut )
  {
    ESP_LOGV("Sound_Processor", "Calculating Right FFT");
    size_t R_Bands_DataBufferByteCount = m_SPIDataLinkMaster.GetTotalByteCountForDataItem("R_BANDS");
    size_t R_MaxBand_DataBufferByteCount = m_SPIDataLinkMaster.GetTotalByteCountForDataItem("R_MAXBAND");
    size_t R_MajorFreq_DataBufferByteCount = m_SPIDataLinkMaster.GetTotalByteCountForDataItem("R_MAJOR_FREQ");
    size_t R_Bands_SampleCount = m_SPIDataLinkMaster.GetSampleCountForDataItem("R_BANDS");
    size_t R_MaxBand_SampleCount = m_SPIDataLinkMaster.GetSampleCountForDataItem("R_MAXBAND");
    size_t R_MajorFreq_SampleCount = m_SPIDataLinkMaster.GetSampleCountForDataItem("R_MAJOR_FREQ");
    
    assert(NUMBER_OF_BANDS == R_Bands_SampleCount);
    assert(1 == R_MaxBand_SampleCount);
    assert(1 == R_MajorFreq_SampleCount);
    
    float R_Bands_DataBuffer[R_Bands_SampleCount];
    assert(sizeof(R_Bands_DataBuffer) == R_Bands_DataBufferByteCount);
    
    MaxBandSoundData_t R_MaxBandDataBuffer;
    assert(sizeof(MaxBandSoundData_t) == R_MaxBand_DataBufferByteCount);
    assert(sizeof(float) == R_MajorFreq_DataBufferByteCount);
    
    float MaxBandMagnitude = 0;
    int16_t MaxBandIndex = 0;
    memset(R_Bands_DataBuffer, 0, R_Bands_DataBufferByteCount);
    AssignToBands(R_Bands_DataBuffer, &m_R_FFT, FFT_SIZE);
    for(int16_t j = 0; j < R_Bands_SampleCount; ++j)
    {
      if(R_Bands_DataBuffer[j] > MaxBandMagnitude)
      {
        MaxBandMagnitude = R_Bands_DataBuffer[j];
        MaxBandIndex = j;
      }
    }
    R_MaxBandDataBuffer.MaxBandNormalizedPower = MaxBandMagnitude;
    R_MaxBandDataBuffer.MaxBandIndex = MaxBandIndex;
    R_MaxBandDataBuffer.TotalBands = R_Bands_SampleCount;

    static bool R_Bands_Push_Successful = true;
    PushValueToQueue(R_Bands_DataBuffer, R_Bands_QueueOut, false, "Right Bands: R_BANDS", R_Bands_Push_Successful);
    static bool R_MaxBand_Push_Successful = true;
    PushValueToQueue(&R_MaxBandDataBuffer, R_MaxBin_QueueOut, false, "Right Max Band: R_MAXBAND", R_MaxBand_Push_Successful);
    static bool R_MajorFreq_Push_Successful = true;
    PushValueToQueue(m_R_FFT.GetMajorPeakPointer(), R_MajorFreq_QueueOut, false, "Right Major Frequency: R_MAJOR_FREQ", R_MajorFreq_Push_Successful);
    m_SPIDataLinkMaster.TriggerEarlyDataTransmit();
  }
}
void Sound_Processor::Calculate_Left_Channel_FFT()
{
  QueueHandle_t L_Bands_QueueOut = m_SPIDataLinkMaster.GetQueueHandleTXForDataItem("L_BANDS");
  QueueHandle_t L_MaxBin_QueueOut = m_SPIDataLinkMaster.GetQueueHandleTXForDataItem("L_MAXBAND");
  QueueHandle_t L_MajorFreq_QueueOut = m_SPIDataLinkMaster.GetQueueHandleTXForDataItem("L_MAJOR_FREQ");
  if( NULL != L_Bands_QueueOut && NULL != L_MaxBin_QueueOut && NULL != L_MajorFreq_QueueOut )
  {
    ESP_LOGV("Sound_Processor", "Calculating Left FFT");
    size_t L_Bands_DataBufferByteCount = m_SPIDataLinkMaster.GetTotalByteCountForDataItem("L_BANDS");
    size_t L_MaxBand_DataBufferByteCount = m_SPIDataLinkMaster.GetTotalByteCountForDataItem("L_MAXBAND");
    size_t L_MajorFreq_DataBufferByteCount = m_SPIDataLinkMaster.GetTotalByteCountForDataItem("L_MAJOR_FREQ");
    size_t L_Bands_SampleCount = m_SPIDataLinkMaster.GetSampleCountForDataItem("L_BANDS");
    size_t L_MaxBand_SampleCount = m_SPIDataLinkMaster.GetSampleCountForDataItem("L_MAXBAND");
    size_t L_MajorFreq_SampleCount = m_SPIDataLinkMaster.GetSampleCountForDataItem("L_MAJOR_FREQ");

    assert(NUMBER_OF_BANDS == L_Bands_SampleCount);
    assert(1 == L_MaxBand_SampleCount);
    assert(1 == L_MajorFreq_SampleCount);
    
    float L_Bands_DataBuffer[L_Bands_SampleCount];
    assert(sizeof(L_Bands_DataBuffer) == L_Bands_DataBufferByteCount);
    
    MaxBandSoundData_t L_MaxBandDataBuffer;
    assert(sizeof(MaxBandSoundData_t) == L_MaxBand_DataBufferByteCount);
    assert(sizeof(float) == L_MajorFreq_DataBufferByteCount);
    
    float MaxBandMagnitude = 0;
    int16_t MaxBandIndex = 0;
    memset(L_Bands_DataBuffer, 0, L_Bands_DataBufferByteCount);
    AssignToBands(L_Bands_DataBuffer, &m_L_FFT, FFT_SIZE);
    for(int16_t j = 0; j < L_Bands_SampleCount; ++j)
    {
      if(L_Bands_DataBuffer[j] > MaxBandMagnitude)
      {
        MaxBandMagnitude = L_Bands_DataBuffer[j];
        MaxBandIndex = j;
      }
    }
    L_MaxBandDataBuffer.MaxBandNormalizedPower = MaxBandMagnitude;
    L_MaxBandDataBuffer.MaxBandIndex = MaxBandIndex;
    L_MaxBandDataBuffer.TotalBands = L_Bands_SampleCount;

    static bool L_Bands_Push_Successful = true;
    PushValueToQueue(L_Bands_DataBuffer, L_Bands_QueueOut, false, "Left Bands: L_BANDS", L_Bands_Push_Successful);
    static bool L_MaxBand_Push_Successful = true;
    PushValueToQueue(&L_MaxBandDataBuffer, L_MaxBin_QueueOut, false, "Left Max Band: L_MAXBAND", L_MaxBand_Push_Successful);
    static bool L_MajorFreq_Push_Successful = true;
    PushValueToQueue(m_L_FFT.GetMajorPeakPointer(), L_MajorFreq_QueueOut, false, "Left Major Frequency: L_MAJOR_FREQ", L_MajorFreq_Push_Successful);
    m_SPIDataLinkMaster.TriggerEarlyDataTransmit();
  }
}
void Sound_Processor::Calculate_Power()
{
  QueueHandle_t QueueIn = GetQueueHandleRXForDataItem("Amplitude_Frames");
  QueueHandle_t QueueOut = m_SPIDataLinkMaster.GetQueueHandleTXForDataItem("Processed_Frame");
  
  if(QueueIn != NULL && QueueOut != NULL && uxQueueMessagesWaiting(QueueIn) > 0)
  {
    size_t AmplitudeFrameCount = GetSampleCountForDataItem("Amplitude_Frames");
    size_t AmplitudeByteCount = GetTotalByteCountForDataItem("Amplitude_Frames");
    size_t PSF_ByteCount = m_SPIDataLinkMaster.GetTotalByteCountForDataItem("Processed_Frame");
    size_t PSF_SampleCount = m_SPIDataLinkMaster.GetSampleCountForDataItem("Processed_Frame");
    assert(1 == PSF_SampleCount);
    assert(sizeof(ProcessedSoundFrame_t) == PSF_ByteCount);
    assert(sizeof(Frame_t) * AmplitudeFrameCount == AmplitudeByteCount);
    
    Frame_t FrameBuffer[AmplitudeFrameCount];
    GetValueFromRXQueue(FrameBuffer, "Amplitude_Frames", AmplitudeByteCount, false, false);
    for(int i = 0; i < AmplitudeFrameCount; ++i)
    {
      bool R_Amplitude_Calculated = false;
      bool L_Amplitude_Calculated = false;
      ProcessedSoundData_t R_PSD;
      ProcessedSoundData_t L_PSD;
      if(true == m_RightSoundData.PushValueAndCalculateSoundData(FrameBuffer[i].channel1))
      {
        R_Amplitude_Calculated = true;
        R_PSD = m_RightSoundData.GetProcessedSoundData();
      }
      if(true == m_LeftSoundData.PushValueAndCalculateSoundData(FrameBuffer[i].channel2))
      {
        L_Amplitude_Calculated = true;
        L_PSD = m_LeftSoundData.GetProcessedSoundData();
      }
      assert(R_Amplitude_Calculated == L_Amplitude_Calculated);
      if(true == R_Amplitude_Calculated && true == L_Amplitude_Calculated)
      {
        ProcessedSoundFrame_t PSF;
        PSF.Channel1 = R_PSD;
        PSF.Channel2 = L_PSD;
        static bool PSF_Push_Successful = true;
        PushValueToQueue(&PSF, QueueOut, false, "Processed Sound Frame: Processed_Frame", PSF_Push_Successful);
        m_SPIDataLinkMaster.TriggerEarlyDataTransmit();
      }
    }
  }
}
void Sound_Processor::AssignToBands(float* Band_Data, FFT_Calculator* FFT_Calculator, int16_t FFT_Size)
{
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
  return (float)(Bin * ((float)I2S_SAMPLE_RATE / (float)(FFT_SIZE)));
}
int Sound_Processor::GetBinForFrequency(float Frequency)
{
  return ((int)((float)Frequency / ((float)I2S_SAMPLE_RATE / (float)(FFT_SIZE))));
}
