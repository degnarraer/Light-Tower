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
#include "AudioBuffer.h"
#include "DataItem.h"
#include <Preferences.h>

class Sound_Processor: public NamedItem
                     , public CommonUtils
{
  public:
    Sound_Processor( String Title
                   , ContinuousAudioBuffer<AUDIO_BUFFER_SIZE> &AudioBuffer
                   , SerialPortMessageManager &CPU1SerialPortMessageManager
                   , SerialPortMessageManager &CPU3SerialPortMessageManager);
    virtual ~Sound_Processor();
    void SetupSoundProcessor();
    
  private:
    Preferences m_Preferences;
    void InitializeNVM(bool Reset);
    void LoadFromNVM();
    
    ContinuousAudioBuffer<AUDIO_BUFFER_SIZE> &m_AudioBuffer;
    Amplitude_Calculator m_RightSoundData = Amplitude_Calculator(AMPLITUDE_BUFFER_FRAME_COUNT, BitLength_16);
    Amplitude_Calculator m_LeftSoundData = Amplitude_Calculator(AMPLITUDE_BUFFER_FRAME_COUNT, BitLength_16);
    FFT_Calculator m_R_FFT = FFT_Calculator(FFT_SIZE, I2S_SAMPLE_RATE, BitLength_16);
    FFT_Calculator m_L_FFT = FFT_Calculator(FFT_SIZE, I2S_SAMPLE_RATE, BitLength_16);
    
    SerialPortMessageManager &m_CPU1SerialPortMessageManager;
    SerialPortMessageManager &m_CPU3SerialPortMessageManager;
    DataItem <float, 1> m_Amplitude_Gain = DataItem<float, 1>("Amplitude Gain", 1.0, RxTxType_Rx_Echo_Value, 1000, m_CPU3SerialPortMessageManager);
    DataItem <float, 1> m_FFT_Gain = DataItem<float, 1>("FFT Gain", 1.7, RxTxType_Rx_Echo_Value, 1000, m_CPU3SerialPortMessageManager);

    //DB Conversion taken from INMP441 Datasheet
    float m_IMNP441_1PA_Offset = 94;          //DB Output at 1PA
    float m_IMNP441_1PA_Value = 420426.0;     //Digital output at 1PA
    uint32_t m_24BitLength = pow(2,24);       //Used for Amplitude of 24 bit MIC values
    uint32_t m_16BitLength = pow(2,16);       //Used for Amplitude of 16 bit FFT values
    uint32_t m_32BitLength = pow(2,32);       //Used for Amplitude of 16 bit FFT values
    
    TaskHandle_t m_ProcessSoundPowerTask;
    static void Static_Calculate_Power(void * parameter);
    void Calculate_Power();
    TaskHandle_t m_ProcessFFTTask;
    static void Static_Calculate_FFTs(void * parameter);
    void Calculate_FFTs();
    void Update_Right_Bands_And_Send_Result();
    void Update_Left_Bands_And_Send_Result();

    void AssignToBands(float* Band_Data, FFT_Calculator* FFT_Calculator, int16_t FFT_Size);
    float GetFreqForBin(int bin);
    int GetBinForFrequency(float Frequency);
    int16_t m_AudioBinLimit;
};

#endif
