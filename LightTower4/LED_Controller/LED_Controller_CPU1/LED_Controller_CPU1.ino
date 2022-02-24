#include "Manager.h"
#include <BluetoothA2DPSink.h>
#include "Tunes.h"
#include "VisualizationPlayer.h"
#include "Models.h"
#include "Tunes.h"

TaskHandle_t DataMoverTask;
TaskHandle_t SerialDataLinkTXTask;
TaskHandle_t SerialDataLinkRXTask;
TaskHandle_t VisualizationTask;

BluetoothA2DPSink m_BTSink;
Bluetooth_Sink m_BT_In = Bluetooth_Sink( "Bluetooth"
                                       , m_BTSink
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
void connection_state_changed(esp_a2d_connection_state_t state, void *ptr){
  Serial.println(m_BTSink.to_str(state));
}

//Callbacks for Bluetooth Sink
// for esp_a2d_audio_state_t see https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/bluetooth/esp_a2dp.html#_CPPv421esp_a2d_audio_state_t
void audio_state_changed(esp_a2d_audio_state_t state, void *ptr){
  Serial.println(m_BTSink.to_str(state));
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
                                , 12                                 // Serial Clock Pin
                                , 13                                 // Word Selection Pin
                                , 14                                 // Serial Data In Pin
                                , I2S_PIN_NO_CHANGE );               // Serial Data Out Pin
                      
I2S_Device m_I2S_Out = I2S_Device( "I2S Out"
                                  , I2S_NUM_1                        // I2S Interface
                                  , i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_TX)
                                  , I2S_SAMPLE_RATE
                                  , I2S_BITS_PER_SAMPLE_32BIT
                                  , I2S_CHANNEL_FMT_RIGHT_LEFT
                                  , i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB)
                                  , I2S_CHANNEL_STEREO
                                  , true                            // Use APLL
                                  , I2S_BUFFER_COUNT                 // Buffer Count
                                  , I2S_CHANNEL_SAMPLE_COUNT         // Buffer Size
                                  , 25                               // Serial Clock Pin
                                  , 26                               // Word Selection Pin
                                  , I2S_PIN_NO_CHANGE                // Serial Data In Pin
                                  , 33 );                            // Serial Data Out Pin

StatisticalEngine m_StatisticalEngine = StatisticalEngine();
StatisticalEngineModelInterface m_StatisticalEngineModelInterface = StatisticalEngineModelInterface(m_StatisticalEngine);
VisualizationPlayer m_VisualizationPlayer = VisualizationPlayer(m_StatisticalEngineModelInterface);
HardwareSerial m_hSerial = Serial2;
SerialDataLink m_SerialDataLink = SerialDataLink("Serial Datalink", m_hSerial);
CalculateFPS m_CalculateFPS("Main Loop", 1000);
TaskScheduler m_Scheduler;
Manager m_Manager = Manager("Manager"
                           , m_StatisticalEngine
                           , m_SerialDataLink
                           , m_BT_In
                           , m_Mic_In
                           , m_I2S_Out);

