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
  m_PixelArray = new PixelArray(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
  m_PixelArray->Clear();
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
  m_LEDController.UpdateLEDs(m_PixelArray);
}

void Visualization::MergeSubViews()
{
  m_PixelArray->Clear();
  for(int v = 0; v < m_MyViews.size(); ++v)
  {
    View *aView = m_MyViews.get(v);
    for(int y = 0; y < m_PixelArray->GetHeight(); ++y)
    {
      for(int x = 0; x < m_PixelArray->GetWidth(); ++x)
      {
        if(true == debugLEDs) Serial << "Pixel Value " << "\tR:" << aView->GetPixel(x, y).red << "\tG:" << aView->GetPixel(x, y).green << "\tB:" << aView->GetPixel(x, y).blue << "\n";
        if( aView->GetPixel(x, y).red != 0 || aView->GetPixel(x, y).green != 0 || aView->GetPixel(x, y).blue != 0 )
        {
          switch(aView->GetMergeType())
          {
            case MergeType_Layer:
              if(true == debugLEDs) Serial << "Set Pixel " << x << "|" << y << " to: " << "\tR:" << aView->GetPixel(x, y).red << "\tG:" << aView->GetPixel(x, y).green << "\tB:" << aView->GetPixel(x, y).blue << "\n";
              m_PixelArray->SetPixel(x, y, aView->GetPixel(x, y));
            break;
            case MergeType_Add:
            {
              if(true == debugLEDs) Serial << "Add Pixel " << x << "|" << y << " to: " << "\tR:" << aView->GetPixel(x, y).red << "\tG:" << aView->GetPixel(x, y).green << "\tB:" << aView->GetPixel(x, y).blue << "\n";
              CRGB pixel;
              pixel.red = qadd8(aView->GetPixel(x, y).red, m_PixelArray->GetPixel(x, y).red);
              pixel.blue = qadd8(aView->GetPixel(x, y).blue, m_PixelArray->GetPixel(x, y).blue);
              pixel.green = qadd8(aView->GetPixel(x, y).green, m_PixelArray->GetPixel(x, y).green);
              m_PixelArray->SetPixel(x, y, pixel);
            }
            break;
            default:
            break;
          }
        }
      }
    }
  }
}
