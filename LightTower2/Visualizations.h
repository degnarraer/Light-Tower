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

#ifndef Visualizations_H
#define Visualizatiosn_H

#include "Visualization.h"

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
    ReducedBandsBandPowerModel m_BandPower0 = ReducedBandsBandPowerModel("Sound Power Model 0", 0, 5, numVisualizations, m_StatisticalEngineModelInterface);
    RainbowColorModel m_ColorModel0 = RainbowColorModel("Color Model 0", 0, numVisualizations-1);
    
    VerticalBarView m_VerticalBar1 = VerticalBarView("Vertical Bar 1", 0, 1*SCREEN_HEIGHT/numVisualizations, SCREEN_WIDTH, SCREEN_HEIGHT/numVisualizations);
    ReducedBandsBandPowerModel m_BandPower1 = ReducedBandsBandPowerModel("Sound Power Model 1", 1, 5, numVisualizations, m_StatisticalEngineModelInterface);
    RainbowColorModel m_ColorModel1 = RainbowColorModel("Color Model 1", 1, numVisualizations-1);
    
    VerticalBarView m_VerticalBar2 = VerticalBarView("Vertical Bar 2", 0, 2*SCREEN_HEIGHT/numVisualizations, SCREEN_WIDTH, SCREEN_HEIGHT/numVisualizations);
    ReducedBandsBandPowerModel m_BandPower2 = ReducedBandsBandPowerModel("Sound Power Model 2", 2, 5, numVisualizations, m_StatisticalEngineModelInterface);
    RainbowColorModel m_ColorModel2 = RainbowColorModel("Color Model 2", 2, numVisualizations-1);
    
    VerticalBarView m_VerticalBar3 = VerticalBarView("Vertical Bar 3", 0, 3*SCREEN_HEIGHT/numVisualizations, SCREEN_WIDTH, SCREEN_HEIGHT/numVisualizations);
    ReducedBandsBandPowerModel m_BandPower3 = ReducedBandsBandPowerModel("Sound Power Model 3", 3, 5, numVisualizations, m_StatisticalEngineModelInterface);
    RainbowColorModel m_ColorModel3 = RainbowColorModel("Color Model 3", 3, numVisualizations-1);
    
    VerticalBarView m_VerticalBar4 = VerticalBarView("Vertical Bar 4", 0, 4*SCREEN_HEIGHT/numVisualizations, SCREEN_WIDTH, SCREEN_HEIGHT/numVisualizations);
    ReducedBandsBandPowerModel m_BandPower4 = ReducedBandsBandPowerModel("Sound Power Model 4", 4, 5, numVisualizations, m_StatisticalEngineModelInterface);
    RainbowColorModel m_ColorModel4 = RainbowColorModel("Color Model 4", 4, numVisualizations-1);
    
    VerticalBarView m_VerticalBar5 = VerticalBarView("Vertical Bar 5", 0, 5*SCREEN_HEIGHT/numVisualizations, SCREEN_WIDTH, SCREEN_HEIGHT/numVisualizations);
    ReducedBandsBandPowerModel m_BandPower5 = ReducedBandsBandPowerModel("Sound Power Model 5", 5, 5, numVisualizations, m_StatisticalEngineModelInterface);
    RainbowColorModel m_ColorModel5 = RainbowColorModel("Color Model 5", 5, numVisualizations-1);
    
    VerticalBarView m_VerticalBar6 = VerticalBarView("Vertical Bar 6", 0, 6*SCREEN_HEIGHT/numVisualizations, SCREEN_WIDTH, SCREEN_HEIGHT/numVisualizations);
    ReducedBandsBandPowerModel m_BandPower6 = ReducedBandsBandPowerModel("Sound Power Model 6", 6, 5, numVisualizations, m_StatisticalEngineModelInterface);
    RainbowColorModel m_ColorModel6 = RainbowColorModel("Color Model 6", 6, numVisualizations-1);
    
    VerticalBarView m_VerticalBar7 = VerticalBarView("Vertical Bar 7", 0, 7*SCREEN_HEIGHT/numVisualizations, SCREEN_WIDTH, SCREEN_HEIGHT/numVisualizations);
    ReducedBandsBandPowerModel m_BandPower7 = ReducedBandsBandPowerModel("Sound Power Model 7", 7, 5, numVisualizations, m_StatisticalEngineModelInterface);
    RainbowColorModel m_ColorModel7 = RainbowColorModel("Color Model 7", 7, numVisualizations-1);
};

