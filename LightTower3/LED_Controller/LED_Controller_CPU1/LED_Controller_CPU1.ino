#include "Manager.h"
#include "Tunes.h"
#include "VisualizationPlayer.h"
#include "Models.h"
#include "Tunes.h"
#include "esp_log.h"

unsigned long LoopCountTimer = 0;
TaskHandle_t DataMoverTask;
uint32_t DataMoveTaskLoopCount = 0;

TaskHandle_t VisualizationTask;
uint32_t VisualizationTaskLoopCount = 0;

TaskHandle_t TaskMonitorTask;
uint32_t TaskMonitorTaskLoopCount = 0;

TaskHandle_t SPI_RX_Task;
uint32_t SPI_RX_TaskLoopCount = 0;

BluetoothA2DPSink m_BTSink;
Bluetooth_Sink m_BT_In = Bluetooth_Sink( "Bluetooth"
                                       , m_BTSink
                                       , "LED Tower of Power"
                                       , I2S_NUM_1                          // I2S Interface
                                       , i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_TX)
                                       , I2S_SAMPLE_RATE
                                       , I2S_BITS_PER_SAMPLE_16BIT
                                       , I2S_CHANNEL_FMT_RIGHT_LEFT
                                       , i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB)
                                       , I2S_CHANNEL_STEREO
                                       , true                               // Use APLL                                    
                                       , I2S_BUFFER_COUNT                   // Buffer Count
                                       , I2S_CHANNEL_SAMPLE_COUNT           // Buffer Size
                                       , I2S2_SCLCK_PIN                     // Serial Clock Pin
                                       , I2S2_WD_PIN                        // Word Selection Pin
                                       , I2S2_SDIN_PIN                      // Serial Data In Pin
                                       , I2S2_SDOUT_PIN );                  // Serial Data Out Pin
                                    
//Callbacks for Bluetooth Sink
void data_received_callback() 
{
  m_BT_In.data_received_callback();
}

//Callbacks for Bluetooth Sink
void read_data_stream(const uint8_t *data, uint32_t length)
{
  m_BT_In.read_data_stream(data, length);
}

//Callbacks for Bluetooth Sink
// for esp_a2d_connection_state_t see https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/bluetooth/esp_a2dp.html#_CPPv426esp_a2d_connection_state_t
void connection_state_changed(esp_a2d_connection_state_t state, void *ptr)
{
  ESP_LOGD("Startup", "State: %s", m_BTSink.to_str(state));
}

//Callbacks for Bluetooth Sink
// for esp_a2d_audio_state_t see https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/bluetooth/esp_a2dp.html#_CPPv421esp_a2d_audio_state_t
void audio_state_changed(esp_a2d_audio_state_t state, void *ptr)
{
  ESP_LOGD("Startup", "State: %s", m_BTSink.to_str(state));
}


I2S_Device m_Mic_In = I2S_Device( "Microphone In"
                                , I2S_NUM_0                          // I2S Interface
                                , i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX)
                                , I2S_SAMPLE_RATE
                                , I2S_BITS_PER_SAMPLE_16BIT
                                , I2S_CHANNEL_FMT_RIGHT_LEFT
                                , i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB)
                                , I2S_CHANNEL_STEREO
                                , true                               // Use APLL
                                , I2S_BUFFER_COUNT                   // Buffer Count
                                , I2S_CHANNEL_SAMPLE_COUNT           // Buffer Size
                                , I2S1_SCLCK_PIN                     // Serial Clock Pin
                                , I2S1_WD_PIN                        // Word Selection Pin
                                , I2S1_SDIN_PIN                      // Serial Data In Pin
                                , I2S1_SDOUT_PIN );                  // Serial Data Out Pin

I2S_Device m_I2S_Out = I2S_Device( "I2S Out"
                                 , I2S_NUM_1                        // I2S Interface
                                 , i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_TX)
                                 , I2S_SAMPLE_RATE
                                 , I2S_BITS_PER_SAMPLE_16BIT
                                 , I2S_CHANNEL_FMT_RIGHT_LEFT
                                 , i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB)
                                 , I2S_CHANNEL_STEREO
                                 , true                            // Use APLL
                                 , I2S_BUFFER_COUNT                // Buffer Count
                                 , I2S_CHANNEL_SAMPLE_COUNT        // Buffer Size
                                 , I2S2_SCLCK_PIN                  // Serial Clock Pin
                                 , I2S2_WD_PIN                     // Word Selection Pin
                                 , I2S2_SDIN_PIN                   // Serial Data In Pin
                                 , I2S2_SDOUT_PIN );               // Serial Data Out Pin
                                  
StatisticalEngine m_StatisticalEngine = StatisticalEngine();
StatisticalEngineModelInterface m_StatisticalEngineModelInterface = StatisticalEngineModelInterface(m_StatisticalEngine);
VisualizationPlayer m_VisualizationPlayer = VisualizationPlayer(m_StatisticalEngineModelInterface);

SPIDataLinkSlave m_SPIDataLinkSlave = SPIDataLinkSlave( "SPI Datalink"
                                                      , SPI1_PIN_SCK
                                                      , SPI1_PIN_MISO
                                                      , SPI1_PIN_MOSI
                                                      , SPI1_PIN_SS
                                                      , 2 );
                                                      
CalculateFPS m_CalculateFPS("Main Loop", 1000);
TaskScheduler m_Scheduler;

Manager m_Manager = Manager("Manager", m_StatisticalEngine
                           , m_SPIDataLinkSlave
                           , m_BT_In
                           , m_Mic_In
                           , m_I2S_Out);
                     
