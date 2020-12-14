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
/**
 * @file LightTower2.ino
 * *

 */

#ifndef Visualization_H
#define Visualization_H
#include "Streaming.h"
#include "Models.h"
#include "TaskInterface.h"
#include <LinkedList.h>

class VisualizationInterface: public Task
{
  public:
    VisualizationInterface(){}
    virtual void Setup() = 0;
    virtual void Start() = 0;
    virtual void RunTask() = 0;
    virtual void End() = 0;
};

class View: public ModelEventNotificationCalleeInterface
{
  typedef int position;
  typedef int size;
  public:
    View(position x, position y, size l, size w){}
    position X;
    position Y;
    size Length;
    size Width;

    //Views
    LinkedList<View*> SubViews = LinkedList<View*>();
    View *ParentView;
    void AddChildView(View Child);
    void RemoveChildView(View Child);
    void RemoveAllChildrenViews();
    
    //Models    
    void NewValueNotificationFrom(float Value, ModelEventNotificationCallerInterface &source);
    LinkedList<Model*> Models = LinkedList<Model*>();
    void AddModel(Model aModel);
    void RemoveModel(Model aModel);
    void RemoveAllModels();
};

class Controller
{
  public: 
  Controller(){}
};

class Visualization: public VisualizationInterface
                   , Model
                   , View
                   , Controller
{
  public:
    Visualization(): View(0, 0, NUMLEDS, NUMSTRIPS){}    
    void Setup() {}
    void Start() {}
    void Loop() {}
    void End() {}
    bool m_visualizationStarted = false;
    unsigned long m_resetTimer;
};


//********* VUMeter *********
class VUMeter: public Visualization
{
  public:
    VUMeter(): Visualization(){}
    virtual ~VUMeter()
    {
      if(true == debugMode && debugLevel >= 1) Serial << "Delete VUMeter\n";
    }
    virtual void Start();
    virtual void Loop();
    virtual void End();
  private:        
};

#endif
