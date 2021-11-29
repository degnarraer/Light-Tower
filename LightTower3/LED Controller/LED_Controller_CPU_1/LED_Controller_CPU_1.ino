#include "I2S_Device_Manager.h"
#include "FFT_Calculator.h"
#include "SerialDataLink.h"

TaskHandle_t Task1; //240 MHz CPU
TaskHandle_t Task0; //80 MHz CPU

DataManager m_DataManager;

//Task Schedulers
TaskScheduler m_Task0_Scheduler("Task_0_Scheduler", m_DataManager);
TaskScheduler m_Task1_Scheduler("Task_1_Scheduler", m_DataManager);

//Tasks
I2S_Device_Manager m_I2S_Device_Manager("I2S_Event_Handler", m_DataManager);
FFT_Calculator m_FFT_Calculator("FFT_Calculator", m_DataManager);
SerialDataLink m_SerialDataLink("Serial_Data_Link", m_DataManager);

void setup() {
  Serial.begin(500000);
  delay(500);
  Serial << "Xtal Clock Frequency: " << getXtalFrequencyMhz() << " MHz\n";
  Serial << "CPU Clock Frequency: " << getCpuFrequencyMhz() << " MHz\n";
  Serial << "Apb Clock Frequency: " << getApbFrequency() << " Hz\n";
  
  //Task 0 Tasks
  m_Task0_Scheduler.AddTask(m_I2S_Device_Manager);
  //m_Task0_Scheduler.AddTask(m_FFT_Calculator);
  
  //Task 1 Tasks
  //m_Task1_Scheduler.AddTask(m_SerialDataLink);

  m_DataManager.InitializeDataManager();
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
      1,          // Priority of the task
      &Task1,     // Task handle.
      1);         // Core where the task should run
  delay(500);
}

void Task1Loop(void * parameter)
{
  while(true)
  {
    m_Task1_Scheduler.RunScheduler();
    //TestFunction();
    delay(1);
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

void TestFunction()
{
  static int x = 0;
  int tester[10] = {0*x,1*x,2*x,3*x,4*x,5*x,6*x,7*x,8*x,9*x};
  m_DataManager.SetValue("Test", tester, 10);
  Serial << "Test1: ";
  for(int i = 0; i<10; ++i)
  {
    int *t = m_DataManager.GetValue<int>("Test", 10);
    Serial << t[i] << " ";
  }
  Serial << "\n";
  Serial << "Test2: "; 
  for(int i = 0; i<10; ++i)
  {
    Serial << (int)(m_DataManager.GetValue<int>("Test", 10))[i] << " ";
  }
  Serial << "\n";
  ++x;
  delay(1000);
}

void loop()
{
}
