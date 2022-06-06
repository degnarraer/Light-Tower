#include "Manager.h"
#include "Tunes.h"
#include "VisualizationPlayer.h"
#include "Models.h"
#include "Tunes.h"

#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include "esp_log.h"

TaskHandle_t DataMoverTask;
TaskHandle_t SerialDataLinkTXTask;
TaskHandle_t SerialDataLinkRXTask;
TaskHandle_t VisualizationTask;

BluetoothA2DPSink m_BTSink;
Bluetooth_Sink m_BT_In = Bluetooth_Sink( "Bluetooth"
                                       , m_BTSink
                                       , "LED Tower of Power"
                                       , I2S_NUM_1                          // I2S Interface
                                       , i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_TX)
                                       , I2S_SAMPLE_RATE
                                       , I2S_BITS_PER_SAMPLE_32BIT
                                       , I2S_CHANNEL_FMT_RIGHT_LEFT
                                       , i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB)
                                       , I2S_CHANNEL_STEREO
                                       , true                               // Use APLL                                    
                                       , I2S_BUFFER_COUNT                   // Buffer Count
                                       , I2S_CHANNEL_SAMPLE_COUNT           // Buffer Size
                                       , I2S_PIN_NO_CHANGE                  // Serial Clock Pin
                                       , I2S_PIN_NO_CHANGE                  // Word Selection Pin
                                       , I2S_PIN_NO_CHANGE                  // Serial Data In Pin
                                       , I2S_PIN_NO_CHANGE );               // Serial Data Out Pin
                                    
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
  ESP_LOGV("Debug", "%s, ", __func__);
  ESP_LOGD("Startup", "State: %s", m_BTSink.to_str(state));
}

//Callbacks for Bluetooth Sink
// for esp_a2d_audio_state_t see https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/bluetooth/esp_a2dp.html#_CPPv421esp_a2d_audio_state_t
void audio_state_changed(esp_a2d_audio_state_t state, void *ptr)
{
  ESP_LOGV("Debug", "%s, ", __func__);
  ESP_LOGD("Startup", "State: %s", m_BTSink.to_str(state));
}


I2S_Device m_Mic_In = I2S_Device( "Microphone In"
                                , I2S_NUM_0                          // I2S Interface
                                , i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX)
                                , I2S_SAMPLE_RATE
                                , I2S_BITS_PER_SAMPLE_32BIT
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
                                  , I2S_BITS_PER_SAMPLE_32BIT
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
HardwareSerial m_hSerial = Serial1;
SerialDataLink m_SerialDataLink = SerialDataLink("Serial Datalink", m_hSerial);
CalculateFPS m_CalculateFPS("Main Loop", 1000);
TaskScheduler m_Scheduler;
Manager m_Manager = Manager("Manager"
                           , m_StatisticalEngine
                           , m_SerialDataLink
                           , m_BT_In
                           , m_Mic_In
                           , m_I2S_Out);
                           
void setup()
{
  //ESP32 Serial Communication
  m_hSerial.setRxBufferSize(1000);
  m_hSerial.flush();
  m_hSerial.begin(300000, SERIAL_8E2, HARDWARE_SERIAL_RX_PIN, HARDWARE_SERIAL_TX_PIN); // pins rx2, tx2, 9600 bps, 8 bits no parity 1 stop bit
  m_hSerial.flush();
  
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
  m_SerialDataLink.SetupSerialDataLink();
  m_Scheduler.AddTask(m_CalculateFPS);
  m_Scheduler.AddTask(m_StatisticalEngineModelInterface);
  m_Scheduler.AddTask(m_VisualizationPlayer);
  
  
  xTaskCreatePinnedToCore
  (
    VisualizationTaskLoop,        // Function to implement the task
    "VisualizationTask",          // Name of the task
    20000,                        // Stack size in words
    NULL,                         // Task input parameter
    configMAX_PRIORITIES - 1,     // Priority of the task
    &VisualizationTask,           // Task handle.
    1                             // Core where the task should run
  );
  
  xTaskCreatePinnedToCore
  (
    DataMoverTaskLoop,            // Function to implement the task
    "DataMoverTask",              // Name of the task
    4000,                         // Stack size in words
    NULL,                         // Task input parameter
    configMAX_PRIORITIES - 2,     // Priority of the task
    &DataMoverTask,               // Task handle.
    1                             // Core where the task should run
  );
  
  xTaskCreatePinnedToCore
  (
    SerialDataLinkTXTaskLoop,   // Function to implement the task
    "SerialDataLinkSendTask",   // Name of the task
    4000,                       // Stack size in words
    NULL,                       // Task input parameter
    configMAX_PRIORITIES - 3,   // Priority of the task
    &SerialDataLinkTXTask,      // Task handle.
    1                           // Core where the task should run
  );     
  
  xTaskCreatePinnedToCore
  (
    SerialDataLinkRXTaskLoop,   // Function to implement the task
    "SerialDataLinkRXTask",     // Name of the task
    4000,                       // Stack size in words
    NULL,                       // Task input parameter
    configMAX_PRIORITIES - 3,   // Priority of the task
    &SerialDataLinkRXTask,      // Task handle.
    1                           // Core where the task should run
  );
  
  ESP_LOGE("LED_Controller_CPU1", "Total heap: %d", ESP.getHeapSize());
  ESP_LOGE("LED_Controller_CPU1", "Free heap: %d", ESP.getFreeHeap());
  ESP_LOGE("LED_Controller_CPU1", "Total PSRAM: %d", ESP.getPsramSize());
  ESP_LOGE("LED_Controller_CPU1", "Free PSRAM: %d", ESP.getFreePsram());
}

