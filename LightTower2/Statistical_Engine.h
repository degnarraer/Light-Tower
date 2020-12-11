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
    float power;
    float powerDb;
    float ampGain = 1.0;
    float fftGain = 1.0;
    ADCSampler    sampler;
    void          Setup();
    void          HandleInterrupt();
    void          UpdateSoundData();
    SoundState    GetSoundState();
    int           GetFFTBinIndexForFrequency(float freq);
    float         GetFreqForBin(unsigned int bin);
    int           GetFFTData(int position);

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

    //FFT BAND CIRCULAR BUFFER
  private:
    void UpdateBandArray();
    void UpdateRunningAverageBandArray();
    int BandValues[NUM_BANDS][BAND_SAVE_LENGTH];
    int BandRunningAverageValues[NUM_BANDS][BAND_SAVE_LENGTH];
    int currentBandIndex = -1;
    int currentAverageBandIndex = -1;

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


};

#endif
