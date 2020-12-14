#ifndef Visualization_H
#define Visualization_H

#include "VisualizationInterface.h"

class Model
{
  public: 
    Model(){}
  private:
    float m_PreviousValue;
    float m_CurrentValue;
    void ConnectCallback(Model *cb)
    {
        m_cb = cb;
    }
    Model *m_cb;
    virtual void NewValueNotification(float Value) = 0;
    virtual void ModelUpdate() = 0;
};

class View
{
  typedef int position;
  typedef int size;
  public:
    View(position x, position y, size l, size w){}
    position X;
    position Y;
    size Length;
    size Width;
    View *Children;
    View *Parent;
    void AddChildView(View Child);
    void RemoveChildView(View Child);
    void RemoveAllChildrenViews();
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


class BandAmplitudes: public Visualization
{
  public:
    BandAmplitudes(): Visualization(){}
    virtual ~BandAmplitudes()
    {
      if(true == debugMode && debugLevel >= 1) Serial << "Delete BandAmplitudes\n";
    }
    virtual void Start();
    virtual void Loop();
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

#endif
