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

#include "Sound_Processor.h"

Sound_Processor::Sound_Processor( std::string Title          
                                , FFT_Computer &r_FFT
                                , ContinuousAudioBuffer<AMPLITUDE_AUDIO_BUFFER_SIZE> &Amplitude_AudioBuffer
                                , SerialPortMessageManager &CPU1SerialPortMessageManager
                                , SerialPortMessageManager &CPU3SerialPortMessageManager
                                , IPreferences& preferences )
                                : NamedItem(Title)
                                , m_FFT_Computer(r_FFT)
                                , m_Amplitude_AudioBuffer(Amplitude_AudioBuffer)
                                , m_CPU1SerialPortMessageManager(CPU1SerialPortMessageManager)
                                , m_CPU3SerialPortMessageManager(CPU3SerialPortMessageManager)
                                , m_Preferences(preferences)
{
}
Sound_Processor::~Sound_Processor()
{
  if (m_ProcessSoundPowerTask)
  {
      vTaskDelete(m_ProcessSoundPowerTask);
      m_ProcessSoundPowerTask = nullptr;
      ESP_LOGD("Destructor", "Deleted m_ProcessSoundPowerTask.");
  }

  if (m_MessageQueueProcessorTask)
  {
      vTaskDelete(m_MessageQueueProcessorTask);
      m_MessageQueueProcessorTask = nullptr;
      ESP_LOGD("Destructor", "Deleted m_MessageQueueProcessorTask.");
  }

  if (m_FFT_Result_Processor_Queue)
  {
      vQueueDelete(m_FFT_Result_Processor_Queue);
      m_FFT_Result_Processor_Queue = nullptr;
      ESP_LOGD("Destructor", "Deleted m_FFT_Result_Processor_Queue.");
  }
}
void Sound_Processor::Setup()
{
  SetupAllSetupCallees();
  m_AudioBinLimit = GetBinForFrequency(MAX_VISUALIZATION_FREQUENCY);
  
  m_FFT_Result_Processor_Queue = xQueueCreate(FFT_COMPUTE_QUEUE_SIZE, sizeof(FFT_Bin_Data_Set_t*) );
  if(m_FFT_Result_Processor_Queue) ESP_LOGD("Setup", "FFT Result Processor Queue Created.");
  else ESP_LOGE("Setup", "ERROR! Error creating the FFT Result Processor Queue.");

  if( xTaskCreate( Static_FFT_Result_Processor_Task, "Message FFT Result Processor", 10000, this, FFT_COMPUTE_TASK_PRIORITY, &m_MessageQueueProcessorTask ) != pdTRUE )
  ESP_LOGE("Setup", "ERROR! Unable to create task.");
  
  //if( xTaskCreate( Static_Calculate_Power, "Sound Power Task", 10000, this, THREAD_PRIORITY_MEDIUM, &m_ProcessSoundPowerTask ) != pdTRUE )
  //ESP_LOGE("Setup", "ERROR! Unable to create task.");
  
  m_FFT_Computer.Setup(&StaticFFT_Results_Callback, this);
}

void Sound_Processor::FFT_Results_Callback(std::unique_ptr<FFT_Bin_Data_Set_t> sp_FFT_Bin_Data_Set)
{
  static LogWithRateLimit FFT_Results_Callback_RLL(1000, ESP_LOG_INFO);
  static LogWithRateLimit FFT_Results_Callback_Queue_Success_RLL(1000, ESP_LOG_INFO);
  static LogWithRateLimit FFT_Results_Callback_Queue_Fail_RLL(1000, ESP_LOG_INFO);

  FFT_Results_Callback_RLL.Log(ESP_LOG_INFO, "FFT_Results_Callback", "FFT_Results_Callback.");

  FFT_Bin_Data_Set_t* p_fft_Bin_Data_Set_raw = sp_FFT_Bin_Data_Set.release();
  if(m_FFT_Result_Processor_Queue && xQueueSend(m_FFT_Result_Processor_Queue, &p_fft_Bin_Data_Set_raw, pdMS_TO_TICKS(0)) == pdTRUE)
  {
    FFT_Results_Callback_Queue_Success_RLL.Log(ESP_LOG_INFO, "FFT_Results_Callback", "Queued FFT Data.");
  }
  else
  {
    FFT_Results_Callback_Queue_Fail_RLL.Log(ESP_LOG_INFO, "FFT_Results_Callback", "WARNING! Unable to Queue FFT Data.");
    delete p_fft_Bin_Data_Set_raw;
  }
}
    
