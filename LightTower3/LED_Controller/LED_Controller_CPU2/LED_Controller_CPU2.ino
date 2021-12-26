#include "Manager.h"
#include "Serial_Datalink_Config.h"
#include <BluetoothA2DPSource.h>
#include "VisualizationPlayer.h"
#include "Models.h"
#include "Tunes.h"

#define I2S_BUFFER_COUNT 10
#define I2S_BUFFER_SIZE 100

TaskHandle_t ManagerTask;
TaskHandle_t SerialDataTask;
TaskHandle_t VisualizationTask;

I2S_Device m_I2S_In = I2S_Device( "I2S_In"
                                , I2S_NUM_0
                                , i2s_mode_t(I2S_MODE_SLAVE | I2S_MODE_RX)
                                , 44100
                                , I2S_BITS_PER_SAMPLE_32BIT
                                , I2S_CHANNEL_FMT_RIGHT_LEFT
                                , i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB)
                                , I2S_CHANNEL_STEREO
                                , I2S_BUFFER_COUNT
                                , I2S_BUFFER_SIZE
                                , 12
                                , 13
                                , 14
                                , I2S_PIN_NO_CHANGE );
SerialDataLink m_SerialDatalink = SerialDataLink("Serial Datalink");
TaskScheduler m_Scheduler;
CalculateFPS m_CalculateFPS("Main Loop", 1000);
StatisticalEngine m_StatisticalEngine = StatisticalEngine();
StatisticalEngineModelInterface m_StatisticalEngineModelInterface = StatisticalEngineModelInterface(m_StatisticalEngine);
VisualizationPlayer m_VisualizationPlayer = VisualizationPlayer(m_StatisticalEngineModelInterface);
Manager m_Manager = Manager("Manager", m_SerialDatalink, m_StatisticalEngine, m_I2S_In);

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
    ManagerTaskLoop,                // Function to implement the task
    "ManagerTask",                  // Name of the task
    10000,                          // Stack size in words
    NULL,                           // Task input parameter
    configMAX_PRIORITIES - 10,      // Priority of the task
    &ManagerTask,                   // Task handle.
    0                               // Core where the task should run
  );
  
  xTaskCreatePinnedToCore
  (
    SerialDataTaskLoop,             // Function to implement the task
    "SerialDataTask",               // Name of the task
    10000,                          // Stack size in words
    NULL,                           // Task input parameter
    configMAX_PRIORITIES - 10,      // Priority of the task
    &SerialDataTask,                // Task handle.
    0                               // Core where the task should run
  );
  
  xTaskCreatePinnedToCore
  (
    VisualizationTaskLoop,          // Function to implement the task
    "VisualizationTask",            // Name of the task
    50000,                          // Stack size in words
    NULL,                           // Task input parameter
    configMAX_PRIORITIES - 10,      // Priority of the task
    &VisualizationTask,             // Task handle.
    0                               // Core where the task should run
  );
}

void loop() {
  // put your main code here, to run repeatedly:

}

void ManagerTaskLoop(void * parameter)
{
  while(true)
  {
    //m_Manager.RunTask();
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
}

void SerialDataTaskLoop(void * parameter)
{
  while(true)
  {
    m_SerialDatalink.CheckForNewSerialData();
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
}

void VisualizationTaskLoop(void * parameter)
{
  m_Scheduler.AddTask(m_CalculateFPS);
  m_Scheduler.AddTask(m_StatisticalEngineModelInterface);
  m_Scheduler.AddTask(m_VisualizationPlayer);
  while(true)
  {
    m_Scheduler.RunScheduler();
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
}
