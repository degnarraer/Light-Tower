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
#include "Visualizations.h"

//VU METER
Visualization* VUMeter::GetInstance(StatisticalEngineModelInterface &StatisticalEngineModelInterface, LEDController &LEDController)
{
  if(true == debugMemory) Serial << "VUMeter: Get Instance\n";
  VUMeter *vis = new VUMeter(StatisticalEngineModelInterface, LEDController);
  return vis;
}
void VUMeter::SetupVisualization()
{
  AddView(m_VerticalBar);
  AddModel(m_SoundPower);
  AddModel(m_ColorModel);
  m_VerticalBar.ConnectBarHeightModel(m_SoundPower);
  m_VerticalBar.ConnectBarColorModel(m_ColorModel);
}
bool VUMeter::CanRunVisualization(){ return true; }
void VUMeter::RunVisualization(){}

//VU METER 8 Band
Visualization* VUMeter8Band::GetInstance(StatisticalEngineModelInterface &StatisticalEngineModelInterface, LEDController &LEDController)
{
  if(true == debugMemory) Serial << "VUMeter8Band: Get Instance\n";
  VUMeter8Band *vis = new VUMeter8Band(StatisticalEngineModelInterface, LEDController);
  return vis;
}
void VUMeter8Band::SetupVisualization()
{
  
  AddView(m_VerticalBar0);
  m_VerticalBar0.ConnectBarHeightModel(m_BandPower0);
  m_VerticalBar0.ConnectBarColorModel(m_ColorModel0);
  AddModel(m_BandPower0);
  AddModel(m_ColorModel0);
  
  AddView(m_VerticalBar1);
  m_VerticalBar1.ConnectBarHeightModel(m_BandPower1);
  m_VerticalBar1.ConnectBarColorModel(m_ColorModel1);
  AddModel(m_BandPower1);
  AddModel(m_ColorModel1);
  
  AddView(m_VerticalBar2);
  m_VerticalBar2.ConnectBarHeightModel(m_BandPower2);
  m_VerticalBar2.ConnectBarColorModel(m_ColorModel2);
  AddModel(m_BandPower2);
  AddModel(m_ColorModel2);
  
  AddView(m_VerticalBar3);
  m_VerticalBar3.ConnectBarHeightModel(m_BandPower3);
  m_VerticalBar3.ConnectBarColorModel(m_ColorModel3);
  AddModel(m_BandPower3);
  AddModel(m_ColorModel3);
  
  AddView(m_VerticalBar4);
  m_VerticalBar4.ConnectBarHeightModel(m_BandPower4);
  m_VerticalBar4.ConnectBarColorModel(m_ColorModel4);
  AddModel(m_BandPower4);
  AddModel(m_ColorModel4);
  
  AddView(m_VerticalBar5);
  m_VerticalBar5.ConnectBarHeightModel(m_BandPower5);
  m_VerticalBar5.ConnectBarColorModel(m_ColorModel5);
  AddModel(m_BandPower5);
  AddModel(m_ColorModel5);
  
  AddView(m_VerticalBar6);
  m_VerticalBar6.ConnectBarHeightModel(m_BandPower6);
  m_VerticalBar6.ConnectBarColorModel(m_ColorModel6);
  AddModel(m_BandPower6);
  AddModel(m_ColorModel6);
  
  AddView(m_VerticalBar7);
  m_VerticalBar7.ConnectBarHeightModel(m_BandPower7);
  m_VerticalBar7.ConnectBarColorModel(m_ColorModel7);
  AddModel(m_BandPower7);
  AddModel(m_ColorModel7);
}
bool VUMeter8Band::CanRunVisualization(){ return true; }
void VUMeter8Band::RunVisualization(){}

//VU METER 3 Band
Visualization* VUMeter3Band::GetInstance(StatisticalEngineModelInterface &StatisticalEngineModelInterface, LEDController &LEDController)
{
  if(true == debugMemory) Serial << "VUMeter3Band: Get Instance\n";
  VUMeter3Band *vis = new VUMeter3Band(StatisticalEngineModelInterface, LEDController);
  return vis;
}
void VUMeter3Band::SetupVisualization()
{
  AddView(m_VerticalBar0);
  AddModel(m_BandPower0);
  AddModel(m_ColorModel0);
  m_VerticalBar0.ConnectBarHeightModel(m_BandPower0);
  m_VerticalBar0.ConnectBarColorModel(m_ColorModel0);
  
  AddView(m_VerticalBar1);
  AddModel(m_BandPower1);
  AddModel(m_ColorModel1);
  m_VerticalBar1.ConnectBarHeightModel(m_BandPower1);
  m_VerticalBar1.ConnectBarColorModel(m_ColorModel1);
  
  AddView(m_VerticalBar2);
  AddModel(m_BandPower2);
  AddModel(m_ColorModel2);
  m_VerticalBar2.ConnectBarHeightModel(m_BandPower2);
  m_VerticalBar2.ConnectBarColorModel(m_ColorModel2);
}
bool VUMeter3Band::CanRunVisualization(){ return true; }
void VUMeter3Band::RunVisualization(){}


