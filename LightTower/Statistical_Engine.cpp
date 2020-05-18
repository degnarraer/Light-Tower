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

void StatisticalEngine::HandleInterrupt()
{
  sampler.HandleInterrupt();
}

void StatisticalEngine::Setup()
{
  if(true == debugMode && debugLevel >= 0) Serial << "StatisticalEngine: Setup Complete\n";
  sampler.SetSampleRateAndStart(SAMPLE_RATE);
}

void StatisticalEngine::UpdateSoundData()
{
  int i = 0;
  while( i < MAX_BUFFERS_TO_PROCESS
      && sampler.GetNumberOfReadings() > 0 
      && true == NewDataReady() )
  {
    AnalyzeSound();
    UpdateSoundState();
    ++i;
    //if(true == debugMode && debugLevel >= 0 && i >= MAX_BUFFERS_TO_PROCESS) Serial << "2\n";
    if(true == debugMode && debugLevel >= 3) Serial << "Min: " << m_signalMin << "\tMax: " << m_signalMax << "\tIntegrator: " << m_silenceIntegrator << "\tPower: " << power << "\tPower Db: " << powerDb << "\n";
  }
}

bool StatisticalEngine::NewDataReady()
{
  if(false == testMode)
  {
    if (sampler.IsAvailable())
    {
      int bufferLength = 0;
      uint16_t* cBuf = sampler.GetFilledBuffer(&bufferLength);
      for (int i = 0; i < bufferLength; i=i+NUM_CHANNELS)
      {
        m_data[i/NUM_CHANNELS] = cBuf[i+2];
        if(i == 0)
        {
          fftGain = 1.0 + ((FFT_GAIN - 1) - ((FFT_GAIN - 1) * log10(ADDBITS - cBuf[0])/log10(ADDBITS)));
          ampGain = 1.0 + ((POWER_GAIN - 1) - ((POWER_GAIN - 1) * log10(ADDBITS - cBuf[1])/log10(ADDBITS)));
        }
      }
      sampler.SetReadCompleted();
      if(true == debugMode && debugLevel >= 3) Serial << "Amp Gain: " << ampGain << "\tFFT Gain: " << fftGain << "\n";
      return true;
    }
    else
    {
      return false; 
    }
  }
  else
  {
    int testCases = (sizeof(testData)/sizeof(*testData));
    int testCase = testCount%testCases;
    if(true == debugMode && debugLevel >= 4) Serial << "Test Case: " << testCase + 1 << " of: " << testCases << "\n";
    FillDataBufferHelper( testCase
                        , testData[testCase].a1
                        , testData[testCase].f1
                        , testData[testCase].a2
                        , testData[testCase].f2
                        , testData[testCase].a3
                        , testData[testCase].f3
                        , testData[testCase].a4
                        , testData[testCase].f4
                        , testData[testCase].a5
                        , testData[testCase].f5
                        , testData[testCase].a6
                        , testData[testCase].f6 );
    ++holdCount;
    if(holdCount%holdTime == 0)
    {
      if(testCase + 1 >= testCases)
      {
        m_cb->TestSequenceComplete();
      }
      ++testCount;
    }
    return true;
  }
}

void StatisticalEngine::EnableTestMode()
{
  testMode = true;
}

void StatisticalEngine::DisableTestMode()
{
  testMode = false;  
}

bool StatisticalEngine::GetTestMode()
{
  return testMode;
}

void StatisticalEngine::TestFunctions()
{
  
}

void StatisticalEngine::SetTestData(int16_t data[FFT_MAX])
{
  memset(BinValues, 0, sizeof(BinValues[0][0]) * BINS * BIN_SAVE_LENGTH);
  memset(BinRunningAverageValues, 0, sizeof(BinRunningAverageValues[0][0]) * BINS * BIN_SAVE_LENGTH);
  currentAverageBinIndex = 0;
  currentBinIndex = 0;
  
  for(int i = 0; i < CHANNEL_SIZE; ++i)
  {
    m_data[i] = data[i];
  }
  for(int i = 0; i < BIN_SAVE_LENGTH; ++i)
  {
    UpdateBinArray(); 
  }
  currentAverageBinIndex = 0;
  currentBinIndex = 0;
}

