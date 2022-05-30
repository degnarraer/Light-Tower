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
  m_MyVisiualizationInstantiations.add(VUMeter::GetInstance);
  m_MyVisiualizationInstantiations.add(VUMeter3Band::GetInstance);
  m_MyVisiualizationInstantiations.add(VUMeter8Band::GetInstance);
  m_MyVisiualizationInstantiations.add(Waterfall::GetInstance);
  m_MyVisiualizationInstantiations.add(Fire::GetInstance);
  m_MyVisiualizationInstantiations.add(WaterFireFromCenter::GetInstance);
  m_MyVisiualizationInstantiations.add(WaterFireFromEdge::GetInstance);
  m_MyVisiualizationInstantiations.add(VerticalBandTower::GetInstance);
  m_MyVisiualizationInstantiations.add(ScrollingBands::GetInstance);
  m_MyVisiualizationInstantiations.add(ScrollingMaxBand::GetInstance);
  m_MyVisiualizationInstantiations.add(RotatingSprites::GetInstance);
  m_MyVisiualizationInstantiations.add(BallShooter::GetInstance);
  m_MyVisiualizationInstantiations.add(SolidColorTower::GetInstance);
  m_MyVisiualizationInstantiations.add(VerticalBassSpriteTower::GetInstance);
  m_MyVisiualizationInstantiations.add(PowerPerBinTower::GetInstance);
  m_MyVisiualizationInstantiations.add(Rotating4Sprites::GetInstance);

  bool testVisualization = true;
  if(true == testVisualization)
  {
    m_Duration = 10000000;
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
  if(m_CurrentDuration >= m_Duration)
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
  GetInstanceFunctionPointer GetInstanceFunctionPointer = m_MyVisiualizationInstantiations.get( random(0, m_MyVisiualizationInstantiations.size()));
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
  GetInstanceFunctionPointer GetInstanceFunctionPointer = m_MyVisiualizationInstantiations.get( random(0, m_MyVisiualizationInstantiations.size()));
  m_CurrentVisualization = GetInstanceFunctionPointer(m_StatisticalEngineModelInterface, m_LEDController);
  AddTask(*m_CurrentVisualization);
  m_StartTime = millis();
  if(true == debugMemory) Serial << "VisualizationPlayer::Getting Next Visualization: Task Count: " << GetTaskCount() << "\n";
}
