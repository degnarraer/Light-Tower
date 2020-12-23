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
#include "Views.h"
#include "Streaming.h"
#include "Models.h"
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
               
    virtual ~Visualization()
    {
      if(true == debugMemory) Serial << "Delete Visualization\n";
      
    }
    StatisticalEngineModelInterface &m_StatisticalEngineModelInterface;
    LEDController &m_LEDController;
    PixelStruct& GetPixelStruct() { return m_MyPixelStruct; }
    
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
    virtual ~VUMeter()
    {
      if(true == debugMemory) Serial << "VUMeter: Deleted";
    }

    //Visualization
    static Visualization* GetInstance(StatisticalEngineModelInterface &StatisticalEngineModelInterface, LEDController &LEDController);
    void SetupVisualization();
    bool CanRunVisualization();
    void RunVisualization();
  private:
    SoundPowerModel m_SoundPower = SoundPowerModel("Sound Power Model", m_StatisticalEngineModelInterface);
    RandomColorFadingModel m_ColorModel = RandomColorFadingModel("Color Model", 5000);
    VerticalBarView m_VerticalBar = VerticalBarView("Vertical Bar", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
};

//********* 8 Band VUMeter *********
class VUMeter8Band: public Visualization
{
  public:
    VUMeter8Band( StatisticalEngineModelInterface &StatisticalEngineModelInterface, LEDController &LEDController) 
                : Visualization( StatisticalEngineModelInterface, LEDController){}
    virtual ~VUMeter8Band()
    {
      if(true == debugMemory) Serial << "VUMeter8Band: Deleted";
    }

    //Visualization
    static Visualization* GetInstance(StatisticalEngineModelInterface &StatisticalEngineModelInterface, LEDController &LEDController);
    void SetupVisualization();
    bool CanRunVisualization();
    void RunVisualization();
  private:
    float numVisualizations = 8.0;
    VerticalBarView m_VerticalBar0 = VerticalBarView("Vertical Bar 0", 0, 0*SCREEN_HEIGHT/numVisualizations, SCREEN_WIDTH, SCREEN_HEIGHT/numVisualizations);
    ReducedBandsBandPowerModel m_BandPower0 = ReducedBandsBandPowerModel("Sound Power Model 0", 0, 0, 8, m_StatisticalEngineModelInterface);
    RainbowColorModel m_ColorModel0 = RainbowColorModel("Color Model 0", 0, numVisualizations-1);
    
    VerticalBarView m_VerticalBar1 = VerticalBarView("Vertical Bar 1", 0, 1*SCREEN_HEIGHT/numVisualizations, SCREEN_WIDTH, SCREEN_HEIGHT/numVisualizations);
    ReducedBandsBandPowerModel m_BandPower1 = ReducedBandsBandPowerModel("Sound Power Model 1", 1, 0, 8, m_StatisticalEngineModelInterface);
    RainbowColorModel m_ColorModel1 = RainbowColorModel("Color Model 1", 1, numVisualizations-1);
    
    VerticalBarView m_VerticalBar2 = VerticalBarView("Vertical Bar 2", 0, 2*SCREEN_HEIGHT/numVisualizations, SCREEN_WIDTH, SCREEN_HEIGHT/numVisualizations);
    ReducedBandsBandPowerModel m_BandPower2 = ReducedBandsBandPowerModel("Sound Power Model 2", 2, 0, 8, m_StatisticalEngineModelInterface);
    RainbowColorModel m_ColorModel2 = RainbowColorModel("Color Model 2", 2, numVisualizations-1);
    
    VerticalBarView m_VerticalBar3 = VerticalBarView("Vertical Bar 3", 0, 3*SCREEN_HEIGHT/numVisualizations, SCREEN_WIDTH, SCREEN_HEIGHT/numVisualizations);
    ReducedBandsBandPowerModel m_BandPower3 = ReducedBandsBandPowerModel("Sound Power Model 3", 3, 0, 8, m_StatisticalEngineModelInterface);
    RainbowColorModel m_ColorModel3 = RainbowColorModel("Color Model 3", 3, numVisualizations-1);
    
    VerticalBarView m_VerticalBar4 = VerticalBarView("Vertical Bar 4", 0, 4*SCREEN_HEIGHT/numVisualizations, SCREEN_WIDTH, SCREEN_HEIGHT/numVisualizations);
    ReducedBandsBandPowerModel m_BandPower4 = ReducedBandsBandPowerModel("Sound Power Model 4", 4, 0, 8, m_StatisticalEngineModelInterface);
    RainbowColorModel m_ColorModel4 = RainbowColorModel("Color Model 4", 4, numVisualizations-1);
    
    VerticalBarView m_VerticalBar5 = VerticalBarView("Vertical Bar 5", 0, 5*SCREEN_HEIGHT/numVisualizations, SCREEN_WIDTH, SCREEN_HEIGHT/numVisualizations);
    ReducedBandsBandPowerModel m_BandPower5 = ReducedBandsBandPowerModel("Sound Power Model 5", 5, 0, 8, m_StatisticalEngineModelInterface);
    RainbowColorModel m_ColorModel5 = RainbowColorModel("Color Model 5", 5, numVisualizations-1);
    
    VerticalBarView m_VerticalBar6 = VerticalBarView("Vertical Bar 6", 0, 6*SCREEN_HEIGHT/numVisualizations, SCREEN_WIDTH, SCREEN_HEIGHT/numVisualizations);
    ReducedBandsBandPowerModel m_BandPower6 = ReducedBandsBandPowerModel("Sound Power Model 6", 6, 0, 8, m_StatisticalEngineModelInterface);
    RainbowColorModel m_ColorModel6 = RainbowColorModel("Color Model 6", 6, numVisualizations-1);
    
    VerticalBarView m_VerticalBar7 = VerticalBarView("Vertical Bar 7", 0, 7*SCREEN_HEIGHT/numVisualizations, SCREEN_WIDTH, SCREEN_HEIGHT/numVisualizations);
    ReducedBandsBandPowerModel m_BandPower7 = ReducedBandsBandPowerModel("Sound Power Model 7", 7, 0, 8, m_StatisticalEngineModelInterface);
    RainbowColorModel m_ColorModel7 = RainbowColorModel("Color Model 7", 7, numVisualizations-1);
};

#endif
