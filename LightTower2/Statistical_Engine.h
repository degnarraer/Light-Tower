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

#ifndef StatisticalEngine_H
#define StatisticalEngine_H

#include <limits.h>
#include "TaskInterface.h"
#include "Streaming.h"
#include "Tunes.h"
#include "ADCSampler.h"
#include <assert.h>

enum SoundState
{
  LastingSilenceDetected,
  SilenceDetected,
  SoundDetected,
};

enum BandDataType
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

class StatisticalEngine : public Task
                        , public MicrophoneMeasureCallerInterface
{ 
  public:
    StatisticalEngine()
      : Task("StatisticalEngine")
      , m_Power(0)
      , m_PowerDb(0){}
  
    void HandleADCInterrupt();
    SoundState GetSoundState();

  //Main Data Interface
  int GetFFTBinIndexForFrequency(float freq);
  float GetFreqForBin(unsigned int bin);
  int GetFFTData(int position);  
  
  //Power Getters
  float GetSoundPower();
  
  //Band Data Getters
  int GetBandValue(unsigned int band, unsigned int depth);
  float GetBandAverage(unsigned band, unsigned int depth);
  float GetBandAverageForABandOutOfNBands(unsigned band, unsigned int depth, unsigned int TotalBands);
  
  private:
    //BAND Circular Buffer
    static const unsigned int m_NumBands = 8;
    int BandValues[m_NumBands][BAND_SAVE_LENGTH];
    int currentBandIndex = -1;
    int BandRunningAverageValues[m_NumBands][BAND_SAVE_LENGTH];
    int currentAverageBandIndex = -1;
    void UpdateBandArray();
    void UpdateRunningAverageBandArray();
  
    //Task Interface
    void          Setup();
    bool          CanRunMyTask();
    void          RunMyTask();

    //Mic Data Variable
    ADCSampler    m_Sampler;
    float m_Power;
    float m_PowerDb;
    int16_t m_data[FFT_MAX];
    int m_signalMin;
    int m_signalMax;
    float m_AmpGain = 1.0;
    float m_FFTGain = 1.0;
    
    void GetSampledSoundData();
    bool NewDataReady();
    void AnalyzeSound();
    void  UpdateSoundState();
    void  setup_AtoD();

 //Sound Detection
  private:
    const int     m_silenceDetectedThreshold = silenceDetectedThreshold;
    const int     m_soundDetectedThreshold = soundDetectedThreshold;
    const int     m_silenceIntegratorMax = silenceIntegratorMax;
    int           m_silenceIntegrator = 0;
    const int     m_silenceIntegratorMin = 0;  
    long          m_silenceStartTime;
    unsigned long m_startMicros;
    unsigned long m_previousMicros;
    unsigned long m_currentMicros;
    unsigned long m_finalMicros;
    const int     m_soundAdder = soundAdder;
    const int     m_silenceSubtractor = silenceSubtractor;
    SoundState    soundState = SoundState::SilenceDetected;
};

#endif
