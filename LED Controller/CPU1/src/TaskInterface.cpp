/*
    Light Tower by Rob Shockency
    Copyright (C) 2020 Rob Shockency degnarraer@yahoo.com

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version of the License, or
    (at your option) any later version. 3

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "TaskInterface.h"
#include <typeinfo>

void TaskSchedulerTask::AddTask(TaskSchedulerTask&task)
{
  m_Scheduler.AddTask(task);
}
void TaskSchedulerTask::AddTasks(std::vector<TaskSchedulerTask*> &tasks)
{
  m_Scheduler.AddTasks(tasks);
}
bool TaskSchedulerTask::RemoveTask(TaskSchedulerTask&task)
{
  return m_Scheduler.RemoveTask(task);
}
void TaskSchedulerTask::RunScheduler()
{
  m_Scheduler.RunScheduler();
}

void TaskScheduler::RunScheduler()
{
  if(true == debugTasks && m_MyTasks.size() > 0) Serial << "TaskScheduler Trying to Run " << m_MyTasks.size() << " Task(s)\n";
  for(int t = 0; t < m_MyTasks.size(); ++t)
  {
    TaskSchedulerTask *aTask = m_MyTasks[t];
    aTask->RunMyPreTask();
    aTask->RunScheduler();
    if(true==aTask->CanRunMyScheduledTask())
    {
      if(true == debugTasks) Serial << "TaskScheduler Running Task: " << aTask->GetTaskTitle() << "\n";
      aTask->RunMyScheduledTask();
    }
    else
    {
      if(true == debugTasks) Serial << "TaskScheduler Task Not Ready: " << aTask->GetTaskTitle() << "\n";
    }
    aTask->RunMyPostTask();
  }
}
void TaskScheduler::AddTask(TaskSchedulerTask &task)
{
  if(true == debugTasks) Serial << "TaskScheduler Adding Task: " << task.GetTaskTitle() << "\n";
  m_MyTasks.push_back(&task);
  if(false == task.GetIsSetup())
  {
    if(true == debugTasks) Serial << "TaskScheduler Setting Up Task: " << task.GetTaskTitle() << "\n";
    task.Setup();
    task.SetIsSetup(true);
  }
}
void TaskScheduler::AddTasks(std::vector<TaskSchedulerTask*> &tasks)
{
  for(int t = 0; t < tasks.size(); ++t)
  {
    m_MyTasks.push_back(tasks[t]);
  }
}
bool TaskScheduler::RemoveTask(TaskSchedulerTask &task)
{
  bool taskFound = false;
  for(int i = 0; i < m_MyTasks.size(); ++i)
  {
    if(m_MyTasks[i] == &task)
    {
      taskFound = true;
      m_MyTasks.erase(m_MyTasks.begin() + i);
      break;
    }
  }
  if(true == taskFound)
  {
    if(true == debugTasks || true == debugMemory) Serial << "TaskScheduler Successfully Removed Task: " << task.GetTaskTitle() << "\n";
    return true;
  }
  else
  {
    if(true == debugTasks || true == debugMemory) Serial << "TaskScheduler failed to Remove Task: " << task.GetTaskTitle() << "\n";
    return false;
  }
}
