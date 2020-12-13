#include "TaskInterface.h"


void TaskScheduler::Setup()
{
  if(true == debugMode && debugLevel >= 0) Serial << "TaskScheduler: Setup Start\n";
  Task **tpp = m_tasks;
  for(int t = 0; t < m_numTasks; ++t)
  {
    Task *tp = *tpp;
    if(true == debugTasks) Serial << "TaskScheduler: Setup Task: " << tp->GetTaskTitle() << " Start\n";
    tp->Setup();
    if(true == debugTasks) Serial << "TaskScheduler: Setup Task: " << tp->GetTaskTitle() << " Complete\n";
    ++tpp;
  }
  if(true == debugMode && debugLevel >= 0) Serial << "TaskScheduler: Setup Complete\n";
}
void TaskScheduler::RunTasks()
{
  if(true == debugTasks) Serial << "TaskScheduler: RunTasks: Start\n";
  Task **tpp = m_tasks;
  for(int t = 0; t < m_numTasks; ++t)
  {
    Task *tp = *tpp;
    if(true == tp->CanRunTask())
    {
      if(true == debugTasks) Serial << "TaskScheduler: RunTask: " << tp->GetTaskTitle() << ": Start\n";
      tp->RunTask();
      if(true == debugTasks) Serial << "TaskScheduler: RunTask: " << tp->GetTaskTitle() << ": Complete\n";
      break;
    }
    ++tpp;
  }
  if(true == debugTasks) Serial << "TaskScheduler: RunTasks: Complete\n";
}
