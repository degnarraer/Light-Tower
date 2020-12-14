

#ifndef TaskInterface_H
#define TaskInterface_H

#include <Arduino.h>
#include <LinkedList.h>
#include "Streaming.h"
#include "Tunes.h"

class Task
{
  public:
    Task(): m_Title("Unnamed"){}
    Task(String title): m_Title(title){}
    String m_Title;
    bool m_IsSetup = false;
    virtual void Setup() = 0;
    virtual void RunTask() = 0;
    virtual bool CanRunTask() = 0;
    String GetTaskTitle() 
    {
      return m_Title;
    }
    void SetTaskTitle(String title) { m_Title = title; }
};

class TaskScheduler
{
  public:
    TaskScheduler(){}
    void RunTasks();
    void AddTask(Task &task);
    unsigned int GetTaskCount() {return myTasks.size();}
  private:
    LinkedList<Task*> myTasks = LinkedList<Task*>();
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
    bool CanRunTask()
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
    void RunTask()
    {
      m_startMillis = millis();
      if(true == debugFPS) Serial << "FPS for " << m_Title << ": " << m_frameCount / (m_lapsedTime/1000.0) << "\n";
      m_frameCount = 0;
    }
  private:
    unsigned long m_startMillis;
    unsigned long m_currentMillis;
    unsigned int m_frameCount;
    
};

#endif
