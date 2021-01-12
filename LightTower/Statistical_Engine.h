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
 * @file Statistical_Engine.h
 *
 *
 */

#ifndef StatisticalEngine_H
#define StatisticalEngine_H

#include <limits.h>
#include "Streaming.h"
#include "Tunes.h"
#include "ADCSampler.h"

enum SoundState
{
  LastingSilenceDetected,
  SilenceDetected,
  SoundDetected,
};

enum BinDataType
{
  INSTANT,
  AVERAGE
};

enum DataChannelType
{
  MICROPHONE_STANDARD,
  MICROPHONE_AUTOGAIN,
  MICROPHONE_GAIN_VALUE,
  FFT_GAIN_VALUE
};

struct MinMax
{
  float min = 0;
  float max = 0;
};

typedef float db;
struct MinMaxDb
{
  db min = 0.0;
  db max = 0.0;
};

class MicrophoneMeasureCalleeInterface
{
public:
    virtual void MicrophoneStateChange(SoundState) = 0;
    virtual void TestSequenceComplete() = 0;
};

class MicrophoneMeasureCallerInterface
{
public:
    void ConnectCallback(MicrophoneMeasureCalleeInterface *cb)
    {
        m_cb = cb;
    }
    MicrophoneMeasureCalleeInterface *m_cb;
};

class StatisticalEngine : public MicrophoneMeasureCallerInterface
{ 
  public:
    StatisticalEngine()
    {
      power = 0;
    }
    ADCSampler    sampler;
    void          Setup();
    void          HandleInterrupt();
    void          UpdateSoundData();
    SoundState    GetSoundState();
    int           GetFFTBinIndexForFrequency(float freq);
    float         GetFreqForBin(unsigned int bin);
    int           GetFFTData(int position);
    
    //Testing
  public:
    void          EnableTestMode();
    void          DisableTestMode();
    bool          GetTestMode();
  private:
    void          TestFunctions();
    void          SetTestData(int16_t data[FFT_MAX]);
    void          PrintDataBuffer(char tag[10]);
    int           testCount = 0;
    int           holdCount = 1;
    const int     holdTime = 100;
    int           t_count;
    float         testAmpGain = 1.0;
    float         testFFTGain = 1.0;
    float         ampGain = 1.0;
    float         fftGain = 1.0;
    float         t = 1/SAMPLE_RATE;
    
    //STATISTICS
  public:
    float         GetVariance(unsigned int bin, int depth, enum BinDataType binDataType);
    float         GetStandardDeviation(unsigned int bin, int depth, enum BinDataType binDataType); 
    
    //FFT BIN FUNCTIONS
    //Bin data contains int values
  public:
    MinMax        GetFFTBinMinMaxAverage(unsigned int bin, int depth, enum BinDataType binDataType);
    int           GetMaxFFTBin(int maxBinToTest);
    int           GetMaxFFTBinFromRange(int minBinToTest, int maxBinToTest);
    int           GetMaxRSSBin(int maxBinToTest, int depth, enum BinDataType binDataType);
    int           GetFFTBinData(unsigned int bin, int depth, enum BinDataType binDataType);
    int           GetFFTBinAverage(unsigned int bin, int depth, enum BinDataType binDataType);
    int           GetFFTBinMax(unsigned int bin, int depth, enum BinDataType binDataType);
    int           GetFFTBinMin(unsigned int bin, int depth, enum BinDataType binDataType);
    int           GetFFTBinRSS(unsigned int bin, int depth, enum BinDataType binDataType);
    int           GetFFTBinRMS(unsigned int bin, int depth, enum BinDataType binDataType);
    float         GetAverageOfFreqRange(float startFreq, float endFreq, int depth, enum BinDataType binDataType);
    float         GetMinOfFreqRange(float startFreq, float endFreq, int depth, enum BinDataType binDataType);
    float         GetMaxOfFreqRange(float startFreq, float endFreq, int depth, enum BinDataType binDataType);