void StatisticalEngine::UpdateBinArray()
{
  ++currentBinIndex;
  if(currentBinIndex >= BIN_SAVE_LENGTH )
  {
    if(true == debugMode && debugLevel >= 4) Serial << "Bin Array Rollover\n";
    currentBinIndex = 0;
  }
  for(int i = 0; i < BINS; ++i)
  {
    int result = 0;
    if(m_data[i] > ADDBITS - 1)
    {
      result = ADDBITS - 1;
    }
    else
    {
      result = m_data[i];
    }
    if(false == binArrayInitialized)
    {
      for(int j = 0; j < BIN_SAVE_LENGTH; ++j)
      {
        BinValues[i][j] = result;
      }
    }
    else
    {
      BinValues[i][currentBinIndex] = result; 
    }
  }
  binArrayInitialized = true;
  if(currentBinIndex >= BIN_SAVE_LENGTH - 1 )
  {
    UpdateRunningAverageBinArray();
  }
  if(true == debugMode && debugLevel >= 5)
  {
    for(int i = 0; i < BINS; ++i)
    {
      Serial << "Inst Bin: " << i << "\t";
      for(int j = 0; j < BIN_SAVE_LENGTH; ++j)
      {
        Serial << BinValues[i][j] << " \t ";
      }
      Serial << "\n";
    }
    Serial << "\n"; 
  }
}

void StatisticalEngine::UpdateRunningAverageBinArray()
{
  ++currentAverageBinIndex;
  if(currentAverageBinIndex >= BIN_SAVE_LENGTH)
  {
    if(true == debugMode && debugLevel >= 4) Serial << "Bin Running Average Array Rollover\n";
    currentAverageBinIndex = 0;
  }
  for(int i = 0; i < BINS; ++i)
  {
    int result = GetFFTBinAverage(i, BIN_SAVE_LENGTH, BinDataType::INSTANT);
    if(false == runningAverageBinArrayInitialized)
    {
      for(int j = 0; j < BIN_SAVE_LENGTH; ++j)
      {
        BinRunningAverageValues[i][j] = result;
      }
    }
    else
    {
      BinRunningAverageValues[i][currentAverageBinIndex] = result; 
    }
  }
  runningAverageBinArrayInitialized = true;
  if(true == debugMode && debugLevel >= 5)
  {
    for(int i = 0; i < BINS; ++i)
    {
      Serial << "Avg Bin: " << i << "\t";
      for(int j = 0; j < BIN_SAVE_LENGTH; ++j)
      {
        Serial << BinRunningAverageValues[i][j] << " \t ";
      }
      Serial << "\n";
    }
    Serial << "\n"; 
  }
}

void StatisticalEngine::FillDataBufferHelper(int testCase, float  a1, float  f1, float  a2, float  f2, float  a3, float  f3, float  a4, float  f4, float  a5, float  f5, float  a6, float  f6) 
{
  if(true == debugMode && debugLevel >= 4) Serial << "Test Case: " << testCase << "\n:";
  t_count = 0;
  for (int i = 0; i < CHANNEL_SIZE; ++i)
  {
    m_data[i] = ( (a1 * ADDBITS/2 * sin(2.0 * M_PI  * f1  * t_count*t)) + 
                  (a2 * ADDBITS/2 * sin(2.0 * M_PI  * f2  * t_count*t)) + 
                  (a3 * ADDBITS/2 * sin(2.0 * M_PI  * f3  * t_count*t)) +
                  (a4 * ADDBITS/2 * sin(2.0 * M_PI  * f4  * t_count*t)) + 
                  (a5 * ADDBITS/2 * sin(2.0 * M_PI  * f5  * t_count*t)) + 
                  (a6 * ADDBITS/2 * sin(2.0 * M_PI  * f6  * t_count*t)) );
    ++t_count;
  }
}

