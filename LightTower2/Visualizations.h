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

#ifndef Visualization_H
#define Visualization_H

#include "Statistical_Engine.h"
#include "Streaming.h"
#include "Models.h"
#include "Views.h"
#include "TaskInterface.h"
#include "LEDControllerInterface.h"
#include <LinkedList.h>

class VisualizationEventNotificationCallerInterface;

class VisualizationEventNotificationCalleeInterface
{
public:
    virtual void VisualizationCompleteNotificationFrom(VisualizationEventNotificationCallerInterface &source) = 0;
};

class VisualizationEventNotificationCallerInterface
{
  public:
    void RegisterForNotification(VisualizationEventNotificationCalleeInterface &callee)
    {
      if(true == debugModelNotifications) Serial << "VisualizationEventNotificationCallerInterface: Add: ";        
      myCallees.add(&callee);
    }
    void DeRegisterForNotification(VisualizationEventNotificationCalleeInterface &callee)
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
    void SendVisualizationCompleteNotificationToCalleesFrom(VisualizationEventNotificationCallerInterface &source)
    {
      for(int i = 0; i < myCallees.size(); ++i)
      {
        myCallees.get(i)->VisualizationCompleteNotificationFrom(source);
      }
    }
  private:
    LinkedList<VisualizationEventNotificationCalleeInterface*> myCallees = LinkedList<VisualizationEventNotificationCalleeInterface*>();
};

class Visualization: public Task
                   , VisualizationEventNotificationCallerInterface
                   , ModelEventNotificationCalleeInterface
{
  public:
    Visualization( StatisticalEngineModelInterface &StatisticalEngineModelInterface, 
                   LEDController &LEDController) : Task("Visualization")
                                                 , m_StatisticalEngineModelInterface(StatisticalEngineModelInterface)
                                                 , m_LEDController(LEDController)
                                                 , ModelEventNotificationCalleeInterface(){}
    ~Visualization()
    {
      if(true == debugMode && debugLevel >= 1) Serial << "Delete Visualization\n";
    }
    StatisticalEngineModelInterface &m_StatisticalEngineModelInterface;
    LEDController &m_LEDController;
    void AddSubView(View &view);
    void AddModel(Model &model);
    
    virtual Visualization* GetInstance(StatisticalEngineModelInterface &StatisticalEngineModelInterface, LEDController &LEDController) = 0;
    virtual void SetupVisualization() = 0;
    virtual bool CanRunVisualization() = 0;
    virtual void RunVisualization() = 0;

    //Task Interface
    void Setup();
    bool CanRunMyTask();
    void RunMyTask();

    //ModelEventNotificationCalleeInterface
    virtual void NewValueNotificationFrom(float Value, ModelEventNotificationCallerInterface &source) = 0;
  protected:
    unsigned int m_Duration = 0;
  private:
    LinkedList<View*> m_MyViews = LinkedList<View*>();
    LinkedList<Model*> m_MyModels = LinkedList<Model*>();
    LinkedList<Task*> m_MyTasks = LinkedList<Task*>();
};


//********* VUMeter *********
class VUMeter: public Visualization
{
  public:
    VUMeter( StatisticalEngineModelInterface &StatisticalEngineModelInterface, LEDController &LEDController) : Visualization( StatisticalEngineModelInterface, LEDController)
    {
    }
    ~VUMeter()
    {
      if(true == debugMode && debugLevel >= 1) Serial << "Delete VUMeter\n";
    }

    //Visualization
    Visualization* GetInstance(StatisticalEngineModelInterface &StatisticalEngineModelInterface, LEDController &LEDController)
    {
      VUMeter *vis = new VUMeter(StatisticalEngineModelInterface, LEDController);
      return vis;
    }
    void NewValueNotificationFrom(float Value, ModelEventNotificationCallerInterface &source) 
    {
    }
    void SetupVisualization()
    {
      AddModel(m_SoundPower);
      AddSubView(m_VerticalBar);
      m_VerticalBar.SetModel(m_SoundPower);
    }
    bool CanRunVisualization(){ return true; }
    void RunVisualization()
    {
      m_LEDController.UpdateLEDs(m_VerticalBar.GetPixels());
    }
  private:
    VerticalBarView m_VerticalBar = VerticalBarView(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, "Vertical Bar");
    SoundPowerModel m_SoundPower = SoundPowerModel(m_StatisticalEngineModelInterface, "Power Model");       
};

#endif
