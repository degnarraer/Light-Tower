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
    void RegisterForNotification(VisualizationEventNotificationCalleeInterface &callee);
    void DeRegisterForNotification(VisualizationEventNotificationCalleeInterface &callee);
    void SendVisualizationCompleteNotificationToCalleesFrom(VisualizationEventNotificationCallerInterface &source);
  private:
    LinkedList<VisualizationEventNotificationCalleeInterface*> myCallees = LinkedList<VisualizationEventNotificationCalleeInterface*>();
};

class Visualization: public Task
                   , VisualizationEventNotificationCallerInterface
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
    void AddView(View &View);
    void AddModel(Model &Model);
    void RemoveAllModels();
  private:
    LinkedList<View*> m_MyViews = LinkedList<View*>();
    LinkedList<Model*> m_MyModels = LinkedList<Model*>();
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
    Visualization* GetInstance(StatisticalEngineModelInterface &StatisticalEngineModelInterface, LEDController &LEDController);
    void SetupVisualization();
    bool CanRunVisualization();
    void RunVisualization();
  private:
    SoundPowerModel m_SoundPower = SoundPowerModel("Sound Power Model", m_StatisticalEngineModelInterface);
    VerticalBarView m_VerticalBar = VerticalBarView("Vertical Bar", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
};

#endif
