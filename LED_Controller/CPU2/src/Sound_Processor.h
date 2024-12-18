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

#pragma once

#include "arduinoFFT.h"
#include "FFT_Computer.h"
#include "Amplitude_Calculator.h"
#include <DataTypes.h>
#include <Helpers.h>
#include "Tunes.h"
#include "Streaming.h"
#include "float.h"
#include "AudioBuffer.h"
#include "DataItem/DataItems.h"

class Sound_Processor: public NamedItem
                     , public CommonUtils
                     , public SetupCallerInterface
{
  public:
    Sound_Processor( std::string Title
                   , FFT_Computer &r_FFT
                   , ContinuousAudioBuffer<AMPLITUDE_AUDIO_BUFFER_SIZE> &Amplitude_AudioBuffer
                   , SerialPortMessageManager &CPU1SerialPortMessageManager
                   , SerialPortMessageManager &CPU3SerialPortMessageManager
                   , IPreferences& preferences );
    virtual ~Sound_Processor();
    void Setup();
    static void StaticFFT_Results_Callback(float *leftMagnitudes, float* rightMagnitudes, size_t count, void* args)
    {
      Sound_Processor *aSound_Processor = static_cast<Sound_Processor*>(args);
      aSound_Processor->FFT_Results_Callback(leftMagnitudes, rightMagnitudes, count);
    }
    void FFT_Results_Callback(float *leftMagnitudes, float* rightMagnitudes, size_t count);
    
  private:
    ContinuousAudioBuffer<AMPLITUDE_AUDIO_BUFFER_SIZE> &m_Amplitude_AudioBuffer;
    Amplitude_Calculator m_RightSoundData = Amplitude_Calculator(AMPLITUDE_BUFFER_FRAME_COUNT, DataWidth_16);
    Amplitude_Calculator m_LeftSoundData = Amplitude_Calculator(AMPLITUDE_BUFFER_FRAME_COUNT, DataWidth_16);
    FFT_Computer &m_FFT_Computer;
    SerialPortMessageManager &m_CPU1SerialPortMessageManager;
    SerialPortMessageManager &m_CPU3SerialPortMessageManager;
    IPreferences& m_Preferences;
    bool m_BufferReadError = false;

    const float m_Amplitude_Gain_InitialValue = 1.1;
    DataItemWithPreferences<float, 1> m_Amplitude_Gain = DataItemWithPreferences<float, 1>( "Amp_Gain"
                                                                                          , m_Amplitude_Gain_InitialValue
                                                                                          , RxTxType_Rx_Echo_Value
                                                                                          , 5000
                                                                                          , &m_Preferences
                                                                                          , &m_CPU3SerialPortMessageManager
                                                                                          , NULL
                                                                                          , this );

    const float m_FFT_Gain_InitialValue = 1.1;
    DataItemWithPreferences<float, 1> m_FFT_Gain = DataItemWithPreferences<float, 1>( "FFT_Gain"
                                                                                    , m_FFT_Gain_InitialValue
                                                                                    , RxTxType_Rx_Echo_Value
                                                                                    , 5000
                                                                                    , &m_Preferences
                                                                                    , &m_CPU3SerialPortMessageManager
                                                                                    , NULL
                                                                                    , this );
    
    MaxBandSoundData_t m_R_Max_Band_InitialValue = MaxBandSoundData_t();
    DataItem<MaxBandSoundData_t, 1> m_R_Max_Band = DataItem<MaxBandSoundData_t, 1>( "R_Max_Band"
                                                                                  , m_R_Max_Band_InitialValue
                                                                                  , RxTxType_Tx_On_Change
                                                                                  , 0
                                                                                  , &m_CPU1SerialPortMessageManager
                                                                                  , NULL
                                                                                  , this );
    float m_R_Bands_InitialValue = 0.0;
    DataItem<float, 32> m_R_Bands1 = DataItem<float, 32>( "R_Bands"
                                                       , m_R_Bands_InitialValue
                                                       , RxTxType_Tx_On_Change
                                                       , 0
                                                       , &m_CPU1SerialPortMessageManager
                                                       , NULL
                                                       , this );
    DataItem<float, 32> m_R_Bands3 = DataItem<float, 32>( "R_Bands"
                                                       , m_R_Bands_InitialValue
                                                       , RxTxType_Tx_On_Change
                                                       , 0
                                                       , &m_CPU3SerialPortMessageManager
                                                       , NULL
                                                       , this );
    
    MaxBandSoundData_t m_L_Max_Band_InitialValue = MaxBandSoundData_t();
    DataItem<MaxBandSoundData_t, 1> m_L_Max_Band = DataItem<MaxBandSoundData_t, 1>( "L_Max_Band"
                                                                                  , m_L_Max_Band_InitialValue
                                                                                  , RxTxType_Tx_On_Change
                                                                                  , 0
                                                                                  , &m_CPU1SerialPortMessageManager
                                                                                  , NULL
                                                                                  , this );
    
    float m_L_Bands_InitialValue = 0.0;
    DataItem<float, 32> m_L_Bands1 = DataItem<float, 32>( "L_Bands"
                                                       , m_L_Bands_InitialValue
                                                       , RxTxType_Tx_On_Change
                                                       , 0
                                                       , &m_CPU1SerialPortMessageManager
                                                       , NULL
                                                       , this );
    DataItem<float, 32> m_L_Bands3 = DataItem<float, 32>( "L_Bands"
                                                       , m_L_Bands_InitialValue
                                                       , RxTxType_Tx_On_Change
                                                       , 0
                                                       , &m_CPU3SerialPortMessageManager
                                                       , NULL
                                                       , this );

    ProcessedSoundFrame_t PSF_InitialValue = ProcessedSoundFrame_t();
    DataItem<ProcessedSoundFrame_t, 1> PSF = DataItem<ProcessedSoundFrame_t, 1>( "PSF"
                                                                               , PSF_InitialValue
                                                                               , RxTxType_Tx_On_Change
                                                                               , 0
                                                                               , &m_CPU1SerialPortMessageManager
                                                                               , NULL
                                                                               , this );
    
    //DB Conversion taken from INMP441 Datasheet
    float m_IMNP441_1PA_Offset = 94;          //DB Output at 1PA
    float m_IMNP441_1PA_Value = 420426.0;     //Digital output at 1PA
    
    uint32_t m_24BitLength = 1 << 24;     //Used for Amplitude of 24 bit MIC values
    uint32_t m_16BitLength = 1 << 16;     //Used for Amplitude of 16 bit FFT values
    uint64_t m_32BitLength = 1ULL << 32;  //Used for Amplitude of 32 bit FFT values
    
    TaskHandle_t m_ProcessSoundPowerTask;
    static void Static_Calculate_Power(void * parameter);
    void Calculate_Power();
    void Update_Right_Bands_And_Send_Result(float* magnitudes, size_t count);
    void Update_Left_Bands_And_Send_Result(float* magnitudes, size_t count);

    void AssignToBands(float* Band_Data, float* magnitudes, size_t count);
    float GetFreqForBin(int bin);
    int GetBinForFrequency(float Frequency);
    int16_t m_AudioBinLimit;
};
