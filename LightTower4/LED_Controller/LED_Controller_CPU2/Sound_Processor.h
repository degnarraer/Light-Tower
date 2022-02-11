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

#ifndef SOUND_PROCESSOR_H
#define SOUND_PROCESSOR_H

#include <Arduino.h>
#include <Adafruit_ZeroFFT.h>
#include <DataTypes.h>
#include <Helpers.h>
#include <FIR.h>
#include "ArduinoLinq.hpp"
#include "Streaming.h"
#include "Tunes.h"
#include "float.h"
#include "Serial_Datalink_Config.h"
#include "SimpleDownSampler.h"

class Sound_Processor: public NamedItem
                     , public CommonUtils
                     , public QueueManager
{
  public:
    Sound_Processor(String Title, SerialDataLink &SerialDataLink);
    virtual ~Sound_Processor();
    void SetupSoundProcessor(size_t ChannelInputByteCount, int SampleRate, int Large_FFT_Length, int Small_FFT_Length);

    //QueueManager
    DataItemConfig_t* GetDataItemConfig() { return m_ItemConfig; }
    size_t GetDataItemConfigCount() { return m_ConfigCount; }
  private:
    QueueManager* m_QueueManager;
    SerialDataLink& m_SerialDataLink;
  
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
    
    int32_t m_FFT_Large_Right_Buffer_Index = 0;
    int32_t m_FFT_Large_Left_Buffer_Index = 0;
    int32_t m_FFT_Small_Right_Buffer_Index = 0;
    int32_t m_FFT_Small_Left_Buffer_Index = 0;
    
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

    //44100 Hz 32 Bit Sound
    public:
      void ProcessEventQueue()
      {
        Sound_32Bit_44100Hz_MoveRightChannelData();
        Sound_32Bit_44100Hz_MoveLeftChannelData();
      }
    private:
      void Sound_32Bit_44100Hz_MoveRightChannelData();
      void Sound_32Bit_44100Hz_MoveLeftChannelData();
    
    public:
      void ProcessSoundPower()
      {
        Sound_32Bit_44100Hz_Calculate_Right_Channel_Power();
        Sound_32Bit_44100Hz_Calculate_Left_Channel_Power();
      }
    private:
      int32_t m_RightMinimumValue = INT32_MAX;
      int32_t m_RightMaximumValue = INT32_MIN;
      int32_t m_LeftMinimumValue = INT32_MAX;
      int32_t m_LeftMaximumValue = INT32_MIN;
      int32_t m_RightPowerLoopCount = 0;
      int32_t m_LeftPowerLoopCount = 0;
      int16_t m_PowerLoopCountTarget = I2S_SAMPLE_RATE / 100;
      void Sound_32Bit_44100Hz_Calculate_Right_Channel_Power();
      void Sound_32Bit_44100Hz_Calculate_Left_Channel_Power();

    public:
      void FilterAndDownSample()
      {
        Sound_16Bit_44100Hz_Right_Channel_Filter_DownSample();
        Sound_16Bit_44100Hz_Left_Channel_Filter_DownSample();
      }
    private:
      SimpleDownSampler m_LeftDownSampler;
      SimpleDownSampler m_RightDownSampler;
      const int16_t m_DownSampleCount = I2S_SAMPLE_COUNT / (I2S_SAMPLE_RATE/DOWN_SAMPLED_RATE);
      void Sound_16Bit_44100Hz_Right_Channel_Filter_DownSample();
      void Sound_16Bit_44100Hz_Left_Channel_Filter_DownSample();

    //44100 Hz 16 Bit Sound
    
    public:
      void ProcessFFT()
      {
        Sound_16Bit_44100Hz_Right_Channel_FFT();
        Sound_16Bit_44100Hz_Left_Channel_FFT();
      }
    private:
      void Sound_16Bit_44100Hz_Right_Channel_FFT();
      void Sound_16Bit_44100Hz_Left_Channel_FFT();

    //6300 Hz 16 Bit Sound
    public:
      void ProcessMaxBand()
      {
        ProcessRightChannelMaxBand();
        ProcessLeftChannelMaxBand();
      }
    private:
      void ProcessRightChannelMaxBand();
      void ProcessLeftChannelMaxBand();
 
    void AssignToBins(float& Band_Data, int16_t* FFT_Data, int16_t FFT_Length, int16_t SampleRate);
    float GetFreqForBin(unsigned int bin, int16_t FFT_Length, int16_t SampleRate);
    int16_t GetBinForFrequency(float Frequency, int16_t FFT_Length, int16_t SampleRate);
    int16_t m_AudioBinLimit;

    //QueueManager Configuration
    static const size_t m_ConfigCount = 12;
    DataItemConfig_t m_ItemConfig[m_ConfigCount]
    {
      { "R_RAW32_IN",       DataType_Int32_t,   I2S_SAMPLE_COUNT,   Transciever_RX,   10 },
      { "L_RAW32_IN",       DataType_Int32_t,   I2S_SAMPLE_COUNT,   Transciever_RX,   10 },
      { "R_PSD_IN",         DataType_Int32_t,   I2S_SAMPLE_COUNT,   Transciever_RX,   10 },
      { "L_PSD_IN",         DataType_Int32_t,   I2S_SAMPLE_COUNT,   Transciever_RX,   10 },
      { "R_DOWNSAMPLE_IN",  DataType_Int16_t,   I2S_SAMPLE_COUNT,   Transciever_RX,   10 },
      { "L_DOWNSAMPLE_IN",  DataType_Int16_t,   I2S_SAMPLE_COUNT,   Transciever_RX,   10 },
      { "R_RAW16_IN",       DataType_Int16_t,   I2S_SAMPLE_COUNT,   Transciever_RX,   10 },
      { "L_RAW16_IN",       DataType_Int16_t,   I2S_SAMPLE_COUNT,   Transciever_RX,   10 },
      { "R_FFT_IN",         DataType_Int16_t,   I2S_SAMPLE_COUNT,   Transciever_RX,   10 },
      { "L_FFT_IN",         DataType_Int16_t,   I2S_SAMPLE_COUNT,   Transciever_RX,   10 },
      { "R_MAXBIN_IN",      DataType_Int16_t,   m_DownSampleCount,  Transciever_RX,   10 },
      { "L_MAXBIN_IN",      DataType_Int16_t,   m_DownSampleCount,  Transciever_RX,   10 },
    };

    /*
    FIR filter designed with
    http://t-filter.engineerjs.com/
    sampling frequency: 44100 Hz

    fixed point precision: 16 bits
    
    * 0 Hz - 3000 Hz
      gain = 1
      desired ripple = 5 dB
      actual ripple = n/a
    
    * 4000 Hz - 22050 Hz
      gain = 0
      desired attenuation = -40 dB
      actual attenuation = n/a
    
    */
    static const int32_t FILTER_TAP_NUM_0_to_3k = 49;
    float filter_taps_0_to_3k[FILTER_TAP_NUM_0_to_3k] = {
      -132,
      100,
      200,
      354,
      535,
      712,
      847,
      901,
      845,
      662,
      361,
      -24,
      -435,
      -797,
      -1025,
      -1045,
      -807,
      -292,
      473,
      1420,
      2448,
      3431,
      4247,
      4784,
      4972,
      4784,
      4247,
      3431,
      2448,
      1420,
      473,
      -292,
      -807,
      -1045,
      -1025,
      -797,
      -435,
      -24,
      361,
      662,
      845,
      901,
      847,
      712,
      535,
      354,
      200,
      100,
      -132
    };
    FIR<float, FILTER_TAP_NUM_0_to_3k> Right_Channel_fir_lp_0_to_3k;
    FIR<float, FILTER_TAP_NUM_0_to_3k> Left_Channel_fir_lp_0_to_3k;
};

#endif
