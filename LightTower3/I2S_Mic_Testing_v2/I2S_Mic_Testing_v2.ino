#include "I2S_EventHandler.h"

TaskHandle_t Task1; //240 MHz CPU
TaskHandle_t Task0; //80 MHz CPU


DataManager dataManager;
TaskScheduler scheduler0(dataManager);
TaskScheduler scheduler1(dataManager);
I2S_EventHandler i2S_EventHandler("Event Handler", dataManager);

void setup() {
  Serial.begin(500000);
  scheduler1.AddTask(i2S_EventHandler);
  
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
    scheduler1.RunScheduler();
  }
}
void Task0Code(void * parameter)
{
  for(;;)
  {
    scheduler0.RunScheduler();
  }
}

void loop()
{
}
