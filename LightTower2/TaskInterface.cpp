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
  if(true == debugTasks) Serial << "TaskScheduler: Setup Complete\n";
}
void TaskScheduler::RunTaskLoops()
{
  if(true == debugTasks) Serial << "TaskScheduler: RunTaskLoops: Start\n";
  Task **tpp = m_tasks;
  for(int t = 0; t < m_numTasks; ++t)
  {
    Task *tp = *tpp;
    if(true == tp->CanRunTaskLoop())
    {
      if(true == debugTasks) Serial << "TaskScheduler: RunTaskLoop: " << tp->GetTaskTitle() << ": Start\n";
      tp->RunTaskLoop();
      if(true == debugTasks) Serial << "TaskScheduler: RunTaskLoop: " << tp->GetTaskTitle() << ": Complete\n";
      break;
    }
    ++tpp;
  }
  if(true == debugTasks) Serial << "TaskScheduler: RunTaskLoops: Complete\n";
}
