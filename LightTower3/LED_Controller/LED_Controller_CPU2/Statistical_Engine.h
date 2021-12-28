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

#define STATISTICAL_ENGINE_MEMORY_DEBUG true

#include <limits.h>
#include "TaskInterface.h"
#include "Streaming.h"
#include "Tunes.h"
#include "Helpers.h"

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
    void ConnectMicrophoneMeasureCallerInterfaceCallback(MicrophoneMeasureCalleeInterface *cb)
    {
        m_cb = cb;
    }
    MicrophoneMeasureCalleeInterface *m_cb;
};

class StatisticalEngine : public NamedItem
                        , public Task
                        , public MicrophoneMeasureCallerInterface
                        , public CommonUtils
{
  public:
    StatisticalEngine()
      : NamedItem("StatisticalEngine")
      , Task(GetTitle())
      , m_Power(0)
      , m_PowerDb(0){}
    virtual ~StatisticalEngine()
    {
      FreeMemory();
    }
    SoundState GetSoundState();

    void SetProcessFFTStatus(bool value) {m_ProcessFFT = value; }
    bool GetProcessFFTStatus() {return m_ProcessFFT; }
    void AllocateMemory();
    void FreeMemory();
  
    //Main Data Interface
    int GetFFTBinIndexForFrequency(float freq);
    float GetFreqForBin(unsigned int bin);
    int GetFFTData(int position);  
  
    //Right Channel Input Data Queues
    QueueHandle_t GetFFTRightBandDataInputQueue() { return m_FFT_Right_BandData_Input_Buffer_Queue; }
    size_t GetFFTRightBandDataBufferSize() { return m_BandInputByteCount; }
    QueueHandle_t GetRightChannelProcessedSoundBufferQueue() { return m_Right_Channel_Processed_Sound_Buffer_Queue; }
    size_t GetRightChannelProcessedSoundBufferSize() { return sizeof(m_Right_Channel_Processed_Sound_Data); }

  
    //Left Channel Input Data Queues
    QueueHandle_t GetFFTLeftBandDataInputQueue() { return m_FFT_Left_BandData_Input_Buffer_Queue; }
    size_t GetFFTLeftBandDataBufferSize() { return m_BandInputByteCount; }
    QueueHandle_t GetLeftChannelProcessedSoundBufferQueue() { return m_Left_Channel_Processed_Sound_Buffer_Queue; }
    size_t GetLeftChannelProcessedSoundBufferSize() { return sizeof(m_Left_Channel_Processed_Sound_Data); }

  
    //Power Getters
    float GetNormalizedSoundPower();
    
    //Band Data Getters
    unsigned int GetNumberOfBands() { return m_NumBands; }
    int GetBandValue(unsigned int band, unsigned int depth);
    float GetBandAverage(unsigned band, unsigned int depth);
    int GetBandAverageForABandOutOfNBands(unsigned band, unsigned int depth, unsigned int TotalBands);
  
  private:
    bool m_ProcessFFT = true;
    //BAND Circular Buffer
    bool m_NewBandDataReady = false;
    static const unsigned int m_NumBands = 32; //Need way to set this
    int BandValues[m_NumBands][BAND_SAVE_LENGTH];
    int currentBandIndex = -1;
    int BandRunningAverageValues[m_NumBands][BAND_SAVE_LENGTH];
    int currentAverageBandIndex = -1;
    bool NewBandDataReady();
    void UpdateBandArray();
    void UpdateRunningAverageBandArray();

    //Task Interface
    void Setup();
    void RunMyPreTask(){}
    bool CanRunMyScheduledTask();
    void RunMyScheduledTask();
    void RunMyPostTask(){}
    
    //Sound State Output
    bool m_NewSoundDataReady = false;
    float m_Power;
    float m_PowerDb;
    int m_signalMin;
    int m_signalMax;
    float m_AmpGain = 1.0;
    float m_FFTGain = 1.0;
    bool NewSoundDataReady();
    void UpdateSoundState();

 //Sound Detection
  private:
    bool m_MemoryIsAllocated = false;
    size_t m_BandInputByteCount = sizeof(int16_t) * m_NumBands;
    
    //Right Channel Input Sound Data
    int16_t* m_Right_Band_Values;
    QueueHandle_t m_FFT_Right_BandData_Input_Buffer_Queue = NULL;
    QueueHandle_t m_Right_Channel_Processed_Sound_Buffer_Queue = NULL;
    ProcessedSoundData_t m_Right_Channel_Processed_Sound_Data;

    //Left Channel Input Sound Data
    int16_t* m_Left_Band_Values;
    QueueHandle_t m_FFT_Left_BandData_Input_Buffer_Queue = NULL;
    QueueHandle_t m_Left_Channel_Processed_Sound_Buffer_Queue = NULL;
    ProcessedSoundData_t m_Left_Channel_Processed_Sound_Data;

    //Sound Detection
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