void Sound_Processor::Static_FFT_Result_Processor_Task(void * parameter)
{
  Sound_Processor *aSound_Processor = (Sound_Processor*)parameter;
  aSound_Processor->FFT_Result_Processor_Task();
}

void Sound_Processor::FFT_Result_Processor_Task()
{
  while(true)
  {
    static LogWithRateLimit FFT_Results_Processor_Task_RLL(1000, ESP_LOG_INFO);
    static LogWithRateLimit FFT_Results_Processor_Task_Queue_Error_RLL(1000, ESP_LOG_ERROR);
    if(m_FFT_Result_Processor_Queue)
    {
      size_t messages = uxQueueMessagesWaiting(m_FFT_Result_Processor_Queue);
      for(int i = 0; i < messages; ++i)
      {
        FFT_Bin_Data_Set_t* p_FFT_Bin_Data_Set_raw = nullptr;
        if(xQueueReceive(m_FFT_Result_Processor_Queue, &p_FFT_Bin_Data_Set_raw, portMAX_DELAY) == pdTRUE )
        {
          FFT_Results_Processor_Task_RLL.Log(ESP_LOG_INFO, "FFT_Result_Processor_Task", "Processing FFT Data from Queue.");
            std::unique_ptr<FFT_Bin_Data_Set_t, PsMallocDeleter> sp_FFT_Bin_Data_Set(p_FFT_Bin_Data_Set_raw);
            Update_Left_Bands_And_Send_Result(sp_FFT_Bin_Data_Set->Left_Channel.get(), sp_FFT_Bin_Data_Set->Count);
            Update_Right_Bands_And_Send_Result(sp_FFT_Bin_Data_Set->Right_Channel.get(), sp_FFT_Bin_Data_Set->Count);
        }
        else
        {
          FFT_Results_Processor_Task_Queue_Error_RLL.Log(ESP_LOG_ERROR, "FFT_Result_Processor_Task", "ERROR! Unable to receive queue item.");
        }
        vTaskDelay(pdMS_TO_TICKS(10));
      }
      vTaskDelay(pdMS_TO_TICKS(10));
    }
    else
    {
      ESP_LOGE("FFT_Result_Processor_Task", "FFT_Result_Processor_Queue is not initialized!");
      vTaskDelay(pdMS_TO_TICKS(100));
    }
  }
}

void Sound_Processor::Update_Right_Bands_And_Send_Result(FFT_Bin_Data_t* bin_Data, size_t count)
{
    String message;
    float R_Bands_DataBuffer[NUMBER_OF_BANDS] = {0.0};
    MaxBandSoundData_t R_MaxBand;
    float MaxBandMagnitude = 0.0;
    int16_t MaxBandIndex = 0;

    AssignToBands(R_Bands_DataBuffer, bin_Data, count);
    for(size_t i = 0; i < NUMBER_OF_BANDS; ++i)
    {
      if(i != 0) message += "|";
      message += String(R_Bands_DataBuffer[i]);
      if(R_Bands_DataBuffer[i] > MaxBandMagnitude)
      {
        MaxBandMagnitude = R_Bands_DataBuffer[i];
        MaxBandIndex = i;
      }
    }
    ESP_LOGD("Update_Right_Bands_And_Send_Result", "Right Bands: %s", message.c_str());
    m_R_Bands1.SetValue(R_Bands_DataBuffer, NUMBER_OF_BANDS);
    m_R_Bands3.SetValue(R_Bands_DataBuffer, NUMBER_OF_BANDS);
    R_MaxBand.MaxBandNormalizedPower = MaxBandMagnitude;
    R_MaxBand.MaxBandIndex = MaxBandIndex;
    R_MaxBand.TotalBands = NUMBER_OF_BANDS;
    m_R_Max_Band.SetValue(R_MaxBand);
}

void Sound_Processor::Update_Left_Bands_And_Send_Result(FFT_Bin_Data_t* bin_Data, size_t count)
{
    String message;
    float L_Bands_DataBuffer[NUMBER_OF_BANDS] = {0.0};
    MaxBandSoundData_t L_MaxBand;
    float MaxBandMagnitude = 0.0;
    int16_t MaxBandIndex = 0;

    AssignToBands(L_Bands_DataBuffer, bin_Data, count);
    for(size_t i = 0; i < NUMBER_OF_BANDS; ++i)
    {
      if(i != 0) message += "|";
      message += String(L_Bands_DataBuffer[i]);
      if(L_Bands_DataBuffer[i] > MaxBandMagnitude)
      {
        MaxBandMagnitude = L_Bands_DataBuffer[i];
        MaxBandIndex = i;
      }
    }
    ESP_LOGD("Update_Left_Bands_And_Send_Result", "Left Bands: %s", message.c_str());
    m_L_Bands1.SetValue(L_Bands_DataBuffer, NUMBER_OF_BANDS);
    m_L_Bands3.SetValue(L_Bands_DataBuffer, NUMBER_OF_BANDS);
    L_MaxBand.MaxBandNormalizedPower = MaxBandMagnitude;
    L_MaxBand.MaxBandIndex = MaxBandIndex;
    L_MaxBand.TotalBands = NUMBER_OF_BANDS;
    m_L_Max_Band.SetValue(L_MaxBand);
}

