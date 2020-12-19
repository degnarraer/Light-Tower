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
                                                 , m_LEDController(LEDController){}
    ~Visualization()
    {
      if(true == debugMode && debugLevel >= 1) Serial << "Delete Visualization\n";
    }
    StatisticalEngineModelInterface &m_StatisticalEngineModelInterface;
    LEDController &m_LEDController;
    void AddView(View &View);
    void AddModel(Model &Model);
    PixelStruct& GetPixelStruct() { return m_MyPixelStruct; }
    
    virtual Visualization* GetInstance(StatisticalEngineModelInterface &StatisticalEngineModelInterface, LEDController &LEDController) = 0;
    virtual void SetupVisualization() = 0;
    virtual bool CanRunVisualization() = 0;
    virtual void RunVisualization() = 0;

    //Task Interface
    void Setup();
    bool CanRunMyTask();
    void RunMyTask();

  protected:
    unsigned int m_Duration = 0;
    LinkedList<View*> m_MyViews = LinkedList<View*>();
  private:
    PixelStruct m_MyPixelStruct;
    void MergeSubViews();
};


//********* VUMeter *********
class VUMeter: public Visualization
{
  public:
    VUMeter( StatisticalEngineModelInterface &StatisticalEngineModelInterface, LEDController &LEDController) : Visualization( StatisticalEngineModelInterface, LEDController){}
    ~VUMeter(){if(true == debugMode && debugLevel >= 1) Serial << "Delete VUMeter\n";}

    //Visualization
    Visualization* GetInstance(StatisticalEngineModelInterface &StatisticalEngineModelInterface, LEDController &LEDController)
    {
      VUMeter *vis = new VUMeter(StatisticalEngineModelInterface, LEDController);
      return vis;
    }
    void NewFloatValueNotificationFrom(float Value, ModelEventNotificationCallerInterface &source){}
    void SetupVisualization()
    {
      m_VerticalBar1.SetModel(m_SoundPower);
      AddView(m_VerticalBar1);
      m_VerticalBar2.SetModel(m_SoundPower);
      AddView(m_VerticalBar2);
      m_VerticalBar3.SetModel(m_SoundPower);
      AddView(m_VerticalBar3);
      m_VerticalBar4.SetModel(m_SoundPower);
      AddView(m_VerticalBar4);
      m_VerticalBar5.SetModel(m_SoundPower);
      AddView(m_VerticalBar5);
      m_VerticalBar6.SetModel(m_SoundPower);
      AddView(m_VerticalBar6);
      AddModel(m_SoundPower);
    }
    bool CanRunVisualization(){ return true; }
    void RunVisualization()
    {
    }
  private:
    SoundPowerModel m_SoundPower = SoundPowerModel("Power Model", m_StatisticalEngineModelInterface);
    VerticalBarView m_VerticalBar1 = VerticalBarView("Vertical Bar1", 100, 0, 0, SCREEN_WIDTH, 10);
    VerticalBarView m_VerticalBar2 = VerticalBarView("Vertical Bar2", 101, 0, 10, SCREEN_WIDTH, 10);
    VerticalBarView m_VerticalBar3 = VerticalBarView("Vertical Bar3", 102, 0, 20, SCREEN_WIDTH, 10);
    VerticalBarView m_VerticalBar4 = VerticalBarView("Vertical Bar4", 103, 0, 30, SCREEN_WIDTH, 10);
    VerticalBarView m_VerticalBar5 = VerticalBarView("Vertical Bar5", 104, 0, 40, SCREEN_WIDTH, 10);
    VerticalBarView m_VerticalBar6 = VerticalBarView("Vertical Bar6", 105, 0, 50, SCREEN_WIDTH, 10);
};

#endif