//********* 3 Band VUMeter *********
class VUMeter3Band: public Visualization
{
  public:
    VUMeter3Band( StatisticalEngineModelInterface &StatisticalEngineModelInterface, LEDController &LEDController) 
                : Visualization( StatisticalEngineModelInterface, LEDController){}
    virtual ~VUMeter3Band()
    {
      if(true == debugMemory) Serial << "VUMeter3Band: Deleted";
    }

    //Visualization
    static Visualization* GetInstance(StatisticalEngineModelInterface &StatisticalEngineModelInterface, LEDController &LEDController);
    void SetupVisualization();
    bool CanRunVisualization();
    void RunVisualization();
  private:
    float numVisualizations = 3.0;
    VerticalBarView m_VerticalBar0 = VerticalBarView("Vertical Bar 0", 0, 0*SCREEN_HEIGHT/numVisualizations, SCREEN_WIDTH, SCREEN_HEIGHT/numVisualizations);
    ReducedBandsBandPowerModel m_BandPower0 = ReducedBandsBandPowerModel("Sound Power Model 0", 0, 5, 3, m_StatisticalEngineModelInterface);
    RainbowColorModel m_ColorModel0 = RainbowColorModel("Color Model 0", 0, numVisualizations-1);
    
    VerticalBarView m_VerticalBar1 = VerticalBarView("Vertical Bar 1", 0, 1*SCREEN_HEIGHT/numVisualizations, SCREEN_WIDTH, SCREEN_HEIGHT/numVisualizations);
    ReducedBandsBandPowerModel m_BandPower1 = ReducedBandsBandPowerModel("Sound Power Model 1", 1, 5, 3, m_StatisticalEngineModelInterface);
    RainbowColorModel m_ColorModel1 = RainbowColorModel("Color Model 1", 1, numVisualizations-1);
    
    VerticalBarView m_VerticalBar2 = VerticalBarView("Vertical Bar 2", 0, 2*SCREEN_HEIGHT/numVisualizations, SCREEN_WIDTH, SCREEN_HEIGHT/numVisualizations);
    ReducedBandsBandPowerModel m_BandPower2 = ReducedBandsBandPowerModel("Sound Power Model 2", 2, 5, 3, m_StatisticalEngineModelInterface);
    RainbowColorModel m_ColorModel2 = RainbowColorModel("Color Model 2", 2, numVisualizations-1);
};

//********* Waterfall *********
class Waterfall: public Visualization
{
  public:
    Waterfall( StatisticalEngineModelInterface &StatisticalEngineModelInterface, LEDController &LEDController) 
             : Visualization( StatisticalEngineModelInterface, LEDController){}
    virtual ~Waterfall() { if(true == debugMemory) Serial << "Waterfall: Deleted"; }

    //Visualization
    static Visualization* GetInstance(StatisticalEngineModelInterface &StatisticalEngineModelInterface, LEDController &LEDController);
    void SetupVisualization();
    bool CanRunVisualization();
    void RunVisualization();
  private:
    ScrollingView m_ScrollingView = ScrollingView("Scrolling View", ScrollDirection_Down, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    ColorSpriteView m_Sprite0 = ColorSpriteView("Sprite", 0, SCREEN_HEIGHT - 1, 4, 1);
    SoundPowerModel m_PowerModel = SoundPowerModel("Sound Power Model", m_StatisticalEngineModelInterface);
    RandomColorFadingModel m_ColorModel = RandomColorFadingModel("Color Fading Model", 10000);
    SettableColorPowerModel m_PowerColorModel = SettableColorPowerModel("Settable Power Model");
};

//********* Fire *********
class Fire: public Visualization
{
  public:
    Fire( StatisticalEngineModelInterface &StatisticalEngineModelInterface, LEDController &LEDController) 
        : Visualization( StatisticalEngineModelInterface, LEDController){}
    virtual ~Fire(){ if(true == debugMemory) Serial << "Fire: Deleted"; }

    //Visualization
    static Visualization* GetInstance(StatisticalEngineModelInterface &StatisticalEngineModelInterface, LEDController &LEDController);
    void SetupVisualization();
    bool CanRunVisualization();
    void RunVisualization();
  private:    
    ScrollingView m_ScrollingView = ScrollingView("Scrolling View", ScrollDirection_Up, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    ColorSpriteView m_Sprite0 = ColorSpriteView("Sprite", 0, 0, 4, 1);
    SoundPowerModel m_PowerModel = SoundPowerModel("Sound Power Model", m_StatisticalEngineModelInterface);
    RandomColorFadingModel m_ColorModel = RandomColorFadingModel("Color Fading Model", 10000);
    SettableColorPowerModel m_PowerColorModel = SettableColorPowerModel("Settable Power Model");
};

//********* WaterFireFromCenter *********
class WaterFireFromCenter: public Visualization
{
  public:
    WaterFireFromCenter( StatisticalEngineModelInterface &StatisticalEngineModelInterface, LEDController &LEDController) 
                       : Visualization( StatisticalEngineModelInterface, LEDController){}
    virtual ~WaterFireFromCenter() { if(true == debugMemory) Serial << "Fire: Deleted"; }