void StatisticalEngine::AnalyzeSound()
{
  float db = 0.0;
  int avg = 0;
  int peakToPeak = 0;
  int signalMax = -MAX_POWER;
  int signalMin = MAX_POWER;
  
  if(true == testMode)
  {
    ampGain = testAmpGain;
    fftGain = testFFTGain;
  }
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
  if(true == debugPlotMic) PlotData();
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
  if(true == debugPlotFFT) PlotData();
  UpdateBinArray();
  if(true == debugMode && debugLevel >= 3) Serial << "Min: " << m_signalMin << "\tMax: " << m_signalMax << "\tPower: " << power << "\tPower Db: " << powerDb << "\n";
  PrintDataBuffer("After FFT: ");
}
void StatisticalEngine::PlotData()
{
  for(int i=0; i < CHANNEL_SIZE; i++)
  {
    Serial.println(m_data[i]);
  };
}
void StatisticalEngine::PrintDataBuffer(char tag[10])
{
  if(true == debugMode && debugLevel >= 4)
  {
    for(int i = 0; i < FFT_MAX / 2; ++i)
    {
      //print the frequency
      Serial.print(FFT_BIN(i, SAMPLE_RATE, FFT_MAX));
      Serial.print(" Hz: ");
    
      //print the corresponding FFT output
      Serial.println(m_data[i]);
    }
  }
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

SoundState StatisticalEngine::GetSoundState()
{
  return soundState;
}

int StatisticalEngine::GetMaxFFTBin(int maxBinToTest)
{
  int maxBin = 0;
  int max = 0;
  for (int i = 0; i < BINS && i <= maxBinToTest; ++i)
  {
    if(GetFFTData(i) > max)
    {
      max = GetFFTData(i);
      maxBin = i;
    }
  }
  return maxBin;
}

int StatisticalEngine::GetMaxRSSBin(int maxBinToTest, int depth, enum BinDataType binDataType)
{
  int maxBin = 0;
  int max = 0;
  for (int i = 0; i < BINS && i <= maxBinToTest; ++i)
  {
    int result = GetFFTBinRSS(i, depth, BinDataType::INSTANT);
    if(result > max)
    {
      max = result;
      maxBin = i;
    }
  }
  return maxBin;
}

int StatisticalEngine::GetMaxFFTBinFromRange(int minBinToTest, int maxBinToTest)
{
  int maxBin = 0;
  int max = 0;
  for (int i = minBinToTest; i < BINS && i <= maxBinToTest; ++i)
  {
    if(GetFFTData(i) > max)
    {
      max = GetFFTData(i);
      maxBin = i;
    }
  }
  return maxBin;
}

int StatisticalEngine::GetFFTData(int position)
{
  if(position < BINS)
  {
    int result = m_data[position];
    return result;
  }
  else
  {
    return 0;
  }
}

int StatisticalEngine::GetFFTBinData(unsigned int bin, int depth, enum BinDataType binDataType)
{
  int result;
  switch(binDataType)
  {
    case BinDataType::INSTANT:
      if(bin < BINS && depth < BIN_SAVE_LENGTH)
      {
        int position = 0;
        if (depth <= currentBinIndex)
        {
          position = currentBinIndex - depth;
        }
        else
        {
          position = BIN_SAVE_LENGTH - (depth - currentBinIndex);
        }
        result = BinValues[bin][position];
        if(true == debugMode && debugLevel >= 5) Serial << "Instant:\tBin: " << bin << " " << "Depth: " << depth << " " << "Result: " << result << "\n";
        return result;
      }
      else
      {
        if(true == debugMode) Serial << "!!ERROR: Bin Array Out of Bounds\n";
        return 0;
      }
      break;
    case BinDataType::AVERAGE:
      if(bin < BINS && depth < BIN_SAVE_LENGTH)
      {
        int position = 0;
        if (depth <= currentAverageBinIndex)
        {
          position = currentAverageBinIndex - depth;
        }
        else
        {
          position = BIN_SAVE_LENGTH - (depth - currentAverageBinIndex);
        }
        result = BinRunningAverageValues[bin][position];
        if(true == debugMode && debugLevel >= 5) Serial << "Average:\tBin: " << bin << " " << "Depth: " << depth << " " << "Result: " << result << "\n";
        return result;
      }
      else
      {
        if(true == debugMode) Serial << "!!ERROR: Bin Average Array Out of Bounds\n";
        return 0;
      }
    break;
    default:
        return 0;
    break;
  }
}

int StatisticalEngine::GetFFTBinAverage(unsigned int bin, int depth, enum BinDataType binDataType)
{
  int result = 0;
  int count = 0;
  for(int i = 0; i < BIN_SAVE_LENGTH && i <= depth; ++i)
  {
    result += GetFFTBinData(bin, i, binDataType);
    ++count;
  }
  result /= count;
  if(true == debugMode && debugLevel >= 5) Serial << "GetFFTBinAverage Bin: " << bin << "\tDepth: " << depth << "\tResult: " << result <<"\n";
  return result;
}

int StatisticalEngine::GetFFTBinMax(unsigned int bin, int depth, enum BinDataType binDataType)
{
  int maximum = 0;
  for(int i = 0; i < BIN_SAVE_LENGTH && i <= depth; ++i)
  {
    int value = GetFFTBinData(bin, i, binDataType);
    if(value > maximum) maximum = value;
  }
  return maximum;
}

int StatisticalEngine::GetFFTBinMin(unsigned int bin, int depth, enum BinDataType binDataType)
{
  int minimum = INT_MAX;
  for(int i = 0; i < BIN_SAVE_LENGTH && i <= depth; ++i)
  {
    int value = GetFFTBinData(bin, i, binDataType);
    if(value < minimum) minimum = value;
  }
  return minimum;
}

MinMax StatisticalEngine::GetFFTBinMinMaxAverage(unsigned int bin, int depth, enum BinDataType binDataType)
{
  MinMax result;
  int avg = GetFFTBinAverage(bin, depth, binDataType);
  int maxTotal = 0;
  int maxCount = 0;
  int minTotal = 0;
  int minCount = 0;
  for(int i = 0;  i < BIN_SAVE_LENGTH && i <= depth; ++i)
  {
    int binResult = GetFFTBinData(bin, i, binDataType);
    if(binResult == avg)
    {
      maxTotal += binResult;
      ++maxCount;
      minTotal += binResult;
      ++minCount;
    }
    else if(binResult > avg)
    {
      maxTotal += binResult;
      ++maxCount;
    }
    else
    {
      minTotal += binResult;
      ++minCount;
    }
    if(true == debugMode && debugLevel >= 5) Serial << bin << "|" << i << "|" << avg << "|" << binResult << "\t\t" << minTotal << "|" << minCount << "\t|\t" << maxTotal << "|" << maxCount << "\n";
  }
  if(minCount == 0)
  {
    result.min = 0.0;
  }
  else
  {
    result.min = minTotal/minCount;
  }
  if(maxCount == 0)
  {
    result.max = 0.0;
  }
  else
  {
    result.max = maxTotal/maxCount;
  }
  if(true == debugNanInf && (result.min == result.max || isnan(result.min) || isnan(result.max) || isinf(result.min) || isinf(result.max))) Serial << "GetFFTBinMinMaxAverage Bin: " << bin << "\tDepth: " << depth << "\tAverage: " << avg << "\tResult Min: " << result.min << "\tResult Max: " << result.max <<"\n";
  return result;
}

int GetFFTBinMinAverage(unsigned int bin)
{
  
}

int StatisticalEngine::GetFFTBinRSS(unsigned int bin, int depth, enum BinDataType binDataType)
{
  int total = 0;
  for(int i = 0; i < BIN_SAVE_LENGTH && i <= depth; ++i)
  {
    int result = GetFFTBinData(bin, i, binDataType);
    total = total + result*result;
  }
  int sqroot = sqrt(total);
  return sqroot;
}

int StatisticalEngine::GetFFTBinRMS(unsigned int bin, int depth, enum BinDataType binDataType)
{
  int total = 0;
  int count = 0;
  for(int i = 0; i < BIN_SAVE_LENGTH && i <= depth; ++i)
  {
    int result = GetFFTBinData(bin, i, binDataType);
    total = total + result*result;
    ++count;
  }
  if(count == 0) return 0;
  total /= count;
  int sqroot = sqrt(total);
  return sqroot;
}

int StatisticalEngine::GetFFTBinIndexForFrequency(float freq)
{
  if(freq > MAX_DISPLAYED_FREQ) freq = MAX_DISPLAYED_FREQ;
  if(freq < 0) freq = 0.0;
  int index = (freq/(SAMPLE_RATE/2)) * (BINS);
  return index;
}

float  StatisticalEngine::GetFreqForBin(unsigned int bin)
{
  if(bin > BINS) bin = BINS;
  if(bin < 0) bin = 0;
  return FFT_BIN(bin, SAMPLE_RATE, FFT_MAX);
}


db StatisticalEngine::GetFFTBinAverageDb(unsigned int bin, int depth, enum BinDataType binDataType)
{
  int result = 0;
  int count = 0;
  for(int i = 0; i < BIN_SAVE_LENGTH && i <= depth; ++i)
  {
    result += GetFFTBinData(bin, i, binDataType);
    ++count;
  }
  if(count == 0) return 0;
  if(result == 0) return 0;
  return 20*log10(result / count);
}

float  StatisticalEngine::GetRMSOfFreqRange(float startFreq, float endFreq, int depth, enum BinDataType binDataType)
{
  int startBin = GetFFTBinIndexForFrequency(startFreq);
  int endBin = GetFFTBinIndexForFrequency(endFreq);
  int total = 0;
  int i = startBin;
  int count = 0;
  while(i <= endBin)
  {
    for(int j = 0; j<BIN_SAVE_LENGTH && j<=depth; ++j)
    {
      int result = GetFFTBinData(i, j, binDataType);
      total = total + result * result; 
      ++count;
    }
    ++i;
  }
  if(count == 0) return 0;
  float  avg = total/count;
  float  sqroot = sqrt(avg);
  return sqroot;
}

db StatisticalEngine::GetRMSDbOfFreqRange(float startFreq, float endFreq, int depth, enum BinDataType binDataType)
{
  int startBin = GetFFTBinIndexForFrequency(startFreq);
  int endBin = GetFFTBinIndexForFrequency(endFreq);
  int total = 0;
  int i = startBin;
  int count = 0;
  while(i <= endBin)
  {
    for(int j = 0; j<BIN_SAVE_LENGTH && j<=depth; ++j)
    {
      int result = GetFFTBinData(i, j, binDataType);
      total = total + result * result; 
      ++count;
    }
    ++i;
  }
  if(count == 0) return 0;
  float  avg = total/count;
  float  sqroot = sqrt(avg);
  if(sqroot <= 0) return 0;
  return 20*log10(sqroot);
}

db StatisticalEngine::GetRSSDbOfFreqRange(float startFreq, float endFreq, int depth, enum BinDataType binDataType)
{
  int startBin = GetFFTBinIndexForFrequency(startFreq);
  int endBin = GetFFTBinIndexForFrequency(endFreq);
  int total = 0;
  int i = startBin;
  while(i <= endBin)
  {
    for(int j = 0; j < BIN_SAVE_LENGTH && j <= depth; ++j)
    {
      total = total + GetFFTBinData(i, j, binDataType)*GetFFTBinData(i, j, binDataType);
    }
    ++i;
  }
  db sqroot = sqrt(total);
  if(sqroot <= 0) return 0;
  return 20*log10(sqroot);
}

float  StatisticalEngine::GetRSSOfFreqRange(float startFreq, float endFreq, int depth, enum BinDataType binDataType)
{
  int startBin = GetFFTBinIndexForFrequency(startFreq);
  int endBin = GetFFTBinIndexForFrequency(endFreq);
  int total = 0;
  int i = startBin;
  while(i <= endBin)
  {
    for(int j = 0; j < BIN_SAVE_LENGTH && j <= depth; ++j)
    {
      total = total + GetFFTBinData(i, j, binDataType)*GetFFTBinData(i, j, binDataType);
    }
    ++i;
  }
  float  sqroot = sqrt(total);
  if(sqroot <= 0) return 0;
  return sqroot;
}

db StatisticalEngine::GetMinDbOfFreqRange(float startFreq, float endFreq, int depth, enum BinDataType binDataType)
{
  int startBin = GetFFTBinIndexForFrequency(startFreq);
  int endBin = GetFFTBinIndexForFrequency(endFreq);
  int minimum = INT_MAX;
  int i = startBin;
  while(i <= endBin)
  {
    for(int j = 0; j < BIN_SAVE_LENGTH && j <= depth; ++j)
    {
      int result = GetFFTBinData(i, j, binDataType);
      if(result < minimum) minimum = result;
    }
    ++i;
  }
  if(minimum <= 0) return 0;
  return 20*log10(minimum);
}

db StatisticalEngine::GetMaxDbOfFreqRange(float startFreq, float endFreq, int depth, enum BinDataType binDataType)
{
  int startBin = GetFFTBinIndexForFrequency(startFreq);
  int endBin = GetFFTBinIndexForFrequency(endFreq);
  int maximum = 0;
  int i = startBin;
  while(i <= endBin)
  {
    for(int j = 0; j < BIN_SAVE_LENGTH && j <= depth; ++j)
    {
      int result = GetFFTBinData(i, j, binDataType);
      if(result > maximum) maximum = result;
    }
    ++i;
  }
  if(maximum <= 0) return 0;
  return 20*log10(maximum);
}

db StatisticalEngine::GetAverageDbOfFreqRange(float startFreq, float endFreq, int depth, enum BinDataType binDataType)
{
  if(true == debugMode && debugLevel >= 5) Serial << "GetAverageDbOfFreqRange: " << "    Start Freq: " << startFreq << "    End Freq: " << endFreq;
  int startBin = GetFFTBinIndexForFrequency(startFreq);
  int endBin = GetFFTBinIndexForFrequency(endFreq);
  int totalCount = 0;
  int result = 0;
  int i = startBin;
  while(i <= endBin)
  {
    for(int j = 0; j < BIN_SAVE_LENGTH && j <= depth; ++j)
    {
      result += GetFFTBinData(i, j, binDataType);
      ++totalCount;
      if(true == debugMode && debugLevel >= 5) Serial << i << "|" << j << "|" << startBin << "|" << endBin << "\t|\t" << result << "|" << totalCount << "\n";
    }
    ++i;
  }
  if(0 == totalCount)
  {
    if(true == debugMode && debugLevel >= 4) Serial << "Result 0: " << result << "\n";
    return 0.0;
  }
  else
  {
    result = (float )result/(float )totalCount;
    if(result > 0)
    {
      result = 20*log10(result);
      if(true == debugMode && debugLevel >= 4) Serial << "Result: " << result << "\n";
      return (db)result;
    }
    else
    {
      return 0.0;
    }
  }
}

float  StatisticalEngine::GetAverageOfFreqRange(float startFreq, float endFreq, int depth, enum BinDataType binDataType)
{
  int startBin = GetFFTBinIndexForFrequency(startFreq);
  int endBin = GetFFTBinIndexForFrequency(endFreq);
  int total = 0;
  int i = startBin;
  int totalCount = 0;
  float  result = 0.0;
  while(i <= endBin)
  {
    for(int j = 0;  j < BIN_SAVE_LENGTH && j <= depth; ++j)
    {
      result += GetFFTBinData(i, j, binDataType);
      ++totalCount;
    }
    ++i;
  }
  if(0 == totalCount) return 0;
  if(result <= 0) return 0;
  return result/totalCount;
}

float StatisticalEngine::GetMinOfFreqRange(float startFreq, float endFreq, int depth, enum BinDataType binDataType)
{
  int startBin = GetFFTBinIndexForFrequency(startFreq);
  int endBin = GetFFTBinIndexForFrequency(endFreq);
  int minimum = INT_MAX;
  int i = startBin;
  while(i <= endBin)
  {
    for(int j = 0; j < BIN_SAVE_LENGTH && j <= depth; ++j)
    {
      int result = GetFFTBinData(i, j, binDataType);
      if(result < minimum) minimum = result;
    }
    ++i;
  }
  return minimum;
}

float StatisticalEngine::GetMaxOfFreqRange(float startFreq, float endFreq, int depth, enum BinDataType binDataType)
{
  int startBin = GetFFTBinIndexForFrequency(startFreq);
  int endBin = GetFFTBinIndexForFrequency(endFreq);
  int maximum = 0;
  int i = startBin;
  while(i <= endBin)
  {
    for(int j = 0; j < BIN_SAVE_LENGTH && j <= depth; ++j)
    {
      int result = GetFFTBinData(i, j, binDataType);
      if(result > maximum) maximum = result;
    }
    ++i;
  }
  return maximum;
}


MinMaxDb StatisticalEngine::GetFFTBinMinMaxAverageDbOfFreqRange(float startFreq, float endFreq, int depth, enum BinDataType binDataType)
{
  if(true == debugMode && debugLevel >= 5) Serial << "GetFFTBinMinMaxAverageDbOfFreqRange: " << "    Start Freq: " << startFreq << "    End Freq: " << endFreq;
  MinMaxDb result;
  int startBin = GetFFTBinIndexForFrequency(startFreq);
  int endBin = GetFFTBinIndexForFrequency(endFreq);
  int maxTotal = 0;
  int maxCount = 0;
  int minTotal = 0;
  int minCount = 0;
  int i = startBin;
  int average = GetAverageOfFreqRange(startFreq, endFreq, depth, binDataType);
  while(i <= endBin)
  {
    for(int j = 0;  j < BIN_SAVE_LENGTH && j <= depth; ++j)
    {
      int value = GetFFTBinData(i, j, binDataType);
      if(value == average)
      {
        maxTotal += value;
        ++maxCount;
        minTotal += value;
        ++minCount;
      }
      else if(value > average)
      {
        maxTotal += value;
        ++maxCount;
      }
      else
      {
        minTotal += value;
        ++minCount;
      }
      if(true == debugMode && debugLevel >= 5) Serial << i << "|" << j << "|" << startBin << "|" << endBin << "    |    " << value << "|" << average << "    |    " << minTotal << "|" << minCount << "    |    " << maxTotal << "|" << maxCount << "\n";
    }
    ++i;
  }
  if(minCount != 0 && minTotal > 0 && minTotal > minCount)
  {
    result.min = 20*log10(minTotal/minCount);
  }
  else
  {
    result.min = 0.0;
  }
  if(maxCount != 0 && maxTotal > 0 && maxTotal > maxCount)
  {
    result.max = 20*log10(maxTotal/maxCount);
  }
  else
  {
    result.max = 0.0;
  }
  if(true == debugNanInf && (result.min == result.max || isnan(result.min) || isnan(result.max) || isinf(result.min) || isinf(result.max))) Serial << "GetFFTBinMinMaxAverageDbOfFreqRange:  Min Data: " << minTotal << "|" << minCount << "|" << result.min << "\tMax: " << maxTotal << "|" << maxCount << "|" << result.max << "\n";
  return result;
}

MinMax StatisticalEngine::GetFFTBinMinMaxAverageOfFreqRange(float startFreq, float endFreq, int depth, enum BinDataType binDataType)
{
  int startBin = GetFFTBinIndexForFrequency(startFreq);
  int endBin = GetFFTBinIndexForFrequency(endFreq);
  MinMax result;
  int avg = GetAverageOfFreqRange(startFreq, endFreq, depth, binDataType);
  unsigned int maxTotal = 0;
  int maxCount = 0;
  unsigned int minTotal = 0;
  int minCount = 0;
  int i = startBin;
  while(i <= endBin)
  {
    for(int j = 0;  j<BIN_SAVE_LENGTH && j <= depth; ++j)
    {
      int binResult = GetFFTBinData(i, j, binDataType);
      if(binResult == avg)
      {
        maxTotal += binResult;
        ++maxCount;
        minTotal += binResult;
        ++minCount;
      }
      else if(binResult > avg)
      {
        maxTotal += binResult;
        ++maxCount;
      }
      else
      {
        minTotal += binResult;
        ++minCount;
      }
      if(true == debugMode && debugLevel >= 5) Serial << i << "|" << j << "|" << startBin << "|" << endBin << "\t|\t" << minTotal << "|" << minCount << "\t|\t" << maxTotal << "|" << maxCount << "\n";
    }
    ++i;
  }
  if(minCount != 0 && minTotal > 0)
  {
    result.min = (minTotal/minCount);
  }
  else
  {
    result.min = 0;
  }
  if(maxCount != 0 && maxTotal > 0)
  {
    result.max = (maxTotal/maxCount);
  }
  else
  {
    result.max = 0;
  }
  if(true == debugNanInf && (isnan(result.min) || isnan(result.max))) Serial << "GetFFTBinMinMaxAverageOfFreqRange:  Min Data: " << minTotal << "|" << minCount << "|" << result.min << "\tMax: " << maxTotal << "|" << maxCount << "|" << result.max << "\n";
  return result;
}
float  StatisticalEngine::GetVariance(unsigned int bin, int depth, enum BinDataType binDataType)
{
  float  mean = GetFFTBinAverage(bin, depth, binDataType);
  
  // Compute sum squared  
  // differences with mean. 
  float  sqDiff = 0;
  int count = 0;
  for (int j = 0; j < BIN_SAVE_LENGTH && j <= depth; ++j)
  {
    sqDiff += (GetFFTBinData(bin, j, binDataType) - mean) *  (GetFFTBinData(bin, j, binDataType) - mean);
    ++count;
  } 
  return sqDiff / count;
}

float  StatisticalEngine::GetStandardDeviation(unsigned int bin, int depth, enum BinDataType binDataType) 
{ 
    return sqrt(GetVariance(bin, depth, binDataType));
}

db StatisticalEngine::GetVarianceDbOfFreqRange(float startFreq, float endFreq, int depth, enum BinDataType binDataType)
{
  int startBin = GetFFTBinIndexForFrequency(startFreq);
  int endBin = GetFFTBinIndexForFrequency(endFreq);
  float  mean = GetAverageDbOfFreqRange(startFreq, endFreq, depth, binDataType);
  
  // Compute sum squared  
  // differences with mean. 
  float  sqDiff = 0;
  int count = 0;
  for (int i = 0; i < endBin; ++i)
  {
    for (int j = 0; j < BIN_SAVE_LENGTH && j <= depth; ++j)
    {
      sqDiff += (GetFFTBinData(i, j, binDataType) - mean) *  (GetFFTBinData(i, j, binDataType) - mean);
      ++count;
    } 
  }
  return 20*log10(sqDiff / count);
}

float  StatisticalEngine::GetStandardDeviationDbOfFreqRange(float startFreq, float endFreq, int depth, enum BinDataType binDataType)
{
  return sqrt(GetVarianceDbOfFreqRange(startFreq, endFreq, depth, binDataType));
}
