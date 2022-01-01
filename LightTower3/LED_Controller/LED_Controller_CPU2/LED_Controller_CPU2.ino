#include "Manager.h"
#include "Serial_Datalink_Config.h"
#include <BluetoothA2DPSource.h>
#include "VisualizationPlayer.h"
#include "Models.h"
#include "Tunes.h"

#define I2S_BUFFER_COUNT 10
#define I2S_BUFFER_SIZE 100

TaskHandle_t ManagerTask;
TaskHandle_t SerialDataRXTask;
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
                                
StatisticalEngine m_StatisticalEngine = StatisticalEngine();
StatisticalEngineModelInterface m_StatisticalEngineModelInterface = StatisticalEngineModelInterface(m_StatisticalEngine);
VisualizationPlayer m_VisualizationPlayer = VisualizationPlayer(m_StatisticalEngineModelInterface);
HardwareSerial m_hSerial = Serial2;
SerialDataLink m_SerialDatalink = SerialDataLink("Serial Datalink", m_hSerial);
Manager m_Manager = Manager("Manager", m_SerialDatalink, m_StatisticalEngine, m_I2S_In);
CalculateFPS m_CalculateFPS("Main Loop", 1000);
TaskScheduler m_Scheduler;

void setup() {
  
  //ESP32 Serial Communication
  m_hSerial.end();
  m_hSerial.setRxBufferSize(1024);
  m_hSerial.begin(9600, SERIAL_8N1, 16, 17); // pins 16 rx2, 17 tx2, 19200 bps, 8 bits no parity 1 stop bit
  m_hSerial.updateBaudRate(250000); //For whatever reason, if I set it to 500000 in setup, it crashes a lot of the time.
  m_hSerial.flush();
  
  //PC Serial Communication
  Serial.end();
  Serial.begin(500000);
  Serial.flush();
  
  Serial << "Serial Datalink Configured\n";
  Serial << "Xtal Clock Frequency: " << getXtalFrequencyMhz() << " MHz\n";
  Serial << "CPU Clock Frequency: " << getCpuFrequencyMhz() << " MHz\n";
  Serial << "Apb Clock Frequency: " << getApbFrequency() << " Hz\n";

  m_I2S_In.Setup();
  m_Manager.Setup();
  m_SerialDatalink.SetupSerialDataLink();
  
  m_Scheduler.AddTask(m_CalculateFPS);
  m_Scheduler.AddTask(m_StatisticalEngineModelInterface);
  m_Scheduler.AddTask(m_VisualizationPlayer);
  
  xTaskCreatePinnedToCore
  (
    ManagerTaskLoop,                // Function to implement the task
    "ManagerTask",                  // Name of the task
    2000,                           // Stack size in words
    NULL,                           // Task input parameter
    configMAX_PRIORITIES - 1,       // Priority of the task
    &ManagerTask,                   // Task handle.
    0                               // Core where the task should run
  );
  
  xTaskCreatePinnedToCore
  (
    SerialDataRXTaskLoop,           // Function to implement the task
    "SerialDataRXTask",             // Name of the task
    2000,                           // Stack size in words
    NULL,                           // Task input parameter
    configMAX_PRIORITIES - 1,       // Priority of the task
    &SerialDataRXTask,              // Task handle.
    0                               // Core where the task should run
  );
  
  xTaskCreatePinnedToCore
  (
    VisualizationTaskLoop,          // Function to implement the task
    "VisualizationTask",            // Name of the task
    100000,                         // Stack size in words
    NULL,                           // Task input parameter
    configMAX_PRIORITIES - 4,       // Priority of the task
    &VisualizationTask,             // Task handle.
    1                               // Core where the task should run
  );
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
    m_Manager.RunTask();
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
}

void SerialDataRXTaskLoop(void * parameter)
{
  while(true)
  {
    yield();
    m_SerialDatalink.GetRXData();
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
}

void VisualizationTaskLoop(void * parameter)
{
  while(true)
  {
    yield();
    m_Scheduler.RunScheduler();
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
}
