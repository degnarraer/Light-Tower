#include "I2S_EventHandler.h"

TaskHandle_t Task1; //240 MHz CPU
TaskHandle_t Task0; //80 MHz CPU


DataManager m_DataManager;
TaskScheduler m_Task0_Scheduler(m_DataManager);
TaskScheduler m_Task1_Scheduler(m_DataManager);
I2S_EventHandler i2S_EventHandler("Event Handler", m_DataManager);

void setup() {
  Serial.begin(500000);
  delay(500);
  Serial << "Xtal Clock Frequency: " << getXtalFrequencyMhz() << " MHz\n"; // In MHz
  Serial << "CPU Clock Frequency: " << getCpuFrequencyMhz() << " MHz\n"; // In MHz
  Serial << "Apb Clock Frequency: " << getApbFrequency() << " Hz\n"; // In Hz
  m_Task1_Scheduler.AddTask(i2S_EventHandler);
  xTaskCreatePinnedToCore(
      Task1Code, /* Function to implement the task */
      "Task1", /* Name of the task */
      2000,  /* Stack size in words */
      NULL,  /* Task input parameter */
      10,  /* Priority of the task */
      &Task1,  /* Task handle. */
      1); /* Core where the task should run */
  delay(500);
  xTaskCreatePinnedToCore(
      Task0Code,  // Function to implement the task
      "Task0",    // Name of the task
      2000,      // Stack size in words
      NULL,       // Task input parameter
      5,         // Priority of the task
      &Task0,     // Task handle.
      0);         // Core where the task should run
  delay(500);
}

void Task1Code(void * parameter)
{
  for(;;)
  {
    m_Task1_Scheduler.RunScheduler();
  }
}
void Task0Code(void * parameter)
{
  for(;;)
  {
    m_Task0_Scheduler.RunScheduler();
  }
}

void loop()
{
}
