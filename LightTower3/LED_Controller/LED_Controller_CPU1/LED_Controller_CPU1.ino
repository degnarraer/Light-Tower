#include "Manager.h"
#include "FFT_Calculator.h"
#include "Serial_Datalink_Config.h"

TaskHandle_t Task0;
TaskHandle_t Task1;
TaskHandle_t Task2;
TaskHandle_t Task3;

FFT_Calculator m_FFT_Calculator = FFT_Calculator("FFT Calculator");
SerialDataLink m_SerialDatalink = SerialDataLink("Serial Datalink");
Manager m_Manager = Manager("Manager", m_FFT_Calculator, m_SerialDatalink);

void setup() {
  Serial.begin(500000);
  delay(500);
  Serial << "Xtal Clock Frequency: " << getXtalFrequencyMhz() << " MHz\n";
  Serial << "CPU Clock Frequency: " << getCpuFrequencyMhz() << " MHz\n";
  Serial << "Apb Clock Frequency: " << getApbFrequency() << " Hz\n";

  m_SerialDatalink.Setup();
  m_Manager.Setup();
  
  xTaskCreatePinnedToCore
  (
    Task0Loop,            // Function to implement the task
    "Task0",              // Name of the task
    10000,                // Stack size in words
    NULL,                 // Task input parameter
    1,                    // Priority of the task
    &Task0,               // Task handle.
    0                     // Core where the task should run
  );
  delay(500);
   
  xTaskCreatePinnedToCore
  (
    Task1Loop,            // Function to implement the task
    "Task1",              // Name of the task
    10000,                // Stack size in words
    NULL,                 // Task input parameter
    1,                    // Priority of the task
    &Task1,               // Task handle.
    0                     // Core where the task should run
  );                   
  delay(500);

  xTaskCreatePinnedToCore
  (
    Task2Loop,            // Function to implement the task
    "Task2",              // Name of the task
    10000,                 // Stack size in words
    NULL,                 // Task input parameter
    1,                    // Priority of the task
    &Task2,               // Task handle.
    1                     // Core where the task should run
  );                   
  delay(500); 

  xTaskCreatePinnedToCore
  (
    Task3Loop,            // Function to implement the task
    "Task3",              // Name of the task
    10000,                 // Stack size in words
    NULL,                 // Task input parameter
    1,                    // Priority of the task
    &Task3,               // Task handle.
    1                     // Core where the task should run
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
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
}
void Task1Loop(void * parameter)
{
  while(true)
  {
    m_FFT_Calculator.ProcessEventQueue();
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
}
void Task2Loop(void * parameter)
{
  while(true)
  {
    m_SerialDatalink.ProcessEventQueue();
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
}
void Task3Loop(void * parameter)
{
  while(true)
  {
    m_SerialDatalink.CheckForNewSerialData();
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
}
