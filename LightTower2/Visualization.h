#ifndef Visualization_H
#define Visualization_H

#include "Statistical_Engine.h"
#include "Views.h"
#include "Streaming.h"
#include "Models.h"
#include "TaskInterface.h"
#include "LEDControllerInterface.h"
#include <LinkedList.h>

class VisualizationEventNotificationCallerInterface;

class VisualizationEventNotificationCalleeInterface
{
public:
    virtual void VisualizationCompleteNotificationFrom(VisualizationEventNotificationCallerInterface &source) = 0;
};

class VisualizationEventNotificationCallerInterface
{
  public:
    void RegisterForNotification(VisualizationEventNotificationCalleeInterface &callee);
    void DeRegisterForNotification(VisualizationEventNotificationCalleeInterface &callee);
    void SendVisualizationCompleteNotificationToCalleesFrom(VisualizationEventNotificationCallerInterface &source);
  private:
    LinkedList<VisualizationEventNotificationCalleeInterface*> myCallees = LinkedList<VisualizationEventNotificationCalleeInterface*>();
};

class Visualization: public View
                   , VisualizationEventNotificationCallerInterface
{
  public:
    Visualization( StatisticalEngineModelInterface &StatisticalEngineModelInterface, 
                   LEDController &LEDController) : View("Visualization", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT)
                                                 , m_StatisticalEngineModelInterface(StatisticalEngineModelInterface)
                                                 , m_LEDController(LEDController){}
               
    virtual ~Visualization()
    {
      if(true == debugMemory) Serial << "Delete: Visualization\n";
      DeleteAllNewedObjects();
    }
    StatisticalEngineModelInterface &m_StatisticalEngineModelInterface;
    LEDController &m_LEDController;

    //View
    void SetupView();
    bool CanRunViewTask();
    void RunViewTask();
    
    virtual void SetupVisualization() = 0;
    virtual bool CanRunVisualization() = 0;
    virtual void RunVisualization() = 0;

  protected:
    void AddView(View &view);
    void AddNewedView(View &view);
    void AddModel(Model &model);
    void AddNewedModel(Model &model);
    void DeleteAllNewedObjects();
  private:
    LinkedList<Model*> m_MyModels = LinkedList<Model*>();
    LinkedList<View*> m_MyNewedViews = LinkedList<View*>();
    LinkedList<Model*> m_MyNewedModels = LinkedList<Model*>();
};

#endif
