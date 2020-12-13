#ifndef VisualizationController_H
#define VisualizationController_H

#include "Statistical_Engine.h"
#include "Visualizations.h"
#include "LEDControllerInterface.h"
#include "TaskInterface.h"

class VisualizationController: public Task
                             , MicrophoneMeasureCalleeInterface
                             , InterruptHandler
{
  public:
    VisualizationController(): Task("VisualizationController"){}
    void HandleInterrupt() { m_StatisticalEngine.HandleInterrupt(); }

    //Task Interface
    void Setup();
    bool CanRunTaskLoop(){ return true; }
    void RunTaskLoop();

    //MicrophoneMeasureCalleeInterface
    void MicrophoneStateChange(SoundState){}
  private:
    StatisticalEngine m_StatisticalEngine;
    TaskScheduler m_Scheduler;
};

#endif
