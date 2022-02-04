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
#include <FIR.h>
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
    void SetupSoundProcessor(size_t ChannelInputByteCount, int SampleRate, int Large_FFT_Length, int Small_FFT_Length);
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
    size_t m_ChannelInputByteCount = 0;
    size_t m_ChannelInputSampleCount = 0;
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
    int16_t* m_DataBuffer1;
    int16_t* m_DataBuffer2;
    int32_t* m_DataBuffer3;
    int32_t* m_DataBuffer4;
    int16_t* m_DataBuffer5;
    int16_t* m_DataBuffer6;
    
    int32_t m_FFT_Large_Right_Buffer_Index = 0;
    int32_t m_FFT_Large_Left_Buffer_Index = 0;
    int32_t m_FFT_Small_Right_Buffer_Index = 0;
    int32_t m_FFT_Small_Left_Buffer_Index = 0;

    const int16_t m_DownSampleRatio = I2S_SAMPLE_RATE / DOWN_SAMPLED_RATE; 
    int32_t m_FFT_Small_Right_DownSampleCount = 0;
    int32_t m_FFT_Small_Left_DownSampleCount = 0;
    
    //CALCULATED OUTPUTS
    int32_t* m_RightChannel_Filtered_0k_to_3k;
    int32_t* m_LeftChannel_Filtered_0k_to_3k;
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
    float m_Gain = 1.0;
    float m_FFT_Gain = 1.0;
    float m_FFT_Out_Gain = 1.0;

    //DB Conversion taken from INMP441 Datasheet
    float m_IMNP441_1PA_Offset = 94;      //DB Output at 1PA
    float m_IMNP441_1PA_Value = 420426.0; //Digital output at 1PA
    uint32_t m_24BitMax = pow(2,24);      //Used for Amplitude of 24 bit MIC values
    uint32_t m_16BitMax = pow(2,16);      //Used for Amplitude of 16 bit FFT values

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
    
    void AssignToBins(float& Band_Data, int16_t* FFT_Data, int16_t FFT_Length, int16_t SampleRate);
    float GetFreqForBin(unsigned int bin, int16_t FFT_Length, int16_t SampleRate);
    int16_t GetBinForFrequency(float Frequency, int16_t FFT_Length, int16_t SampleRate);
    int16_t m_AudioBinLimit;

    //QueueManager Configuration
    static const size_t m_ConfigCount = 16;
    DataItemConfig_t m_ItemConfig[m_ConfigCount]
    {
      { "R_RAW32_IN",   DataType_Int32_t,               I2S_CHANNEL_SAMPLE_COUNT,       Transciever_RX,   3 },
      { "L_RAW32_IN",   DataType_Int32_t,               I2S_CHANNEL_SAMPLE_COUNT,       Transciever_RX,   3 },
      { "R_RAW16_IN",   DataType_Int16_t,               I2S_CHANNEL_SAMPLE_COUNT,       Transciever_RX,   3 },
      { "L_RAW16_IN",   DataType_Int16_t,               I2S_CHANNEL_SAMPLE_COUNT,       Transciever_RX,   3 },
      { "R_FFT_IN",     DataType_Int16_t,               I2S_CHANNEL_SAMPLE_COUNT,       Transciever_RX,   ceil(FFT_LARGE_SIZE / I2S_CHANNEL_SAMPLE_COUNT) },
      { "L_FFT_IN",     DataType_Int16_t,               I2S_CHANNEL_SAMPLE_COUNT,       Transciever_RX,   ceil(FFT_LARGE_SIZE / I2S_CHANNEL_SAMPLE_COUNT) },
      { "R_PSD_IN",     DataType_Int32_t,               I2S_CHANNEL_SAMPLE_COUNT,       Transciever_RX,   3 },
      { "L_PSD_IN",     DataType_Int32_t,               I2S_CHANNEL_SAMPLE_COUNT,       Transciever_RX,   3 },
      { "R_MAXBIN_IN",  DataType_Int16_t,               I2S_CHANNEL_SAMPLE_COUNT,       Transciever_RX,   ceil(FFT_SMALL_SIZE / I2S_CHANNEL_SAMPLE_COUNT) * m_DownSampleRatio },
      { "L_MAXBIN_IN",  DataType_Int16_t,               I2S_CHANNEL_SAMPLE_COUNT,       Transciever_RX,   ceil(FFT_SMALL_SIZE / I2S_CHANNEL_SAMPLE_COUNT) * m_DownSampleRatio },
      { "R_FFT",        DataType_Float,                 NUMBER_OF_BANDS,                Transciever_TX,   3 },
      { "L_FFT",        DataType_Float,                 NUMBER_OF_BANDS,                Transciever_TX,   3 },
      { "R_PSD",        DataType_ProcessedSoundData_t,  1,                              Transciever_TX,   3 },
      { "L_PSD",        DataType_ProcessedSoundData_t,  1,                              Transciever_TX,   3 },
      { "R_MAXBIN",     DataType_MaxBinSoundData_t,     1,                              Transciever_TX,   3 },
      { "L_MAXBIN",     DataType_MaxBinSoundData_t,     1,                              Transciever_TX,   3 },
    };

    /*
    FIR filter designed with
    http://t-filter.appspot.com
    sampling frequency: 48000 Hz
    
    * 0 Hz - 3000 Hz
      gain = 1
      desired ripple = 5 dB
      actual ripple = 3.988772934245028 dB
    
    * 4000 Hz - 24000 Hz
      gain = 0
      desired attenuation = -40 dB
      actual attenuation = -40.38404164331486 dB
      
    */
    static const int32_t FILTER_TAP_NUM_0_to_3k = 55;
    float filter_taps_0_to_3k[FILTER_TAP_NUM_0_to_3k] = 
    {
      -172,
      -12,
      43,
      138,
      269,
      424,
      583,
      720,
      806,
      815,
      731,
      547,
      275,
      -56,
      -403,
      -708,
      -914,
      -964,
      -819,
      -460,
      106,
      841,
      1683,
      2550,
      3352,
      4001,
      4423,
      4569,
      4423,
      4001,
      3352,
      2550,
      1683,
      841,
      106,
      -460,
      -819,
      -964,
      -914,
      -708,
      -403,
      -56,
      275,
      547,
      731,
      815,
      806,
      720,
      583,
      424,
      269,
      138,
      43,
      -12,
      -172
    };
    FIR<float, FILTER_TAP_NUM_0_to_3k> Right_Channel_fir_lp_0_to_3k;
    FIR<float, FILTER_TAP_NUM_0_to_3k> Left_Channel_fir_lp_0_to_3k;

};

#endif
