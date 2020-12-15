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
    else
    {
      if(true == debugTasks) Serial << "TaskScheduler: RunTask: " << aTask->GetTaskTitle() << ": Not Ready\n";
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
void TaskScheduler::AddTasks(LinkedList<Task*> &tasks)
{
  for(int t = 0; t < tasks.size(); ++t)
  {
    myTasks.add(tasks.get(t));
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