    //FREQUENCY RANGE FUNCTIONS
    //Frequency functions output decibals
  public:
    MinMaxDb      GetFFTBinMinMaxAverageDbOfFreqRange(float startFreq, float endFreq, int depth, enum BinDataType binDataType);
    MinMax        GetFFTBinMinMaxAverageOfFreqRange(float startFreq, float endFreq, int depth, enum BinDataType binDataType);
    db            GetFFTBinAverageDb(unsigned int bin, int depth, enum BinDataType binDataType);
    float         GetRMSOfFreqRange(float startFreq, float endFreq, int depth, enum BinDataType binDataType);
    db            GetRMSDbOfFreqRange(float startFreq, float endFreq, int depth, enum BinDataType binDataType);
    float         GetRSSOfFreqRange(float startFreq, float endFreq, int depth, enum BinDataType binDataType);
    db            GetRSSDbOfFreqRange(float startFreq, float endFreq, int depth, enum BinDataType binDataType);
    db            GetMinDbOfFreqRange(float startFreq, float endFreq, int depth, enum BinDataType binDataType);
    db            GetMaxDbOfFreqRange(float startFreq, float endFreq, int depth, enum BinDataType binDataType);
    db            GetAverageDbOfFreqRange(float startFreq, float endFreq, int depth, enum BinDataType binDataType);
    db            GetVarianceDbOfFreqRange(float startFreq, float endFreq, int depth, enum BinDataType binDataType);
    db            GetStandardDeviationDbOfFreqRange(float startFreq, float endFreq, int depth, enum BinDataType binDataType);
    float         power;
    float         powerDb;

 //Sound Detection
  public:
    const int     m_silenceDetectedThreshold = silenceDetectedThreshold;
    const int     m_soundDetectedThreshold = soundDetectedThreshold;
    const int     m_silenceIntegratorMax = silenceIntegratorMax;
    int           m_silenceIntegrator = 0;
    const int     m_silenceIntegratorMin = 0;
  private:
    long          m_silenceStartTime;
    unsigned long m_startMicros;
    unsigned long m_previousMicros;
    unsigned long m_currentMicros;
    unsigned long m_finalMicros;
    const int     m_soundAdder = soundAdder;
    const int     m_silenceSubtractor = silenceSubtractor;
    SoundState    soundState = SoundState::SilenceDetected;

  //Helpers
  private:
    bool          NewDataReady();
    void          PlotData();
    void          FillDataBufferHelper(int testCase, float  a1, float  f1, float  a2, float  f2, float  a3, float  f3, float  a4, float  f4, float  a5, float  f5, float  a6, float  f6);
    void          AnalyzeSound();
    void          UpdateSoundState();
    void          setup_AtoD();
    float         m_calculatedSampleRate;
    int16_t       m_data[FFT_MAX];
    int           m_signalMin;
    int           m_signalMax;

    //FFT BIN CIRCULAR BUFFER
  private:
    void UpdateBinArray();
    void UpdateRunningAverageBinArray();
    int BinValues[BINS][BIN_SAVE_LENGTH];
    int BinRunningAverageValues[BINS][BIN_SAVE_LENGTH];
    int currentBinIndex = -1;
    int currentAverageBinIndex = -1;
    bool runningAverageBinArrayInitialized = false;
    bool binArrayInitialized = false;    
};

struct TestData
{
  float   f1;
  float   a1;
  float   f2;
  float   a2;
  float   f3;
  float   a3;
  float   f4;
  float   a4;
  float   f5;
  float   a5;
  float   f6;
  float   a6;
};

