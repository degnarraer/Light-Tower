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
    virtual void NewFloatValueNotificationFrom(float Value, ModelEventNotificationCallerInterface &source) = 0;
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
    Model(String Title, StatisticalEngineModelInterface &StatisticalEngineModelInterface): Task(Title)
                                                                                         , ModelEventNotificationCallerInterface() 
                                                                                         , m_StatisticalEngineModelInterface(StatisticalEngineModelInterface){}
    ~Model(){}

    //ModelEventNotificationCallerInterface
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
    bool RemoveModel(Model &Model);

  private:
    //Task
    void Setup();
    bool CanRunMyTask();
    void RunMyTask();
    LinkedList<Model*> m_MyModels = LinkedList<Model*>();
};

class StatisticalEngineModelInterface : public Task
                                      , ADCInterruptHandler 
                                      , MicrophoneMeasureCalleeInterface
{
  public:
    StatisticalEngineModelInterface() : Task("StatisticalEngineModelInterface"){}
    ~StatisticalEngineModelInterface(){}

    //StatisticalEngine Getters
    float GetSoundPower() { return m_StatisticalEngine.GetSoundPower(); }
    float GetBandAverage(unsigned int band, unsigned int depth) { return m_StatisticalEngine.GetBandAverage(band, depth); }
    float GetBandAverageForABandOutOfNBands(unsigned int band, unsigned int depth, unsigned int totalBands) { return m_StatisticalEngine.GetBandAverageForABandOutOfNBands(band, depth, totalBands); }
  
    //ADCInterruptHandler
    void HandleADCInterrupt() { m_StatisticalEngine.HandleADCInterrupt(); }
    
    //MicrophoneMeasureCalleeInterface
    void MicrophoneStateChange(SoundState){}
    void AddModel(Model &Model);
    bool RemoveModel(Model &Model);

  private:
    StatisticalEngine m_StatisticalEngine;
    ModelNewValueProcessor m_ModelNewValueProcessor;

    //Task
    void Setup();
    bool CanRunMyTask();
    void RunMyTask();
};

class SoundPowerModel: public Model
{
  public:
    SoundPowerModel(String Title, StatisticalEngineModelInterface &StatisticalEngineModelInterface): Model(Title, StatisticalEngineModelInterface){}
    ~SoundPowerModel(){}
    
     //Model
    void UpdateValue() { SetCurrentValue(m_StatisticalEngineModelInterface.GetSoundPower()); }
  private:
     //Model
    void SetupModel(){}
    bool CanRunModelTask(){ return true; }
    void RunModelTask(){}
};

class BandPowerModel: public Model
{
  public:
    BandPowerModel(String Title, unsigned int Band, StatisticalEngineModelInterface &StatisticalEngineModelInterface): Model(Title, StatisticalEngineModelInterface)
                                                                                                                             , m_Band(Band){}
    ~BandPowerModel(){}
    
     //Model
    void UpdateValue()
    {
      float value = (m_StatisticalEngineModelInterface.GetBandAverage(m_Band, 1) / (float)ADDBITS);
      if(true == debugModels) Serial << "BandPowerModel value: " << value << " for band: " << m_Band << "\n";
      SetCurrentValue( value );
    }
  private:
     //Model
    unsigned int m_Band = 0;
    void SetupModel(){}
    bool CanRunModelTask(){ return true; }
    void RunModelTask(){}
};

class ReducedBandsBandPowerModel: public Model
{
  public:
    ReducedBandsBandPowerModel( String Title
                              , unsigned int band
                              , unsigned int depth
                              , unsigned int totalBands
                              , StatisticalEngineModelInterface &StatisticalEngineModelInterface)
                              : Model(Title, StatisticalEngineModelInterface)
                              , m_Band(band)
                              , m_Depth(depth)
                              , m_TotalBands(totalBands){}
    ~ReducedBandsBandPowerModel(){}
    
     //Model
    void UpdateValue()
    {
      float value = (m_StatisticalEngineModelInterface.GetBandAverageForABandOutOfNBands(m_Band, m_Depth, m_TotalBands) / (float)ADDBITS);
      if(true == debugModels) Serial << "ReducedBandsBandPowerModel value: " << value << " for band: " << m_Band << " of " << m_TotalBands << " bands\n";
      SetCurrentValue( value );
    }
  private:
     //Model
    unsigned int m_Band = 0;
    unsigned int m_Depth = 0;
    unsigned int m_TotalBands = 0;
    void SetupModel(){}
    bool CanRunModelTask(){ return true; }
    void RunModelTask(){}
};
#endif
