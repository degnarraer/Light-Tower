#include "I2S_EventHandler.h"

TaskScheduler scheduler;
I2S_EventHandler i2S_EventHandler;

void setup() {
  Serial.begin(500000);
  scheduler.AddTask(i2S_EventHandler);
}

void loop()
{
  scheduler.RunScheduler();
}