void setup()
{
  //PC Serial Communication
  Serial.flush();
  Serial.begin(500000);
  Serial.flush();
     
  //PC Serial Communication
  ESP_LOGV("LED_Controller1", "%s, ", __func__);
  ESP_LOGI("LED_Controller1", "Serial Datalink Configured");
  ESP_LOGI("LED_Controller1", "Xtal Clock Frequency: %i MHz", getXtalFrequencyMhz());
  ESP_LOGI("LED_Controller1", "CPU Clock Frequency: %i MHz", getCpuFrequencyMhz());
  ESP_LOGI("LED_Controller1", "Apb Clock Frequency: %i Hz", getApbFrequency());
  
  m_BTSink.set_stream_reader(read_data_stream, true);
  m_BTSink.set_on_data_received(data_received_callback);  
  m_Manager.Setup();
  m_SPIDataLinkSlave.SetupSPIDataLink();
  m_Scheduler.AddTask(m_CalculateFPS);
  m_Scheduler.AddTask(m_StatisticalEngineModelInterface);
  m_Scheduler.AddTask(m_VisualizationPlayer);

  xTaskCreatePinnedToCore( DataMoverTaskLoop,     "DataMoverTask",        2000,  NULL,   configMAX_PRIORITIES - 10,   &DataMoverTask,       0 );
  xTaskCreatePinnedToCore( TaskMonitorTaskLoop,   "TaskMonitorTaskTask",  2000,  NULL,   configMAX_PRIORITIES - 1,    &TaskMonitorTask,     0 );
  xTaskCreatePinnedToCore( SPI_RX_TaskLoop,       "SPI_RX_Task",          3000,  NULL,   1,                           &SPI_RX_Task,         0 );
  xTaskCreatePinnedToCore( VisualizationTaskLoop, "VisualizationTask",    4000,  NULL,   2,                           &VisualizationTask,   1 ); //This has to be core 1 for some reason else bluetooth interfeeres with LEDs and makes them flicker
  
  ESP_LOGE("LED_Controller_CPU1", "Total heap: %d", ESP.getHeapSize());
  ESP_LOGE("LED_Controller_CPU1", "Free heap: %d", ESP.getFreeHeap());
  ESP_LOGE("LED_Controller_CPU1", "Total PSRAM: %d", ESP.getPsramSize());
  ESP_LOGE("LED_Controller_CPU1", "Free PSRAM: %d", ESP.getFreePsram());
}

void loop()
{
}

void VisualizationTaskLoop(void * parameter)
{
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xFrequency = 10; //delay for mS
  while(true)
  {
    ++VisualizationTaskLoopCount;
    m_Scheduler.RunScheduler();
    vTaskDelayUntil( &xLastWakeTime, xFrequency );
  }
}

void SPI_RX_TaskLoop(void * parameter)
{
  while(true)
  {
    yield();
    ++SPI_RX_TaskLoopCount;
    m_SPIDataLinkSlave.ProcessDataRXEventQueue();
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }  
}

void TaskMonitorTaskLoop(void * parameter)
{
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xFrequency = 5000; //delay for mS
  while(true)
  {
    unsigned long CurrentTime = millis();
    ++TaskMonitorTaskLoopCount;

    if(true == TASK_LOOP_COUNT_DEBUG)
    {
      unsigned long DeltaTimeSeconds = (CurrentTime - LoopCountTimer) / 1000;
      ESP_LOGE("LED_Controller1", "DataMoveTaskLoopCount: %f", (float)DataMoveTaskLoopCount/(float)DeltaTimeSeconds);
      ESP_LOGE("LED_Controller1", "VisualizationTaskLoopCount: %f", (float)VisualizationTaskLoopCount/(float)DeltaTimeSeconds);
      ESP_LOGE("LED_Controller1", "TaskMonitorTaskLoopCount: %f", (float)TaskMonitorTaskLoopCount/(float)DeltaTimeSeconds);
      ESP_LOGE("LED_Controller1", "SPI_RX_TaskLoopCount: %f", (float)SPI_RX_TaskLoopCount/(float)DeltaTimeSeconds);
      DataMoveTaskLoopCount = 0;
      VisualizationTaskLoopCount = 0;
      TaskMonitorTaskLoopCount = 0;
      SPI_RX_TaskLoopCount = 0;
    }

    size_t StackSizeThreshold = 100;
    if( uxTaskGetStackHighWaterMark(DataMoverTask) < StackSizeThreshold )ESP_LOGW("LED_Controller1", "WARNING! DataMoverTask: Stack Size Low");
    if( uxTaskGetStackHighWaterMark(VisualizationTask) < StackSizeThreshold )ESP_LOGW("LED_Controller1", "WARNING! VisualizationTask: Stack Size Low");
    
    if(true == TASK_STACK_SIZE_DEBUG)
    {
      ESP_LOGE("LED_Controller1", "TaskMonitorTaskTask Free Heap: %i", uxTaskGetStackHighWaterMark(TaskMonitorTask));
      ESP_LOGE("LED_Controller1", "DataMoverTaskTask Free Heap: %i", uxTaskGetStackHighWaterMark(DataMoverTask));
      ESP_LOGE("LED_Controller1", "SPI_RX_Task Free Heap: %i", uxTaskGetStackHighWaterMark(SPI_RX_Task));
      ESP_LOGE("LED_Controller1", "VisualizationTask Free Heap: %i", uxTaskGetStackHighWaterMark(VisualizationTask));
    }
    LoopCountTimer = CurrentTime;
    vTaskDelayUntil( &xLastWakeTime, xFrequency );
  }
}

void DataMoverTaskLoop(void * parameter)
{
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xFrequency = 10; //delay for mS
  while(true)
  {
    ++DataMoveTaskLoopCount;
    m_Manager.ProcessEventQueue();
    vTaskDelayUntil( &xLastWakeTime, xFrequency );
  }
}
