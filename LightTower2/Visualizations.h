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

#ifndef Visualization_H
#define Visualization_H
#include "Statistical_Engine.h"
#include "Streaming.h"
#include "Models.h"
#include "Views.h"
#include "TaskInterface.h"
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
{
  public:
    Visualization(StatisticalEngineInterface &statisticalEngineInterface) : Task("Visualization")
                                                                          , m_StatisticalEngineInterface(statisticalEngineInterface){}
    ~Visualization()
    {
      if(true == debugMode && debugLevel >= 1) Serial << "Delete Visualization\n";
    }
    StatisticalEngineInterface m_StatisticalEngineInterface;
    void AddSubView(View &view);
    void AddModel(Model &model);
    
    virtual Visualization* GetInstance(StatisticalEngineInterface &statisticalEngineInterface) = 0;
    virtual void SetupVisualization() = 0;
    virtual bool CanRunVisualization() = 0;
    virtual void RunVisualization() = 0;

    //Task Interface
    void Setup();
    bool CanRunMyTask();
    void RunTask();
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
    VUMeter(StatisticalEngineInterface &statisticalEngineInterface) : Visualization(statisticalEngineInterface)
    {
      AddSubView(m_VerticalBar);
      AddModel(m_SoundPower);
    }
    ~VUMeter()
    {
      if(true == debugMode && debugLevel >= 1) Serial << "Delete VUMeter\n";
    }

    //Visualization
    Visualization* GetInstance(StatisticalEngineInterface &statisticalEngineInterface)
    {
      VUMeter *vis = new VUMeter(statisticalEngineInterface);
      return vis;
    }
    void SetupVisualization(){}
    bool CanRunVisualization(){ return true; }
    void RunVisualization(){}
  private:
    VerticalBar m_VerticalBar = VerticalBar(0, 0, NUMSTRIPS, NUMLEDS);
    SoundPower m_SoundPower;       
};

#endif