const TestData testData[197] PROGMEM = 
{ { 10000.0/6,   0.0,  2*10000.0/6,  0.0,   3*10000.0/6,   0.0,  4*10000.0/6,   0.0,  5*10000.0/6,   0.0,  6*10000.0/6,   0.0 },
  { 10000.0/6,   0.1,  2*10000.0/6,  0.1,   3*10000.0/6,   0.1,  4*10000.0/6,   0.1,  5*10000.0/6,   0.1,  6*10000.0/6,   0.1 },
  { 10000.0/6,   0.2,  2*10000.0/6,  0.2,   3*10000.0/6,   0.2,  4*10000.0/6,   0.2,  5*10000.0/6,   0.2,  6*10000.0/6,   0.2 },
  { 10000.0/6,   0.3,  2*10000.0/6,  0.3,   3*10000.0/6,   0.3,  4*10000.0/6,   0.3,  5*10000.0/6,   0.3,  6*10000.0/6,   0.3 },
  { 10000.0/6,   0.4,  2*10000.0/6,  0.4,   3*10000.0/6,   0.4,  4*10000.0/6,   0.4,  5*10000.0/6,   0.4,  6*10000.0/6,   0.4 },
  { 10000.0/6,   0.5,  2*10000.0/6,  0.5,   3*10000.0/6,   0.5,  4*10000.0/6,   0.5,  5*10000.0/6,   0.5,  6*10000.0/6,   0.5 },
  { 10000.0/6,   0.6,  2*10000.0/6,  0.6,   3*10000.0/6,   0.6,  4*10000.0/6,   0.6,  5*10000.0/6,   0.6,  6*10000.0/6,   0.6 },
  { 10000.0/6,   0.7,  2*10000.0/6,  0.7,   3*10000.0/6,   0.7,  4*10000.0/6,   0.7,  5*10000.0/6,   0.7,  6*10000.0/6,   0.7 },
  { 10000.0/6,   0.8,  2*10000.0/6,  0.8,   3*10000.0/6,   0.8,  4*10000.0/6,   0.8,  5*10000.0/6,   0.8,  6*10000.0/6,   0.8 },
  { 10000.0/6,   0.9,  2*10000.0/6,  0.9,   3*10000.0/6,   0.9,  4*10000.0/6,   0.9,  5*10000.0/6,   0.9,  6*10000.0/6,   0.9 },
  { 10000.0/6,   1.0,  2*10000.0/6,  1.0,   3*10000.0/6,   1.0,  4*10000.0/6,   1.0,  5*10000.0/6,   1.0,  6*10000.0/6,   1.0 }, //11

  { 10000.0/6,   1.0,  2*10000.0/6,  1.0,   3*10000.0/6,   1.0,  4*10000.0/6,   1.0,  5*10000.0/6,   1.0,  6*10000.0/6,   1.0 },
  { 10000.0/6,   0.9,  2*10000.0/6,  0.9,   3*10000.0/6,   0.9,  4*10000.0/6,   0.9,  5*10000.0/6,   0.9,  6*10000.0/6,   0.9 },
  { 10000.0/6,   0.8,  2*10000.0/6,  0.8,   3*10000.0/6,   0.8,  4*10000.0/6,   0.8,  5*10000.0/6,   0.8,  6*10000.0/6,   0.8 },
  { 10000.0/6,   0.7,  2*10000.0/6,  0.7,   3*10000.0/6,   0.7,  4*10000.0/6,   0.7,  5*10000.0/6,   0.7,  6*10000.0/6,   0.7 },
  { 10000.0/6,   0.6,  2*10000.0/6,  0.6,   3*10000.0/6,   0.6,  4*10000.0/6,   0.6,  5*10000.0/6,   0.6,  6*10000.0/6,   0.6 },
  { 10000.0/6,   0.5,  2*10000.0/6,  0.5,   3*10000.0/6,   0.5,  4*10000.0/6,   0.5,  5*10000.0/6,   0.5,  6*10000.0/6,   0.5 },
  { 10000.0/6,   0.4,  2*10000.0/6,  0.4,   3*10000.0/6,   0.4,  4*10000.0/6,   0.4,  5*10000.0/6,   0.4,  6*10000.0/6,   0.4 },
  { 10000.0/6,   0.3,  2*10000.0/6,  0.3,   3*10000.0/6,   0.3,  4*10000.0/6,   0.3,  5*10000.0/6,   0.3,  6*10000.0/6,   0.3 },
  { 10000.0/6,   0.2,  2*10000.0/6,  0.2,   3*10000.0/6,   0.2,  4*10000.0/6,   0.2,  5*10000.0/6,   0.2,  6*10000.0/6,   0.2 },
  { 10000.0/6,   0.1,  2*10000.0/6,  0.1,   3*10000.0/6,   0.1,  4*10000.0/6,   0.1,  5*10000.0/6,   0.1,  6*10000.0/6,   0.1 },  
  { 10000.0/6,   0.0,  2*10000.0/6,  0.0,   3*10000.0/6,   0.0,  4*10000.0/6,   0.0,  5*10000.0/6,   0.0,  6*10000.0/6,   0.0 }, //22

  { 10000.0/6,   0.0,  2*10000.0/6,  0.0,   3*10000.0/6,   0.0,  4*10000.0/6,   0.0,  5*10000.0/6,   0.0,  6*10000.0/6,   0.0 },  
  { 10000.0/6,   0.0,  2*10000.0/6,  0.0,   3*10000.0/6,   0.0,  4*10000.0/6,   0.0,  5*10000.0/6,   0.0,  6*10000.0/6,   0.0 },
  
  { 10000.0/6,   0.0,  2*10000.0/6,  0.0,   3*10000.0/6,   0.0,  4*10000.0/6,   0.0,  5*10000.0/6,   0.0,  6*10000.0/6,   0.0 },
  { 10000.0/6,   0.2,  2*10000.0/6,  0.0,   3*10000.0/6,   0.0,  4*10000.0/6,   0.0,  5*10000.0/6,   0.0,  6*10000.0/6,   0.0 },
  { 10000.0/6,   0.4,  2*10000.0/6,  0.0,   3*10000.0/6,   0.0,  4*10000.0/6,   0.0,  5*10000.0/6,   0.0,  6*10000.0/6,   0.0 },
  { 10000.0/6,   0.6,  2*10000.0/6,  0.0,   3*10000.0/6,   0.0,  4*10000.0/6,   0.0,  5*10000.0/6,   0.0,  6*10000.0/6,   0.0 },
  { 10000.0/6,   0.8,  2*10000.0/6,  0.0,   3*10000.0/6,   0.0,  4*10000.0/6,   0.0,  5*10000.0/6,   0.0,  6*10000.0/6,   0.0 },
  { 10000.0/6,   1.0,  2*10000.0/6,  0.0,   3*10000.0/6,   0.0,  4*10000.0/6,   0.0,  5*10000.0/6,   0.0,  6*10000.0/6,   0.0 }, //30
  
  { 10000.0/6,   1.0,  2*10000.0/6,  0.0,   3*10000.0/6,   0.0,  4*10000.0/6,   0.0,  5*10000.0/6,   0.0,  6*10000.0/6,   0.0 },
  { 10000.0/6,   0.8,  2*10000.0/6,  0.0,   3*10000.0/6,   0.0,  4*10000.0/6,   0.0,  5*10000.0/6,   0.0,  6*10000.0/6,   0.0 },
  { 10000.0/6,   0.6,  2*10000.0/6,  0.0,   3*10000.0/6,   0.0,  4*10000.0/6,   0.0,  5*10000.0/6,   0.0,  6*10000.0/6,   0.0 },
  { 10000.0/6,   0.4,  2*10000.0/6,  0.0,   3*10000.0/6,   0.0,  4*10000.0/6,   0.0,  5*10000.0/6,   0.0,  6*10000.0/6,   0.0 },
  { 10000.0/6,   0.2,  2*10000.0/6,  0.0,   3*10000.0/6,   0.0,  4*10000.0/6,   0.0,  5*10000.0/6,   0.0,  6*10000.0/6,   0.0 },
  { 10000.0/6,   0.0,  2*10000.0/6,  0.0,   3*10000.0/6,   0.0,  4*10000.0/6,   0.0,  5*10000.0/6,   0.0,  6*10000.0/6,   0.0 }, //36
  
  { 10000.0/6,   0.0,  2*10000.0/6,  0.0,   3*10000.0/6,   0.0,  4*10000.0/6,   0.0,  5*10000.0/6,   0.0,  6*10000.0/6,   0.0 },
  { 10000.0/6,   0.0,  2*10000.0/6,  0.2,   3*10000.0/6,   0.0,  4*10000.0/6,   0.0,  5*10000.0/6,   0.0,  6*10000.0/6,   0.0 },
  { 10000.0/6,   0.0,  2*10000.0/6,  0.4,   3*10000.0/6,   0.0,  4*10000.0/6,   0.0,  5*10000.0/6,   0.0,  6*10000.0/6,   0.0 },
  { 10000.0/6,   0.0,  2*10000.0/6,  0.6,   3*10000.0/6,   0.0,  4*10000.0/6,   0.0,  5*10000.0/6,   0.0,  6*10000.0/6,   0.0 },
  { 10000.0/6,   0.0,  2*10000.0/6,  0.8,   3*10000.0/6,   0.0,  4*10000.0/6,   0.0,  5*10000.0/6,   0.0,  6*10000.0/6,   0.0 },
  { 10000.0/6,   0.0,  2*10000.0/6,  1.0,   3*10000.0/6,   0.0,  4*10000.0/6,   0.0,  5*10000.0/6,   0.0,  6*10000.0/6,   0.0 }, //42
  
  { 10000.0/6,   0.0,  2*10000.0/6,  1.0,   3*10000.0/6,   0.0,  4*10000.0/6,   0.0,  5*10000.0/6,   0.0,  6*10000.0/6,   0.0 },
  { 10000.0/6,   0.0,  2*10000.0/6,  0.8,   3*10000.0/6,   0.0,  4*10000.0/6,   0.0,  5*10000.0/6,   0.0,  6*10000.0/6,   0.0 },
  { 10000.0/6,   0.0,  2*10000.0/6,  0.6,   3*10000.0/6,   0.0,  4*10000.0/6,   0.0,  5*10000.0/6,   0.0,  6*10000.0/6,   0.0 },
  { 10000.0/6,   0.0,  2*10000.0/6,  0.4,   3*10000.0/6,   0.0,  4*10000.0/6,   0.0,  5*10000.0/6,   0.0,  6*10000.0/6,   0.0 },
  { 10000.0/6,   0.0,  2*10000.0/6,  0.2,   3*10000.0/6,   0.0,  4*10000.0/6,   0.0,  5*10000.0/6,   0.0,  6*10000.0/6,   0.0 },
  { 10000.0/6,   0.0,  2*10000.0/6,  0.0,   3*10000.0/6,   0.0,  4*10000.0/6,   0.0,  5*10000.0/6,   0.0,  6*10000.0/6,   0.0 }, //48
  
  { 10000.0/6,   0.0,  2*10000.0/6,  0.0,   3*10000.0/6,   0.0,  4*10000.0/6,   0.0,  5*10000.0/6,   0.0,  6*10000.0/6,   0.0 },
  { 10000.0/6,   0.0,  2*10000.0/6,  0.0,   3*10000.0/6,   0.2,  4*10000.0/6,   0.0,  5*10000.0/6,   0.0,  6*10000.0/6,   0.0 },
  { 10000.0/6,   0.0,  2*10000.0/6,  0.0,   3*10000.0/6,   0.4,  4*10000.0/6,   0.0,  5*10000.0/6,   0.0,  6*10000.0/6,   0.0 },
  { 10000.0/6,   0.0,  2*10000.0/6,  0.0,   3*10000.0/6,   0.6,  4*10000.0/6,   0.0,  5*10000.0/6,   0.0,  6*10000.0/6,   0.0 },
  { 10000.0/6,   0.0,  2*10000.0/6,  0.0,   3*10000.0/6,   0.8,  4*10000.0/6,   0.0,  5*10000.0/6,   0.0,  6*10000.0/6,   0.0 },
  { 10000.0/6,   0.0,  2*10000.0/6,  0.0,   3*10000.0/6,   1.0,  4*10000.0/6,   0.0,  5*10000.0/6,   0.0,  6*10000.0/6,   0.0 }, //54
  
  { 10000.0/6,   0.0,  2*10000.0/6,  0.0,   3*10000.0/6,   1.0,  4*10000.0/6,   0.0,  5*10000.0/6,   0.0,  6*10000.0/6,   0.0 },
  { 10000.0/6,   0.0,  2*10000.0/6,  0.0,   3*10000.0/6,   0.8,  4*10000.0/6,   0.0,  5*10000.0/6,   0.0,  6*10000.0/6,   0.0 },
  { 10000.0/6,   0.0,  2*10000.0/6,  0.0,   3*10000.0/6,   0.6,  4*10000.0/6,   0.0,  5*10000.0/6,   0.0,  6*10000.0/6,   0.0 },
  { 10000.0/6,   0.0,  2*10000.0/6,  0.0,   3*10000.0/6,   0.4,  4*10000.0/6,   0.0,  5*10000.0/6,   0.0,  6*10000.0/6,   0.0 },
  { 10000.0/6,   0.0,  2*10000.0/6,  0.0,   3*10000.0/6,   0.2,  4*10000.0/6,   0.0,  5*10000.0/6,   0.0,  6*10000.0/6,   0.0 },
  { 10000.0/6,   0.0,  2*10000.0/6,  0.0,   3*10000.0/6,   0.0,  4*10000.0/6,   0.0,  5*10000.0/6,   0.0,  6*10000.0/6,   0.0 }, //60
  
  { 10000.0/6,   0.0,  2*10000.0/6,  0.0,   3*10000.0/6,   0.0,  4*10000.0/6,   0.0,  5*10000.0/6,   0.0,  6*10000.0/6,   0.0 },
  { 10000.0/6,   0.0,  2*10000.0/6,  0.0,   3*10000.0/6,   0.0,  4*10000.0/6,   0.2,  5*10000.0/6,   0.0,  6*10000.0/6,   0.0 },
  { 10000.0/6,   0.0,  2*10000.0/6,  0.0,   3*10000.0/6,   0.0,  4*10000.0/6,   0.4,  5*10000.0/6,   0.0,  6*10000.0/6,   0.0 },
  { 10000.0/6,   0.0,  2*10000.0/6,  0.0,   3*10000.0/6,   0.0,  4*10000.0/6,   0.6,  5*10000.0/6,   0.0,  6*10000.0/6,   0.0 },
  { 10000.0/6,   0.0,  2*10000.0/6,  0.0,   3*10000.0/6,   0.0,  4*10000.0/6,   0.8,  5*10000.0/6,   0.0,  6*10000.0/6,   0.0 },
  { 10000.0/6,   0.0,  2*10000.0/6,  0.0,   3*10000.0/6,   0.0,  4*10000.0/6,   1.0,  5*10000.0/6,   0.0,  6*10000.0/6,   0.0 }, //66
  
  { 10000.0/6,   0.0,  2*10000.0/6,  0.0,   3*10000.0/6,   0.0,  4*10000.0/6,   1.0,  5*10000.0/6,   0.0,  6*10000.0/6,   0.0 },
  { 10000.0/6,   0.0,  2*10000.0/6,  0.0,   3*10000.0/6,   0.0,  4*10000.0/6,   0.8,  5*10000.0/6,   0.0,  6*10000.0/6,   0.0 },
  { 10000.0/6,   0.0,  2*10000.0/6,  0.0,   3*10000.0/6,   0.0,  4*10000.0/6,   0.6,  5*10000.0/6,   0.0,  6*10000.0/6,   0.0 },
  { 10000.0/6,   0.0,  2*10000.0/6,  0.0,   3*10000.0/6,   0.0,  4*10000.0/6,   0.4,  5*10000.0/6,   0.0,  6*10000.0/6,   0.0 },
  { 10000.0/6,   0.0,  2*10000.0/6,  0.0,   3*10000.0/6,   0.0,  4*10000.0/6,   0.2,  5*10000.0/6,   0.0,  6*10000.0/6,   0.0 },
  { 10000.0/6,   0.0,  2*10000.0/6,  0.0,   3*10000.0/6,   0.0,  4*10000.0/6,   0.0,  5*10000.0/6,   0.0,  6*10000.0/6,   0.0 }, //72
  
  { 10000.0/6,   0.0,  2*10000.0/6,  0.0,   3*10000.0/6,   0.0,  4*10000.0/6,   0.0,  5*10000.0/6,   0.0,  6*10000.0/6,   0.0 },
  { 10000.0/6,   0.0,  2*10000.0/6,  0.0,   3*10000.0/6,   0.0,  4*10000.0/6,   0.0,  5*10000.0/6,   0.2,  6*10000.0/6,   0.0 },
  { 10000.0/6,   0.0,  2*10000.0/6,  0.0,   3*10000.0/6,   0.0,  4*10000.0/6,   0.0,  5*10000.0/6,   0.4,  6*10000.0/6,   0.0 },
  { 10000.0/6,   0.0,  2*10000.0/6,  0.0,   3*10000.0/6,   0.0,  4*10000.0/6,   0.0,  5*10000.0/6,   0.6,  6*10000.0/6,   0.0 },
  { 10000.0/6,   0.0,  2*10000.0/6,  0.0,   3*10000.0/6,   0.0,  4*10000.0/6,   0.0,  5*10000.0/6,   0.8,  6*10000.0/6,   0.0 },
  { 10000.0/6,   0.0,  2*10000.0/6,  0.0,   3*10000.0/6,   0.0,  4*10000.0/6,   0.0,  5*10000.0/6,   1.0,  6*10000.0/6,   0.0 }, //78
  
  { 10000.0/6,   0.0,  2*10000.0/6,  0.0,   3*10000.0/6,   0.0,  4*10000.0/6,   0.0,  5*10000.0/6,   1.0,  6*10000.0/6,   0.0 },
  { 10000.0/6,   0.0,  2*10000.0/6,  0.0,   3*10000.0/6,   0.0,  4*10000.0/6,   0.0,  5*10000.0/6,   0.8,  6*10000.0/6,   0.0 },
  { 10000.0/6,   0.0,  2*10000.0/6,  0.0,   3*10000.0/6,   0.0,  4*10000.0/6,   0.0,  5*10000.0/6,   0.6,  6*10000.0/6,   0.0 },
  { 10000.0/6,   0.0,  2*10000.0/6,  0.0,   3*10000.0/6,   0.0,  4*10000.0/6,   0.0,  5*10000.0/6,   0.4,  6*10000.0/6,   0.0 },
  { 10000.0/6,   0.0,  2*10000.0/6,  0.0,   3*10000.0/6,   0.0,  4*10000.0/6,   0.0,  5*10000.0/6,   0.2,  6*10000.0/6,   0.0 },
  { 10000.0/6,   0.0,  2*10000.0/6,  0.0,   3*10000.0/6,   0.0,  4*10000.0/6,   0.0,  5*10000.0/6,   0.0,  6*10000.0/6,   0.0 }, //84
  
  { 10000.0/6,   0.0,  2*10000.0/6,  0.0,   3*10000.0/6,   0.0,  4*10000.0/6,   0.0,  5*10000.0/6,   0.0,  6*10000.0/6,   0.0 },
  { 10000.0/6,   0.0,  2*10000.0/6,  0.0,   3*10000.0/6,   0.0,  4*10000.0/6,   0.0,  5*10000.0/6,   0.0,  6*10000.0/6,   0.2 },
  { 10000.0/6,   0.0,  2*10000.0/6,  0.0,   3*10000.0/6,   0.0,  4*10000.0/6,   0.0,  5*10000.0/6,   0.0,  6*10000.0/6,   0.4 },
  { 10000.0/6,   0.0,  2*10000.0/6,  0.0,   3*10000.0/6,   0.0,  4*10000.0/6,   0.0,  5*10000.0/6,   0.0,  6*10000.0/6,   0.6 },
  { 10000.0/6,   0.0,  2*10000.0/6,  0.0,   3*10000.0/6,   0.0,  4*10000.0/6,   0.0,  5*10000.0/6,   0.0,  6*10000.0/6,   0.8 },
  { 10000.0/6,   0.0,  2*10000.0/6,  0.0,   3*10000.0/6,   0.0,  4*10000.0/6,   0.0,  5*10000.0/6,   0.0,  6*10000.0/6,   1.0 }, //90
  
  { 10000.0/6,   0.0,  2*10000.0/6,  0.0,   3*10000.0/6,   0.0,  4*10000.0/6,   0.0,  5*10000.0/6,   0.0,  6*10000.0/6,   1.0 },
  { 10000.0/6,   0.0,  2*10000.0/6,  0.0,   3*10000.0/6,   0.0,  4*10000.0/6,   0.0,  5*10000.0/6,   0.0,  6*10000.0/6,   0.8 },
  { 10000.0/6,   0.0,  2*10000.0/6,  0.0,   3*10000.0/6,   0.0,  4*10000.0/6,   0.0,  5*10000.0/6,   0.0,  6*10000.0/6,   0.6 },
  { 10000.0/6,   0.0,  2*10000.0/6,  0.0,   3*10000.0/6,   0.0,  4*10000.0/6,   0.0,  5*10000.0/6,   0.0,  6*10000.0/6,   0.4 },
  { 10000.0/6,   0.0,  2*10000.0/6,  0.0,   3*10000.0/6,   0.0,  4*10000.0/6,   0.0,  5*10000.0/6,   0.0,  6*10000.0/6,   0.2 },
  { 10000.0/6,   0.0,  2*10000.0/6,  0.0,   3*10000.0/6,   0.0,  4*10000.0/6,   0.0,  5*10000.0/6,   0.0,  6*10000.0/6,   0.0 },  //96

  
  { 0.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 },
  { 100.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 },
  { 200.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 },
  { 300.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 },
  { 400.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 },
  { 500.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 },
  { 600.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 },
  { 700.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 },
  { 800.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 },
  { 900.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 }, //106
  { 1000.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 },
  { 1100.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 },
  { 1200.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 },
  { 1300.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 },
  { 1400.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 },
  { 1500.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 },
  { 1600.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 },
  { 1700.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 },
  { 1800.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 },
  { 1900.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 }, //116
  { 2000.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 },
  { 2100.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 },
  { 2200.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 },
  { 2300.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 },
  { 2400.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 },
  { 2500.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 },
  { 2600.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 },
  { 2700.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 },
  { 2800.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 },
  { 2900.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 }, //126
  { 3000.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 },
  { 3100.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 },
  { 3200.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 },
  { 3300.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 },
  { 3400.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 },
  { 3500.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 },
  { 3600.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 },
  { 3700.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 },
  { 3800.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 },
  { 3900.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 }, //136
  { 4000.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 },
  { 4100.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 },
  { 4200.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 },
  { 4300.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 },
  { 4400.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 },
  { 4500.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 },
  { 4600.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 },
  { 4700.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 },
  { 4800.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 },
  { 4900.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 }, //146
  { 5000.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 },
  { 5100.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 },
  { 5200.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 },
  { 5300.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 },
  { 5400.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 },
  { 5500.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 },
  { 5600.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 },
  { 5700.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 },
  { 5800.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 },
  { 5900.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 }, //156
  { 6000.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 },
  
  { 6100.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 },
  { 6200.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 },
  { 6300.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 },
  { 6400.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 },
  { 6500.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 },
  { 6600.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 },
  { 6700.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 },
  { 6800.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 },
  { 6900.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 }, //166
  { 7000.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 },
  { 7100.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 },
  { 7200.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 },
  { 7300.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 },
  { 7400.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 },
  { 7500.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 },
  { 7600.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 },
  { 7700.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 },
  { 7800.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 },
  { 7900.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 }, //176
  { 8000.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 },
  { 8100.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 },
  { 8200.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 },
  { 8300.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 },
  { 8400.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 },
  { 8500.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 },
  { 8600.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 },
  { 8700.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 },
  { 8800.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 },
  { 8900.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 }, //186
  { 9000.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 }, 
  { 9100.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 },
  { 9200.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 },
  { 9300.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 },
  { 9400.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 },
  { 9500.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 },
  { 9600.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 },
  { 9700.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 },
  { 9800.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 },
  { 9900.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 }, //196
  { 10000.0,   1.0,  0.0,  0.0,   0.0,   0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0 } }; 



#endif
