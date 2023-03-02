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
  myCallees.push_back(&callee);
}
void VisualizationEventNotificationCallerInterface::RegisterForNotification(VisualizationEventNotificationCalleeInterface &callee, String context)
{
  CallerInterfaceData cid;
  cid.Callee = &callee;
  cid.Context = context;
  myCalleesWithContext.push_back(cid);
}
void VisualizationEventNotificationCallerInterface::DeRegisterForNotification(VisualizationEventNotificationCalleeInterface &callee)
{
  for(int i = 0; i < myCallees.size(); ++i)
  {
    if(myCallees[i] == &callee)
    {
      myCallees.erase(myCallees.begin() + i);
      break;
    }
  }
}
void VisualizationEventNotificationCallerInterface::DeRegisterForNotification(VisualizationEventNotificationCalleeInterface &callee, String context)
{
  CallerInterfaceData cid;
  cid.Callee = &callee;
  cid.Context = context;
  for(int i = 0; i < myCalleesWithContext.size(); ++i)
  {
    if(myCalleesWithContext[i] == cid)
    {
      myCalleesWithContext.erase(myCalleesWithContext.begin() + i);
      break;
    }
  }
}
void VisualizationEventNotificationCallerInterface::SendVisualizationCompleteNotificationToCalleesFrom(VisualizationEventNotificationCallerInterface &source)
{
  for(int i = 0; i < myCallees.size(); ++i)
  {
    myCallees[i]->VisualizationCompleteNotificationFrom(source);
  }
}

//************** Visualization **************
void Visualization::SetupMyView()
{
  SetupVisualization();
}
bool Visualization::CanRunMyViewScheduledTask()
{
  MergeSubViews();
  return CanRunVisualization();
}
void Visualization::RunMyViewScheduledTask()
{
  RunVisualization();
  m_LEDController.UpdateLEDs(m_PixelArray);
}
void Visualization::AddView(View &view)
{
  AddSubView(view, true);
}
void Visualization::AddView(View &view, bool clearViewBeforeMerge)
{
  AddSubView(view, clearViewBeforeMerge);
}
void Visualization::AddModel(Model &model)
{ 
  m_MyModels.push_back(&model);
  AddTask(model);
}
void Visualization::AddNewedModel(Model &model)
{
  m_MyNewedModels.push_back(&model);
  m_MyModels.push_back(&model);
  AddTask(model);
}
void Visualization::AddNewedView(View &view)
{
  AddSubView(view, true);
  m_MyNewedViews.insert(m_MyNewedViews.begin(), &view);
}
void Visualization::AddNewedView(View &view, bool clearViewBeforeMerge)
{
  AddSubView(view, clearViewBeforeMerge);
  m_MyNewedViews.insert(m_MyNewedViews.begin(), &view);
}
void Visualization::DeleteAllNewedObjects()
{
  for(int m = 0; m < m_MyNewedViews.size(); ++m)
  {
    View *view = m_MyNewedViews[m];
    delete view;
  }
  for(int m = 0; m < m_MyNewedModels.size(); ++m)
  {
    Model *model = m_MyNewedModels[m];
    delete model;
  }
}
