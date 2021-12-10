#include "Manager.h"
#include "FFT_Calculator.h"
#include "Serial_Datalink_Config.h"

#define I2S_BUFFER_COUNT 10
#define I2S_BUFFER_SIZE 100

TaskHandle_t ManagerTask;
TaskHandle_t FFTTask;
TaskHandle_t SerialDataLinkTask;

FFT_Calculator m_FFT_Calculator = FFT_Calculator("FFT Calculator");
SerialDataLink m_SerialDatalink = SerialDataLink("Serial Datalink");
Bluetooth_Device m_BT = Bluetooth_Device("Bluetooth");
I2S_Device m_Mic = I2S_Device( "Microphone"
                             , I2S_NUM_0
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
                                 , I2S_NUM_1
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
  xTaskCreatePinnedToCore
  (
    ManagerTaskLoop,          // Function to implement the task
    "ManagerTask",            // Name of the task
    10000,                    // Stack size in words
    NULL,                     // Task input parameter
    configMAX_PRIORITIES - 6, // Priority of the task
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
    0                         // Core where the task should run
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
  m_Manager.Setup();
  for(;;)
  {
    m_Manager.RunTask();
    yield();
  }
}
void FFTTaskLoop(void * parameter)
{
  for(;;)
  {
    m_FFT_Calculator.ProcessEventQueue();
    yield();
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
}
void SerialDataLinkTaskLoop(void * parameter)
{
  m_SerialDatalink.Setup();
  for(;;)
  {
    m_SerialDatalink.ProcessEventQueue();
    m_SerialDatalink.CheckForNewSerialData();
    yield();
  }
}
