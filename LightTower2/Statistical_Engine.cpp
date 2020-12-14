    /*
    Light Tower by Rob Shockency
    Copyright (C) 2019 Rob Shockency degnarraer@yahoo.com

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
    */
/**
 * @file Statistical_Engine.cpp
 *
 *
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

bool StatisticalEngine::CanRunTask()
{
  if(true == m_Sampler.IsAvailable())
  {
    if(true == calculateFPS2.CanRunTask()) { calculateFPS2.RunTask(); }
    return true;
  }
  else
  {
    return false;
  }
}
void StatisticalEngine::RunTask()
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
    if(true == debugMode && debugLevel >= 3) Serial << "Min: " << m_signalMin << "\tMax: " << m_signalMax << "\tIntegrator: " << m_silenceIntegrator << "\tPower: " << power << "\tPower Db: " << powerDb << "\n";
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
        fftGain = 1.0 + ((FFT_GAIN - 1) - ((FFT_GAIN - 1) * log10((float)ADDBITS - cBuf[0])/log10((float)ADDBITS)));
        ampGain = 1.0 + ((POWER_GAIN - 1) - ((POWER_GAIN - 1) * log10((float)ADDBITS - cBuf[1])/log10((float)ADDBITS)));
      }
    }
    m_Sampler.SetReadCompleted();
    if(true == debugMode && debugLevel >= 3) Serial << "Amp Gain: " << ampGain << "\tFFT Gain: " << fftGain << "\n";
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
  
  if(true == debugMode && debugLevel >= 3) Serial << "Amplitude Gain: " << ampGain << "\t" << "FFT Gain: " << fftGain << "\n";
  for(int i = 0; i < CHANNEL_SIZE; ++i)
  {
      avg += m_data[i];
  }
  avg = avg/CHANNEL_SIZE;
  for(int i=0; i < CHANNEL_SIZE; i++)
  {
    int result = ((m_data[i] - avg) * ampGain);
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
    result = result * fftGain;
    if(result > INT16_MAX) result = INT16_MAX;
    if(result < INT16_MIN) result = INT16_MIN;
    m_data[i] = result;
  };
  m_signalMin = signalMin;
  m_signalMax = signalMax;
  peakToPeak = m_signalMax - m_signalMin;
  power = ((float)peakToPeak / (float)ADDBITS);
  if(peakToPeak > 0)
  {
    db = 20*log10(peakToPeak/100.0);
  }
  else
  {
    db = 0.0;
  }
  powerDb = db / MAX_DB;
  if(powerDb > 1.0) powerDb = 1.0;
  if(powerDb < 0.0) powerDb = 0.0;
  if(power > 1.0) power = 1.0;
  if(power < 0.0) power = 0.0;
  ZeroFFT(m_data, FFT_MAX);
  UpdateBandArray();
  if(true == debugMode && debugLevel >= 3) Serial << "Min: " << m_signalMin << "\tMax: " << m_signalMax << "\tPower: " << power << "\tPower Db: " << powerDb << "\n";
}

void StatisticalEngine::UpdateSoundState()
{
  int delta = 0;
  float gain = 0.0;
  if(power >= SOUND_DETECT_THRESHOLD)
  {
    float  numerator = power - SOUND_DETECT_THRESHOLD;
    float  denomanator = SOUND_DETECT_THRESHOLD;
    if(numerator < 0) numerator = 0;
    gain = (numerator/denomanator);
    delta = m_soundAdder * gain;
  }
  else
  {
    float  numerator = SOUND_DETECT_THRESHOLD - power;
    float  denomanator = SOUND_DETECT_THRESHOLD;
    if(numerator < 0) numerator = 0;
    gain = (numerator/denomanator);
    delta = m_silenceSubtractor * gain;
  }
  m_silenceIntegrator += delta;
  if(m_silenceIntegrator < m_silenceIntegratorMin) m_silenceIntegrator = m_silenceIntegratorMin;
  if(m_silenceIntegrator > m_silenceIntegratorMax) m_silenceIntegrator = m_silenceIntegratorMax;
  if(true == debugMode && debugLevel >= 3) Serial << "Power Db: " << powerDb << "\tGain: " << gain << "\tDelta: " << delta << "\tSilence Integrator: " << m_silenceIntegrator << "\tSound State: " << soundState << "\n";
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
  for(int i = 0; i < NUM_BANDS; ++i)
  {
    BandValues[i][currentBandIndex] = 0;
  }
  for(int i = 0; i < BINS; ++i)
  {
    float freq = GetFreqForBin(i);
    int bandIndex = 0;
    if(freq > 0 && freq <= 100) bandIndex = 0;
    if(freq > 100 && freq <= 200) bandIndex = 1;
    if(freq > 200 && freq <= 400) bandIndex = 2;
    if(freq > 400 && freq <= 800) bandIndex = 3;
    if(freq > 800 && freq <= 1600) bandIndex = 4;
    if(freq > 1600 && freq <= 3200) bandIndex = 5;
    if(freq > 3200 && freq <= 6400) bandIndex = 6;
    if(freq > 6400 && freq <= 12800) bandIndex = 7;
    BandValues[bandIndex][currentBandIndex] += m_data[i];
  }
  if(currentBandIndex >= BAND_SAVE_LENGTH - 1)
  {
    UpdateRunningAverageBandArray();
  }
  if(true == debugMode && debugLevel >= 1) Serial << "BAND VALUES: " << BandValues[0][currentBandIndex] << "\t" 
                                                                     << BandValues[1][currentBandIndex] << "\t"  
                                                                     << BandValues[2][currentBandIndex] << "\t"  
                                                                     << BandValues[3][currentBandIndex] << "\t"  
                                                                     << BandValues[4][currentBandIndex] << "\t"  
                                                                     << BandValues[5][currentBandIndex] << "\t" 
                                                                     << BandValues[6][currentBandIndex] << "\t" 
                                                                     << BandValues[7][currentBandIndex] << "\n";
}

void StatisticalEngine::UpdateRunningAverageBandArray()
{
  ++currentAverageBandIndex;
  if(currentAverageBandIndex >= BAND_SAVE_LENGTH)
  {
    if(true == debugMode && debugLevel >= 2) Serial << "Band Running Average Array Rollover\n";
    currentAverageBandIndex = 0;
  }
  for(int i = 0; i < NUM_BANDS; ++i)
  {
    BandRunningAverageValues[i][currentAverageBandIndex] = GetBandAverage(i, BAND_SAVE_LENGTH);
  }
  if(true == debugMode && debugLevel >= 1) Serial << "BAND AVG VALUES: " << BandRunningAverageValues[0][currentAverageBandIndex] << "\t" 
                                                                         << BandRunningAverageValues[1][currentAverageBandIndex] << "\t"  
                                                                         << BandRunningAverageValues[2][currentAverageBandIndex] << "\t"  
                                                                         << BandRunningAverageValues[3][currentAverageBandIndex] << "\t"  
                                                                         << BandRunningAverageValues[4][currentAverageBandIndex] << "\t"  
                                                                         << BandRunningAverageValues[5][currentAverageBandIndex] << "\t" 
                                                                         << BandRunningAverageValues[6][currentAverageBandIndex] << "\t" 
                                                                         << BandRunningAverageValues[7][currentAverageBandIndex] << "\n"; 
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
  if(band < NUM_BANDS && depth < BAND_SAVE_LENGTH)
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

float StatisticalEngine::GetBandAverage(int band, int depth)
{
  float result = 0.0;
  unsigned int count = 0;
  for(int i = 0; i < BAND_SAVE_LENGTH && i <= depth; ++i)
  {
    result += GetBandValue(band, i);
    ++count;
  }
  result /= count;
  if(true == debugMode && debugLevel >= 5) Serial << "GetBandAverage Band: " << band << "\tDepth: " << depth << "\tResult: " << result <<"\n";
  return result;
}
