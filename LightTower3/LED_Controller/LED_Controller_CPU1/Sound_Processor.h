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

#ifndef I2S_SOUND_PROCESSOR_H
#define I2S_SOUND_PROCESSOR_H

#define NUMBER_OF_BANDS 32

#define SOUND_PROCESSOR_DEBUG false
#define SOUND_PROCESSOR_QUEUE_DEBUG false
#define SOUND_PROCESSOR_LOOPS_DEBUG false
#define SOUND_PROCESSOR_INPUTDATA_DEBUG false
#define SOUND_PROCESSOR_OUTPUTDATA_DEBUG false


#include <Arduino.h>
#include <Adafruit_ZeroFFT.h>
#include <DataTypes.h>
#include <Helpers.h>
#include "Streaming.h"

class Sound_Processor: public NamedItem
                     , public CommonUtils
                     , public QueueManager
{
  public:
    Sound_Processor(String Title);
    virtual ~Sound_Processor();
    void SetupSoundProcessor(size_t InputByteCount, int SampleRate, int FFT_Length);
    void ProcessEventQueue();
    void ProcessFFTEventQueue();
    void ProcessSoundPowerEventQueue();

    //QueueManager
    DataItemConfig_t* GetDataItemConfig() { return m_ItemConfig; }
    size_t GetDataItemConfigCount() { return m_ConfigCount; }
  private:
    QueueManager* m_QueueManager;
  
    //CONFIGURATION
    size_t m_InputByteCount = 0;
    size_t m_InputSampleCount = 0;
    size_t m_BytesToRead = 0;
    size_t m_BandOutputByteCount = 0;
    int m_SampleRate = 0;
    int m_FFT_Length = 0;

    //Memory Management
    bool m_MemoryIsAllocated = false;
    void AllocateMemory();
    void FreeMemory();

    //CHANNEL DATA INPUT
    int m_FFT_Right_Buffer_Index = 0;
    int m_FFT_Left_Buffer_Index = 0;
    
    //CALCULATED OUTPUTS
    int16_t* m_FFT_Right_Data;
    int16_t* m_FFT_Left_Data;
    
    //Right Channel Processed FFT Band Data
    int16_t* m_Right_Band_Values;
    
    //Left Channel Processed FFT Band Data
    int16_t* m_Left_Band_Values;

    //Right Channel Calculated Outputs
    ProcessedSoundData_t m_Right_Channel_Processed_Sound_Data;

    //Left Channel Calculated Outputs
    ProcessedSoundData_t m_Left_Channel_Processed_Sound_Data;

    void ProcessRightChannelSoundData();
    void ProcessRightChannelFFT();
    void ProcessRightChannelPower();
    
    void ProcessLeftChannelSoundData();
    void ProcessLeftChannelFFT();
    void ProcessLeftChannelPower();
    float GetFreqForBin(unsigned int bin);

    //QueueManager Configuration
    static const size_t m_ConfigCount = 10;
    DataItemConfig_t m_ItemConfig[m_ConfigCount]
    {
      { "R_RAW_IN",   DataType_Int32_t,               100,      Transciever_RX },
      { "L_RAW_IN",   DataType_Int32_t,               100,      Transciever_RX },
      { "R_BAND_IN",  DataType_Int32_t,               100,      Transciever_RX },
      { "L_BAND_IN",  DataType_Int32_t,               100,      Transciever_RX },
      { "R_PSD_IN",   DataType_Int32_t,               100,      Transciever_RX },
      { "L_PSD_IN",   DataType_Int32_t,               100,      Transciever_RX },
      { "R_FFT",      DataType_Int16_t,               32,       Transciever_TX },
      { "L_FFT",      DataType_Int16_t,               32,       Transciever_TX },
      { "R_PSD",      DataType_ProcessedSoundData_t,  1,        Transciever_TX },
      { "L_PSD",      DataType_ProcessedSoundData_t,  1,        Transciever_TX }
    };
};



#endif
