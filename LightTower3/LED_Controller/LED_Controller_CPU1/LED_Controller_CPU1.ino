#include "Manager.h"
#include "FFT_Calculator.h"
#include "Serial_Datalink_Config.h"
#include <BluetoothA2DPSink.h>

#define I2S_BUFFER_COUNT 10
#define I2S_BUFFER_SIZE 100

TaskHandle_t ManagerTask;
TaskHandle_t FFTTask;
TaskHandle_t SerialDataLinkTask;

FFT_Calculator m_FFT_Calculator = FFT_Calculator("FFT Calculator");
SerialDataLink m_SerialDatalink = SerialDataLink("Serial Datalink");

BluetoothA2DPSink m_BTSink;
Bluetooth_Sink m_BT = Bluetooth_Sink( "Bluetooth"
                                    , m_BTSink
                                    , I2S_NUM_1                 // I2S Interface
                                    , i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_TX)
                                    , 44100
                                    , I2S_BITS_PER_SAMPLE_32BIT
                                    , I2S_CHANNEL_FMT_RIGHT_LEFT
                                    , i2s_comm_format_t(I2S_COMM_FORMAT_STAND_I2S)
                                    , I2S_CHANNEL_STEREO
                                    , 10                        // Buffer Count
                                    , 60                        // Buffer Size
                                    , 25                        // Serial Clock Pin
                                    , 26                        // Word Selection Pin
                                    , I2S_PIN_NO_CHANGE         // Serial Data In Pin
                                    , 33  );                    // Serial Data Out Pin
                                    
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



I2S_Device m_Mic = I2S_Device( "Microphone"
                             , I2S_NUM_0                 // I2S Interface
                             , i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX)
                             , 44100
                             , I2S_BITS_PER_SAMPLE_32BIT
                             , I2S_CHANNEL_FMT_RIGHT_LEFT
                             , i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB)
                             , I2S_CHANNEL_STEREO
                             , I2S_BUFFER_COUNT          // Buffer Count
                             , I2S_BUFFER_SIZE           // Buffer Size
                             , 12                        // Serial Clock Pin
                             , 13                        // Word Selection Pin
                             , 14                        // Serial Data In Pin
                             , I2S_PIN_NO_CHANGE         // Serial Data Out Pin
                             , 32 );                     // Mute Pin
                      
I2S_Device m_Speaker = I2S_Device( "Speaker"
                                 , I2S_NUM_1                 // I2S Interface
                                 , i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_TX)
                                 , 44100
                                 , I2S_BITS_PER_SAMPLE_32BIT
                                 , I2S_CHANNEL_FMT_RIGHT_LEFT
                                 , i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB)
                                 , I2S_CHANNEL_STEREO
                                 , I2S_BUFFER_COUNT          // Buffer Count
                                 , I2S_BUFFER_SIZE           // Buffer Size
                                 , 25                        // Serial Clock Pin
                                 , 26                        // Word Selection Pin
                                 , I2S_PIN_NO_CHANGE         // Serial Data In Pin
                                 , 33                        // Serial Data Out Pin
                                 , I2S_PIN_NO_CHANGE );      // Mute Pin
  
Manager m_Manager = Manager("Manager"
                           , m_FFT_Calculator
                           , m_SerialDatalink
                           , m_BT
                           , m_Mic
                           , m_Speaker);

void setup() {
  Serial.begin(500000);
  delay(500);
  Serial << "Xtal Clock Frequency: " << getXtalFrequencyMhz() << " MHz\n";
  Serial << "CPU Clock Frequency: " << getCpuFrequencyMhz() << " MHz\n";
  Serial << "Apb Clock Frequency: " << getApbFrequency() << " Hz\n";
  
  m_Mic.Setup();
  m_Speaker.Setup();
  m_BT.Setup();
  m_Manager.Setup();
  m_SerialDatalink.Setup();
  
  m_BTSink.set_stream_reader(read_data_stream);
  m_BTSink.set_on_data_received(data_received_callback);
  xTaskCreatePinnedToCore
  (
    ManagerTaskLoop,          // Function to implement the task
    "ManagerTask",            // Name of the task
    20000,                    // Stack size in words
    NULL,                     // Task input parameter
    configMAX_PRIORITIES - 3, // Priority of the task
    &ManagerTask,             // Task handle.
    0                         // Core where the task should run
  );
  delay(500);
   
  xTaskCreatePinnedToCore
  (
    FFTTaskLoop,                // Function to implement the task
    "FFTTask",                  // Name of the task
    10000,                    // Stack size in words
    NULL,                     // Task input parameter
    configMAX_PRIORITIES - 6, // Priority of the task
    &FFTTask,                   // Task handle.
    1                         // Core where the task should run
  );                   
  delay(500);

  xTaskCreatePinnedToCore
  (
    SerialDataLinkTaskLoop,                  // Function to implement the task
    "SerialDataLinkTask",                    // Name of the task
    10000,                      // Stack size in words
    NULL,                       // Task input parameter
    configMAX_PRIORITIES - 20,  // Priority of the task
    &SerialDataLinkTask,                     // Task handle.
    1                           // Core where the task should run
  );     
  delay(500);
}

void loop() {
  // put your main code here, to run repeatedly:

}

void ManagerTaskLoop(void * parameter)
{
  for(;;)
  {
    yield();
    m_Manager.RunTask();
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
}
void FFTTaskLoop(void * parameter)
{
  for(;;)
  {
    yield();
    m_FFT_Calculator.ProcessEventQueue();
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}
void SerialDataLinkTaskLoop(void * parameter)
{
  for(;;)
  {
    yield();
    m_SerialDatalink.ProcessEventQueue();
    m_SerialDatalink.CheckForNewSerialData();
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
}
