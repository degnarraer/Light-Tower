/*
    Light Tower by Rob Shockency
    Copyright (C) 2021 Rob Shockency degnarraer@yahoo.com

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

#include "Task.h"
#include <typeinfo>

void Task::AddTask(Task &task)
{
  m_Scheduler.AddTask(task);
}
bool Task::RemoveTask(Task &task)
{
  m_Scheduler.RemoveTask(task);
}
void Task::RunScheduler()
{
  m_Scheduler.RunScheduler(); 
}
void TaskScheduler::RunScheduler()
{
  for(int t = 0; t < m_MyTasks.size(); ++t)
  {
    Task *aTask = m_MyTasks.get(t);
    aTask->RunScheduler();
    if(true==aTask->CanRunMyTask())
    {
      aTask->RunMyTask();
    }
  }
}
void TaskScheduler::AddTask(Task &task)
{
  bool taskFound = false;
  for(int i = 0; i < m_MyTasks.size(); ++i)
  {
    if(m_MyTasks.get(i) == &task)
    {
      taskFound = true;
      break;
    }
  }
  if(true != taskFound)
  {
    if(true == TASKS_DEBUG) Serial << m_Title << ": Adding Task: " << task.GetTaskTitle() << "\n";
    m_MyTasks.add(&task);
  }
  if(false == task.GetIsSetup())
  {
    if(true == TASKS_DEBUG) Serial << m_Title << ": Setting Up Task: " << task.GetTaskTitle() << "\n";
    task.Setup();
    task.SetIsSetup(true);
  }
}
bool TaskScheduler::RemoveTask(Task &task)
{
  bool taskFound = false;
  for(int i = 0; i < m_MyTasks.size(); ++i)
  {
    if(m_MyTasks.get(i) == &task)
    {
      taskFound = true;
      if(true == TASKS_DEBUG) Serial << m_Title << ": Removing Task: " << m_MyTasks.get(i)->GetTaskTitle() << "\n";
      m_MyTasks.remove(i);
      break;
    }
  }
  if(true == taskFound)
  {
    return true;
  }
  else
  {
    return false;
  }
}
