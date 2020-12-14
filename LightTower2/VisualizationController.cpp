#include "VisualizationController.h"

void VisualizationController::Setup()
{
  m_Scheduler.AddTask(m_StatisticalEngine);
  m_StatisticalEngine.ConnectCallback(this);
}
void VisualizationController::RunTask()
{
  m_Scheduler.RunTasks();
}
