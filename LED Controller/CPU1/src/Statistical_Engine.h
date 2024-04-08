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


#pragma once
#define STATISTICAL_ENGINE_MEMORY_DEBUG false
#define STATISTICAL_ENGINE_DATA_DEBUG false

#include <limits.h>
#include "TaskInterface.h"
#include "Streaming.h"
#include "Tunes.h"
#include "Helpers.h"

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

class SoundMeasureCalleeInterface
{
public:
    virtual void SoundStateChange(SoundState_t State) = 0;
};

class SoundMeasureCallerInterface
{
  public:
    void RegisterForSoundStateChangeNotification(SoundMeasureCalleeInterface *user)
    {
      m_MyUsers.push_back(user);
    }
    void DeRegisterForSoundStateChangeNotification(SoundMeasureCalleeInterface *user)
    {
      for (int i = 0; i < m_MyUsers.size(); ++i)
      {
        if (m_MyUsers[i] == user)
        {
          m_MyUsers.erase(m_MyUsers.begin() + i);
          break;
        }
      }
    }
  protected:
    void SendNewValueNotificationToUsers(SoundState_t State)
    {
      for (int i = 0; i < m_MyUsers.size(); ++i)
      {
        m_MyUsers[i]->SoundStateChange(State);
      }
    }
  private:
    std::vector<SoundMeasureCalleeInterface*> m_MyUsers = std::vector<SoundMeasureCalleeInterface*>();
};

