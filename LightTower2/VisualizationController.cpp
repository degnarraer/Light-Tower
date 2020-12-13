#include "VisualizationController.h"

void VisualizationController::Setup()
{
  m_Scheduler.SetTasks(m_Tasks, sizeof(m_Tasks)/sizeof(m_Tasks[0]));
  m_StatisticalEngine.ConnectCallback(this);
  m_Scheduler.Setup();
}
void VisualizationController::RunTask()
{
  m_Scheduler.RunTasks();
}
