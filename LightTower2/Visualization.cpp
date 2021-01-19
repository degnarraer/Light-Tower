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

//************** VisualizationEventNotificationCallerInterface **************
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

//************** Visualization **************
void Visualization::SetupView()
{
  SetupVisualization();
}
bool Visualization::CanRunViewTask()
{
  return CanRunVisualization();
}
void Visualization::RunViewTask()
{
  MergeSubViews(true);
  RunVisualization();
  m_LEDController.UpdateLEDs(m_PixelArray);
}
void Visualization::AddView(View &view)
{
  AddSubView(view);
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
  AddSubView(view);
  m_MyNewedViews.add(&view);
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
