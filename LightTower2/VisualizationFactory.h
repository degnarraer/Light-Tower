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
 
#ifndef VisualizationFactory_H
#define VisualizationFactory_H

#include "Streaming.h"
#include "Visualizations.h"
#include "TaskInterface.h"
#include "Statistical_Engine.h"
#include "LEDControllerInterface.h"


class VisualizationFactory : public Task
{
  public:
    VisualizationFactory(StatisticalEngineModelInterface &StatisticalEngineModelInterface) : Task("VisualizationFactory")
                                                                                           , m_StatisticalEngineModelInterface(StatisticalEngineModelInterface){}
    ~VisualizationFactory(){}

  private:
    StatisticalEngineModelInterface &m_StatisticalEngineModelInterface;
    LEDController m_LEDController;
    VUMeter *m_VUMeter;
    
    //Task Interface
    void Setup();
    bool CanRunMyTask();
    void RunMyTask();
};

#endif
