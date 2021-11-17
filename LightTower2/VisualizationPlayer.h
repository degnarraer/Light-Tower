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
 
#ifndef VisualizationPlayer_H
#define VisualizationPlayer_H

#include "Streaming.h"
#include "VisualizationFactory.h"
#include "TaskInterface.h"
#include "Statistical_Engine.h"
#include "LEDControllerInterface.h"


extern "C" char* sbrk(int incr);
class VisualizationPlayer : public Task
{
  public:
    VisualizationPlayer(StatisticalEngineModelInterface &StatisticalEngineModelInterface) : Task("VisualizationPlayer")
                                                                                          , m_StatisticalEngineModelInterface(StatisticalEngineModelInterface){}
    virtual ~VisualizationPlayer(){}

  private:
    StatisticalEngineModelInterface &m_StatisticalEngineModelInterface;
    LEDController m_LEDController;

    unsigned long m_StartTime;
    unsigned long m_CurrentTime;
    unsigned long m_Duration;
    unsigned long m_CurrentDuration;
    void GetNextVisualization();
    void GetRandomVisualization();
    
    //Task Interface
    void Setup();
    void RunMyPreTask(){}
    bool CanRunMyScheduledTask();
    void RunMyScheduledTask();
    void RunMyPostTask(){}
        
    Visualization *m_CurrentVisualization;
    Visualization *m_PreviousVisualization;
    typedef Visualization* (* GetInstanceFunctionPointer)(StatisticalEngineModelInterface &, LEDController &);
    LinkedList<GetInstanceFunctionPointer> m_MyVisiualizationInstantiations = LinkedList<GetInstanceFunctionPointer>();
    LinkedList<Visualization*> m_MyQueue = LinkedList<Visualization*>();
    
    void PrintFreeMemory(String text)
    {
      if(true == debugFreeMemory ) Serial << text << freeMemory() << "\n";
    }
    
    int freeMemory()
    {
      char top;
      return &top - reinterpret_cast<char*>(sbrk(0));
    }
};

#endif
