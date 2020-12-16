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
/**
 * @file LightTower2.ino
 * *

 */

#include "TaskInterface.h"
#include <typeinfo>

void Task::AddTask(Task &task)
{
  m_Scheduler.AddTask(task);
}
void Task::AddTasks(LinkedList<Task*> &tasks)
{
  m_Scheduler.AddTasks(tasks);
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
  if(true == debugTasks) Serial << "TaskScheduler: Try to Run " << myTasks.size() << " Task(s): Start\n";
  for(int t = 0; t < myTasks.size(); ++t)
  {
    Task *aTask = myTasks.get(t);
    if(true==aTask->CanRunMyTask())
    {
      if(true == debugTasks) Serial << "TaskScheduler: RunMyTask: " << aTask->GetTaskTitle() << ": Start\n";
      aTask->RunMyTask();
      aTask->RunScheduler();
      if(true == debugTasks) Serial << "TaskScheduler: RunMyTask: " << aTask->GetTaskTitle() << ": Complete\n";
    }
    else
    {
      if(true == debugTasks) Serial << "TaskScheduler: RunMyTask: " << aTask->GetTaskTitle() << ": Not Ready\n";
    }
  }
  if(true == debugTasks) Serial << "TaskScheduler: RunScheduler: Complete\n";
}
void TaskScheduler::AddTask(Task &task)
{
  if(true == debugTasks) Serial << "TaskScheduler: Adding Task: " << task.GetTaskTitle() << "\n";
  myTasks.add(&task);
  if(false == task.GetIsSetup())
  {
    if(true == debugTasks) Serial << "TaskScheduler: Setup: " << task.GetTaskTitle() << ": Start\n";
    task.Setup();
    task.SetIsSetup(true);
    if(true == debugTasks) Serial << "TaskScheduler: Setup: " << task.GetTaskTitle() << ": Complete\n";
  }
}
void TaskScheduler::AddTasks(LinkedList<Task*> &tasks)
{
  for(int t = 0; t < tasks.size(); ++t)
  {
    myTasks.add((Task*)tasks.get(t));
  }
}
bool TaskScheduler::RemoveTask(Task &task)
{
  if(true == debugTasks) Serial << "TaskScheduler: Remove Task: " << task.GetTaskTitle() << ": Start\n";
  bool taskFound = false;
  for(int i = 0; i < myTasks.size(); ++i)
  {
    if(myTasks.get(i) == &task)
    {
      taskFound = true;
      myTasks.remove(i);
      break;
    }
  }
  if(true == taskFound)
  {
    if(true == debugTasks) Serial << "TaskScheduler: Remove Task: " << task.GetTaskTitle() << ": Success\n";
    return true;
  }
  else
  {
    if(true == debugTasks) Serial << "TaskScheduler: Remove Task: " << task.GetTaskTitle() << ": Fail\n";
    return false;
  }
}
