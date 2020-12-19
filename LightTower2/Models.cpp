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
 
#include "Models.h"

void Model::Setup()
{
  SetupModel();
}
bool Model::CanRunMyTask()
{
  return CanRunModelTask();
}
void Model::RunMyTask()
{
  RunModelTask();
}

void ModelEventNotificationCallerInterface::RegisterForNotification(ModelEventNotificationCalleeInterface &callee)
{
  if(true == debugModelNotifications) Serial << "ModelEventNotificationCallerInterface: Added\n";        
  m_MyCallees.add(&callee);
}
void ModelEventNotificationCallerInterface::DeRegisterForNotification(ModelEventNotificationCalleeInterface &callee)
{
  for(int i = 0; i < m_MyCallees.size(); ++i)
  {
    if(m_MyCallees.get(i) == &callee)
    {
      m_MyCallees.remove(i);
      break;
    }
  }
}
bool ModelEventNotificationCallerInterface::HasUser() 
{
  return (m_MyCallees.size() > 0)? true:false;
}
void ModelEventNotificationCallerInterface::SendNewValueNotificationToCallees(float value, ModelEventNotificationCallerInterface &source)
{
  if(true == debugModelNotifications) Serial << "ModelEventNotificationCallerInterface: Sending New Value Notification with Value: " << value << "\n"; 
  for(int i = 0; i < m_MyCallees.size(); ++i)
  {
    if(true == debugModelNotifications) Serial << "ModelEventNotificationCallerInterface: Sending Notification " << i << "\n"; 
    m_MyCallees.get(i)->NewFloatValueNotificationFrom(value, source);
  }
}

void Model::SetCurrentValue(float value)
{
  m_CurrentValue = value;
  if(m_PreviousValue != m_CurrentValue)
  {
    SendNewValueNotificationToCallees(m_CurrentValue, *this);
    m_PreviousValue = m_CurrentValue;
  }
}

void ModelNewValueProcessor::AddModel(Model &Model)
{
  if(true == debugModelNewValueProcessor) Serial << "ModelNewValueProcessor: Add Model\n";
  m_MyModels.add(&Model);
}
bool ModelNewValueProcessor::RemoveModel(Model &Model)
{
  bool modelFound = false;
  for(int m = 0; m < m_MyModels.size(); ++m)
  {
    if(m_MyModels.get(m) == &Model)
    {
      modelFound = true;
      m_MyModels.remove(m);
      break;
    }
  }
  if(true == modelFound)
  {
    if(true == debugModelNewValueProcessor) Serial << "ModelNewValueProcessor Successfully Removed Task: " << Model.GetTaskTitle() << "\n";
    return true;
  }
  else
  {
    if(true == debugModelNewValueProcessor) Serial << "ModelNewValueProcessor failed to Remove Task: " << Model.GetTaskTitle() << "\n";
    return false;
  }
}

void ModelNewValueProcessor::Setup()
{
}
bool ModelNewValueProcessor::CanRunMyTask()
{
  bool result = false;
  for(int m = 0; m < m_MyModels.size(); ++m)
  {
    result |= m_MyModels.get(m)->HasUser();
    if(result = true) break;
  }
  return result;
}
void ModelNewValueProcessor::RunMyTask()
{
  for(int m = 0; m < m_MyModels.size(); ++m)
  {
    Model *aModel = m_MyModels.get(m);
    if(true == aModel->HasUser())
    {
      aModel->UpdateValue();
    }
  }
}

void StatisticalEngineModelInterface::Setup()
{ 
  m_StatisticalEngine.ConnectCallback(this);
  AddTask(m_StatisticalEngine);
  AddTask(m_ModelNewValueProcessor);
}
bool StatisticalEngineModelInterface::CanRunMyTask()
{ 
  return true; 
}
void StatisticalEngineModelInterface::RunMyTask()
{
}
void StatisticalEngineModelInterface::AddModel(Model &Model) 
{
  m_ModelNewValueProcessor.AddModel(Model); 
}
bool StatisticalEngineModelInterface::RemoveModel(Model &Model)
{
  m_ModelNewValueProcessor.RemoveModel(Model); 
}
