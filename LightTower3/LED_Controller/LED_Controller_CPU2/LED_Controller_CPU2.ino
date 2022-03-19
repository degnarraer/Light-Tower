#include "Manager.h"
#include "Tunes.h"

#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include "esp_log.h"

#define c3_frequency  130.81

TaskHandle_t ManagerTask;
TaskHandle_t ProcessSoundPowerTask;
TaskHandle_t ProcessFFTTask;
TaskHandle_t SerialDataLinkTXTask;
TaskHandle_t SerialDataLinkRXTask;

I2S_Device m_I2S_In = I2S_Device( "I2S_In"
                                 , I2S_NUM_0
                                 , i2s_mode_t(I2S_MODE_SLAVE | I2S_MODE_RX)
                                 , I2S_SAMPLE_RATE
                                 , I2S_BITS_PER_SAMPLE_32BIT
                                 , I2S_CHANNEL_FMT_RIGHT_LEFT
                                 , i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB)
                                 , I2S_CHANNEL_STEREO
                                 , true                               // Use APLL
                                 , I2S_BUFFER_COUNT                   // Buffer Count
                                 , I2S_SAMPLE_COUNT                   // Buffer Size
                                 , 12                                 // Serial Clock Pin
                                 , 13                                 // Word Selection Pin
                                 , 14                                 // Serial Data In Pin
                                 , I2S_PIN_NO_CHANGE );               // Serial Data Out Pin 
                      
I2S_Device m_I2S_Out = I2S_Device( "I2S_Out"
                                  , I2S_NUM_1                        // I2S Interface
                                  , i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_TX)
                                  , I2S_SAMPLE_RATE
                                  , I2S_BITS_PER_SAMPLE_32BIT
                                  , I2S_CHANNEL_FMT_RIGHT_LEFT
                                  , i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB)
                                  , I2S_CHANNEL_STEREO
                                  , true                             // Use APLL
                                  , I2S_BUFFER_COUNT                 // Buffer Count
                                  , I2S_SAMPLE_COUNT                 // Buffer Size
                                  , 25                               // Serial Clock Pin
                                  , 26                               // Word Selection Pin
                                  , I2S_PIN_NO_CHANGE                // Serial Data In Pin
                                  , 33 );  

HardwareSerial m_hSerial = Serial2;
SerialDataLink m_SerialDataLink = SerialDataLink("Serial Datalink", m_hSerial);
Sound_Processor m_SoundProcessor = Sound_Processor("Sound Processor", m_SerialDataLink);
BluetoothA2DPSource a2dp_source;
Manager m_Manager = Manager("Manager", m_SoundProcessor, m_SerialDataLink, a2dp_source, m_I2S_In, m_I2S_Out);

//Bluetooth Source Callback
int32_t get_data_channels(Frame *frame, int32_t channel_len)
{
  return m_Manager.get_data_channels(frame, channel_len);
}

