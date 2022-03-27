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
#include "arduinoFFT.h"
#include "FFT_Calculator.h"
#include <DataTypes.h>
#include <Helpers.h>
#include "Tunes.h"
#include "Streaming.h"
#include "float.h"
#include "Serial_Datalink_Config.h"

class Sound_Processor: public NamedItem
                     , public CommonUtils
                     , public QueueManager
{
  public:
    Sound_Processor(String Title, SerialDataLink &SerialDataLink);
    virtual ~Sound_Processor();
    void SetupSoundProcessor();

    //QueueManager
    DataItemConfig_t* GetDataItemConfig() { return m_ItemConfig; }
    size_t GetDataItemConfigCount() { return m_ConfigCount; }
  private:
    QueueManager* m_QueueManager;
    SerialDataLink& m_SerialDataLink;

    //Memory Management
    bool m_MemoryIsAllocated = false;
    void AllocateMemory();
    void FreeMemory();
    
    //Adjustments
    float m_Gain = 1.0;
    float m_FFT_Gain = 1.0;
    float m_FFT_Out_Gain = 1.0;
    float m_Band_Gain = 1.0;
    
    //CALCULATED OUTPUTS
    FFT_Calculator m_R_FFT = FFT_Calculator(FFT_SIZE, I2S_SAMPLE_RATE, m_32BitMax);
    FFT_Calculator m_L_FFT = FFT_Calculator(FFT_SIZE, I2S_SAMPLE_RATE, m_32BitMax);
    float m_MajorPeak_R = 0.0;
    float m_MajorPeak_L = 0.0;

    //DB Conversion taken from INMP441 Datasheet
    float m_IMNP441_1PA_Offset = 94;      //DB Output at 1PA
    float m_IMNP441_1PA_Value = 420426.0; //Digital output at 1PA
    uint32_t m_24BitMax = pow(2,24);      //Used for Amplitude of 24 bit MIC values
    uint32_t m_16BitMax = pow(2,16);      //Used for Amplitude of 16 bit FFT values
    uint32_t m_32BitMax = pow(2,32);      //Used for Amplitude of 16 bit FFT values

    public:
      void ProcessSoundPower()
      {
        Sound_32Bit_44100Hz_Calculate_Right_Channel_Power();
        Sound_32Bit_44100Hz_Calculate_Left_Channel_Power();
      }
    private:
      int32_t m_RightPowerCalculationCount = 0;
      int32_t m_LeftPowerCalculationCount = 0;
      const int16_t m_PowerCalculationsPerSecond = 100;
      void Sound_32Bit_44100Hz_Calculate_Right_Channel_Power();
      void Sound_32Bit_44100Hz_Calculate_Left_Channel_Power();
      ProcessedSoundData_t m_R_ProcessedSoundData;
      ProcessedSoundData_t m_L_ProcessedSoundData;

    public:
      void ProcessFFT()
      {
        Sound_32Bit_44100Hz_Right_Channel_FFT();
        Sound_32Bit_44100Hz_Left_Channel_FFT();
      }
    private:
      void Sound_32Bit_44100Hz_Right_Channel_FFT();
      void Sound_32Bit_44100Hz_Left_Channel_FFT();
 
    void AssignToBands(float* Band_Data, float* FFT_Data, int16_t FFT_Size);
    float GetFreqForBin(int bin);
    int GetBinForFrequency(float Frequency);
    int16_t m_AudioBinLimit;

    //QueueManager Configuration
    static const size_t m_ConfigCount = 4;
    DataItemConfig_t m_ItemConfig[m_ConfigCount]
    {
      { "R_PSD_IN", DataType_Int32_t, I2S_SAMPLE_COUNT,   Transciever_RX,   1 },
      { "L_PSD_IN", DataType_Int32_t, I2S_SAMPLE_COUNT,   Transciever_RX,   1 },
      { "R_FFT_IN", DataType_Int32_t, I2S_SAMPLE_COUNT,   Transciever_RX,   FFT_SIZE/I2S_SAMPLE_COUNT },
      { "L_FFT_IN", DataType_Int32_t, I2S_SAMPLE_COUNT,   Transciever_RX,   FFT_SIZE/I2S_SAMPLE_COUNT },
    };
};

#endif
