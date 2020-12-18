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
    m_MyCallees.get(i)->NewValueNotificationFrom(value, source);
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
  m_MyModels.add(&Model);
}
//Task
void ModelNewValueProcessor::Setup(){};
bool ModelNewValueProcessor::CanRunMyTask()
{
  bool result = false;
  for(int m = 0; m < m_MyModels.size(); ++m)
  {
    result |= m_MyModels.get(m)->HasUser();
    if(result = true) break;
  }
  return result;
};
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
};
