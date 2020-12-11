
#include "TaskInterface.h"
#include "Streaming.h"
#include "LEDControllerInterface.h"
#include "Tunes.h"


class VisualizationInterface: public TaskInterface
                            , LEDControllerInterface
{
  public:
    VisualizationInterface(){}
    virtual void Setup() = 0;
    virtual void Start() = 0;
    virtual bool Loop() = 0;
    virtual void End() = 0;
};
