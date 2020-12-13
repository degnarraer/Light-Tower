#include "VisualizationController.h"

void VisualizationController::Setup()
{
  m_StatisticalEngine.ConnectCallback(this);
  Task *tasks[1] = {&m_StatisticalEngine};
  m_Scheduler.SetTasks(tasks, 1);
  m_Scheduler.Setup();
}
void VisualizationController::RunTaskLoop()
{
  delay(1000);
  m_Scheduler.RunTaskLoops();
}
