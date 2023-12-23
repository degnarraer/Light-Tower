/*
    Light Tower by Rob Shockency
    Copyright (C) 2021 Rob Shockency degnarraer@yahoo.com

    This program is free software: you can redistribute it and/or modifyVision8_LTT
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

#include "Manager.h"
#include "Tunes.h"
#include "esp_log.h"
#include "DataItem.h"

unsigned long LoopCountTimer = 0;
TaskHandle_t Manager_20mS_Task;
uint32_t Manager_20mS_TaskLoopCount = 0;

TaskHandle_t Manager_1000mS_Task;
uint32_t Manager_1000mS_TaskLoopCount = 0;

TaskHandle_t Manager_300000mS_Task;
uint32_t Manager_300000mS_TaskLoopCount = 0;

TaskHandle_t ProcessSoundPowerTask;
uint32_t ProcessSoundPowerTaskLoopCount = 0;

TaskHandle_t ProcessFFTTask;
uint32_t ProcessFFTTaskLoopCount = 0;

TaskHandle_t ProcessSPI_CPU1_TXTask;
uint32_t ProcessSPI_CPU1_TXTaskLoopCount = 0;

TaskHandle_t ProcessSPI_CPU3_TXTask;
uint32_t ProcessSPI_CPU3_TXTaskLoopCount = 0;

TaskHandle_t TaskMonitorTask;
uint32_t TaskMonitorTaskLoopCount = 0;

I2S_Device m_I2S_In = I2S_Device( "I2S_In"
                                , I2S_NUM_1
                                , i2s_mode_t(I2S_MODE_SLAVE | I2S_MODE_RX)
                                , I2S_SAMPLE_RATE
                                , I2S_BITS_PER_SAMPLE_16BIT
                                , I2S_CHANNEL_FMT_RIGHT_LEFT
                                , i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB)
                                , I2S_CHANNEL_STEREO
                                , true                               // Use APLL
                                , I2S_BUFFER_COUNT                   // Buffer Count
                                , I2S_SAMPLE_COUNT                   // Buffer Size
                                , I2S1_SCLCK_PIN                     // Serial Clock Pin
                                , I2S1_WD_PIN                        // Word Selection Pin
                                , I2S1_SDIN_PIN                      // Serial Data In Pin
                                , I2S1_SDOUT_PIN );                  // Serial Data Out Pin 

BluetoothA2DPSource a2dp_source;
Bluetooth_Source m_BT_Out = Bluetooth_Source( "Bluetooth Source"
                                            , a2dp_source );
                                                         

ContinuousAudioBuffer<AUDIO_BUFFER_SIZE> m_AudioBuffer;                                            

DataSerializer m_DataSerializer;
SerialPortMessageManager m_CPU1SerialPortMessageManager = SerialPortMessageManager("CPU1", Serial1, m_DataSerializer);
SerialPortMessageManager m_CPU3SerialPortMessageManager = SerialPortMessageManager("CPU3", Serial2, m_DataSerializer);


Sound_Processor m_SoundProcessor = Sound_Processor( "Sound Processor"
                                                  , m_AudioBuffer
                                                  , m_CPU1SerialPortMessageManager
                                                  , m_CPU3SerialPortMessageManager );                                            

Manager m_Manager = Manager( "Manager"
                           , m_SoundProcessor
                           , m_CPU1SerialPortMessageManager
                           , m_CPU3SerialPortMessageManager
                           , m_BT_Out
                           , m_I2S_In
                           , m_AudioBuffer );


int32_t SetBTTxData(uint8_t *Data, int32_t channel_len)
{
  return m_Manager.SetBTTxData(Data, channel_len);
}

static bool ConnectToThisSSID(const char* ssid, esp_bd_addr_t address, int32_t rssi)
{
  return m_BT_Out.ConnectToThisSSID(ssid, address, rssi);
}
void setup() 
{
  //PC Serial Communication
  Serial.begin(500000);
  Serial1.begin(500000, SERIAL_8N1, CPU1_RX, CPU1_TX);
  Serial2.begin(500000, SERIAL_8N1, CPU3_RX, CPU3_TX);
  
  m_CPU1SerialPortMessageManager.SetupSerialPortMessageManager();
  m_CPU3SerialPortMessageManager.SetupSerialPortMessageManager();
  
  m_I2S_In.Setup();
  a2dp_source.set_ssid_callback(ConnectToThisSSID);
  m_BT_Out.Setup();
  m_BT_Out.SetMusicDataCallback(SetBTTxData);
  m_Manager.Setup();
  
  //xTaskCreatePinnedToCore( ProcessFFTTaskLoop,        "ProcessFFTTask",         5000,   NULL,   configMAX_PRIORITIES - 1,   &ProcessFFTTask,          0 );
  //xTaskCreatePinnedToCore( Manager_20mS_TaskLoop,     "Manager_20mS_Task",      5000,   NULL,   configMAX_PRIORITIES - 1,   &Manager_20mS_Task,       1 );
  //xTaskCreatePinnedToCore( Manager_1000mS_TaskLoop,   "Manager_1000mS_Task",    5000,   NULL,   configMAX_PRIORITIES - 3,   &Manager_1000mS_Task,     1 );
  //xTaskCreatePinnedToCore( Manager_300000mS_TaskLoop, "Manager_300000mS_Task",  5000,   NULL,   configMAX_PRIORITIES - 3,   &Manager_300000mS_Task,   1 );
  //xTaskCreatePinnedToCore( ProcessSoundPowerTaskLoop, "ProcessSoundPowerTask",  5000,   NULL,   configMAX_PRIORITIES - 4,   &ProcessSoundPowerTask,   1 );
  //xTaskCreatePinnedToCore( TaskMonitorTaskLoop,       "TaskMonitorTask",        5000,   NULL,   configMAX_PRIORITIES - 4,   &TaskMonitorTask,         1 );
  
  ESP_LOGE("LED_Controller2", "Xtal Clock Frequency: %i MHz", getXtalFrequencyMhz());
  ESP_LOGE("LED_Controller2", "CPU Clock Frequency: %i MHz", getCpuFrequencyMhz());
  ESP_LOGE("LED_Controller2", "Apb Clock Frequency: %i Hz", getApbFrequency());
  ESP_LOGE("LED_Controller_CPU2", "Total heap: %d", ESP.getHeapSize());
  ESP_LOGE("LED_Controller_CPU2", "Free heap: %d", ESP.getFreeHeap());
  ESP_LOGE("LED_Controller_CPU2", "Total PSRAM: %d", ESP.getPsramSize());
  ESP_LOGE("LED_Controller_CPU2", "Free PSRAM: %d", ESP.getFreePsram());
}

void loop()
{
}

void ProcessSoundPowerTaskLoop(void * parameter)
{
  const TickType_t xFrequency = 20;
  TickType_t xLastWakeTime = xTaskGetTickCount();
  while(true)
  {
    yield();
    delay(1); //allow idle task to run for Watchdog
    vTaskDelayUntil( &xLastWakeTime, xFrequency );
    ++ProcessSoundPowerTaskLoopCount;
    m_SoundProcessor.ProcessSoundPower();
  }
}

void ProcessFFTTaskLoop(void * parameter)
{
  while(true)
  {
    ++ProcessFFTTaskLoopCount;
    m_SoundProcessor.ProcessFFT();
    vTaskDelay(20/portTICK_PERIOD_MS);
  }
}

void Manager_20mS_TaskLoop(void * parameter)
{
  const TickType_t xFrequency = 10;
  TickType_t xLastWakeTime = xTaskGetTickCount();
  while(true)
  {
    vTaskDelayUntil( &xLastWakeTime, xFrequency );
    ++Manager_20mS_TaskLoopCount;
    m_Manager.ProcessEventQueue20mS();
  }
}

void Manager_1000mS_TaskLoop(void * parameter)
{
  const TickType_t xFrequency = 1000;
  TickType_t xLastWakeTime = xTaskGetTickCount();
  while(true)
  {
    vTaskDelayUntil( &xLastWakeTime, xFrequency );
    ++Manager_1000mS_TaskLoopCount;
    m_Manager.ProcessEventQueue1000mS();
  }
}

void Manager_300000mS_TaskLoop(void * parameter)
{
  const TickType_t xFrequency = 300000;
  TickType_t xLastWakeTime = xTaskGetTickCount();
  while(true)
  {
    vTaskDelayUntil( &xLastWakeTime, xFrequency );
    ++Manager_300000mS_TaskLoopCount;
    m_Manager.ProcessEventQueue300000mS();
  }
}

void TaskMonitorTaskLoop(void * parameter)
{
  const TickType_t xFrequency = 5000;
  TickType_t xLastWakeTime = xTaskGetTickCount();
  while(true)
  {
    vTaskDelayUntil( &xLastWakeTime, xFrequency );
    unsigned long CurrentTime = millis();
    ++TaskMonitorTaskLoopCount;
    if(true == TASK_LOOP_COUNT_DEBUG)
    {
      unsigned long DeltaTimeSeconds = (CurrentTime - LoopCountTimer) / 1000;
      ESP_LOGE("LED_Controller1", "ProcessSoundPowerTaskLoopCount: %f", (float)ProcessSoundPowerTaskLoopCount/(float)DeltaTimeSeconds);
      ESP_LOGE("LED_Controller1", "ProcessFFTTaskLoopCount: %f", (float)ProcessFFTTaskLoopCount/(float)DeltaTimeSeconds);
      ESP_LOGE("LED_Controller1", "Manager_20mS_TaskLoopCount: %f", (float)Manager_20mS_TaskLoopCount/(float)DeltaTimeSeconds);
      ESP_LOGE("LED_Controller1", "Manager_1000mS_TaskLoopCount: %f", (float)Manager_1000mS_TaskLoopCount/(float)DeltaTimeSeconds);
      ESP_LOGE("LED_Controller1", "Manager_300000mS_TaskLoopCount: %f", (float)Manager_300000mS_TaskLoopCount/(float)DeltaTimeSeconds);
      ESP_LOGE("LED_Controller1", "ProcessSPI_CPU1_TXTaskLoopCount: %f", (float)ProcessSPI_CPU1_TXTaskLoopCount/(float)DeltaTimeSeconds);
      ESP_LOGE("LED_Controller1", "ProcessSPI_CPU3_TXTaskLoopCount: %f", (float)ProcessSPI_CPU3_TXTaskLoopCount/(float)DeltaTimeSeconds);
      ESP_LOGE("LED_Controller1", "TaskMonitorTaskLoopCount: %f", (float)TaskMonitorTaskLoopCount/(float)DeltaTimeSeconds);
      ProcessSoundPowerTaskLoopCount = 0;
      ProcessFFTTaskLoopCount = 0;
      Manager_20mS_TaskLoopCount = 0;
      Manager_1000mS_TaskLoopCount = 0;
      Manager_300000mS_TaskLoopCount = 0;
      ProcessSPI_CPU1_TXTaskLoopCount = 0;
      ProcessSPI_CPU3_TXTaskLoopCount = 0;
      TaskMonitorTaskLoopCount = 0;
    }

    size_t StackSizeThreshold = 100;
    if( uxTaskGetStackHighWaterMark(Manager_20mS_Task) < StackSizeThreshold )ESP_LOGW("LED_Controller2", "WARNING! Manager_20mS_Task: Stack Size Low");
    if( uxTaskGetStackHighWaterMark(Manager_1000mS_Task) < StackSizeThreshold )ESP_LOGW("LED_Controller2", "WARNING! Manager_1000mS_Task: Stack Size Low");
    if( uxTaskGetStackHighWaterMark(Manager_300000mS_Task) < StackSizeThreshold )ESP_LOGW("LED_Controller2", "WARNING! Manager_300000mS_Task: Stack Size Low");
    if( uxTaskGetStackHighWaterMark(ProcessSoundPowerTask) < StackSizeThreshold )ESP_LOGW("LED_Controller2", "WARNING! ProcessSoundPowerTask: Stack Size Low");
    if( uxTaskGetStackHighWaterMark(ProcessFFTTask) < StackSizeThreshold )ESP_LOGW("LED_Controller2", "WARNING! ProcessFFTTask: Stack Size Low");
    if( uxTaskGetStackHighWaterMark(ProcessSPI_CPU1_TXTask) < StackSizeThreshold )ESP_LOGW("LED_Controller2", "WARNING! ProcessSPI_CPU1_TXTask: Stack Size Low");
    if( uxTaskGetStackHighWaterMark(ProcessSPI_CPU3_TXTask) < StackSizeThreshold )ESP_LOGW("LED_Controller2", "WARNING! ProcessSPI_CPU3_TXTask: Stack Size Low");
    if( uxTaskGetStackHighWaterMark(TaskMonitorTask) < StackSizeThreshold )ESP_LOGW("LED_Controller2", "WARNING! TaskMonitorTask: Stack Size Low");
    
    if(true == TASK_STACK_SIZE_DEBUG)
    {
      ESP_LOGE("LED_Controller2", "Manager_20mS_Task Free Heap: %i", uxTaskGetStackHighWaterMark(Manager_20mS_Task));
      ESP_LOGE("LED_Controller2", "Manager_1000mS_Task Free Heap: %i", uxTaskGetStackHighWaterMark(Manager_1000mS_Task));
      ESP_LOGE("LED_Controller2", "Manager_300000mS_Task Free Heap: %i", uxTaskGetStackHighWaterMark(Manager_300000mS_Task));
      ESP_LOGE("LED_Controller2", "ProcessSoundPowerTask Free Heap: %i", uxTaskGetStackHighWaterMark(ProcessSoundPowerTask));
      ESP_LOGE("LED_Controller2", "ProcessFFTTask Free Heap: %i", uxTaskGetStackHighWaterMark(ProcessFFTTask));
      ESP_LOGE("LED_Controller2", "ProcessSPI_CPU1_TXTask Free Heap: %i", uxTaskGetStackHighWaterMark(ProcessSPI_CPU1_TXTask));
      ESP_LOGE("LED_Controller2", "ProcessSPI_CPU3_TXTask Free Heap: %i", uxTaskGetStackHighWaterMark(ProcessSPI_CPU3_TXTask));
      ESP_LOGE("LED_Controller2", "TaskMonitorTask Free Heap: %i", uxTaskGetStackHighWaterMark(TaskMonitorTask));
    }
    LoopCountTimer = CurrentTime;
  }
}