void setup() {
  //ESP32 Serial Communication
  m_hSerial.setRxBufferSize(4096);
  m_hSerial.begin(300000, SERIAL_8N1, 16, 17); // pins 16 rx2, 17 tx2, 9600 bps, 8 bits no parity 1 stop bit
  m_hSerial.flush();
    
  //PC Serial Communication
  Serial.begin(500000); // 9600 bps, 8 bits no parity 1 stop bit

  //ESP_LOGD("LED_Controller2", "%s, ", __func__);
  ESP_LOGI("LED_Controller2", "Serial Datalink Configured");
  ESP_LOGI("LED_Controller2", "Xtal Clock Frequency: %i MHz", getXtalFrequencyMhz());
  ESP_LOGI("LED_Controller2", "CPU Clock Frequency: %i MHz", getCpuFrequencyMhz());
  ESP_LOGI("LED_Controller2", "Apb Clock Frequency: %i Hz", getApbFrequency());
 
  m_I2S_Out.Setup();
  m_I2S_In.Setup();
  m_Manager.Setup();
  m_SerialDataLink.SetupSerialDataLink();
  a2dp_source.set_auto_reconnect(true);
  a2dp_source.set_ssp_enabled(false);
  a2dp_source.set_local_name("LED Tower of Power");
  //a2dp_source.set_pin_code("0000");
  a2dp_source.start("[AV] Samsung Soundbar MM55 M-Series", get_data_channels);
  //a2dp_source.start("Shock's iPhone", get_data_channels);
  ESP_LOGI("LED_Controller2", "Bluetooth Source Started");
  
  xTaskCreatePinnedToCore
  (
    ManagerTaskLoop,                // Function to implement the task
    "ManagerTask",                  // Name of the task
    4000,                           // Stack size in words
    NULL,                           // Task input parameter
    configMAX_PRIORITIES - 1,       // Priority of the task
    &ManagerTask,                   // Task handle.
    1                               // Core where the task should run
  );
  
  xTaskCreatePinnedToCore
  (
    ProcessSoundPowerTaskLoop,      // Function to implement the task
    "ProcessSoundPowerTask",        // Name of the task
    4000,                           // Stack size in words
    NULL,                           // Task input parameter
    configMAX_PRIORITIES - 1,       // Priority of the task
    &ProcessSoundPowerTask,         // Task handle.
    0                               // Core where the task should run
  );
  
  xTaskCreatePinnedToCore
  (
    ProcessFFTTaskLoop,             // Function to implement the task
    "ProcessFFTTask",               // Name of the task
    4000,                           // Stack size in words
    NULL,                           // Task input parameter
    configMAX_PRIORITIES - 1,       // Priority of the task
    &ProcessFFTTask,                // Task handle.
    0                               // Core where the task should run
  );
  
  xTaskCreatePinnedToCore
  (
    SerialDataLinkTXTaskLoop,       // Function to implement the task
    "SerialDataLinkTXTask",         // Name of the task
    4000,                           // Stack size in words
    NULL,                           // Task input parameter
    configMAX_PRIORITIES - 2,       // Priority of the task
    &SerialDataLinkTXTask,          // Task handle.
    1                               // Core where the task should run
  );

  xTaskCreatePinnedToCore
  (
    SerialDataLinkRXTaskLoop,       // Function to implement the task
    "SerialDataLinkRXTask",         // Name of the task
    2000,                           // Stack size in words
    NULL,                           // Task input parameter
    configMAX_PRIORITIES - 2,       // Priority of the task
    &SerialDataLinkRXTask,          // Task handle.
    1                               // Core where the task should run
  );
  ESP_LOGI("LED_Controller_CPU2", "Free Heap: %i", ESP.getFreeHeap());
}

void loop()
{
  // put your main code here, to run repeatedly:
}

void ManagerTaskLoop(void * parameter)
{
  while(true)
  {
    yield();
    //ESP_LOGV("LED_Controller2", "%s, ", __func__);
    m_Manager.ProcessEventQueue();
    vTaskDelay(5 / portTICK_PERIOD_MS);
  }
}

void ProcessSoundPowerTaskLoop(void * parameter)
{
  while(true)
  {
    yield();
    //ESP_LOGV("LED_Controller2", "%s, ", __func__);
    m_SoundProcessor.ProcessSoundPower();
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

void ProcessFFTTaskLoop(void * parameter)
{
  while(true)
  {
    yield();
    //ESP_LOGV("Function Debug", "%s, ", __func__);
    m_SoundProcessor.ProcessFFT();
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

void SerialDataLinkRXTaskLoop(void * parameter)
{
  while(true)
  {
    yield();
    //ESP_LOGV("LED_Controller2", "%s, ", __func__);
    m_SerialDataLink.ProcessDataRXEventQueue();
    vTaskDelay(5 / portTICK_PERIOD_MS);
  }
}

void SerialDataLinkTXTaskLoop(void * parameter)
{
  while(true)
  {
    yield();
    //ESP_LOGV("LED_Controller2", "%s, ", __func__);
    m_SerialDataLink.ProcessDataTXEventQueue();
    vTaskDelay(5 / portTICK_PERIOD_MS);
  }
}
