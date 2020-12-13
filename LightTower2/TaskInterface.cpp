#include "TaskInterface.h"


void TaskScheduler::Setup()
{
  if(true == debugMode && debugLevel >= 0) Serial << "TaskScheduler: Setup Start\n";
  Task **tpp = m_Tasks;
  for(int t = 0; t < m_NumTasks; ++t)
  {
    Task *tp = *(tpp+t);
    if(true == true) Serial << "TaskScheduler: Setup Task: " << tp->GetTaskTitle() << " Start\n";
    tp->Setup();
    if(true == debugTasks) Serial << "TaskScheduler: Setup Task: " << tp->GetTaskTitle() << " Complete\n";
  }
  if(true == debugMode && debugLevel >= 0) Serial << "TaskScheduler: Setup Complete\n";
}
void TaskScheduler::RunTasks()
{
  if(true == debugTasks) Serial << "TaskScheduler: Run " << m_NumTasks << " Task(s): Start\n";
  Task **tpp = m_Tasks;
  for(int t = 0; t < m_NumTasks; ++t)
  {
    Task *tp = *(tpp+t);
    if(true == tp->CanRunTask())
    {
      if(true == debugTasks) Serial << "TaskScheduler: RunTask: " << tp->GetTaskTitle() << ": Start\n";
      tp->RunTask();
      if(true == debugTasks) Serial << "TaskScheduler: RunTask: " << tp->GetTaskTitle() << ": Complete\n";
    }
  }
  if(true == debugTasks) Serial << "TaskScheduler: RunTasks: Complete\n";
}
