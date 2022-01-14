#include "Manager.h"
#include "Sound_Processor.h"
#include "Serial_Datalink_Config.h"
#include <BluetoothA2DPSink.h>
#include "Tunes.h"

TaskHandle_t ManagerTask;
TaskHandle_t SoundProcessorTask;
TaskHandle_t FFTTask;
TaskHandle_t SoundPowerTask;
TaskHandle_t SoundMaxBandTask;
TaskHandle_t SerialDataLinkTXTask;
TaskHandle_t SerialDataLinkRXTask;

BluetoothA2DPSink m_BTSink;
Bluetooth_Sink m_BT = Bluetooth_Sink( "Bluetooth"
                                    , m_BTSink
                                    , I2S_NUM_1                          // I2S Interface
                                    , i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_TX)
                                    , 44100
                                    , I2S_BITS_PER_SAMPLE_32BIT
                                    , I2S_CHANNEL_FMT_RIGHT_LEFT
                                    , i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB)
                                    , I2S_CHANNEL_STEREO                                    
                                    , 10                                 // Buffer Count
                                    , 40                                 // Buffer Size
                                    , I2S_CHANNEL_SAMPLE_COUNT           // Callback Sample Count
                                    , 5                                  // Queue Count
                                    , 25                                 // Serial Clock Pin
                                    , 26                                 // Word Selection Pin
                                    , I2S_PIN_NO_CHANGE                  // Serial Data In Pin
                                    , 33 );                              // Serial Data Out Pin
                                    
//Callbacks for Bluetooth Sink
void data_received_callback() 
{
  m_BT.data_received_callback();
}

//Callbacks for Bluetooth Sink
void read_data_stream(const uint8_t *data, uint32_t length)
{
  m_BT.read_data_stream(data, length);
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
                                , 44100
                                , I2S_BITS_PER_SAMPLE_32BIT
                                , I2S_CHANNEL_FMT_RIGHT_LEFT
                                , i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB)
                                , I2S_CHANNEL_STEREO
                                , I2S_BUFFER_COUNT                   // Buffer Count
                                , I2S_CHANNEL_SAMPLE_COUNT           // Buffer Size
                                , 12                                 // Serial Clock Pin
                                , 13                                 // Word Selection Pin
                                , 14                                 // Serial Data In Pin
                                , I2S_PIN_NO_CHANGE );               // Serial Data Out Pin );
                      
I2S_Device m_Mic_Out = I2S_Device( "Microphone Out"
                                  , I2S_NUM_1                          // I2S Interface
                                  , i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_TX)
                                  , 44100
                                  , I2S_BITS_PER_SAMPLE_32BIT
                                  , I2S_CHANNEL_FMT_RIGHT_LEFT
                                  , i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB)
                                  , I2S_CHANNEL_STEREO
                                  , I2S_BUFFER_COUNT                   // Buffer Count
                                  , I2S_CHANNEL_SAMPLE_COUNT           // Buffer Size
                                  , 25                                 // Serial Clock Pin
                                  , 26                                 // Word Selection Pin
                                  , I2S_PIN_NO_CHANGE                  // Serial Data In Pin
                                  , 33 );                              // Serial Data Out Pin

HardwareSerial m_hSerial = Serial2;
Sound_Processor m_Sound_Processor = Sound_Processor("Sound Processor");
SerialDataLink m_SerialDatalink = SerialDataLink("Serial Datalink", m_hSerial);
Manager m_Manager = Manager("Manager"
                           , m_Sound_Processor
                           , m_SerialDatalink
                           , m_BT
                           , m_Mic_In
                           , m_Mic_Out);

