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

#include <Arduino.h>
#include <Adafruit_ZeroFFT.h>
#include <DataTypes.h>
#include <Helpers.h>
#include "Streaming.h"
#include "Tunes.h"
#include "float.h"

class Sound_Processor: public NamedItem
                     , public CommonUtils
                     , public QueueManager
{
  public:
    Sound_Processor(String Title);
    virtual ~Sound_Processor();
    void SetupSoundProcessor(size_t InputByteCount, int SampleRate, int Large_FFT_Length, int Small_FFT_Length);
    void ProcessEventQueue();
    void ProcessFFTEventQueue();
    void ProcessSoundPowerEventQueue();
    void ProcessMaxBandEventQueue();

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
    int32_t m_SampleRate = 0;
    int16_t m_Large_FFT_Length = 0;
    int16_t m_Small_FFT_Length = 0;

    //Memory Management
    bool m_MemoryIsAllocated = false;
    void AllocateMemory();
    void FreeMemory();

    //CHANNEL DATA INPUT
    int32_t m_FFT_Large_Right_Buffer_Index = 0;
    int32_t m_FFT_Large_Left_Buffer_Index = 0;
    int32_t m_FFT_Small_Right_Buffer_Index = 0;
    int32_t m_FFT_Small_Left_Buffer_Index = 0;
    
    //CALCULATED OUTPUTS
    int16_t* m_Large_FFT_Right_Data;
    int16_t* m_Large_FFT_Left_Data;
    int16_t* m_Small_FFT_Right_Data;
    int16_t* m_Small_FFT_Left_Data;
    
    //Right Channel Processed FFT Band Data
    float* m_Right_Band_Values;
    MaxBinSoundData_t m_Right_MaxBinSoundData;
    
    //Left Channel Processed FFT Band Data
    float* m_Left_Band_Values;
    MaxBinSoundData_t m_Left_MaxBinSoundData;

    //Adjustments
    float m_Gain = 6.0;
    float m_FFT_Gain = 1.0;

    //Right Channel Calculated Outputs
    ProcessedSoundData_t m_Right_Channel_Processed_Sound_Data;

    //Left Channel Calculated Outputs
    ProcessedSoundData_t m_Left_Channel_Processed_Sound_Data;

    void ProcessRightChannelSoundData();
    void ProcessRightChannelFFT();
    void ProcessRightChannelPower();
    void ProcessRightChannelMaxBand();
    
    void ProcessLeftChannelSoundData();
    void ProcessLeftChannelFFT();
    void ProcessLeftChannelPower();
    void ProcessLeftChannelMaxBand();
    
    void AssignToBins(float& Band_Data, int16_t* FFT_Data, int16_t FFT_Length);
    float GetFreqForBin(unsigned int bin, int16_t FFT_Length);
    int16_t GetBinForFrequency(float Frequency, int16_t FFT_Length);
    int16_t m_10kHz_Bin;

    //QueueManager Configuration
    static const size_t m_ConfigCount = 14;
    DataItemConfig_t m_ItemConfig[m_ConfigCount]
    {
      { "R_RAW_IN",     DataType_Int32_t,               I2S_CHANNEL_SAMPLE_COUNT,       Transciever_RX,   3 },
      { "L_RAW_IN",     DataType_Int32_t,               I2S_CHANNEL_SAMPLE_COUNT,       Transciever_RX,   3 },
      { "R_BAND_IN",    DataType_Int32_t,               I2S_CHANNEL_SAMPLE_COUNT,       Transciever_RX,   FFT_LARGE_SIZE / I2S_CHANNEL_SAMPLE_COUNT },
      { "L_BAND_IN",    DataType_Int32_t,               I2S_CHANNEL_SAMPLE_COUNT,       Transciever_RX,   FFT_LARGE_SIZE / I2S_CHANNEL_SAMPLE_COUNT },
      { "R_PSD_IN",     DataType_Int32_t,               I2S_CHANNEL_SAMPLE_COUNT,       Transciever_RX,   3 },
      { "L_PSD_IN",     DataType_Int32_t,               I2S_CHANNEL_SAMPLE_COUNT,       Transciever_RX,   3 },
      { "R_MAXBIN_IN",  DataType_Int32_t,               I2S_CHANNEL_SAMPLE_COUNT,       Transciever_RX,   FFT_SMALL_SIZE / I2S_CHANNEL_SAMPLE_COUNT * 4 },
      { "L_MAXBIN_IN",  DataType_Int32_t,               I2S_CHANNEL_SAMPLE_COUNT,       Transciever_RX,   FFT_SMALL_SIZE / I2S_CHANNEL_SAMPLE_COUNT * 4 },
      { "R_FFT",        DataType_Float,                 NUMBER_OF_BANDS,                Transciever_TX,   3 },
      { "L_FFT",        DataType_Float,                 NUMBER_OF_BANDS,                Transciever_TX,   3 },
      { "R_PSD",        DataType_ProcessedSoundData_t,  1,                              Transciever_TX,   10 },
      { "L_PSD",        DataType_ProcessedSoundData_t,  1,                              Transciever_TX,   10 },
      { "R_MAXBIN",     DataType_MaxBinSoundData_t,     1,                              Transciever_TX,   10 },
      { "L_MAXBIN",     DataType_MaxBinSoundData_t,     1,                              Transciever_TX,   10 },
    };
};



#endif
