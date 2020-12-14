#include "TaskInterface.h"

void TaskScheduler::SetTasks(LinkedList<Task*> &tasks)
{
  AddTasks(tasks);
}
void TaskScheduler::RunTasks()
{
  if(true == debugTasks) Serial << "TaskScheduler: Run " << myTasks.size() << " Task(s): Start\n";
  
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
void TaskScheduler::AddTask(Task *task)
{
  if(true == debugTasks) Serial << "TaskScheduler: Adding 1 Task";
  myTasks.add(task);
  if(false == task->m_IsSetup)
  {
    task->Setup();
    task->m_IsSetup = true;
  }
}
void TaskScheduler::AddTasks(LinkedList<Task*> &tasks)
{
  if(true == debugTasks) Serial << "TaskScheduler: Adding " << tasks.size() << " Task(s)";
  for(int t = 0; t < tasks.size(); ++t)
  {
    Task *aTask = tasks.get(t);
    myTasks.add(aTask);
    if(false == aTask->m_IsSetup)
    {
      aTask->Setup();
      aTask->m_IsSetup = true;
    }
  }
}
