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

void Visualization::AddSubView(View &view)
{ 
  m_MyViews.add(&view);
  m_MyTasks.add(&view);
}
void Visualization::AddModel(Model &model)
{ 
  m_MyModels.add(&model);
  m_MyTasks.add(&model); 
}

void Visualization::Setup()
{
  SetupVisualization();
  AddTasks(m_MyTasks);
}
bool Visualization::CanRunMyTask()
{
  return CanRunVisualization();
}
void Visualization::RunMyTask()
{
  RunVisualization();
}
