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
 
#include "Visualization.h"


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
void Visualization::AddView(View &view)
{
  m_MyViews.add(&view);
  AddTask(view);
}
void Visualization::AddModel(Model &model)
{ 
  m_MyModels.add(&model);
  AddTask(model);
}
void Visualization::AddNewedModel(Model &model)
{
  m_MyNewedModels.add(&model);
  m_MyModels.add(&model);
  AddTask(model);
}
void Visualization::AddNewedView(View &view)
{
  m_MyViews.add(&view);
  m_MyNewedViews.add(&view);
  AddTask(view);
}
void Visualization::DeleteAllNewedObjects()
{
  for(int m = 0; m < m_MyNewedViews.size(); ++m)
  {
    View *view = m_MyNewedViews.get(m);
    delete view;
  }
  for(int m = 0; m < m_MyNewedModels.size(); ++m)
  {
    Model *model = m_MyNewedModels.get(m);
    delete model;
  }
}
void Visualization::Setup()
{
  if(true == debugVisualization) Serial << "Setup Visualization\n"; 
  m_MyPixelStruct.Clear();
  SetupVisualization();
}
bool Visualization::CanRunMyTask()
{
  return CanRunVisualization();
}
void Visualization::RunMyTask()
{
  MergeSubViews();
  RunVisualization();
  m_LEDController.UpdateLEDs(m_MyPixelStruct);
}

void Visualization::MergeSubViews()
{
  m_MyPixelStruct.Clear();
  for(int v = 0; v < m_MyViews.size(); ++v)
  {
    View *aView = m_MyViews.get(v);
    PixelStruct &aPixelStruct = aView->GetPixelStruct();
    for(int y = 0; y < SCREEN_HEIGHT; ++y)
    {
      for(int x = 0; x < SCREEN_WIDTH; ++x)
      {
        if(true == debugLEDs) Serial << "Pixel Value " << "\tR:" << aPixelStruct.Pixel[x][y].red << "\tG:" << aPixelStruct.Pixel[x][y].green << "\tB:" << aPixelStruct.Pixel[x][y].blue << "\n";
        if(
          aPixelStruct.Pixel[x][y].red != 0 ||
          aPixelStruct.Pixel[x][y].green != 0 ||
          aPixelStruct.Pixel[x][y].blue != 0
          )
          {
            switch(aView->GetMergeType())
            {
              case MergeType_Layer:
                if(true == debugLEDs) Serial << "Set Pixel " << x << "|" << y << " to: " << "\tR:" << aPixelStruct.Pixel[x][y].red << "\tG:" << aPixelStruct.Pixel[x][y].green << "\tB:" << aPixelStruct.Pixel[x][y].blue << "\n";
                m_MyPixelStruct.Pixel[x][y] = aPixelStruct.Pixel[x][y];
              break;
              case MergeType_Add:
                if(true == debugLEDs) Serial << "Add Pixel " << x << "|" << y << " to: " << "\tR:" << aPixelStruct.Pixel[x][y].red << "\tG:" << aPixelStruct.Pixel[x][y].green << "\tB:" << aPixelStruct.Pixel[x][y].blue << "\n";
                m_MyPixelStruct.Pixel[x][y].red = qadd8(aPixelStruct.Pixel[x][y].red, m_MyPixelStruct.Pixel[x][y].red);
                m_MyPixelStruct.Pixel[x][y].blue = qadd8(aPixelStruct.Pixel[x][y].blue, m_MyPixelStruct.Pixel[x][y].blue);
                m_MyPixelStruct.Pixel[x][y].green = qadd8(aPixelStruct.Pixel[x][y].green, m_MyPixelStruct.Pixel[x][y].green);
              break;
              default:
              break;
            }
          }
      }
    }
  }
}