void Sound_Processor::Static_Calculate_Power(void * parameter)
{
  Sound_Processor *aSound_Processor = (Sound_Processor*)parameter;
  aSound_Processor->Calculate_Power();
}
void Sound_Processor::Calculate_Power()
{
  while(true)
  {
    size_t availableFrames = m_Amplitude_AudioBuffer.GetFrameCount();
    if(availableFrames >= AMPLITUDE_BUFFER_FRAME_COUNT)
    {
      std::unique_ptr<Frame_t[], PsMallocDeleter> sp_buffer = std::unique_ptr<Frame_t[], PsMallocDeleter>(static_cast<Frame_t*>(ps_malloc(sizeof(Frame_t*) * AMPLITUDE_BUFFER_FRAME_COUNT)));
      size_t ReadFrameCount = m_Amplitude_AudioBuffer.ReadAudioFrames (sp_buffer.get(), AMPLITUDE_BUFFER_FRAME_COUNT);
      float AmplitudeGain = m_Amplitude_Gain.GetValue();
      ESP_LOGD("Calculate_Power", "Read %i frames of %i. Gain: %f.", ReadFrameCount, availableFrames, AmplitudeGain);
      if(AMPLITUDE_BUFFER_FRAME_COUNT == ReadFrameCount)
      {
        for(int i = 0; i < AMPLITUDE_BUFFER_FRAME_COUNT; ++i)
        {
          bool R_Amplitude_Calculated = false;
          bool L_Amplitude_Calculated = false;
          ProcessedSoundData_t R_PSD;
          ProcessedSoundData_t L_PSD;
          if(m_RightSoundData.PushValueAndCalculateSoundData(sp_buffer[i].channel1, AmplitudeGain))
          {
            R_Amplitude_Calculated = true;
            R_PSD = m_RightSoundData.GetProcessedSoundData();
          }
          if(true == m_LeftSoundData.PushValueAndCalculateSoundData(sp_buffer[i].channel2, AmplitudeGain))
          {
            L_Amplitude_Calculated = true;
            L_PSD = m_LeftSoundData.GetProcessedSoundData();
          }
          assert(R_Amplitude_Calculated == L_Amplitude_Calculated);
          if(true == R_Amplitude_Calculated && true == L_Amplitude_Calculated)
          {
            ProcessedSoundFrame_t PSF_Temp;
            PSF_Temp.Channel1 = R_PSD;
            PSF_Temp.Channel2 = L_PSD;
            PSF.SetValue(PSF_Temp);
          }
        }
      }
    }
  }
}
void Sound_Processor::AssignToBands(float* Band_Data, FFT_Bin_Data_t* bin_Data, size_t count)
{
  String output = "";
  for(int i = 0; i < count; ++i)
  {
    float magnitude = bin_Data[i].NormalizedMagnitude;
    float freq = GetFreqForBin(i);
    int bandIndex = -1;

    //SAE BAND BRAKEDOWN
    /*
    if (freq >= 20 && freq <= 25) bandIndex = 0;
    else if (freq > 25 && freq <= 31.5) bandIndex = 1;
    else if (freq > 31.5 && freq <= 40) bandIndex = 2;
    else if (freq > 40 && freq <= 50) bandIndex = 3;
    else if (freq > 50 && freq <= 63) bandIndex = 4;
    else if (freq > 63 && freq <= 80) bandIndex = 5;
    else if (freq > 80 && freq <= 100) bandIndex = 6;
    else if (freq > 100 && freq <= 125) bandIndex = 7;
    else if (freq > 125 && freq <= 160) bandIndex = 8;
    else if (freq > 160 && freq <= 200) bandIndex = 9;
    else if (freq > 200 && freq <= 250) bandIndex = 10;
    else if (freq > 250 && freq <= 315) bandIndex = 11;
    else if (freq > 315 && freq <= 400) bandIndex = 12;
    else if (freq > 400 && freq <= 500) bandIndex = 13;
    else if (freq > 500 && freq <= 630) bandIndex = 14;
    else if (freq > 630 && freq <= 800) bandIndex = 15;
    else if (freq > 800 && freq <= 1000) bandIndex = 16;
    else if (freq > 1000 && freq <= 1250) bandIndex = 17;
    else if (freq > 1250 && freq <= 1600) bandIndex = 18;
    else if (freq > 1600 && freq <= 2000) bandIndex = 19;
    else if (freq > 2000 && freq <= 2500) bandIndex = 20;
    else if (freq > 2500 && freq <= 3150) bandIndex = 21;
    else if (freq > 3150 && freq <= 4000) bandIndex = 22;
    else if (freq > 4000 && freq <= 5000) bandIndex = 23;
    else if (freq > 5000 && freq <= 6300) bandIndex = 24;
    else if (freq > 6300 && freq <= 8000) bandIndex = 25;
    else if (freq > 8000 && freq <= 10000) bandIndex = 26;
    else if (freq > 10000 && freq <= 12500) bandIndex = 27;
    else if (freq > 12500 && freq <= 16000) bandIndex = 28;
    else if (freq > 16000 && freq <= 20000) bandIndex = 29;
    else if (freq > 20000 && freq <= 25000) bandIndex = 30;
    else if (freq > 25000 && freq <= 32000) bandIndex = 31;
    */
    //MY BAND BRAKEDOWN
    
    if(freq > 0 && freq <= 43) bandIndex = 0;
    else if(freq > 43 && freq <= 86) bandIndex = 1;
    else if(freq > 86 && freq <= 129) bandIndex = 2;
    else if(freq > 129 && freq <= 172) bandIndex = 3;
    else if(freq > 172 && freq <= 215) bandIndex = 4;
    else if(freq > 215 && freq <= 258) bandIndex = 5;
    else if(freq > 258 && freq <= 301) bandIndex = 6;
    else if(freq > 301 && freq <= 344) bandIndex = 7;
    else if(freq > 344 && freq <= 388) bandIndex = 8;
    else if(freq > 388 && freq <= 431) bandIndex = 9;
    else if(freq > 431 && freq <= 474) bandIndex = 10;
    else if(freq > 474 && freq <= 517) bandIndex = 11;
    else if(freq > 517 && freq <= 560) bandIndex = 12;
    else if(freq > 560 && freq <= 603) bandIndex = 13;
    else if(freq > 603 && freq <= 646) bandIndex = 14;
    else if(freq > 646 && freq <= 689) bandIndex = 15;
    else if(freq > 689 && freq <= 800) bandIndex = 16;
    else if(freq > 800 && freq <= 1000) bandIndex = 17;
    else if(freq > 1000 && freq <= 1250) bandIndex = 18;
    else if(freq > 1250 && freq <= 1600) bandIndex = 19;
    else if(freq > 1600 && freq <= 2000) bandIndex = 20;
    else if(freq > 2000 && freq <= 2500) bandIndex = 21;
    else if(freq > 2500 && freq <= 3150) bandIndex = 22;
    else if(freq > 3150 && freq <= 4000) bandIndex = 23;
    else if(freq > 4000 && freq <= 5000) bandIndex = 24;
    else if(freq > 5000 && freq <= 6300) bandIndex = 25;
    else if(freq > 6300 && freq <= 8000) bandIndex = 26;
    else if(freq > 8000 && freq <= 10000) bandIndex = 27;
    else if(freq > 10000 && freq <= 12500) bandIndex = 28;
    else if(freq > 12500 && freq <= 16000) bandIndex = 29;
    else if(freq > 16000 && freq <= 20000) bandIndex = 30;
    else if(freq > 20000 ) bandIndex = 31;
    
    if(bandIndex >= 0 && freq < I2S_SAMPLE_RATE / 2) Band_Data[bandIndex] += magnitude;
    if(Band_Data[bandIndex] > 1.0)
    {
      Band_Data[bandIndex] = 1.0;
    }
  }
  for(int i = 0; i < 32; ++i)
  {
    if(i!=0) output += "|";
    output += String(Band_Data[i]); 
  }
  ESP_LOGD("AssignToBands", "Band Data: %s", output.c_str());
}

float Sound_Processor::GetFreqForBin(int Bin)
{
  return (float)(Bin * ((float)I2S_SAMPLE_RATE / (float)(FFT_SIZE)));
}

int Sound_Processor::GetBinForFrequency(float Frequency)
{
  return ((int)((float)Frequency / ((float)I2S_SAMPLE_RATE / (float)(FFT_SIZE))));
}
