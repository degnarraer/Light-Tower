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
    void RegisterForNotification(ModelEventNotificationCalleeInterface &callee);
    void DeRegisterForNotification(ModelEventNotificationCalleeInterface &callee);
    bool HasUser();
    void SendNewValueNotificationToCallees(float value, ModelEventNotificationCallerInterface &source);
    virtual void UpdateValue() = 0;
  private:
    LinkedList<ModelEventNotificationCalleeInterface*> m_MyCallees = LinkedList<ModelEventNotificationCalleeInterface*>();
};

class StatisticalEngineModelInterface;
class Model: public Task
           , public ModelEventNotificationCallerInterface
{
  public: 
    Model(StatisticalEngineModelInterface &StatisticalEngineModelInterface, String Title): Task(Title)
                                                                                         , ModelEventNotificationCallerInterface() 
                                                                                         , m_StatisticalEngineModelInterface(StatisticalEngineModelInterface){}
    
    ~Model(){}
    virtual void UpdateValue() = 0;
  protected:
    void SetCurrentValue(float value);
    StatisticalEngineModelInterface &m_StatisticalEngineModelInterface;  
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


class ModelNewValueProcessor : public Task
{
  public:
    ModelNewValueProcessor() : Task("ModelNewValueProcessor"){}
    ~ModelNewValueProcessor(){}
    void AddModel(Model &Model);
    
    //Task
    void Setup();
    bool CanRunMyTask();
    void RunMyTask();
  private:
    LinkedList<Model*> m_MyModels = LinkedList<Model*>();
};

class StatisticalEngineModelInterface : public Task
                                      , ADCInterruptHandler 
                                      , MicrophoneMeasureCalleeInterface
{
  public:
    StatisticalEngineModelInterface() : Task("StatisticalEngineModelInterface"){}
    ~StatisticalEngineModelInterface(){}

    float GetSoundPower() { return m_StatisticalEngine.GetSoundPower(); }
  
    //ADCInterruptHandler
    void HandleADCInterrupt() { m_StatisticalEngine.HandleADCInterrupt(); }
    
    //MicrophoneMeasureCalleeInterface
    void MicrophoneStateChange(SoundState){}
    
    void AddModel(Model &Model) { m_ModelNewValueProcessor.AddModel(Model); }

  private:
    StatisticalEngine m_StatisticalEngine = StatisticalEngine();
    ModelNewValueProcessor m_ModelNewValueProcessor = ModelNewValueProcessor();
    void Setup()
    { 
      m_StatisticalEngine.ConnectCallback(this);
      AddTask(m_StatisticalEngine);
      AddTask(m_ModelNewValueProcessor);
    }
    bool CanRunMyTask(){ return true; }
    void RunMyTask(){ //Only runs scheduler's tasks }
};
class SoundPowerModel: public Model
{
  public:
    SoundPowerModel(StatisticalEngineModelInterface &StatisticalEngineModelInterface, String Title): Model(StatisticalEngineModelInterface, Title){}
    ~SoundPowerModel(){}
    
     //Model
    void UpdateValue() { SetCurrentValue(m_StatisticalEngineModelInterface.GetSoundPower()); }
  private:
     //Model
    void SetupModel(){}
    bool CanRunModelTask(){ return true; }
    void RunModelTask(){}
};
#endif