void setup() {  
  //ESP32 Serial Communication
  m_hSerial.end();
  m_hSerial.setRxBufferSize(1024);
  m_hSerial.begin(9600, SERIAL_8N1, 16, 17); // pins 16 rx2, 17 tx2, 19200 bps, 8 bits no parity 1 stop bit
  m_hSerial.updateBaudRate(400000); //For whatever reason, if I set it to 500000 in setup, it crashes a lot of the time.
  m_hSerial.flush();
  
  Serial.begin(500000);
  Serial.flush();
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
  
  m_BTSink.set_stream_reader(read_data_stream);
  m_BTSink.set_on_data_received(data_received_callback);
  m_Mic_In.Setup();
  m_Mic_Out.Setup();
  m_BT.Setup();
  m_Manager.Setup();
  m_SerialDatalink.SetupSerialDataLink();
  
  xTaskCreatePinnedToCore
  (
    SoundPowerTaskLoop,         // Function to implement the task
    "SoundPowerTask",           // Name of the task
    1000,                        // Stack size in words
    NULL,                       // Task input parameter
    configMAX_PRIORITIES - 2,   // Priority of the task
    &SoundPowerTask,            // Task handle.
    0                           // Core where the task should run
  );
  
  xTaskCreatePinnedToCore
  (
    SoundMaxBandTaskLoop,       // Function to implement the task
    "SoundMaxBandTask",         // Name of the task
    1500,                       // Stack size in words
    NULL,                       // Task input parameter
    configMAX_PRIORITIES - 3,   // Priority of the task
    &SoundMaxBandTask,          // Task handle.
    0                           // Core where the task should run
  );
  
  xTaskCreatePinnedToCore
  (
    FFTTaskLoop,                // Function to implement the task
    "FFTTask",                  // Name of the task
    1000,                       // Stack size in words
    NULL,                       // Task input parameter
    configMAX_PRIORITIES - 4,   // Priority of the task
    &FFTTask,                   // Task handle.
    0                           // Core where the task should run
  );
   
  
  
  xTaskCreatePinnedToCore
  (
    SerialDataLinkTXTaskLoop,   // Function to implement the task
    "SerialDataLinkSendTask",   // Name of the task
    2000,                       // Stack size in words
    NULL,                       // Task input parameter
    configMAX_PRIORITIES - 1,   // Priority of the task
    &SerialDataLinkTXTask,      // Task handle.
    1                           // Core where the task should run
  );     
  
  xTaskCreatePinnedToCore
  (
    SerialDataLinkRXTaskLoop,   // Function to implement the task
    "SerialDataLinkRXTask",     // Name of the task
    1000,                       // Stack size in words
    NULL,                       // Task input parameter
    configMAX_PRIORITIES - 1,   // Priority of the task
    &SerialDataLinkRXTask,      // Task handle.
    1                           // Core where the task should run
  );
  
  xTaskCreatePinnedToCore
  (
    ManagerTaskLoop,            // Function to implement the task
    "ManagerTask",              // Name of the task
    1500,                       // Stack size in words
    NULL,                       // Task input parameter
    configMAX_PRIORITIES - 1,   // Priority of the task
    &ManagerTask,               // Task handle.
    1                           // Core where the task should run
  );
  
  xTaskCreatePinnedToCore
  (
    SoundProcessorTaskLoop,     // Function to implement the task
    "SoundProcessorTask",       // Name of the task
    1500,                       // Stack size in words
    NULL,                       // Task input parameter
    configMAX_PRIORITIES - 1,   // Priority of the task
    &SoundProcessorTask,        // Task handle.
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
    if( uxTaskGetStackHighWaterMark(ManagerTask) < StackSizeThreshold )Serial << "WARNING! ManagerTask: Stack Size Low\n";
    if( uxTaskGetStackHighWaterMark(SoundProcessorTask) < StackSizeThreshold )Serial << "WARNING! SoundProcessorTask: Stack Size Low\n";
    if( uxTaskGetStackHighWaterMark(FFTTask) < StackSizeThreshold )Serial << "WARNING! FFTTask: Stack Size Low\n";
    if( uxTaskGetStackHighWaterMark(SoundPowerTask) < StackSizeThreshold )Serial << "WARNING! SoundPowerTask: Stack Size Low\n";
    if( uxTaskGetStackHighWaterMark(SoundMaxBandTask) < StackSizeThreshold )Serial << "WARNING! SoundMaxBandTask: Stack Size Low\n";
    if( uxTaskGetStackHighWaterMark(SerialDataLinkTXTask) < StackSizeThreshold )Serial << "WARNING! SerialDataLinkTXTask: Stack Size Low\n";
    if( uxTaskGetStackHighWaterMark(SerialDataLinkRXTask) < StackSizeThreshold )Serial << "WARNING! SerialDataLinkRXTask: Stack Size Low\n";
    if(true == HEAP_SIZE_DEBUG)Serial << "Free Heap: " << ESP.getFreeHeap() << "\n";
    if(true == TASK_STACK_SIZE_DEBUG)
    {
      Serial << "ManagerTask: " << uxTaskGetStackHighWaterMark(ManagerTask) << "\n";
      Serial << "SoundProcessorTask: " << uxTaskGetStackHighWaterMark(SoundProcessorTask) << "\n";
      Serial << "FFTTask: " << uxTaskGetStackHighWaterMark(FFTTask) << "\n";
      Serial << "SoundPowerTask: " << uxTaskGetStackHighWaterMark(SoundPowerTask) << "\n";
      Serial << "SoundMaxBandTask: " << uxTaskGetStackHighWaterMark(SoundMaxBandTask) << "\n";
      Serial << "SerialDataLinkTXTask: " << uxTaskGetStackHighWaterMark(SerialDataLinkTXTask) << "\n";
      Serial << "SerialDataLinkRXTask: " << uxTaskGetStackHighWaterMark(SerialDataLinkRXTask) << "\n";
      Serial << "\n";
    }
  }
}

void ManagerTaskLoop(void * parameter)
{  
  Serial << "Started Thread: ManagerTaskLoop\n";\
  for(;;)
  {
    yield();
    m_Manager.RunTask();
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
}

void SoundProcessorTaskLoop(void * parameter)
{
  Serial << "Started Thread: SoundProcessorTaskLoop\n";
  for(;;)
  {
    yield();
    m_Sound_Processor.ProcessEventQueue();
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
}

void FFTTaskLoop(void * parameter)
{
  Serial << "Started Thread: FFTTaskLoop\n";
  for(;;)
  {
    yield();
    m_Sound_Processor.ProcessFFTEventQueue();
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

void SoundPowerTaskLoop(void * parameter)
{
  Serial << "Started Thread: SoundPowerTaskLoop\n";
  for(;;)
  {
    yield();
    m_Sound_Processor.ProcessSoundPowerEventQueue();
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
}

void SoundMaxBandTaskLoop(void * parameter)
{
  Serial << "Started Thread: SoundMaxBandTaskLoop\n";
  for(;;)
  {
    yield();
    m_Sound_Processor.ProcessMaxBandEventQueue();
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
}

void SerialDataLinkTXTaskLoop(void * parameter)
{
  Serial << "Started Thread: SerialDataLinkTXTaskLoop\n";
  for(;;)
  {
    yield();
    m_SerialDatalink.ProcessDataTXEventQueue();
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
}

void SerialDataLinkRXTaskLoop(void * parameter)
{
  Serial << "Started Thread: SerialDataLinkReceiveTaskLoop\n";
  for(;;)
  {
    yield();
    m_SerialDatalink.GetRXData();
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
}
