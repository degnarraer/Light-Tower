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
#include "Statistical_Engine.h"


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
           , public ModelEventNotificationCallerInterface
{
  public: 
    Model(StatisticalEngineInterface &statisticalEngineInterface): Task("Model")
                                                                 , m_StatisticalEngineInterface(statisticalEngineInterface){}
    ~Model(){}
    
  protected:
    void SetCurrentValue(float value)
    {
      m_CurrentValue = value;
      if(m_PreviousValue != m_CurrentValue)
      {
        SendNewValueNotificationToCalleesFrom(m_CurrentValue, *this);
        m_PreviousValue = m_CurrentValue;
      }
    }  
  protected:
    StatisticalEngineInterface m_StatisticalEngineInterface;  
  private:
    void Setup();
    bool CanRunMyTask();
    void RunMyTask();
    virtual void SetupModel() = 0;
    virtual bool CanRunModelTask() = 0;
    virtual void RunModelTask() = 0;
    float m_PreviousValue;
    float m_CurrentValue;
    
};

class SoundPowerModel: public Model
{
  public:
    SoundPowerModel(StatisticalEngineInterface &statisticalEngineInterface): Model(statisticalEngineInterface){}
    ~SoundPowerModel(){}
    float GetSoundPower() { return m_StatisticalEngineInterface.GetSoundPower(); }
  private:  
    void SetupModel(){}
    bool CanRunModelTask(){ return true; }
    void RunModelTask(){}
};

#endif
