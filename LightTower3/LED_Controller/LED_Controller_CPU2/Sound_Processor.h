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
#include "Amplitude_Calculator.h"
#include <DataTypes.h>
#include <Helpers.h>
#include "Tunes.h"
#include "Streaming.h"
#include "float.h"
#include "Serial_Datalink_Config.h"
#include "AudioBuffer.h"

class Sound_Processor: public NamedItem
                     , public CommonUtils
                     , public QueueController
{
  public:
    Sound_Processor( String Title
                   , SPIDataLinkMaster &SPIDataLinkToCPU1
                   , SPIDataLinkMaster &SPIDataLinkToCPU3
                   , ContinuousAudioBuffer<AUDIO_BUFFER_SIZE> &AudioBuffer);
    virtual ~Sound_Processor();
    void SetupSoundProcessor();
    void SetGain(float Gain)
    {
      m_Gain = Gain;
      ESP_LOGE("Sound_Processor", "Set Gain: %f", m_Gain);
      static bool Gain_Push_Successful = true;
      m_SPIDataLinkToCPU3.PushValueToTXQueue(&m_Gain, "Amplitude Gain", 0, Gain_Push_Successful);
    }
    void SetFFTGain(float Gain)
    {
      m_FFT_Gain = Gain;
      ESP_LOGE("Sound_Processor", "Set FFT Gain: %f", m_FFT_Gain);
      static bool FFT_Gain_Push_Successful = true;
      m_SPIDataLinkToCPU3.PushValueToTXQueue(&m_FFT_Gain, "FFT Gain", 0, FFT_Gain_Push_Successful);
    }
    
  private:
    SPIDataLinkMaster &m_SPIDataLinkToCPU1;
    SPIDataLinkMaster &m_SPIDataLinkToCPU3;
    ContinuousAudioBuffer<AUDIO_BUFFER_SIZE> &m_AudioBuffer;
    
    //Memory Management
    bool m_MemoryIsAllocated = false;
    void AllocateMemory();
    void FreeMemory();
    
    //Adjustments
    float m_Gain = 1.0;
    float m_FFT_Gain = 1.7;

    //DB Conversion taken from INMP441 Datasheet
    float m_IMNP441_1PA_Offset = 94;          //DB Output at 1PA
    float m_IMNP441_1PA_Value = 420426.0;     //Digital output at 1PA
    uint32_t m_24BitLength = pow(2,24);       //Used for Amplitude of 24 bit MIC values
    uint32_t m_16BitLength = pow(2,16);       //Used for Amplitude of 16 bit FFT values
    uint32_t m_32BitLength = pow(2,32);       //Used for Amplitude of 16 bit FFT values
    
  public:
    void ProcessSoundPower()
    {
      Calculate_Power();
    }
  private:
    void Calculate_Power();
    Amplitude_Calculator m_RightSoundData = Amplitude_Calculator(AMPLITUDE_BUFFER_FRAME_COUNT, BitLength_16);
    Amplitude_Calculator m_LeftSoundData = Amplitude_Calculator(AMPLITUDE_BUFFER_FRAME_COUNT, BitLength_16);
    
  public:
    void ProcessFFT()
    {
      Calculate_FFTs();
    }
  private:
    void Calculate_FFTs();
    void Update_Right_Bands_And_Send_Result();
    void Update_Left_Bands_And_Send_Result();
    FFT_Calculator m_R_FFT = FFT_Calculator(FFT_SIZE, I2S_SAMPLE_RATE, BitLength_16);
    FFT_Calculator m_L_FFT = FFT_Calculator(FFT_SIZE, I2S_SAMPLE_RATE, BitLength_16);

    void AssignToBands(float* Band_Data, FFT_Calculator* FFT_Calculator, int16_t FFT_Size);
    float GetFreqForBin(int bin);
    int GetBinForFrequency(float Frequency);
    int16_t m_AudioBinLimit;
};

#endif
