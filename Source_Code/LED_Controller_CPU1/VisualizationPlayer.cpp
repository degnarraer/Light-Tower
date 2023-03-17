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

#include "VisualizationPlayer.h"

void VisualizationPlayer::Setup()
{
  //m_MyVisiualizationInstantiations.push_back(VUMeter::GetInstance);
  //m_MyVisiualizationInstantiations.push_back(VUMeter3Band::GetInstance);
  //m_MyVisiualizationInstantiations.push_back(VUMeter8Band::GetInstance);
  //m_MyVisiualizationInstantiations.push_back(Waterfall::GetInstance);
  //m_MyVisiualizationInstantiations.push_back(Fire::GetInstance);
  //m_MyVisiualizationInstantiations.push_back(WaterFireFromCenter::GetInstance);
  //m_MyVisiualizationInstantiations.push_back(WaterFireFromEdge::GetInstance);
  m_MyVisiualizationInstantiations.push_back(VerticalBandTower::GetInstance);
  //m_MyVisiualizationInstantiations.push_back(ScrollingBands::GetInstance);
  m_MyVisiualizationInstantiations.push_back(ScrollingMaxBand::GetInstance);
  //m_MyVisiualizationInstantiations.push_back(RotatingSprites::GetInstance);
  //m_MyVisiualizationInstantiations.push_back(BallShooter::GetInstance);
  m_MyVisiualizationInstantiations.push_back(SolidColorTower::GetInstance);
  //m_MyVisiualizationInstantiations.push_back(VerticalBassSpriteTower::GetInstance);
  //m_MyVisiualizationInstantiations.push_back(PowerPerBinTower::GetInstance);
  //m_MyVisiualizationInstantiations.push_back(Rotating4Sprites::GetInstance);

  if(true == m_TestVisualization)
  {
    m_CurrentVisualization = SolidColorTower::GetInstance(m_StatisticalEngineModelInterface, m_LEDController);
    AddTask(*m_CurrentVisualization);
    m_StartTime = millis();
  }
  else
  {
    GetRandomVisualization();
  }
}
bool VisualizationPlayer::CanRunMyScheduledTask()
{ 
  return true;
}
void VisualizationPlayer::RunMyScheduledTask()
{
  m_CurrentTime = millis();
  m_CurrentDuration = m_CurrentTime - m_StartTime;
  if(m_CurrentDuration >= m_Duration && false == m_TestVisualization)
  {
    GetRandomVisualization();
  }
}

unsigned long m_Duration;
unsigned long m_CurrentDuration;
void VisualizationPlayer::GetNextVisualization()
{
  m_Duration = random(1000,120000);
  RemoveTask(*m_CurrentVisualization);
  delete m_CurrentVisualization;
  GetInstanceFunctionPointer GetInstanceFunctionPointer = m_MyVisiualizationInstantiations[ random(0, m_MyVisiualizationInstantiations.size()) ];
  m_CurrentVisualization = GetInstanceFunctionPointer(m_StatisticalEngineModelInterface, m_LEDController);
  AddTask(*m_CurrentVisualization);
  m_StartTime = millis();
  if(true == debugMemory) Serial << "VisualizationPlayer::Getting Next Visualization: Task Count: " << GetTaskCount() << "\n";
}
void VisualizationPlayer::GetRandomVisualization()
{
  m_Duration = random(1000,120000);
  RemoveTask(*m_CurrentVisualization);
  delete m_CurrentVisualization;
  GetInstanceFunctionPointer GetInstanceFunctionPointer = m_MyVisiualizationInstantiations[ random(0, m_MyVisiualizationInstantiations.size()) ];
  m_CurrentVisualization = GetInstanceFunctionPointer(m_StatisticalEngineModelInterface, m_LEDController);
  AddTask(*m_CurrentVisualization);
  m_StartTime = millis();
  if(true == debugMemory) Serial << "VisualizationPlayer::Getting Next Visualization: Task Count: " << GetTaskCount() << "\n";
}
