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
{
  public:
    Sound_Processor(String Title);
    virtual ~Sound_Processor();
    void Setup(size_t InputByteCount, int SampleRate, int FFT_Length);
    void ProcessEventQueue();
    void ProcessFFTEventQueue();
    void ProcessSoundPowerEventQueue();

    //Input Data Queues
    QueueHandle_t GetFFTRightDataInputQueue() { return m_FFT_Right_Data_Input_Buffer_queue; }
    QueueHandle_t GetFFTLeftDataInputQueue() { return m_FFT_Left_Data_Input_Buffer_queue; }
       
    //Right Channel Output Data Queues
    public: QueueHandle_t GetFFTRightBandDataOutputQueue() { return m_FFT_Right_BandData_Output_Buffer_Queue; }
    public: size_t GetFFTRightBandDataBufferSize() { return m_BandOutputByteCount; }
    public: QueueHandle_t GetRightChannelNormalizedPowerOutputQueue() { return m_Right_Channel_Normalized_Power_Output_Buffer_Queue; }
    public: size_t GetRightChannelNormalizedPowerSize() { return sizeof(m_Right_Channel_Power_Normalized); }
    public: QueueHandle_t GetRightChannelDBOutputQueue() { return m_Right_Channel_DB_Output_Buffer_Queue; }
    public: size_t GetRightChannelDBSize() { return sizeof(m_Right_Channel_Db); }
    public: QueueHandle_t GetRightChannelPowerMinOutputQueue() { return m_Right_Channel_Power_Min_Output_Buffer_Queue; }
    public: size_t GetRightChannelPowerMinSize() { return sizeof(m_Right_Channel_Min); }
    public: QueueHandle_t GetRightChannelPowerMaxOutputQueue() { return m_Right_Channel_Power_Max_Output_Buffer_Queue; }
    public: size_t GetRightChannelPowerMaxSize() { return sizeof(m_Right_Channel_Max); }

    //Left Channel Output Data Queues
    public: QueueHandle_t GetFFTLeftBandDataOutputQueue() { return m_FFT_Left_BandData_Output_Buffer_Queue; }
    public: size_t GetFFTLeftBandDataBufferSize() { return m_BandOutputByteCount; }
    public: QueueHandle_t GetLeftChannelNormalizedPowerOutputQueue() { return m_Left_Channel_Normalized_Power_Output_Buffer_Queue; }
    public: size_t GetLeftChannelNormalizedPowerSize() { return sizeof(m_Left_Channel_Power_Normalized); }
    public: QueueHandle_t GetLeftChannelDBOutputQueue() { return m_Left_Channel_DB_Output_Buffer_Queue; }
    public: size_t GetLeftChannelDBSize() { return sizeof(m_Left_Channel_Db); }
    public: QueueHandle_t GetLeftChannelPowerMinOutputQueue() { return m_Left_Channel_Power_Min_Output_Buffer_Queue; }
    public: size_t GetLeftChannelPowerMinSize() { return sizeof(m_Left_Channel_Min); }
    public: QueueHandle_t GetLeftChannelPowerMaxOutputQueue() { return m_Left_Channel_Power_Max_Output_Buffer_Queue; }
    public: size_t GetLeftChannelPowerMaxSize() { return sizeof(m_Left_Channel_Max); }
    
  private:
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
    QueueHandle_t m_FFT_Right_Data_Input_Buffer_queue = NULL;
    QueueHandle_t m_FFT_Left_Data_Input_Buffer_queue = NULL;
    
    //CALCULATED OUTPUTS
    int16_t* m_FFT_Right_Data;
    int16_t* m_FFT_Left_Data;
    
    //Right Channel Processed FFT Band Data
    QueueHandle_t m_FFT_Right_BandData_Input_Buffer_Queue = NULL;
    int16_t* m_Right_Band_Values;
    QueueHandle_t m_FFT_Right_BandData_Output_Buffer_Queue = NULL;
    
    //Left Channel Processed FFT Band Data
    QueueHandle_t m_FFT_Left_BandData_Input_Buffer_Queue = NULL;
    int16_t* m_Left_Band_Values;
    private: QueueHandle_t m_FFT_Left_BandData_Output_Buffer_Queue = NULL;


    //Right Channel Calculated Outputs
    QueueHandle_t m_Right_Channel_Power_Input_Buffer_Queue = NULL;
    int16_t m_Right_Channel_Power_Normalized;
    int16_t m_Right_Channel_Db;
    int16_t m_Right_Channel_Min;
    int16_t m_Right_Channel_Max;
    private: QueueHandle_t m_Right_Channel_Normalized_Power_Output_Buffer_Queue = NULL;
    private: QueueHandle_t m_Right_Channel_DB_Output_Buffer_Queue = NULL;
    private: QueueHandle_t m_Right_Channel_Power_Min_Output_Buffer_Queue = NULL;
    private: QueueHandle_t m_Right_Channel_Power_Max_Output_Buffer_Queue = NULL;

    //Left Channel Calculated Outputs
    private: QueueHandle_t m_Left_Channel_Power_Input_Buffer_Queue = NULL;
    private: int16_t m_Left_Channel_Power_Normalized;
    private: int16_t m_Left_Channel_Db;
    private: int16_t m_Left_Channel_Min;
    private: int16_t m_Left_Channel_Max;
    private: QueueHandle_t m_Left_Channel_Normalized_Power_Output_Buffer_Queue = NULL;
    private: QueueHandle_t m_Left_Channel_DB_Output_Buffer_Queue = NULL;
    private: QueueHandle_t m_Left_Channel_Power_Min_Output_Buffer_Queue = NULL;
    private: QueueHandle_t m_Left_Channel_Power_Max_Output_Buffer_Queue = NULL;

private: 
    void ProcessRightChannelSoundData();
    void ProcessRightChannelFFT();
    void ProcessRightChannelPower();
    
    void ProcessLeftChannelSoundData();
    void ProcessLeftChannelFFT();
    void ProcessLeftChannelPower();
    float GetFreqForBin(unsigned int bin);
};



#endif
