#include "Manager.h"
#include "Tunes.h"
#include "esp_log.h"

unsigned long LoopCountTimer = 0;
TaskHandle_t ManagerTask;
uint32_t ManagerTaskLoopCount = 0;

TaskHandle_t ProcessSoundPowerTask;
uint32_t ProcessSoundPowerTaskLoopCount = 0;

TaskHandle_t ProcessFFTTask;
uint32_t ProcessFFTTaskLoopCount = 0;

TaskHandle_t ProcessSPITXTask;
uint32_t ProcessSPITXTaskLoopCount = 0;

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
                                            , a2dp_source
                                            , "JBL Flip 6" );

SPIDataLinkMaster m_SPIDataLinkMaster = SPIDataLinkMaster( "SPI Datalink"
                                                         , SPI1_PIN_SCK
                                                         , SPI1_PIN_MISO
                                                         , SPI1_PIN_MOSI
                                                         , SPI1_PIN_SS
                                                         , 1 );

ContinuousAudioBuffer<AUDIO_BUFFER_SIZE> m_AudioBuffer;                                            
Sound_Processor m_SoundProcessor = Sound_Processor( "Sound Processor"
                                                  , m_SPIDataLinkMaster
                                                  , m_AudioBuffer );                                            
Manager m_Manager = Manager("Manager"
                           , m_SoundProcessor
                           , m_SPIDataLinkMaster
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
  Serial.flush();
  Serial.begin(500000);
  Serial.flush();
  
  ESP_LOGV("LED_Controller2", "%s, ", __func__);
  ESP_LOGE("LED_Controller2", "Serial Datalink Configured");
  ESP_LOGE("LED_Controller2", "Xtal Clock Frequency: %i MHz", getXtalFrequencyMhz());
  ESP_LOGE("LED_Controller2", "CPU Clock Frequency: %i MHz", getCpuFrequencyMhz());
  ESP_LOGE("LED_Controller2", "Apb Clock Frequency: %i Hz", getApbFrequency());
  
  m_I2S_In.Setup();
  a2dp_source.set_ssid_callback(ConnectToThisSSID);
  m_BT_Out.Setup();
  m_BT_Out.SetMusicDataCallback(SetBTTxData);
  m_SPIDataLinkMaster.SetupSPIDataLink();
  m_Manager.Setup();

  xTaskCreatePinnedToCore( ProcessFFTTaskLoop,        "ProcessFFTTask",         5000,   NULL,   0,                           &ProcessFFTTask,          0 );
  xTaskCreatePinnedToCore( ProcessSoundPowerTaskLoop, "ProcessSoundPowerTask",  3000,   NULL,   configMAX_PRIORITIES - 10,   &ProcessSoundPowerTask,   0 );
  xTaskCreatePinnedToCore( SPI_TX_TaskLoop,           "SPI TX Task Task",       2000,   NULL,   configMAX_PRIORITIES - 10,   &ProcessSPITXTask,        1 );
  xTaskCreatePinnedToCore( ManagerTaskLoop,           "ManagerTask",            1000,   NULL,   configMAX_PRIORITIES - 10,   &ManagerTask,             1 );
  xTaskCreatePinnedToCore( TaskMonitorTaskLoop,       "TaskMonitorTaskTask",    2000,   NULL,   configMAX_PRIORITIES - 1,    &TaskMonitorTask,         1 );
  
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
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xFrequency = 10; //delay for mS
  while(true)
  {
    yield();
    ++ProcessSoundPowerTaskLoopCount;
    m_SoundProcessor.ProcessSoundPower();
    vTaskDelayUntil( &xLastWakeTime, xFrequency );
  }
}