//Waterfall
Visualization* Waterfall::GetInstance(StatisticalEngineModelInterface &StatisticalEngineModelInterface, LEDController &LEDController)
{
  if(true == debugMemory) Serial << "Waterfall: Get Instance\n";
  Waterfall *vis = new Waterfall(StatisticalEngineModelInterface, LEDController);
  return vis;
}
void Waterfall::SetupVisualization()
{
  AddView(m_ScrollingView);
  m_ScrollingView.AddSubView(m_Sprite0);
  AddModel(m_PowerModel);
  AddModel(m_ColorModel);
  AddModel(m_PowerColorModel);
  m_PowerColorModel.ConnectColorModel(m_ColorModel);
  m_PowerColorModel.ConnectPowerModel(m_PowerModel);
  m_Sprite0.ConnectColorModel(m_PowerColorModel);
}
bool Waterfall::CanRunVisualization()
{ 
  return true; 
}
void Waterfall::RunVisualization()
{
}

//Fire
Visualization* Fire::GetInstance(StatisticalEngineModelInterface &StatisticalEngineModelInterface, LEDController &LEDController)
{
  if(true == debugMemory) Serial << "Fire: Get Instance\n";
  Fire *vis = new Fire(StatisticalEngineModelInterface, LEDController);
  return vis;
}
void Fire::SetupVisualization()
{
  AddView(m_ScrollingView);
  m_ScrollingView.AddSubView(m_Sprite0);
  AddModel(m_PowerModel);
  AddModel(m_ColorModel);
  AddModel(m_PowerColorModel);
  m_PowerColorModel.ConnectColorModel(m_ColorModel);
  m_PowerColorModel.ConnectPowerModel(m_PowerModel);
  m_Sprite0.ConnectColorModel(m_PowerColorModel);
}
bool Fire::CanRunVisualization()
{ 
  return true; 
}
void Fire::RunVisualization()
{
}

//WaterFireFromCenter
Visualization* WaterFireFromCenter::GetInstance(StatisticalEngineModelInterface &StatisticalEngineModelInterface, LEDController &LEDController)
{
  if(true == debugMemory) Serial << "WaterFireFromCenter: Get Instance\n";
  WaterFireFromCenter *vis = new WaterFireFromCenter(StatisticalEngineModelInterface, LEDController);
  return vis;
}
void WaterFireFromCenter::SetupVisualization()
{
  AddView(m_ScrollingView0);
  m_ScrollingView0.AddSubView(m_Sprite0);
  AddModel(m_PowerModel0);
  AddModel(m_ColorModel0);
  AddModel(m_PowerColorModel0);
  m_PowerColorModel0.ConnectColorModel(m_ColorModel0);
  m_PowerColorModel0.ConnectPowerModel(m_PowerModel0);
  m_Sprite0.ConnectColorModel(m_PowerColorModel0);
  
  AddView(m_ScrollingView1);
  m_ScrollingView1.AddSubView(m_Sprite1);
  AddModel(m_PowerModel1);
  AddModel(m_ColorModel1);
  AddModel(m_PowerColorModel1);
  m_PowerColorModel1.ConnectColorModel(m_ColorModel1);
  m_PowerColorModel1.ConnectPowerModel(m_PowerModel1);
  m_Sprite1.ConnectColorModel(m_PowerColorModel1);  
}
bool WaterFireFromCenter::CanRunVisualization()
{ 
  return true; 
}
void WaterFireFromCenter::RunVisualization()
{
}

//VerticalBandTower
Visualization* VerticalBandTower::GetInstance(StatisticalEngineModelInterface &StatisticalEngineModelInterface, LEDController &LEDController)
{
  if(true == debugMemory) Serial << "VerticalBandTower: Get Instance\n";
  VerticalBandTower *vis = new VerticalBandTower(StatisticalEngineModelInterface, LEDController);
  return vis;
}
void VerticalBandTower::SetupVisualization()
{
  int numVisualizations = m_StatisticalEngineModelInterface.GetNumberOfBands();
  for(int i = 0; i < numVisualizations; ++i)
  {
    ColorSpriteView *sprite = new ColorSpriteView("Sprite", 0, (int)round(i*(float)SCREEN_HEIGHT/(float)numVisualizations), SCREEN_WIDTH, (int)round((float)SCREEN_HEIGHT/(float)numVisualizations));
    AddNewedView(*sprite);
    ReducedBandsBandPowerModel *bandPower = new ReducedBandsBandPowerModel("Sound Power Model", i, 0, numVisualizations, m_StatisticalEngineModelInterface);
    AddNewedModel(*bandPower);
    RainbowColorModel *colorModel = new RainbowColorModel("Color Model", i, numVisualizations);
    AddNewedModel(*colorModel);
    SettableColorPowerModel *settableColorPowerModel = new SettableColorPowerModel("Settable Power Model");
    AddNewedModel(*settableColorPowerModel);
    settableColorPowerModel->ConnectColorModel(*colorModel);
    settableColorPowerModel->ConnectPowerModel(*bandPower);
    sprite->ConnectColorModel(*settableColorPowerModel); 
  }
}
bool VerticalBandTower::CanRunVisualization()
{ 
  return true; 
}
void VerticalBandTower::RunVisualization()
{
}
