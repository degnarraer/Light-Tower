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

#ifndef Task_H
#define Task_H

#include <Arduino.h>
#include <LinkedList.h>

class Task;
class TaskScheduler
{
  public:
    TaskScheduler(){}
    void RunScheduler();
    void AddTask(Task &task);
    void AddTasks(LinkedList<Task*> &tasks);
    bool RemoveTask(Task &task);
    unsigned int GetTaskCount() { return m_MyTasks.size(); }
  private:
    LinkedList<Task*> m_MyTasks = LinkedList<Task*>();
};
class Task
{
  public:
    Task(): m_Title("Unnamed"){}
    Task(String title): m_Title(title){}
    String GetTaskTitle() { return m_Title; }
    bool GetIsSetup() { return m_IsSetup; }
    bool SetIsSetup(bool IsSetup) { m_IsSetup = IsSetup; }
    void AddTask(Task &task);
    void AddTasks(LinkedList<Task*> &tasks);
    unsigned int GetTaskCount(){ return m_Scheduler.GetTaskCount(); }
    bool RemoveTask(Task &task);
    void RunScheduler();
    virtual void Setup() = 0;
    virtual bool CanRunMyTask() = 0;
    virtual void RunMyTask() = 0;
  private:
    TaskScheduler m_Scheduler;
    bool m_IsSetup = false;
    String m_Title;
    void SetTaskTitle(String title) { m_Title = title; } 
};

#endif
