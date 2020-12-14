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
 
#ifndef Models_H
#define Models_H
#include "TaskInterface.h"
#include "Statistical_Engine.h"
#include <LinkedList.h>
#include "Streaming.h"
#include "Tunes.h"

class StatisticalEngineModelInterface: public Task
                                     , MicrophoneMeasureCalleeInterface
                                     , ADCInterruptHandler
{
  public:
    StatisticalEngineModelInterface(): Task("StatisticalEngineModelInterface"){}
    StatisticalEngine m_StatisticalEngine;
    TaskScheduler m_Scheduler;
    
    //MicrophoneMeasureCalleeInterface
    void MicrophoneStateChange(SoundState){}
    
    //ADCInterruptHandler
    void HandleADCInterrupt() { m_StatisticalEngine.HandleADCInterrupt(); }
  
  private:
    //Task Interface
    void Setup();
    void RunTask();
    bool CanRunTask() {return true;}
};

class ModelEventNotificationCallerInterface;

class ModelEventNotificationCalleeInterface
{
public:
    virtual void NewValueNotificationFrom(float Value, ModelEventNotificationCallerInterface &source) = 0;
};

class ModelEventNotificationCallerInterface
{
  public:
    void RegisterForNotification(ModelEventNotificationCalleeInterface &callee)
    {
      if(true == debugModelNotifications) Serial << "ModelEventNotificationCallerInterface: Add: ";        
      myCallees.add(&callee);
    }
    void DeRegisterForNotification(ModelEventNotificationCalleeInterface &callee)
    {
      for(int i = 0; i < myCallees.size(); ++i)
      {
        if(myCallees.get(i) == &callee)
        {
          myCallees.remove(i);
          break;
        }
      }
    }
    void SendNewValueNotificationToCalleesFrom(float value, ModelEventNotificationCallerInterface &source)
    {
      for(int i = 0; i < myCallees.size(); ++i)
      {
        myCallees.get(i)->NewValueNotificationFrom(value, source);
      }
    }
  private:
    LinkedList<ModelEventNotificationCalleeInterface*> myCallees = LinkedList<ModelEventNotificationCalleeInterface*>();
};

class Model: public Task
           , ModelEventNotificationCallerInterface
{
  public: 
    Model(){}
  private:
    float m_PreviousValue;
    float m_CurrentValue;
    void SetCurrentValue(float value)
    {
      m_CurrentValue = value;
      if(m_PreviousValue != m_CurrentValue)
      {
        SendNewValueNotificationToCalleesFrom(m_CurrentValue, *this);
        m_PreviousValue = m_CurrentValue;
      }
    }
    virtual void Setup() = 0;
    virtual void RunTask() = 0;
    virtual bool CanRunTask() = 0;
};

class SoundPower: public Model
                , Task
                , StatisticalEngineModelInterface
{
  public:
    SoundPower(): Task("SoundPower"){}
    TaskScheduler Scheduler;
  private:  
    void Setup(){};
    void RunTask(){};
    bool CanRunTask(){};
};

#endif
