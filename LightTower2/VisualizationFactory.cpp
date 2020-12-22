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

void VisualizationFactory::Setup()
{
  //m_MyVisiualizations.add(VUMeter::GetInstance);
  //m_MyVisiualizations.add(VUMeter8Band::GetInstance);
  //GetRandomVisualization();
  m_CurrentVisualization = new VUMeter8Band(m_StatisticalEngineModelInterface, m_LEDController);
  AddTask(*m_CurrentVisualization);
}
bool VisualizationFactory::CanRunMyTask()
{ 
  return true;
}
void VisualizationFactory::RunMyTask()
{
}

unsigned long m_Duration;
unsigned long m_CurrentDuration;
void VisualizationFactory::GetNextVisualization()
{
  /*
  RemoveTask(*m_PreviousVisualization);
  delete m_PreviousVisualization;
  m_PreviousVisualization = m_CurrentVisualization;
  m_CurrentVisualization = m_MyVisiualizations.get( random(0, m_MyVisiualizations.size()) )->GetInstance(m_StatisticalEngineModelInterface, m_LEDController);
  AddTask(*m_CurrentVisualization);
  */
}
void VisualizationFactory::GetRandomVisualization()
{
  /*
  RemoveTask(*m_PreviousVisualization);
  delete m_PreviousVisualization;
  m_PreviousVisualization = m_CurrentVisualization;
  m_CurrentVisualization = m_MyVisiualizations.get( random(0, m_MyVisiualizations.size()) )->GetInstance(m_StatisticalEngineModelInterface, m_LEDController);
  AddTask(*m_CurrentVisualization);
  */
}
