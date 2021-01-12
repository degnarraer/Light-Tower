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
 
#include <Adafruit_ZeroFFT.h>
#include <arm_common_tables.h>
#include <Arduino.h>
#include "Statistical_Engine.h"

CalculateFPS calculateFPS2("Statistical Engine", 1000);
void StatisticalEngine::Setup()
{
  calculateFPS2.Setup();
  m_Sampler.SetSampleRateAndStart(SAMPLE_RATE);
}

bool StatisticalEngine::CanRunMyTask()
{
  if(true == m_Sampler.IsAvailable())
  {
    if(true == calculateFPS2.CanRunMyTask()) { calculateFPS2.RunMyTask(); }
    return true;
  }
  else
  {
    return false;
  }
}
void StatisticalEngine::RunMyTask()
{
  GetSampledSoundData();
}

void StatisticalEngine::HandleADCInterrupt()
{
  m_Sampler.HandleADCInterrupt();
}

void StatisticalEngine::GetSampledSoundData()
{
  int i = 0;
  while( i < MAX_BUFFERS_TO_PROCESS && 
         m_Sampler.GetNumberOfReadings() > 0 &&
         true == NewDataReady() )
  {
    if(true == debugMode && debugLevel >= 3) Serial << "StatisticalEngine: Processing Sound Data\n";
    AnalyzeSound();
    UpdateSoundState();
    ++i;
    if(true == debugMode && debugLevel >= 3) Serial << "Min: " << m_signalMin << "\tMax: " << m_signalMax << "\tIntegrator: " << m_silenceIntegrator << "\tPower: " << m_Power << "\tPower Db: " << m_PowerDb << "\n";
  }
}

bool StatisticalEngine::NewDataReady()
{
  if (true == m_Sampler.IsAvailable())
  {
    int bufferLength = 0;
    uint16_t* cBuf = m_Sampler.GetFilledBuffer(&bufferLength);
    for (int i = 0; i < bufferLength; i=i+NUM_CHANNELS)
    {
      m_data[i/NUM_CHANNELS] = cBuf[i+2];
      if(i == 0)
      {
        m_FFTGain = 1.0 + ((FFT_GAIN - 1) - ((FFT_GAIN - 1) * log10((float)ADDBITS - cBuf[0])/log10((float)ADDBITS)));
        m_AmpGain = 1.0 + ((POWER_GAIN - 1) - ((POWER_GAIN - 1) * log10((float)ADDBITS - cBuf[1])/log10((float)ADDBITS)));
      }
    }
    m_Sampler.SetReadCompleted();
    if(true == debugMode && debugLevel >= 3) Serial << "Amp Gain: " << m_AmpGain << "\tFFT Gain: " << m_FFTGain << "\n";
    return true;
  }
  else
  {
    return false; 
  }
}

void StatisticalEngine::AnalyzeSound()
{
  float db = 0.0;
  int avg = 0;
  int peakToPeak = 0;
  int signalMax = -MAX_POWER;
  int signalMin = MAX_POWER;
  
  if(true == debugMode && debugLevel >= 3) Serial << "Amplitude Gain: " << m_AmpGain << "\t" << "FFT Gain: " << m_FFTGain << "\n";
  for(int i = 0; i < CHANNEL_SIZE; ++i)
  {
      avg += m_data[i];
  }
  avg = avg/CHANNEL_SIZE;
  for(int i=0; i < CHANNEL_SIZE; i++)
  {
    int result = ((m_data[i] - avg) * m_AmpGain);
    if(result > INT16_MAX) result = INT16_MAX;
    if(result < INT16_MIN) result = INT16_MIN;
    if (result > signalMax)
    {
      signalMax = result;
    }
    if (result < signalMin)
    {
      signalMin = result;
    }
    result = result * m_FFTGain;
    if(result > INT16_MAX) result = INT16_MAX;
    if(result < INT16_MIN) result = INT16_MIN;
    m_data[i] = result;
  }
  m_signalMin = signalMin;
  m_signalMax = signalMax;
  peakToPeak = m_signalMax - m_signalMin;
  m_Power = ((float)peakToPeak / (float)ADDBITS);
  if(peakToPeak > 0)
  {
    db = 20*log10(peakToPeak/100.0);
  }
  else
  {
    db = 0.0;
  }
  m_PowerDb = db / MAX_DB;
  if(m_PowerDb > 1.0) m_PowerDb = 1.0;
  if(m_PowerDb < 0.0) m_PowerDb = 0.0;
  if(m_Power > 1.0) m_Power = 1.0;
  if(m_Power < 0.0) m_Power = 0.0;
  if(true == m_ProcessFFT)
  {
    ZeroFFT(m_data, FFT_MAX);
    UpdateBandArray();
  }
  if(true == debugMode && debugLevel >= 3) Serial << "Min: " << m_signalMin << "\tMax: " << m_signalMax << "\tPower: " << m_Power << "\tPower Db: " << m_PowerDb << "\n";
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
  for(int i = 0; i < BINS; ++i)
  {
    float freq = GetFreqForBin(i);
    int bandIndex = 0;
    if(8 == m_NumBands)
    {
      if(freq > 0 && freq <= 100) bandIndex = 0;
      if(freq > 100 && freq <= 200) bandIndex = 1;
      if(freq > 200 && freq <= 400) bandIndex = 2;
      if(freq > 400 && freq <= 800) bandIndex = 3;
      if(freq > 800 && freq <= 1600) bandIndex = 4;
      if(freq > 1600 && freq <= 3200) bandIndex = 5;
      if(freq > 3200 && freq <= 6400) bandIndex = 6;
      if(freq > 6400 && freq <= 12800) bandIndex = 7;
    }
    else if(31 == m_NumBands)
    {
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
    }
    BandValues[bandIndex][currentBandIndex] += m_data[i];
  }
  if(currentBandIndex >= BAND_SAVE_LENGTH - 1)
  {
    UpdateRunningAverageBandArray();
  }
  if(true == debugMode && debugLevel >= 2) 
  {
    Serial << "BAND VALUES: ";
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
    if(true == debugMode && debugLevel >= 5) Serial << "Band: " << band << " " << "Depth: " << depth << " " << "Result: " << result << "\n";
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

float StatisticalEngine::GetBandAverageForABandOutOfNBands(unsigned band, unsigned int depth, unsigned int TotalBands)
{
  assert(("Output Band must be less than total bands", band < TotalBands));
  assert(("Must convert to a smaller number of Bands", TotalBands <= m_NumBands));
  assert(("Cannot convert to 0 Bands", TotalBands > 0));
  int bandSeparation = m_NumBands / TotalBands;
  int startBand = band * bandSeparation;
  int endBand = startBand + bandSeparation;
  float result = 0.0;
  for(int b = startBand; b < endBand; ++b)
  {
    result += GetBandAverage(b, depth);
  }
  if(true == debugVisualization) Serial << "Separation:" << bandSeparation << "\tStart:" << startBand << "\tEnd:" << endBand << "\tResult:" << result << "\n";
  return result;
}

float StatisticalEngine::GetNormalizedSoundPower()
{ 
  if(true == debugSoundPower) Serial << "StatisticalEngine: Get Sound Power: " << m_Power << "\n";
  return m_Power;
}