    //Visualization
    static Visualization* GetInstance(StatisticalEngineModelInterface &StatisticalEngineModelInterface, LEDController &LEDController);
    void SetupVisualization();
    bool CanRunVisualization();
    void RunVisualization();
  private:
    ScrollingView m_ScrollingView0 = ScrollingView("Fire Scrolling View", ScrollDirection_Up, 0, SCREEN_HEIGHT/2, SCREEN_WIDTH, SCREEN_HEIGHT/2);
    ColorSpriteView m_Sprite0 = ColorSpriteView("Fire Sprite 0", 0, SCREEN_HEIGHT/2, 4, 1);
    SoundPowerModel m_PowerModel0 = SoundPowerModel("Sound Power Model", m_StatisticalEngineModelInterface);
    RandomColorFadingModel m_ColorModel0 = RandomColorFadingModel("Color Fading Model", 10000);
    SettableColorPowerModel m_PowerColorModel0 = SettableColorPowerModel("Settable Power Model");
    
    ScrollingView m_ScrollingView1 = ScrollingView("Water Scrolling View", ScrollDirection_Down, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT/2);
    ColorSpriteView m_Sprite1 = ColorSpriteView("Water Sprite 0", 0, SCREEN_HEIGHT/2-1, 4, 1);
    SoundPowerModel m_PowerModel1 = SoundPowerModel("Sound Power Model", m_StatisticalEngineModelInterface);
    RandomColorFadingModel m_ColorModel1 = RandomColorFadingModel("Color Fading Model", 10000);
    SettableColorPowerModel m_PowerColorModel1 = SettableColorPowerModel("Settable Power Model");
};

//********* Vertical Band Tower *********
class VerticalBandTower: public Visualization
{
  public:
    VerticalBandTower( StatisticalEngineModelInterface &StatisticalEngineModelInterface, LEDController &LEDController) 
                     : Visualization( StatisticalEngineModelInterface, LEDController){}
    virtual ~VerticalBandTower(){ if(true == debugMemory) Serial << "VerticalBandTower: Deleted"; }

    //Visualization
    static Visualization* GetInstance(StatisticalEngineModelInterface &StatisticalEngineModelInterface, LEDController &LEDController);
    void SetupVisualization();
    bool CanRunVisualization();
    void RunVisualization();
  private:  
};

//********* scrolling Bands *********
class ScrollingBands: public Visualization
{
  public:
    ScrollingBands( StatisticalEngineModelInterface &StatisticalEngineModelInterface, LEDController &LEDController) 
                  : Visualization( StatisticalEngineModelInterface, LEDController){}
    virtual ~ScrollingBands(){ if(true == debugMemory) Serial << "VerticalBandTower: Deleted"; }

    //Visualization
    static Visualization* GetInstance(StatisticalEngineModelInterface &StatisticalEngineModelInterface, LEDController &LEDController);
    void SetupVisualization();
    bool CanRunVisualization();
    void RunVisualization();
  private:
  