void setup() {
  //ESP32 Serial Communication
  m_hSerial.end();
  m_hSerial.begin(9600, SERIAL_8N1, 16, 17); // pins 16 rx2, 17 tx2, 9600 bps, 8 bits no parity 1 stop bit
  m_hSerial.updateBaudRate(200000); //For whatever reason, if I set it to 400000 in setup, it crashes a lot of the time.
  m_hSerial.flush();
  
  //PC Serial Communication
  Serial.end();
  Serial.begin(9600); // 9600 bps, 8 bits no parity 1 stop bit
  Serial.updateBaudRate(500000); // 500000 bps, 8 bits no parity 1 stop bit
  Serial.flush();
  if(true == STARTUP_DEBUG)
  {
    Serial << "Xtal Clock Frequency: " << getXtalFrequencyMhz() << " MHz\n";
    Serial << "CPU Clock Frequency: " << getCpuFrequencyMhz() << " MHz\n";
    Serial << "Apb Clock Frequency: " << getApbFrequency() << " Hz\n";
  
    Serial << "Data Type Sizes:\n";
    Serial << "QueueHandle_t: " << sizeof(QueueHandle_t) << "\n";
    Serial << "Transciever_T: " << sizeof(Transciever_T) << "\n";
    Serial << "DataType_t: " << sizeof(DataType_t) << "\n";
    Serial << "size_t: " << sizeof(size_t) << "\n";
    Serial << "void: " << sizeof(void) << "\n";
    Serial << "String: " << sizeof(String) << "\n";
  }
  m_BTSink.set_stream_reader(read_data_stream, true);
  m_BTSink.set_on_data_received(data_received_callback);
  m_Manager.Setup();
  m_SerialDataLink.SetupSerialDataLink();
  m_Scheduler.AddTask(m_CalculateFPS);
  m_Scheduler.AddTask(m_StatisticalEngineModelInterface);
  m_Scheduler.AddTask(m_VisualizationPlayer);
  
  xTaskCreatePinnedToCore
  (
    DataMoverTaskLoop,            // Function to implement the task
    "DataMoverTask",              // Name of the task
    4000,                         // Stack size in words
    NULL,                         // Task input parameter
    configMAX_PRIORITIES - 1,     // Priority of the task
    &DataMoverTask,               // Task handle.
    1                             // Core where the task should run
  );
  
  xTaskCreatePinnedToCore
  (
    VisualizationTaskLoop,        // Function to implement the task
    "VisualizationTask",          // Name of the task
    10000,                        // Stack size in words
    NULL,                         // Task input parameter
    configMAX_PRIORITIES - 1,     // Priority of the task
    &VisualizationTask,           // Task handle.
    0                             // Core where the task should run
  );
  
  xTaskCreatePinnedToCore
  (
    SerialDataLinkTXTaskLoop,   // Function to implement the task
    "SerialDataLinkSendTask",   // Name of the task
    4000,                       // Stack size in words
    NULL,                       // Task input parameter
    configMAX_PRIORITIES - 2,   // Priority of the task
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
  Serial << "Free Heap: " << ESP.getFreeHeap() << "\n";
}

unsigned long myTime = millis();
void loop() {
  // put your main code here, to run repeatedly:
  if(millis() - myTime > 10000)
  {
    myTime = millis();
    size_t StackSizeThreshold = 100;
    if( uxTaskGetStackHighWaterMark(DataMoverTask) < StackSizeThreshold )Serial << "WARNING! DataMoverTask: Stack Size Low\n";
    if( uxTaskGetStackHighWaterMark(SerialDataLinkTXTask) < StackSizeThreshold )Serial << "WARNING! SerialDataLinkTXTask: Stack Size Low\n";
    if( uxTaskGetStackHighWaterMark(SerialDataLinkRXTask) < StackSizeThreshold )Serial << "WARNING! SerialDataLinkRXTask: Stack Size Low\n";
    if(true == HEAP_SIZE_DEBUG)Serial << "Free Heap: " << ESP.getFreeHeap() << "\n";
    if(true == TASK_STACK_SIZE_DEBUG)
    {
      Serial << "DataMoverTaskTask: " << uxTaskGetStackHighWaterMark(DataMoverTask) << "\n";
      Serial << "SerialDataLinkTXTask: " << uxTaskGetStackHighWaterMark(SerialDataLinkTXTask) << "\n";
      Serial << "SerialDataLinkRXTask: " << uxTaskGetStackHighWaterMark(SerialDataLinkRXTask) << "\n";
      Serial << "\n";
    }
  }
}

void DataMoverTaskLoop(void * parameter)
{
  for(;;)
  {
    yield();
    m_Manager.ProcessEventQueue();
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
}

void VisualizationTaskLoop(void * parameter)
{
  for(;;)
  {
    yield();
    m_Scheduler.RunScheduler();
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
}

void SerialDataLinkTXTaskLoop(void * parameter)
{
  for(;;)
  {
    yield();
    m_SerialDataLink.ProcessDataTXEventQueue();
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
}

void SerialDataLinkRXTaskLoop(void * parameter)
{
  for(;;)
  {
    yield();
    m_SerialDataLink.ProcessDataRXEventQueue();
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
}
