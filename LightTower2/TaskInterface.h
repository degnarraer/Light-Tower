#include <Arduino.h>
#include "Tunes.h"

class TaskInterface
{
  public:
    TaskInterface(){}
    virtual void Setup() = 0;
    virtual bool Loop() = 0;
};

class CalculateFPS: public TaskInterface
{
  public:
    CalculateFPS(){}
  private:
    unsigned long m_startMillis;
    unsigned long m_currentMillis;
    unsigned int m_frameCount;
    void Setup()
    {
      m_startMillis = millis();
      m_frameCount = 0;
    }
    bool Loop()
    {
      ++m_frameCount;
      m_currentMillis = millis();
      unsigned long lapsedTime = m_currentMillis - m_startMillis;
      if(lapsedTime >= 1000)
      {
        m_startMillis = millis();
        if(true == debugFPS) Serial << "FPS: " << m_frameCount / (lapsedTime/1000.0) << "\n";
        m_frameCount = 0;
      } 
    }
};
