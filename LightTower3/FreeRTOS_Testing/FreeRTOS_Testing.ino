#include "I2S_Manager.h"
#include "FFT_Calculator.h"

TaskHandle_t Task0;
TaskHandle_t Task1;

FFT_Calculator m_FFT_Calculator = FFT_Calculator("FFT Calculator");
I2S_Manager* m_I2S_Manager = new I2S_Manager("I2S Manager", m_FFT_Calculator);

void setup() {
  Serial.begin(500000);
  delay(500);
  Serial << "Xtal Clock Frequency: " << getXtalFrequencyMhz() << " MHz\n";
  Serial << "CPU Clock Frequency: " << getCpuFrequencyMhz() << " MHz\n";
  Serial << "Apb Clock Frequency: " << getApbFrequency() << " Hz\n";

  m_I2S_Manager->Setup();
  
  xTaskCreatePinnedToCore(
    Task0Loop,            // Function to implement the task
    "Task0",              // Name of the task
    20000,                // Stack size in words
    NULL,                 // Task input parameter
    10,                    // Priority of the task
    &Task0,               // Task handle.
    1);                   // Core where the task should run
  delay(500); 
   
  xTaskCreatePinnedToCore(
    Task1Loop,            // Function to implement the task
    "Task1",              // Name of the task
    20000,                // Stack size in words
    NULL,                 // Task input parameter
    1,                    // Priority of the task
    &Task1,               // Task handle.
    0);                   // Core where the task should run
  delay(500);

}

void loop() {
  // put your main code here, to run repeatedly:

}

void Task0Loop(void * parameter)
{
  while(true)
  {
    m_I2S_Manager->RunTask();
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
