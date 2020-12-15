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
    Visualization() : Task("Visualization"){}
    ~Visualization()
    {
      if(true == debugMode && debugLevel >= 1) Serial << "Delete Visualization\n";
    }
    
    //Task Interface
    void Setup()
    {
      m_Scheduler.AddTasks(myViews);
      m_Scheduler.AddTasks(myModels);
      SetupVisualization();
    }
    bool CanRunTask()
    {
      return CanRunVisualization();
    }
    void RunTask()
    {
      RunVisualization();
    }
    virtual void SetupVisualization() = 0;
    virtual bool CanRunVisualization() = 0;
    virtual void RunVisualization() = 0; 
  protected:
    unsigned int m_Duration = 0;
    TaskScheduler m_Scheduler;    
    virtual Visualization* GetInstance() = 0;
    LinkedList<Task*> myViews = LinkedList<Task*>();
    LinkedList<Task*> myModels = LinkedList<Task*>();
};

/*
//********* VUMeter *********
class VUMeter: public Visualization
{
  public:
    VUMeter()
    {
      //myViews.add(&m_VerticalBar);
      //myModels.add(&m_SoundPower);
    }
    ~VUMeter()
    {
      if(true == debugMode && debugLevel >= 1) Serial << "Delete VUMeter\n";
    }

    //Visualization
    Visualization* GetInstance()
    {
      Visualization *vis = new VUMeter();
      return vis;
    }
    void SetupVisualization(){}
    bool CanRunVisualization(){ return true; }
    void RunVisualization(){}
  private:
    VerticalBar m_VerticalBar = VerticalBar(0, 0, NUMSTRIPS, NUMLEDS);
    SoundPower m_SoundPower;       
};
*/
#endif
