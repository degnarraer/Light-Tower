#ifndef Visualization_H
#define Visualization_H
#include "VisualizationInterface.h"
#include <LinkedList.h>


class ModelEventNotificationCallerInterface;

class ModelEventNotificationCalleeInterface
{
public:
    virtual void NewValueNotificationFrom(float Value, ModelEventNotificationCallerInterface &source) = 0;
};

class ModelEventNotificationCallerInterface
{
  public:
    void RegisterForNotification(ModelEventNotificationCalleeInterface &callee)
    {
      if(true == debugModelNotifications) Serial << "ModelEventNotificationCallerInterface: Add: ";        
      myCallees.add(&callee);
    }
    void DeRegisterForNotification(ModelEventNotificationCalleeInterface &callee)
    {
      for(int i = 0; i < myCallees.size(); ++i)
      {
        if(myCallees.get(i) == &callee)
        {
          myCallees.remove(i);
          break;
        }
      }
    }
    void SendNotificationToCalleesFrom(float value, ModelEventNotificationCallerInterface &source)
    {
      for(int i = 0; i < myCallees.size(); ++i)
      {
        myCallees.get(i)->NewValueNotificationFrom(value, source);
      }
    }
  private:
    LinkedList<ModelEventNotificationCalleeInterface*> myCallees = LinkedList<ModelEventNotificationCalleeInterface*>();
};

class Model: public ModelEventNotificationCallerInterface
{
  public: 
    Model(){}
  private:
    float m_PreviousValue;
    float m_CurrentValue;
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

    //Views
    LinkedList<View*> SubViews = LinkedList<View*>();
    View *ParentView;
    void AddChildView(View Child);
    void RemoveChildView(View Child);
    void RemoveAllChildrenViews();
    
    //Models
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