    ScrollingView m_ScrollingView0 = ScrollingView("Upward Scrolling View", ScrollDirection_Up, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    ScrollingView m_ScrollingView1 = ScrollingView("Downward Scrolling View", ScrollDirection_Down, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    
    ColorSpriteView m_Sprite0 = ColorSpriteView("Sprite 0", 0, 0, 1, 1);
    ReducedBandsBandPowerModel m_BandPower0 = ReducedBandsBandPowerModel("Sound Power Model 0", 0, 0, 8, m_StatisticalEngineModelInterface);
    RainbowColorModel m_ColorModel0 = RainbowColorModel("Color Model 1", 0, 7);
    SettableColorPowerModel m_PowerColorModel0 = SettableColorPowerModel("Settable Power Model");
    
    ColorSpriteView m_Sprite1 = ColorSpriteView("Sprite 0", 1, 0, 1, 1);
    ReducedBandsBandPowerModel m_BandPower1 = ReducedBandsBandPowerModel("Sound Power Model 1", 1, 0, 8, m_StatisticalEngineModelInterface);
    RainbowColorModel m_ColorModel1 = RainbowColorModel("Color Model 1", 1, 7);
    SettableColorPowerModel m_PowerColorModel1 = SettableColorPowerModel("Settable Power Model");
    
    ColorSpriteView m_Sprite2 = ColorSpriteView("Sprite 0", 2, 0, 1, 1);
    ReducedBandsBandPowerModel m_BandPower2 = ReducedBandsBandPowerModel("Sound Power Model 2", 2, 0, 8, m_StatisticalEngineModelInterface);
    RainbowColorModel m_ColorModel2 = RainbowColorModel("Color Model 1", 2, 7);
    SettableColorPowerModel m_PowerColorModel2 = SettableColorPowerModel("Settable Power Model");
    
    ColorSpriteView m_Sprite3 = ColorSpriteView("Sprite 0", 3, 0, 1, 1);
    ReducedBandsBandPowerModel m_BandPower3 = ReducedBandsBandPowerModel("Sound Power Model 3", 3, 0, 8, m_StatisticalEngineModelInterface);
    RainbowColorModel m_ColorModel3 = RainbowColorModel("Color Model 1", 3, 7);
    SettableColorPowerModel m_PowerColorModel3 = SettableColorPowerModel("Settable Power Model");

    
    ColorSpriteView m_Sprite4 = ColorSpriteView("Sprite 0", 0, SCREEN_HEIGHT - 1, 1, 1);
    ReducedBandsBandPowerModel m_BandPower4 = ReducedBandsBandPowerModel("Sound Power Model 0", 4, 0, 8, m_StatisticalEngineModelInterface);
    RainbowColorModel m_ColorModel4 = RainbowColorModel("Color Model 1", 4, 7);
    SettableColorPowerModel m_PowerColorModel4 = SettableColorPowerModel("Settable Power Model");
    
    ColorSpriteView m_Sprite5 = ColorSpriteView("Sprite 0", 1, SCREEN_HEIGHT - 1, 1, 1);
    ReducedBandsBandPowerModel m_BandPower5 = ReducedBandsBandPowerModel("Sound Power Model 1", 5, 0, 8, m_StatisticalEngineModelInterface);
    RainbowColorModel m_ColorModel5 = RainbowColorModel("Color Model 1", 5, 7);
    SettableColorPowerModel m_PowerColorModel5 = SettableColorPowerModel("Settable Power Model");
    
    ColorSpriteView m_Sprite6 = ColorSpriteView("Sprite 0", 2, SCREEN_HEIGHT - 1, 1, 1);
    ReducedBandsBandPowerModel m_BandPower6 = ReducedBandsBandPowerModel("Sound Power Model 2", 6, 0, 8, m_StatisticalEngineModelInterface);
    RainbowColorModel m_ColorModel6 = RainbowColorModel("Color Model 1", 6, 7);
    SettableColorPowerModel m_PowerColorModel6 = SettableColorPowerModel("Settable Power Model");
    
    ColorSpriteView m_Sprite7 = ColorSpriteView("Sprite 0", 3, SCREEN_HEIGHT - 1, 1, 1);
    ReducedBandsBandPowerModel m_BandPower7 = ReducedBandsBandPowerModel("Sound Power Model 3", 7, 0, 8, m_StatisticalEngineModelInterface);
    RainbowColorModel m_ColorModel7 = RainbowColorModel("Color Model 1", 7, 7);
    SettableColorPowerModel m_PowerColorModel7 = SettableColorPowerModel("Settable Power Model"); 
};

//********* Scrolling Max Band *********
class ScrollingMaxBand: public Visualization
{
  public:
    ScrollingMaxBand( StatisticalEngineModelInterface &StatisticalEngineModelInterface, LEDController &LEDController) 
                    : Visualization( StatisticalEngineModelInterface, LEDController){}
    virtual ~ScrollingMaxBand(){ if(true == debugMemory) Serial << "ScrollingMaxBand: Deleted"; }

    //Visualization
    static Visualization* GetInstance(StatisticalEngineModelInterface &StatisticalEngineModelInterface, LEDController &LEDController);
    void SetupVisualization();
    bool CanRunVisualization();
    void RunVisualization();
  private:    
    ScrollingView m_ScrollingView = ScrollingView("Scrolling View", ScrollDirection_Up, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    ColorSpriteView m_Sprite0 = ColorSpriteView("Sprite", 0, 0, 4, 1);
    BandDataColorModel m_BandDataColorModel = BandDataColorModel( "Band Data Color Model" );
    MaximumBandModel m_MaxBandModel = MaximumBandModel( "Max Band Model", 0, m_StatisticalEngineModelInterface );
};
#endif