unsigned long myTime = millis();
void loop() {
  // put your main code here, to run repeatedly:
  if(millis() - myTime > 10000)
  {
    myTime = millis();
    size_t StackSizeThreshold = 100;
    if( uxTaskGetStackHighWaterMark(DataMoverTask) < StackSizeThreshold )ESP_LOGW("LED_Controller1", "WARNING! DataMoverTask: Stack Size Low");
    if( uxTaskGetStackHighWaterMark(SerialDataLinkTXTask) < StackSizeThreshold )ESP_LOGW("LED_Controller1", "WARNING! SerialDataLinkTXTask: Stack Size Low");
    if( uxTaskGetStackHighWaterMark(SerialDataLinkRXTask) < StackSizeThreshold )ESP_LOGW("LED_Controller1", "WARNING! SerialDataLinkRXTask: Stack Size Low");
    if( uxTaskGetStackHighWaterMark(VisualizationTask) < StackSizeThreshold )ESP_LOGW("LED_Controller1", "WARNING! VisualizationTask: Stack Size Low");
    
    if(true == TASK_STACK_SIZE_DEBUG)
    {
      ESP_LOGI("LED_Controller1", "DataMoverTaskTask Free Heap: %i", uxTaskGetStackHighWaterMark(DataMoverTask));
      ESP_LOGI("LED_Controller1", "SerialDataLinkTXTask Free Heap: %i", uxTaskGetStackHighWaterMark(SerialDataLinkTXTask));
      ESP_LOGI("LED_Controller1", "SerialDataLinkRXTask Free Heap: %i", uxTaskGetStackHighWaterMark(SerialDataLinkRXTask));
      ESP_LOGI("LED_Controller1", "VisualizationTask Free Heap: %i", uxTaskGetStackHighWaterMark(VisualizationTask));
      Serial << "\n";
    }
  }
}

void DataMoverTaskLoop(void * parameter)
{
  for(;;)
  {
    yield();
    ESP_LOGV("Function Debug", "%s, ", __func__);
    m_Manager.ProcessEventQueue();
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
}

void VisualizationTaskLoop(void * parameter)
{
  for(;;)
  {
    yield();
    ESP_LOGV("Function Debug", "%s, ", __func__);
    //m_Scheduler.RunScheduler();
    vTaskDelay(5 / portTICK_PERIOD_MS);
  }
}

void SerialDataLinkTXTaskLoop(void * parameter)
{
  for(;;)
  {
    yield();
    ESP_LOGV("Function Debug", "%s, ", __func__);
    //m_SerialDataLink.ProcessDataTXEventQueue();
    vTaskDelay(5 / portTICK_PERIOD_MS);
  }
}

void SerialDataLinkRXTaskLoop(void * parameter)
{
  for(;;)
  {
    yield();
    ESP_LOGV("Function Debug", "%s, ", __func__);
    //m_SerialDataLink.ProcessDataRXEventQueue();
    vTaskDelay(5 / portTICK_PERIOD_MS);
  }
}
