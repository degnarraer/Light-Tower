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


void VisualizationEventNotificationCallerInterface::RegisterForNotification(VisualizationEventNotificationCalleeInterface &callee)
{
  if(true == debugModelNotifications) Serial << "VisualizationEventNotificationCallerInterface: Add: ";        
  myCallees.add(&callee);
}
void VisualizationEventNotificationCallerInterface::DeRegisterForNotification(VisualizationEventNotificationCalleeInterface &callee)
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
void VisualizationEventNotificationCallerInterface::SendVisualizationCompleteNotificationToCalleesFrom(VisualizationEventNotificationCallerInterface &source)
{
  for(int i = 0; i < myCallees.size(); ++i)
  {
    myCallees.get(i)->VisualizationCompleteNotificationFrom(source);
  }
}
void Visualization::AddView(View &View)
{
  m_MyViews.add(&View);
  AddTask(View);
}
void Visualization::AddModel(Model &Model)
{ 
  m_MyModels.add(&Model);
  m_StatisticalEngineModelInterface.AddModel(Model);
  AddTask(Model);
}
void Visualization::RemoveAllModels()
{
  for(int m = 0; m < m_MyModels.size(); ++m)
  {
    Model *aModel = m_MyModels.get(m);
    m_StatisticalEngineModelInterface.RemoveModel(*aModel);
  }
}
void Visualization::Setup()
{
  if(true == debugVisualization) Serial << "Setup Visualization\n"; 
  for(int x = 0; x < SCREEN_WIDTH; ++x)
  {
    for(int y = 0; y < SCREEN_HEIGHT; ++ y)
    {
      m_MyPixelStruct.Pixel[x][y] = CRGB::Black;
      if(true == debugVisualization) Serial << "\tR: " << m_MyPixelStruct.Pixel[x][y].red << "\tG: " << m_MyPixelStruct.Pixel[x][y].green << "\tB: " << m_MyPixelStruct.Pixel[x][y].blue << "\n";
    }
  }
  SetupVisualization();
}
bool Visualization::CanRunMyTask()
{
  return CanRunVisualization();
}
void Visualization::RunMyTask()
{
  RunVisualization();
  MergeSubViews();
  m_LEDController.UpdateLEDs(m_MyPixelStruct);
}

void Visualization::MergeSubViews()
{
  for(int x = 0; x < SCREEN_WIDTH; ++x)
  {
    for(int y = 0; y < SCREEN_HEIGHT; ++y)
    {
      m_MyPixelStruct.Pixel[x][y] = CRGB::Black;
    }
  }
  for(int v = 0; v < m_MyViews.size(); ++v)
  {
    View *aView = m_MyViews.get(v);
    PixelStruct &aPixelStruct = aView->GetPixelStruct();
    for(int y = 0; y < SCREEN_HEIGHT; ++y)
    {
      for(int x = 0; x < SCREEN_WIDTH; ++x)
      {
        if(true == debugVisualization) Serial << "Pixel Value " << "\tR:" << aPixelStruct.Pixel[x][y].red << "\tG:" << aPixelStruct.Pixel[x][y].green << "\tB:" << aPixelStruct.Pixel[x][y].blue << "\n";
        if(
          aPixelStruct.Pixel[x][y].red != 0 ||
          aPixelStruct.Pixel[x][y].green != 0 ||
          aPixelStruct.Pixel[x][y].blue != 0
          )
          {
            if(true == debugVisualization) Serial << "Set Pixel " << x << "|" << y << " to: " << "\tR:" << aPixelStruct.Pixel[x][y].red << "\tG:" << aPixelStruct.Pixel[x][y].green << "\tB:" << aPixelStruct.Pixel[x][y].blue << "\n";
            m_MyPixelStruct.Pixel[x][y] = aPixelStruct.Pixel[x][y];
          }
      }
    }
  }
}

//Visualization
Visualization* VUMeter::GetInstance(StatisticalEngineModelInterface &StatisticalEngineModelInterface, LEDController &LEDController)
{
  VUMeter *vis = new VUMeter(StatisticalEngineModelInterface, LEDController);
  return vis;
}
void VUMeter::SetupVisualization()
{
  m_VerticalBar.SetModel(m_SoundPower);
  AddView(m_VerticalBar);
  AddModel(m_SoundPower);
}
bool VUMeter::CanRunVisualization(){ return true; }
void VUMeter::RunVisualization()
{
}
