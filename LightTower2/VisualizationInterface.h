#ifndef VisualizationInterface_H
#define VisualizationInterface_H

#include "TaskInterface.h"
#include "Streaming.h"
#include "LEDControllerInterface.h"
#include "Tunes.h"


class VisualizationInterface: public Task
                            , LEDControllerInterface
{
  public:
    VisualizationInterface(){}
    virtual void Setup() = 0;
    virtual void Start() = 0;
    virtual void RunTaskLoop() = 0;
    virtual void End() = 0;
};

#endif
