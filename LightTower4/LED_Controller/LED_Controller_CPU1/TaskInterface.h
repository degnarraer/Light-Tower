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

#ifndef TaskInterface_H
#define TaskInterface_H

#include <Arduino.h>
#include <LinkedList.h>
#include "Streaming.h"
#include "Tunes.h"

class Task;
class TaskScheduler
{
  public:
    TaskScheduler(){}
    void RunScheduler();
    void AddTask(Task &task);
    void AddTasks(LinkedList<Task*> &tasks);
    bool RemoveTask(Task &task);
    unsigned int GetTaskCount(){return m_MyTasks.size();}
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
    unsigned int GetTaskCount(){return m_Scheduler.GetTaskCount();}
    bool RemoveTask(Task &task);
    void RunScheduler();
    virtual void Setup() = 0;
    virtual void RunMyPreTask() = 0;
    virtual bool CanRunMyScheduledTask() = 0;
    virtual void RunMyScheduledTask() = 0;
    virtual void RunMyPostTask() = 0;
  private:
    TaskScheduler m_Scheduler;
    bool m_IsSetup = false;
    String m_Title;
    void SetTaskTitle(String title) { m_Title = title; }
    
};

class CalculateFPS: public Task
{
  public:
    CalculateFPS(String title, unsigned int updatePeriodMillis)
      : m_Title(title)
      , m_updatePeriodMillis(updatePeriodMillis)
      , Task("CalculateFPS"){}
    String m_Title;
    unsigned int m_updatePeriodMillis;
    unsigned long m_lapsedTime;
    void Setup()
    {
      m_startMillis = millis();
      m_frameCount = 0;
    }
    void RunMyPreTask(){}
    bool CanRunMyScheduledTask()
    {
      ++m_frameCount;
      m_currentMillis = millis();
      m_lapsedTime = m_currentMillis - m_startMillis;
      if(m_lapsedTime >= m_updatePeriodMillis)
      {
        return true;
      }
      else
      {
        return false;
      }
    }
    void RunMyScheduledTask()
    {
      m_startMillis = millis();
      if(true == debugFPS) Serial << "FPS for " << m_Title << ": " << m_frameCount / (m_lapsedTime/1000.0) << "\n";
      m_frameCount = 0;
    }
    void RunMyPostTask(){}
  private:
    unsigned long m_startMillis;
    unsigned long m_currentMillis;
    unsigned int m_frameCount;
    
};

#endif
