/*
    Light Tower by Rob Shockency
    Copyright (C) 2020 Rob Shockency degnarraer@yahoo.com

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
 
#include <arm_common_tables.h>
#include <Arduino.h>
#include "Statistical_Engine.h"


//CalculateFPS calculateFPS2("Statistical Engine", 1000);
void StatisticalEngine::Setup()
{
  if(false == m_MemoryIsAllocated) AllocateMemory();
  SetupQueueManager();
}

bool StatisticalEngine::NewBandDataReady()
{
  bool A = (uxQueueMessagesWaiting(GetQueueHandleRXForDataItem("R_FFT")) > 0);
  bool B = (uxQueueMessagesWaiting(GetQueueHandleRXForDataItem("L_FFT")) > 0);
  //Serial << A << "|" << B << "\n";
  if( A & B )
  {
    m_NewBandDataReady = true;
    return true;
  }
  else
  {
    m_NewBandDataReady = false;
    return false;
  }
}

bool StatisticalEngine::NewSoundDataReady()
{
  bool A = (uxQueueMessagesWaiting(GetQueueHandleRXForDataItem("R_PSD")) > 0);
  bool B = (uxQueueMessagesWaiting(GetQueueHandleRXForDataItem("L_PSD")) > 0);
  //Serial << A << "|" << B << "\n";
  if( A & B )
  {
    m_NewSoundDataReady = true;
    return true;
  }
  else
  {
    m_NewSoundDataReady = false;
    return false;
  }
}

bool StatisticalEngine::CanRunMyScheduledTask()
{
  if( true == NewSoundDataReady() || true == NewBandDataReady() )
  {
    return true;
  }
  else
  {
    return false;
  }
}

void StatisticalEngine::RunMyScheduledTask()
{
  if(true == m_NewSoundDataReady)
  {
    GetValueFromQueue(&m_Right_Channel_Processed_Sound_Data, GetQueueHandleRXForDataItem("R_PSD"), GetByteCountForDataItem("R_PSD"), true, false);
    GetValueFromQueue(&m_Left_Channel_Processed_Sound_Data, GetQueueHandleRXForDataItem("L_PSD"), GetByteCountForDataItem("L_PSD"), true, false);
    
    //To allow the original code to work, we combine the left and right channels into an average
    m_Power = (m_Right_Channel_Processed_Sound_Data.NormalizedPower + m_Left_Channel_Processed_Sound_Data.NormalizedPower) / 2;
    m_PowerDb = (m_Right_Channel_Processed_Sound_Data.PowerDB + m_Left_Channel_Processed_Sound_Data.PowerDB) / 2;
    m_signalMin = (m_Right_Channel_Processed_Sound_Data.Minimum + m_Left_Channel_Processed_Sound_Data.Minimum) / 2;
    m_signalMax = (m_Right_Channel_Processed_Sound_Data.Maximum + m_Left_Channel_Processed_Sound_Data.Maximum) / 2;
    
    if(true == STATISTICAL_ENGINE_DATA_DEBUG) Serial << "L: " << m_Left_Channel_Processed_Sound_Data.NormalizedPower << "|" << m_Left_Channel_Processed_Sound_Data.PowerDB << "|" << m_Left_Channel_Processed_Sound_Data.Minimum << "|" << m_Left_Channel_Processed_Sound_Data.Maximum << "\t" << "R: " << m_Right_Channel_Processed_Sound_Data.NormalizedPower << "|" << m_Right_Channel_Processed_Sound_Data.PowerDB << "|" << m_Right_Channel_Processed_Sound_Data.Minimum << "|" << m_Right_Channel_Processed_Sound_Data.Maximum << "\n";
    UpdateSoundState();
  }

  if(true == m_NewBandDataReady)
  {
    GetValueFromQueue(m_Right_Band_Values, GetQueueHandleRXForDataItem("R_FFT"), GetByteCountForDataItem("R_FFT"), true, false);
    GetValueFromQueue(m_Left_Band_Values, GetQueueHandleRXForDataItem("L_FFT"), GetByteCountForDataItem("L_FFT"), true, false);
    UpdateBandArray(); 
  }
}

void StatisticalEngine::AllocateMemory()
{
  Serial << GetTitle() << ": Allocating Memory.\n";
  m_MemoryIsAllocated = true;
}

void StatisticalEngine::FreeMemory()
{
  Serial << GetTitle() << ": Freeing Memory.\n";
  m_MemoryIsAllocated = false;
}

void StatisticalEngine::UpdateSoundState()
{
  int delta = 0;
  float gain = 0.0;
  if(m_Power >= SOUND_DETECT_THRESHOLD)
  {
    float  numerator = m_Power - SOUND_DETECT_THRESHOLD;
    float  denomanator = SOUND_DETECT_THRESHOLD;
    if(numerator < 0) numerator = 0;
    gain = (numerator/denomanator);
    delta = m_soundAdder * gain;
  }
  else
  {
    float  numerator = SOUND_DETECT_THRESHOLD - m_Power;
    float  denomanator = SOUND_DETECT_THRESHOLD;
    if(numerator < 0) numerator = 0;
    gain = (numerator/denomanator);
    delta = m_silenceSubtractor * gain;
  }
  m_silenceIntegrator += delta;
  if(m_silenceIntegrator < m_silenceIntegratorMin) m_silenceIntegrator = m_silenceIntegratorMin;
  if(m_silenceIntegrator > m_silenceIntegratorMax) m_silenceIntegrator = m_silenceIntegratorMax;
  if(true == debugMode && debugLevel >= 3) Serial << "Power Db: " << m_PowerDb << "\tGain: " << gain << "\tDelta: " << delta << "\tSilence Integrator: " << m_silenceIntegrator << "\tSound State: " << soundState << "\n";
  if((soundState == SoundState::SilenceDetected || soundState == SoundState::LastingSilenceDetected) && m_silenceIntegrator >= m_soundDetectedThreshold)
  {
    if(true == debugMode && debugLevel >= 1) Serial << "Sound Detected\n";
    soundState = SoundState::SoundDetected;
    m_cb->MicrophoneStateChange(soundState);
  }
  else if(soundState == SoundState::SoundDetected && m_silenceIntegrator <= m_silenceDetectedThreshold)
  {
    if(true == debugMode && debugLevel >= 1) Serial << "Silence Detected\n";
    soundState = SoundState::SilenceDetected;
    m_silenceStartTime = millis();
    m_cb->MicrophoneStateChange(soundState);
  }
  else if(soundState == SoundState::SilenceDetected && millis() - m_silenceStartTime >= 120000)
  {
    if(true == debugMode && debugLevel >= 1) Serial << "Lasting Silence Detected\n";
    soundState = SoundState::LastingSilenceDetected;
    m_cb->MicrophoneStateChange(soundState);
  }
}

void StatisticalEngine::UpdateBandArray()
{
  ++currentBandIndex;
  if(currentBandIndex >= BAND_SAVE_LENGTH)
  {
    currentBandIndex = 0;
    if(true == debugMode && debugLevel >= 2) Serial << "Band Array Rollover\n";
  }
  for(int i = 0; i < m_NumBands; ++i)
  {
    BandValues[i][currentBandIndex] = 0;
  }
  for(int i = 0; i < m_NumBands; ++i)
  {
    BandValues[i][currentBandIndex] = (m_Right_Band_Values[i] + m_Left_Band_Values[i]) / 2;
  }
  if(currentBandIndex >= BAND_SAVE_LENGTH - 1)
  {
    UpdateRunningAverageBandArray();
  }
  if(true == debugSetBandValueStatisticalEngine) 
  {
    Serial << "SET BAND VALUES: ";
    for(int i = 0; i < m_NumBands; i++)
    {
      Serial << BandValues[i][currentBandIndex] << "\t";
    }
    Serial << "\n";
  }
}

void StatisticalEngine::UpdateRunningAverageBandArray()
{
  ++currentAverageBandIndex;
  if(currentAverageBandIndex >= BAND_SAVE_LENGTH)
  {
    if(true == debugMode && debugLevel >= 2) Serial << "Band Running Average Array Rollover\n";
    currentAverageBandIndex = 0;
  }
  for(int i = 0; i < m_NumBands; ++i)
  {
    BandRunningAverageValues[i][currentAverageBandIndex] = GetBandAverage(i, BAND_SAVE_LENGTH);
  }
  if(true == debugMode && debugLevel >= 2) 
  {
    Serial << "BAND AVG VALUES: ";
    for(int i = 0; i < m_NumBands; i++)
    {
      Serial << BandRunningAverageValues[0][currentAverageBandIndex] << "\t";
    }
    Serial << "\n";
  }
}

SoundState StatisticalEngine::GetSoundState()
{
  return soundState;
}

float StatisticalEngine::GetFreqForBin(unsigned int bin)
{
  if(bin > BINS) bin = BINS;
  if(bin < 0) bin = 0;
  return FFT_BIN(bin, SAMPLE_RATE, FFT_MAX);
}

int StatisticalEngine::GetBandValue(unsigned int band, unsigned int depth)
{
  int result;
  if(band < m_NumBands && depth < BAND_SAVE_LENGTH)
  {
    int position = 0;
    if (depth <= currentBandIndex)
    {
      position = currentBandIndex - depth;
    }
    else
    {
      position = BAND_SAVE_LENGTH - (depth - currentBandIndex);
    }
    result = BandValues[band][position];
    if(true == debugGetBandValueStatisticalEngine) Serial << "Band: " << band << " " << "Depth: " << depth << " " << "Result: " << result << "\n";
    return result;
  }
  else
  {
    if(true == debugMode) Serial << "!!ERROR: Bin Array Out of Bounds\n";
    return 0;
  }
}

float StatisticalEngine::GetBandAverage(unsigned int band, unsigned int depth)
{
  int total = 0;
  unsigned int count = 0;
  for(int i = 0; i < BAND_SAVE_LENGTH && i <= depth; ++i)
  {
    total += GetBandValue(band, i);
    ++count;
  }
  float result = total / count;
  if(true == debugMode && debugLevel >= 5) Serial << "GetBandAverage Band: " << band << "\tDepth: " << depth << "\tResult: " << result <<"\n";
  return result;
}
int StatisticalEngine::GetBandAverageForABandOutOfNBands(unsigned band, unsigned int depth, unsigned int TotalBands)
{
  assert(band < TotalBands);
  assert(TotalBands <= m_NumBands);
  assert(TotalBands > 0);
  int bandSeparation = m_NumBands / TotalBands;
  int startBand = band * bandSeparation;
  int endBand = startBand + bandSeparation;
  float result = 0.0;
  for(int b = startBand; b < endBand; ++b)
  {
    result += GetBandAverage(b, depth);
  }
  if(true == debugVisualization) Serial << "Separation:" << bandSeparation << "\tStart:" << startBand << "\tEnd:" << endBand << "\tResult:" << result << "\n";
  return (int)round(result);
}

float StatisticalEngine::GetNormalizedSoundPower()
{ 
  if(true == debugSoundPower) Serial << "StatisticalEngine: Get Sound Power: " << m_Power << "\n";
  return m_Power;
}
