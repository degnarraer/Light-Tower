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
    RandomColorFadingModel m_ColorModel = RandomColorFadingModel("Color Model", 5000);
    VerticalBarView m_VerticalBar = VerticalBarView("Vertical Bar", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
};

//********* 8 Band VUMeter *********
class VUMeter8Band: public Visualization
{
  public:
    VUMeter8Band( StatisticalEngineModelInterface &StatisticalEngineModelInterface, LEDController &LEDController) 
                : Visualization( StatisticalEngineModelInterface, LEDController){}
    ~VUMeter8Band(){if(true == debugMode && debugLevel >= 1) Serial << "Delete VUMeter8Band\n";}

    //Visualization
    Visualization* GetInstance(StatisticalEngineModelInterface &StatisticalEngineModelInterface, LEDController &LEDController);
    void SetupVisualization();
    bool CanRunVisualization();
    void RunVisualization();
  private:
    int numBands = m_StatisticalEngineModelInterface.GetNumberOfBands();
    float numVisualizations = 8.0;
    VerticalBarView m_VerticalBar0 = VerticalBarView("Vertical Bar 0", 0, 0*SCREEN_HEIGHT/numVisualizations, SCREEN_WIDTH, SCREEN_HEIGHT/numVisualizations);
    BandPowerModel m_BandPower0 = BandPowerModel("Sound Power Model 0", 0, m_StatisticalEngineModelInterface);
    RainbowColorModel m_ColorModel0 = RainbowColorModel("Color Model 0", 0*(numBands-1)/(numVisualizations-1), numBands-1);
    
    VerticalBarView m_VerticalBar1 = VerticalBarView("Vertical Bar 1", 0, 1*SCREEN_HEIGHT/numVisualizations, SCREEN_WIDTH, SCREEN_HEIGHT/numVisualizations);
    BandPowerModel m_BandPower1 = BandPowerModel("Sound Power Model 1", 1, m_StatisticalEngineModelInterface);
    RainbowColorModel m_ColorModel1 = RainbowColorModel("Color Model 1", 1*(numBands-1)/(numVisualizations-1), numBands-1);
    
    VerticalBarView m_VerticalBar2 = VerticalBarView("Vertical Bar 2", 0, 2*SCREEN_HEIGHT/numVisualizations, SCREEN_WIDTH, SCREEN_HEIGHT/numVisualizations);
    BandPowerModel m_BandPower2 = BandPowerModel("Sound Power Model 2", 2, m_StatisticalEngineModelInterface);
    RainbowColorModel m_ColorModel2 = RainbowColorModel("Color Model 2", 2*(numBands-1)/(numVisualizations-1), numBands-1);
    
    VerticalBarView m_VerticalBar3 = VerticalBarView("Vertical Bar 3", 0, 3*SCREEN_HEIGHT/numVisualizations, SCREEN_WIDTH, SCREEN_HEIGHT/numVisualizations);
    BandPowerModel m_BandPower3 = BandPowerModel("Sound Power Model 3", 3, m_StatisticalEngineModelInterface);
    RainbowColorModel m_ColorModel3 = RainbowColorModel("Color Model 3", 3*(numBands-1)/(numVisualizations-1), numBands-1);
    
    VerticalBarView m_VerticalBar4 = VerticalBarView("Vertical Bar 4", 0, 4*SCREEN_HEIGHT/numVisualizations, SCREEN_WIDTH, SCREEN_HEIGHT/numVisualizations);
    BandPowerModel m_BandPower4 = BandPowerModel("Sound Power Model 4", 4, m_StatisticalEngineModelInterface);
    RainbowColorModel m_ColorModel4 = RainbowColorModel("Color Model 4", 4*(numBands-1)/(numVisualizations-1), numBands-1);
    
    VerticalBarView m_VerticalBar5 = VerticalBarView("Vertical Bar 5", 0, 5*SCREEN_HEIGHT/numVisualizations, SCREEN_WIDTH, SCREEN_HEIGHT/numVisualizations);
    BandPowerModel m_BandPower5 = BandPowerModel("Sound Power Model 5", 5, m_StatisticalEngineModelInterface);
    RainbowColorModel m_ColorModel5 = RainbowColorModel("Color Model 5", 5*(numBands-1)/(numVisualizations-1), numBands-1);
    
    VerticalBarView m_VerticalBar6 = VerticalBarView("Vertical Bar 6", 0, 6*SCREEN_HEIGHT/numVisualizations, SCREEN_WIDTH, SCREEN_HEIGHT/numVisualizations);
    BandPowerModel m_BandPower6 = BandPowerModel("Sound Power Model 6", 6, m_StatisticalEngineModelInterface);
    RainbowColorModel m_ColorModel6 = RainbowColorModel("Color Model 6", 6*(numBands-1)/(numVisualizations-1), numBands-1);
    
    VerticalBarView m_VerticalBar7 = VerticalBarView("Vertical Bar 7", 0, 7*SCREEN_HEIGHT/numVisualizations, SCREEN_WIDTH, SCREEN_HEIGHT/numVisualizations);
    BandPowerModel m_BandPower7 = BandPowerModel("Sound Power Model 7", 7, m_StatisticalEngineModelInterface);
    RainbowColorModel m_ColorModel7 = RainbowColorModel("Color Model 7", 7*(numBands-1)/(numVisualizations-1), numBands-1);
};

#endif
