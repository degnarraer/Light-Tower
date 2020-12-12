#include "VisualizationInterface.h"

class Visualization: public VisualizationInterface
{
  public:
    Visualization(){}    
    void Setup() {}
    void Start() {}
    bool Loop() {}
    void End() {}
    bool m_visualizationStarted = false;
    unsigned long m_resetTimer;
};


class BandAmplitudes: public Visualization
{
  public:
    BandAmplitudes(): Visualization(){}
    virtual ~BandAmplitudes()
    {
      if(true == debugMode && debugLevel >= 1) Serial << "Delete BandAmplitudes\n";
    }
    virtual void Start();
    virtual bool Loop();
    virtual void End();
  private:
    void Tick1();
    void Random();
    unsigned long m_randomTimes[1];
    const int m_gainLimitMax = 1000;
    const int m_gainLimitMin = 1;
    const int m_gainmultiplier = 10;
    int m_gainIntigrator = 100;
        
};
