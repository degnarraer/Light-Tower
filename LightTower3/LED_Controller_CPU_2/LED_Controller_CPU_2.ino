#include "I2S_EventHandler.h"
#include <HardwareSerial.h>

TaskHandle_t Task1; //240 MHz CPU
TaskHandle_t Task0; //80 MHz CPU
HardwareSerial &hSerial = Serial2;

DataManager m_DataManager;
TaskScheduler m_Task0_Scheduler("Task_0_Scheduler", m_DataManager);
TaskScheduler m_Task1_Scheduler("Task_1_Scheduler", m_DataManager);
I2S_EventHandler m_i2S_EventHandler("Event_Handler", m_DataManager);

void setup() {
  Serial.begin(500000);
  hSerial.begin(500000, SERIAL_8N1, 16, 17); // pins 16 rx2, 17 tx2, 19200 bps, 8 bits no parity 1 stop bit

  delay(500);
  Serial << "Xtal Clock Frequency: " << getXtalFrequencyMhz() << " MHz\n"; // In MHz
  Serial << "CPU Clock Frequency: " << getCpuFrequencyMhz() << " MHz\n"; // In MHz
  Serial << "Apb Clock Frequency: " << getApbFrequency() << " Hz\n"; // In Hz
  m_Task0_Scheduler.AddTask(m_i2S_EventHandler);
  xTaskCreatePinnedToCore(
      Task0Loop,  // Function to implement the task
      "Task0",    // Name of the task
      5000,       // Stack size in words
      NULL,       // Task input parameter
      1,          // Priority of the task
      &Task0,     // Task handle.
      0);         // Core where the task should run
  delay(500);
  xTaskCreatePinnedToCore(
      Task1Loop,  // Function to implement the task
      "Task1",    // Name of the task
      5000,       // Stack size in words
      NULL,       // Task input parameter
      1,         // Priority of the task
      &Task1,     // Task handle.
      1);         // Core where the task should run
  delay(500);
}

void Task1Loop(void * parameter)
{
  while(true)
  {
    m_Task1_Scheduler.RunScheduler();
    int bytesWrote = hSerial.println("I am Here!");
    Serial << "Wrote " << bytesWrote << " Bytes\n";
    delay(1000);
  }
}
void Task0Loop(void * parameter)
{
  while(true)
  {
    m_Task0_Scheduler.RunScheduler();
    delay(1);
  }
}

void loop()
{
}
