#ifndef VisualizationController_H
#define VisualizationController_H

#include "Statistical_Engine.h"
#include "Visualizations.h"
#include "LEDControllerInterface.h"
#include "TaskInterface.h"

class VisualizationController: public Task
                             , MicrophoneMeasureCalleeInterface
                             , ADCInterruptHandler
{
  public:
    VisualizationController(): Task("VisualizationController"){}
    void HandleADCInterrupt() { m_StatisticalEngine.HandleADCInterrupt(); }
  protected:
    //MicrophoneMeasureCalleeInterface
    void MicrophoneStateChange(SoundState){}
    
    //Task Interface
    void Setup();
    bool CanRunTask(){ return true; }
    void RunTask();
  private:
    StatisticalEngine m_StatisticalEngine;
    TaskScheduler m_Scheduler;
};

#endif
