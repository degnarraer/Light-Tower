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
#include <LinkedList.h>
#include "Streaming.h"
#include "Tunes.h"


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

class Model: public ModelEventNotificationCallerInterface
{
  public: 
    Model(){}
    ~Model(){}
    
  protected:
    TaskScheduler m_Scheduler;  
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
};

class SoundPower: public Model
                , public Task
{
  public:
    SoundPower() : Task("Sound Power"){}
    ~SoundPower(){}
    TaskScheduler m_Scheduler;
  private:  
    void Setup(){}
    void RunTask(){}
    bool CanRunTask(){ return true; }
};

#endif