class StatisticalEngine : public NamedItem
                        , public Task
                        , public SoundMeasureCallerInterface
                        , public QueueManager
{
  public:
    StatisticalEngine()
      : NamedItem("StatisticalEngine")
      , Task(GetTitle())
      , QueueManager(GetTitle() + "_QueueManager", m_StatisticalEngineConfigCount)
      , m_Power(0)
      , m_PowerDb(0)
      {
        pthread_mutexattr_t Attr;
        pthread_mutexattr_init(&Attr);
        pthread_mutexattr_settype(&Attr, PTHREAD_MUTEX_RECURSIVE);
        if(0 != pthread_mutex_init(&m_BandValuesLock, &Attr)){ ESP_LOGE("Statistical Engine", "Failed to Create Lock");}      
        if(0 != pthread_mutex_init(&m_ProcessedSoundDataLock, &Attr)){ ESP_LOGE("Statistical Engine", "Failed to Create Lock");}
        if(0 != pthread_mutex_init(&m_MaxBinSoundDataLock, &Attr)){ ESP_LOGE("Statistical Engine", "Failed to Create Lock");}
      }
    virtual ~StatisticalEngine()
    {
      FreeMemory();
    }
    
    //SoundState
    SoundState_t GetSoundState();
    
    //FFT Processing Status
    void SetProcessFFTStatus(bool value) {m_ProcessFFT = value; }
    bool GetProcessFFTStatus() {return m_ProcessFFT; }
  
    //Main Data Interface
    int GetFFTBinIndexForFrequency(float freq);
    float GetFreqForBin(unsigned int bin);
    int GetFFTData(int position);  

    //Power Getters
    float GetNormalizedSoundPower();

    //SoundDataGetters
    public:
      MaxBandSoundData_t GetMaxBandSoundData() 
      {
        if(m_Right_MaxBandSoundData.MaxBandNormalizedPower > m_Left_MaxBandSoundData.MaxBandNormalizedPower)
        {
          return m_Right_MaxBandSoundData;
        }
        else
        {
          return m_Left_MaxBandSoundData;
        }
      }
      MaxBandSoundData_t GetMaxBinRightSoundData() { return m_Right_MaxBandSoundData; }
      MaxBandSoundData_t GetMaxBinLeftSoundData() { return m_Left_MaxBandSoundData; }
    
      //Band Data Getters
      unsigned int GetNumberOfBands() { return m_NumBands; }
      float GetBandValue(unsigned int band, unsigned int depth);
      float GetBandAverage(unsigned band, unsigned int depth);
      float GetBandAverageForABandOutOfNBands(unsigned band, unsigned int depth, unsigned int TotalBands);
  
  private:
    void AllocateMemory();
    void FreeMemory();
    bool m_MemoryIsAllocated = false;

    //QueueManager
    static const size_t m_StatisticalEngineConfigCount = 7;
    DataItemConfig_t m_ItemConfig[m_StatisticalEngineConfigCount]
    {
      { "R_BANDS",          DataType_Float_t,                 32, Transciever_RX,   4 },
      { "L_BANDS",          DataType_Float_t,                 32, Transciever_RX,   4 },
      { "Processed_Frame",  DataType_ProcessedSoundFrame_t,   1,  Transciever_RX,   4 },
      { "R_MAXBAND",        DataType_MaxBandSoundData_t,      1,  Transciever_RX,   4 },
      { "L_MAXBAND",        DataType_MaxBandSoundData_t,      1,  Transciever_RX,   4 },
      { "R_MAJOR_FREQ",     DataType_Float_t,                 1,  Transciever_RX,   4 },
      { "L_MAJOR_FREQ",     DataType_Float_t,                 1,  Transciever_RX,   4 },
    };
    DataItemConfig_t* GetDataItemConfig() { return m_ItemConfig; }
    size_t GetDataItemConfigCount() { return m_StatisticalEngineConfigCount; }
    bool m_ProcessFFT = true;
    
    //BAND Circular Buffer
    pthread_mutex_t m_BandValuesLock;
    static const unsigned int m_NumBands = 32; //Need way to set this
    float BandValues[m_NumBands][BAND_SAVE_LENGTH];
    int currentBandIndex = -1;
    float BandRunningAverageValues[m_NumBands][BAND_SAVE_LENGTH];
    int currentAverageBandIndex = -1;
    bool m_NewBandDataReady = false;
    unsigned long m_NewBandDataCurrentTime = 0;
    unsigned long m_NewBandDataTimeOut = 1000;
    bool m_NewBandDataTimedOut = false;
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
    unsigned long m_NewSoundDataCurrentTime = 0;
    unsigned long m_NewSoundDataTimeOut = 1000;
    bool m_NewSoundDataTimedOut = false;
    float m_AmpGain = 1.0;
    float m_FFTGain = 10.0;
    bool NewSoundDataReady();
    static void StaticUpdateSoundState(void * Parameters);
    void UpdateSoundState();

    size_t m_BandInputByteCount = sizeof(float) * m_NumBands;
    
    //Right Channel Input Sound Data
    pthread_mutex_t m_ProcessedSoundDataLock;
    float m_Right_Band_Values[m_NumBands];
    ProcessedSoundData_t m_Right_Channel_Processed_Sound_Data;

    //Left Channel Input Sound Data
    float m_Left_Band_Values[m_NumBands];
    ProcessedSoundData_t m_Left_Channel_Processed_Sound_Data;

    float m_Power;
    float m_PowerDb;
    int32_t m_signalMin;
    int32_t m_signalMax;
    
    //Max Bin Sound Data
    pthread_mutex_t m_MaxBinSoundDataLock;
    bool m_NewMaxBandSoundDataReady = false;
    unsigned long m_NewMaxBandSoundDataCurrentTime = 0;
    unsigned long m_NewMaxBandSoundDataTimeOut = 1000;
    bool m_NewMaxBandSoundDataTimedOut = false;
    bool NewMaxBandSoundDataReady();
    MaxBandSoundData_t m_Right_MaxBandSoundData;
    MaxBandSoundData_t m_Left_MaxBandSoundData;

    //Sound Detection
    TaskHandle_t  m_SoundDetectionTask;
    const int     m_silenceDetectedThreshold = silenceDetectedThreshold;
    const int     m_soundDetectedThreshold = soundDetectedThreshold;
    const int     m_silenceIntegratorMax = silenceIntegratorMax;
    int           m_silenceIntegrator = 0;
    const int     m_silenceIntegratorMin = 0;  
    long          m_silenceStartTime;

    unsigned long m_previousMicros;
    unsigned long m_currentMicros;
    const int     m_soundAdder = soundAdder;
    const int     m_silenceSubtractor = silenceSubtractor;
    SoundState_t  m_soundState = SoundState_t::SilenceDetected;
    bool          m_SoundDetected = false;
};
