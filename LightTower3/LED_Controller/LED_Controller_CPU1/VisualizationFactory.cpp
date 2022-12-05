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
#include "VisualizationFactory.h"

//VU METER
Visualization* VUMeter::GetInstance(StatisticalEngineModelInterface &StatisticalEngineModelInterface, LEDController &LEDController)
{
  if(true == debugMemory) Serial << "VUMeter: Get Instance\n";
  VUMeter *vis = new VUMeter(StatisticalEngineModelInterface, LEDController);
  return vis;
}
void VUMeter::SetupVisualization()
{
  AddView(m_PeakSprite0, false);
  AddView(m_VerticalBar);
  AddView(m_Background);

  AddModel(m_SoundPower);
  AddModel(m_BandPower0);
  AddModel(m_VerticalBar);
  AddModel(m_ColorModel);
  AddModel(m_GravitationalModel);
  m_VerticalBar.ConnectBarHeightModel(m_SoundPower);
  m_VerticalBar.ConnectBarColorModel(m_ColorModel);
  m_GravitationalModel.ConnectYPositionModel(m_VerticalBar);
  m_PeakSprite0.ConnectPowerModel(m_BandPower0);
  m_PeakSprite0.ConnectYPositionModel(m_GravitationalModel);
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
  AddView(m_FloorSprite0);
  AddView(m_FloorSprite1);
  AddView(m_FloorSprite2);
  AddView(m_FloorSprite3);
  AddView(m_FloorSprite4);
  AddView(m_FloorSprite5);
  AddView(m_FloorSprite6);
  AddView(m_FloorSprite7);
  
  AddView(m_PeakSprite0);
  AddView(m_PeakSprite1);
  AddView(m_PeakSprite2);
  AddView(m_PeakSprite3);
  AddView(m_PeakSprite4);
  AddView(m_PeakSprite5);
  AddView(m_PeakSprite6);
  AddView(m_PeakSprite7);
  
  AddView(m_VerticalBar0);
  AddView(m_VerticalBar1);
  AddView(m_VerticalBar2);
  AddView(m_VerticalBar3);
  AddView(m_VerticalBar4);
  AddView(m_VerticalBar5);
  AddView(m_VerticalBar6);
  AddView(m_VerticalBar7);
  AddView(m_Background);
  
  m_VerticalBar0.ConnectBarHeightModel(m_BandPower0);
  m_VerticalBar0.ConnectBarColorModel(m_ColorModel0);
  m_PeakSprite0.ConnectYPositionModel(m_GravitationalModel0);
  m_GravitationalModel0.ConnectPositionModel(m_VerticalBar0);
  AddModel(m_BandPower0);
  AddModel(m_ColorModel0);
  AddModel(m_VerticalBar0);
  AddModel(m_GravitationalModel0); 
  
  m_VerticalBar1.ConnectBarHeightModel(m_BandPower1);
  m_VerticalBar1.ConnectBarColorModel(m_ColorModel1);
  m_PeakSprite1.ConnectYPositionModel(m_GravitationalModel1);
  m_GravitationalModel1.ConnectPositionModel(m_VerticalBar1);
  AddModel(m_BandPower1);
  AddModel(m_ColorModel1);
  AddModel(m_VerticalBar1);
  AddModel(m_GravitationalModel1);
  
  m_VerticalBar2.ConnectBarHeightModel(m_BandPower2);
  m_VerticalBar2.ConnectBarColorModel(m_ColorModel2);
  m_PeakSprite2.ConnectYPositionModel(m_GravitationalModel2);
  m_GravitationalModel2.ConnectPositionModel(m_VerticalBar2);
  AddModel(m_BandPower2);
  AddModel(m_ColorModel2);
  AddModel(m_VerticalBar2);
  AddModel(m_GravitationalModel2);
  
  m_VerticalBar3.ConnectBarHeightModel(m_BandPower3);
  m_VerticalBar3.ConnectBarColorModel(m_ColorModel3);
  m_PeakSprite3.ConnectYPositionModel(m_GravitationalModel3);
  m_GravitationalModel3.ConnectPositionModel(m_VerticalBar3);
  AddModel(m_BandPower3);
  AddModel(m_ColorModel3);
  AddModel(m_VerticalBar3);
  AddModel(m_GravitationalModel3);
  
  m_VerticalBar4.ConnectBarHeightModel(m_BandPower4);
  m_VerticalBar4.ConnectBarColorModel(m_ColorModel4);
  m_PeakSprite4.ConnectYPositionModel(m_GravitationalModel4);
  m_GravitationalModel4.ConnectPositionModel(m_VerticalBar4);
  AddModel(m_BandPower4);
  AddModel(m_ColorModel4);
  AddModel(m_VerticalBar4);
  AddModel(m_GravitationalModel4);
  
  m_VerticalBar5.ConnectBarHeightModel(m_BandPower5);
  m_VerticalBar5.ConnectBarColorModel(m_ColorModel5);
  m_PeakSprite5.ConnectYPositionModel(m_GravitationalModel5);
  m_GravitationalModel5.ConnectPositionModel(m_VerticalBar5);
  AddModel(m_BandPower5);
  AddModel(m_ColorModel5);
  AddModel(m_VerticalBar5);
  AddModel(m_GravitationalModel5);
  
  m_VerticalBar6.ConnectBarHeightModel(m_BandPower6);
  m_VerticalBar6.ConnectBarColorModel(m_ColorModel6);
  m_PeakSprite6.ConnectYPositionModel(m_GravitationalModel6);
  m_GravitationalModel6.ConnectPositionModel(m_VerticalBar6);
  AddModel(m_BandPower6);
  AddModel(m_ColorModel6);
  AddModel(m_VerticalBar6);
  AddModel(m_GravitationalModel6);
  
  m_VerticalBar7.ConnectBarHeightModel(m_BandPower7);
  m_VerticalBar7.ConnectBarColorModel(m_ColorModel7);
  m_PeakSprite7.ConnectYPositionModel(m_GravitationalModel7);
  m_GravitationalModel7.ConnectPositionModel(m_VerticalBar7);
  AddModel(m_BandPower7);
  AddModel(m_ColorModel7);
  AddModel(m_VerticalBar7);
  AddModel(m_GravitationalModel7);
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
  AddView(m_FloorSprite0);
  AddView(m_FloorSprite1);
  AddView(m_FloorSprite2);
  
  AddView(m_PeakSprite0);
  AddView(m_PeakSprite1);
  AddView(m_PeakSprite2);
  
  AddView(m_VerticalBar0);
  AddView(m_VerticalBar1);
  AddView(m_VerticalBar2);

  AddView(m_Background);
  
  AddModel(m_BandPower0);
  AddModel(m_ColorModel0);
  AddModel(m_VerticalBar0);
  
  AddModel(m_GravitationalModel0);
  m_VerticalBar0.ConnectBarHeightModel(m_BandPower0);
  m_VerticalBar0.ConnectBarColorModel(m_ColorModel0);
  m_PeakSprite0.ConnectYPositionModel(m_GravitationalModel0);
  m_GravitationalModel0.ConnectPositionModel(m_VerticalBar0);
  
  AddModel(m_BandPower1);
  AddModel(m_ColorModel1);
  AddModel(m_VerticalBar1);
  AddModel(m_GravitationalModel1);
  m_VerticalBar1.ConnectBarHeightModel(m_BandPower1);
  m_VerticalBar1.ConnectBarColorModel(m_ColorModel1);
  m_PeakSprite1.ConnectYPositionModel(m_GravitationalModel1);
  m_GravitationalModel1.ConnectPositionModel(m_VerticalBar1);
  
  AddModel(m_BandPower2);
  AddModel(m_ColorModel2);
  AddModel(m_VerticalBar2);
  AddModel(m_GravitationalModel2);
  m_VerticalBar2.ConnectBarHeightModel(m_BandPower2);
  m_VerticalBar2.ConnectBarColorModel(m_ColorModel2);
  m_PeakSprite2.ConnectYPositionModel(m_GravitationalModel2);
  m_GravitationalModel2.ConnectPositionModel(m_VerticalBar2);
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

//WaterFireFromEdge
Visualization* WaterFireFromEdge::GetInstance(StatisticalEngineModelInterface &StatisticalEngineModelInterface, LEDController &LEDController)
{
  if(true == debugMemory) Serial << "WaterFireFromEdge: Get Instance\n";
  WaterFireFromEdge *vis = new WaterFireFromEdge(StatisticalEngineModelInterface, LEDController);
  return vis;
}
void WaterFireFromEdge::SetupVisualization()
{
  AddView(m_FadingView0, false);
  AddView(m_FadingView1, true);
  
  m_FadingView0.AddSubView(m_ScrollingView0);
  m_ScrollingView0.AddSubView(m_Sprite0);
  m_FadingView1.AddSubView(m_ScrollingView1);
  m_ScrollingView1.AddSubView(m_Sprite1);
  
  AddModel(m_PowerModel0);
  AddModel(m_ColorModel0);
  AddModel(m_PowerColorModel0);
  m_PowerColorModel0.ConnectColorModel(m_ColorModel0);
  m_PowerColorModel0.ConnectPowerModel(m_PowerModel0);
  m_Sprite0.ConnectColorModel(m_PowerColorModel0);
  
  AddModel(m_PowerModel1);
  AddModel(m_ColorModel1);
  AddModel(m_PowerColorModel1);
  m_PowerColorModel1.ConnectColorModel(m_ColorModel1);
  m_PowerColorModel1.ConnectPowerModel(m_PowerModel1);
  m_Sprite1.ConnectColorModel(m_PowerColorModel1);  
}
bool WaterFireFromEdge::CanRunVisualization()
{ 
  return true; 
}
void WaterFireFromEdge::RunVisualization()
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
  int numBands = m_StatisticalEngineModelInterface.GetNumberOfBands();
  for(int i = 0; i < numBands; ++i)
  {
    int yPosition1 = (int)round(i*(float)SCREEN_HEIGHT/(float)numBands);
    int yPosition2 = (int)round((i+1)*(float)SCREEN_HEIGHT/(float)numBands);
    int visHeight = yPosition2 - yPosition1;
    int band = i;
    if(true == debugVisualization) Serial << "Index:" << i << "\tY:" << yPosition1 << "\tH:" << visHeight << "\tB:" << band << " of " << numBands << "\n";
    ColorSpriteView *sprite = new ColorSpriteView("Sprite", 0, yPosition1, SCREEN_WIDTH, visHeight, MergeType_Layer);
    AddNewedView(*sprite, true);
    
    BandPowerModel *bandPower = new BandPowerModel("Band Power Model", i, m_StatisticalEngineModelInterface);
    AddNewedModel(*bandPower);
    
    RainbowColorModel *colorModel = new RainbowColorModel("Color Model", i, numBands);
    AddNewedModel(*colorModel);
    
    SettableColorPowerModel *settableColorPowerModel = new SettableColorPowerModel("Settable Power Model");
    AddNewedModel(*settableColorPowerModel);
    
    settableColorPowerModel->ConnectColorModel(*colorModel);
    settableColorPowerModel->ConnectPowerModel(*bandPower);
    sprite->ConnectColorModel(*settableColorPowerModel); 
  }
  ColorSpriteView *background = new ColorSpriteView("Background", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, CRGB::Black, MergeType_Layer);
  AddNewedView(*background, true);
}
bool VerticalBandTower::CanRunVisualization()
{ 
  return true; 
}
void VerticalBandTower::RunVisualization()
{
}
//ScrollingBands
Visualization* ScrollingBands::GetInstance(StatisticalEngineModelInterface &StatisticalEngineModelInterface, LEDController &LEDController)
{
  if(true == debugMemory) Serial << "ScrollingBands: Get Instance\n";
  ScrollingBands *vis = new ScrollingBands(StatisticalEngineModelInterface, LEDController);
  return vis;
}
void ScrollingBands::SetupVisualization()
{
  AddView(m_FadingView0, false);
  AddView(m_FadingView1, true);
  
  AddModel(m_BandPower0);
  AddModel(m_BandPower1);
  AddModel(m_BandPower2);
  AddModel(m_BandPower3);
  AddModel(m_BandPower4);
  AddModel(m_BandPower5);
  AddModel(m_BandPower6);
  AddModel(m_BandPower7);
  
  AddModel(m_ColorModel0);
  AddModel(m_ColorModel1);
  AddModel(m_ColorModel2);
  AddModel(m_ColorModel3);
  AddModel(m_ColorModel4);
  AddModel(m_ColorModel5);
  AddModel(m_ColorModel6);
  AddModel(m_ColorModel7);

  AddModel(m_PowerColorModel0);
  AddModel(m_PowerColorModel1);
  AddModel(m_PowerColorModel2);
  AddModel(m_PowerColorModel3);
  AddModel(m_PowerColorModel4);
  AddModel(m_PowerColorModel5);
  AddModel(m_PowerColorModel6);
  AddModel(m_PowerColorModel7);

  m_PowerColorModel0.ConnectPowerModel(m_BandPower0);
  m_PowerColorModel1.ConnectPowerModel(m_BandPower1);
  m_PowerColorModel2.ConnectPowerModel(m_BandPower2);
  m_PowerColorModel3.ConnectPowerModel(m_BandPower3);
  m_PowerColorModel4.ConnectPowerModel(m_BandPower4);
  m_PowerColorModel5.ConnectPowerModel(m_BandPower5);
  m_PowerColorModel6.ConnectPowerModel(m_BandPower6);
  m_PowerColorModel7.ConnectPowerModel(m_BandPower7);
  
  m_PowerColorModel0.ConnectColorModel(m_ColorModel0);
  m_PowerColorModel1.ConnectColorModel(m_ColorModel1);
  m_PowerColorModel2.ConnectColorModel(m_ColorModel2);
  m_PowerColorModel3.ConnectColorModel(m_ColorModel3);
  m_PowerColorModel4.ConnectColorModel(m_ColorModel4);
  m_PowerColorModel5.ConnectColorModel(m_ColorModel5);
  m_PowerColorModel6.ConnectColorModel(m_ColorModel6);
  m_PowerColorModel7.ConnectColorModel(m_ColorModel7);
  
  m_Sprite0.ConnectColorModel(m_PowerColorModel0);
  m_Sprite1.ConnectColorModel(m_PowerColorModel1);
  m_Sprite2.ConnectColorModel(m_PowerColorModel2);
  m_Sprite3.ConnectColorModel(m_PowerColorModel3);
  m_Sprite4.ConnectColorModel(m_PowerColorModel4);
  m_Sprite5.ConnectColorModel(m_PowerColorModel5);
  m_Sprite6.ConnectColorModel(m_PowerColorModel6);
  m_Sprite7.ConnectColorModel(m_PowerColorModel7);

  m_RotateView0.AddSubView(m_Sprite0);
  m_RotateView0.AddSubView(m_Sprite1);
  m_RotateView0.AddSubView(m_Sprite2);
  m_RotateView0.AddSubView(m_Sprite3);
  m_RotateView1.AddSubView(m_Sprite4);
  m_RotateView1.AddSubView(m_Sprite5);
  m_RotateView1.AddSubView(m_Sprite6);
  m_RotateView1.AddSubView(m_Sprite7);

  m_ScrollingView0.AddSubView(m_RotateView0);
  m_ScrollingView1.AddSubView(m_RotateView1);

  m_FadingView0.AddSubView(m_ScrollingView0);
  m_FadingView1.AddSubView(m_ScrollingView1);
}
bool ScrollingBands::CanRunVisualization()
{
  return true;
}
void ScrollingBands::RunVisualization()
{
}

Visualization* ScrollingMaxBand::GetInstance(StatisticalEngineModelInterface &StatisticalEngineModelInterface, LEDController &LEDController)
{
  if(true == debugMemory) Serial << "ScrollingMaxBand: Get Instance\n";
  ScrollingMaxBand *vis = new ScrollingMaxBand(StatisticalEngineModelInterface, LEDController);
  return vis;
}
void ScrollingMaxBand::SetupVisualization()
{
  AddSubView(m_ScrollingView);
  AddModel(m_MaxBandModel);
  AddModel(m_BandDataColorModel);
  m_ScrollingView.AddSubView(m_Sprite0);
  m_BandDataColorModel.ConnectBandDataModel(m_MaxBandModel);
  m_Sprite0.ConnectColorModel(m_BandDataColorModel);
}
bool ScrollingMaxBand::CanRunVisualization()
{
  return true;
}
void ScrollingMaxBand::RunVisualization()
{
}
