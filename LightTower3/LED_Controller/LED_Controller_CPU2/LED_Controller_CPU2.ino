#include "Manager.h"
#include "Serial_Datalink_Config.h"

#define I2S_BUFFER_COUNT 10
#define I2S_BUFFER_SIZE 100

TaskHandle_t Task0;

I2S_Device m_I2S_In = I2S_Device( "I2S_In"
                                , I2S_NUM_0
                                , i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX)
                                , 44100
                                , I2S_BITS_PER_SAMPLE_32BIT
                                , I2S_CHANNEL_FMT_RIGHT_LEFT
                                , i2s_comm_format_t(I2S_COMM_FORMAT_I2S)
                                , I2S_CHANNEL_STEREO
                                , I2S_BUFFER_COUNT
                                , I2S_BUFFER_SIZE
                                , 12
                                , 13
                                , 14
                                , I2S_PIN_NO_CHANGE );
Manager m_Manager = Manager("Manager", m_I2S_In);
SerialDataLink m_SerialDatalink = SerialDataLink("Serial Datalink");

void setup() {
  Serial.begin(500000);
  delay(500);
  Serial << "Xtal Clock Frequency: " << getXtalFrequencyMhz() << " MHz\n";
  Serial << "CPU Clock Frequency: " << getCpuFrequencyMhz() << " MHz\n";
  Serial << "Apb Clock Frequency: " << getApbFrequency() << " Hz\n";

  m_SerialDatalink.Setup();
  m_I2S_In.Setup();
  m_Manager.Setup();
  
  xTaskCreatePinnedToCore
  (
    Task0Loop,                      // Function to implement the task
    "Task0",                        // Name of the task
    10000,                          // Stack size in words
    NULL,                           // Task input parameter
    configMAX_PRIORITIES - 10,      // Priority of the task
    &Task0,                         // Task handle.
    0                               // Core where the task should run
  );
  delay(500);
}

void loop() {
  // put your main code here, to run repeatedly:

}

void Task0Loop(void * parameter)
{
  while(true)
  {
    m_Manager.RunTask();
    m_SerialDatalink.CheckForNewSerialData();
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
}
