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
      m_VerticalBar0.SetModel(m_Band0Power);
      AddView(m_VerticalBar0);
      AddModel(m_Band0Power);
      
      m_VerticalBar1.SetModel(m_Band1Power);
      AddView(m_VerticalBar1);
      AddModel(m_Band1Power);
      
      m_VerticalBar2.SetModel(m_Band2Power);
      AddView(m_VerticalBar2);
      AddModel(m_Band2Power);
      
      m_VerticalBar3.SetModel(m_Band3Power);
      AddView(m_VerticalBar3);
      AddModel(m_Band3Power);
      
      m_VerticalBar4.SetModel(m_Band4Power);
      AddView(m_VerticalBar4);
      AddModel(m_Band4Power);
      
      m_VerticalBar5.SetModel(m_Band5Power);
      AddView(m_VerticalBar5);
      AddModel(m_Band5Power);
      
      m_VerticalBar6.SetModel(m_Band6Power);
      AddView(m_VerticalBar6);
      AddModel(m_Band6Power);
      
      m_VerticalBar7.SetModel(m_Band7Power);
      AddView(m_VerticalBar7);
      AddModel(m_Band7Power);
    }
    bool CanRunVisualization(){ return true; }
    void RunVisualization()
    {
    }
  private:
    BandPowerModel m_Band0Power = BandPowerModel("Band Power Model 0", 0, m_StatisticalEngineModelInterface);
    BandPowerModel m_Band1Power = BandPowerModel("Band Power Model 1", 1, m_StatisticalEngineModelInterface);
    BandPowerModel m_Band2Power = BandPowerModel("Band Power Model 2", 2, m_StatisticalEngineModelInterface);
    BandPowerModel m_Band3Power = BandPowerModel("Band Power Model 3", 3, m_StatisticalEngineModelInterface);
    BandPowerModel m_Band4Power = BandPowerModel("Band Power Model 4", 4, m_StatisticalEngineModelInterface);
    BandPowerModel m_Band5Power = BandPowerModel("Band Power Model 5", 5, m_StatisticalEngineModelInterface);
    BandPowerModel m_Band6Power = BandPowerModel("Band Power Model 6", 6, m_StatisticalEngineModelInterface);
    BandPowerModel m_Band7Power = BandPowerModel("Band Power Model 7", 7, m_StatisticalEngineModelInterface);
    VerticalBarView m_VerticalBar0 = VerticalBarView("Vertical Bar0", 100, 0, 0, SCREEN_WIDTH, 7);
    VerticalBarView m_VerticalBar1 = VerticalBarView("Vertical Bar1", 101, 0, 7, SCREEN_WIDTH, 7);
    VerticalBarView m_VerticalBar2 = VerticalBarView("Vertical Bar2", 102, 0, 14, SCREEN_WIDTH, 7);
    VerticalBarView m_VerticalBar3 = VerticalBarView("Vertical Bar3", 103, 0, 21, SCREEN_WIDTH, 7);
    VerticalBarView m_VerticalBar4 = VerticalBarView("Vertical Bar4", 104, 0, 28, SCREEN_WIDTH, 7);
    VerticalBarView m_VerticalBar5 = VerticalBarView("Vertical Bar5", 105, 0, 35, SCREEN_WIDTH, 7);
    VerticalBarView m_VerticalBar6 = VerticalBarView("Vertical Bar6", 106, 0, 42, SCREEN_WIDTH, 7);
    VerticalBarView m_VerticalBar7 = VerticalBarView("Vertical Bar7", 107, 0, 49, SCREEN_WIDTH, 7);
};

#endif
