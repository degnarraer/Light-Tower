#ifndef TaskInterface_H
#define TaskInterface_H

#include <Arduino.h>
#include "Streaming.h"
#include "Tunes.h"

class Task
{
  public:
    Task(): m_Title("Unnamed"){}
    Task(String title): m_Title(title){}
    String m_Title;
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
    TaskScheduler(Task **tasks, unsigned int numTasks)
      : m_tasks(tasks)
      , m_numTasks(numTasks){}
    void SetTasks(Task **tasks, unsigned int numTasks)
    {
      m_tasks = tasks;
      m_numTasks = numTasks;
    }
    void Setup();
    void RunTasks();
  private:
    Task **m_tasks;
    unsigned int m_numTasks = 0;
};

class CalculateFPS: public Task
{
  public:
    CalculateFPS(String title, unsigned int updatePeriodMillis)
      : m_updatePeriodMillis(updatePeriodMillis)
      , Task("CalculateFPS"){}
    String m_Title;
    int m_updatePeriodMillis;
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
