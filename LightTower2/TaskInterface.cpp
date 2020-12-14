#include "TaskInterface.h"

void TaskScheduler::RunTasks()
{
  if(true == debugTasks) Serial << "TaskScheduler: Try to Run " << myTasks.size() << " Task(s): Start\n";
  for(int t = 0; t < myTasks.size(); ++t)
  {
    Task *aTask = myTasks.get(t);
    if(true==aTask->CanRunTask())
    {
      if(true == debugTasks) Serial << "TaskScheduler: RunTask: " << aTask->GetTaskTitle() << ": Start\n";
      aTask->RunTask();
      if(true == debugTasks) Serial << "TaskScheduler: RunTask: " << aTask->GetTaskTitle() << ": Complete\n";
    }
  }
  if(true == debugTasks) Serial << "TaskScheduler: RunTasks: Complete\n";
}
void TaskScheduler::AddTask(Task &task)
{
  if(true == debugTasks) Serial << "TaskScheduler: Adding Task: " << task.GetTaskTitle() << "\n";
  myTasks.add(&task);
  if(false == task.m_IsSetup)
  {
    if(true == debugTasks) Serial << "TaskScheduler: Setup: " << task.GetTaskTitle() << ": Start\n";
    task.Setup();
    task.m_IsSetup = true;
    if(true == debugTasks) Serial << "TaskScheduler: Setup: " << task.GetTaskTitle() << ": Complete\n";
  }
}