void ProcessFFTTaskLoop(void * parameter)
{
  while(true)
  {
    yield();
    ++ProcessFFTTaskLoopCount;
    m_SoundProcessor.ProcessFFT();
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
}

void ManagerTaskLoop(void * parameter)
{
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xFrequency = 10; //delay for mS
  while(true)
  {
    yield();
    ++ManagerTaskLoopCount;
    m_Manager.ProcessEventQueue();
    vTaskDelayUntil( &xLastWakeTime, xFrequency );
  }
}

void SPI_TX_TaskLoop(void * parameter)
{
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xFrequency = 10; //delay for mS
  while(true)
  {
    yield();
    ++ProcessSPITXTaskLoopCount;
    m_SPIDataLinkMaster.ProcessDataTXEventQueue();
    vTaskDelayUntil( &xLastWakeTime, xFrequency );
  }
}

void TaskMonitorTaskLoop(void * parameter)
{
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xFrequency = 5000; //delay for mS
  while(true)
  {
    yield();
    unsigned long CurrentTime = millis();
    ++TaskMonitorTaskLoopCount;
    if(true == TASK_LOOP_COUNT_DEBUG)
    {
      unsigned long DeltaTimeSeconds = (CurrentTime - LoopCountTimer) / 1000;
      ESP_LOGE("LED_Controller1", "ProcessSoundPowerTaskLoopCount: %f", (float)ProcessSoundPowerTaskLoopCount/(float)DeltaTimeSeconds);
      ESP_LOGE("LED_Controller1", "ProcessFFTTaskLoopCount: %f", (float)ProcessFFTTaskLoopCount/(float)DeltaTimeSeconds);
      ESP_LOGE("LED_Controller1", "ManagerTaskLoopCount: %f", (float)ManagerTaskLoopCount/(float)DeltaTimeSeconds);
      ESP_LOGE("LED_Controller1", "ProcessSPITXTaskLoopCount: %f", (float)ProcessSPITXTaskLoopCount/(float)DeltaTimeSeconds);
      ESP_LOGE("LED_Controller1", "TaskMonitorTaskLoopCount: %f", (float)TaskMonitorTaskLoopCount/(float)DeltaTimeSeconds);
      ProcessSoundPowerTaskLoopCount = 0;
      ProcessFFTTaskLoopCount = 0;
      ManagerTaskLoopCount = 0;
      ProcessSPITXTaskLoopCount = 0;
      TaskMonitorTaskLoopCount = 0;
    }

    size_t StackSizeThreshold = 100;
    if( uxTaskGetStackHighWaterMark(ManagerTask) < StackSizeThreshold )ESP_LOGW("LED_Controller2", "WARNING! ManagerTask: Stack Size Low");
    if( uxTaskGetStackHighWaterMark(ProcessSoundPowerTask) < StackSizeThreshold )ESP_LOGW("LED_Controller2", "WARNING! ProcessSoundPowerTask: Stack Size Low");
    if( uxTaskGetStackHighWaterMark(ProcessFFTTask) < StackSizeThreshold )ESP_LOGW("LED_Controller2", "WARNING! ProcessFFTTask: Stack Size Low");
    if( uxTaskGetStackHighWaterMark(ProcessSPITXTask) < StackSizeThreshold )ESP_LOGW("LED_Controller2", "WARNING! ProcessSPITXTask: Stack Size Low");
    if( uxTaskGetStackHighWaterMark(TaskMonitorTask) < StackSizeThreshold )ESP_LOGW("LED_Controller2", "WARNING! TaskMonitorTask: Stack Size Low");
    
    if(true == TASK_STACK_SIZE_DEBUG)
    {
      ESP_LOGE("LED_Controller2", "ManagerTask Free Heap: %i", uxTaskGetStackHighWaterMark(ManagerTask));
      ESP_LOGE("LED_Controller2", "ProcessSoundPowerTask Free Heap: %i", uxTaskGetStackHighWaterMark(ProcessSoundPowerTask));
      ESP_LOGE("LED_Controller2", "ProcessFFTTask Free Heap: %i", uxTaskGetStackHighWaterMark(ProcessFFTTask));
      ESP_LOGE("LED_Controller2", "ProcessSPITXTask Free Heap: %i", uxTaskGetStackHighWaterMark(ProcessSPITXTask));
      ESP_LOGE("LED_Controller2", "TaskMonitorTask Free Heap: %i", uxTaskGetStackHighWaterMark(TaskMonitorTask));
    }
    LoopCountTimer = CurrentTime;
    vTaskDelayUntil( &xLastWakeTime, xFrequency );
  }
}
