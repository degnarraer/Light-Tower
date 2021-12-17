#include "Manager.h"
#include "Serial_Datalink_Config.h"

TaskHandle_t Task0;

Manager m_Manager = Manager("Manager");
SerialDataLink m_SerialDatalink = SerialDataLink("Serial Datalink");

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
