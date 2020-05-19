
    /*
    Light Tower by Rob Shockency
    Copyright (C) 2019 Rob Shockency degnarraer@yahoo.com

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
    */
/**
 * @file ActiveVisualizationController.cpp
 *
 *
 */
 
#include "ActiveVisualizationController.h"
#include <typeinfo>

extern "C" char* sbrk(int incr);

int freeMemory()
{
  char top;
  return &top - reinterpret_cast<char*>(sbrk(0));
}

void ActiveVisualizationController::Setup()
{
  if(true == debugMode && debugLevel >= 0) Serial << "Active Visualization Controller: Setup\n";
  pinMode(2, INPUT);      // set pin to input
  digitalWrite(2, HIGH);  // turn on pullup resistors
  m_statisticalEngine.Setup();
  m_statisticalEngine.ConnectCallback(this);
  
  AddSceneConfigToQueue(VisualizationEntries::VisualizationEntries_InstantSwitch, VisualizationEntries_SoundDetectionTester, UINT_MAX);  
  /*
  AddSceneConfigToQueue(VisualizationEntries::VisualizationEntries_InstantSwitch, VisualizationEntries_ColorFadingTower, 0);
  AddSceneConfigToQueue(VisualizationEntries::VisualizationEntries_MixerMergeTransition, VisualizationEntries::VisualizationEntries_SoundDetectionTester, 10000);
  AddSceneConfigToQueue(VisualizationEntries::VisualizationEntries_InstantSwitch, VisualizationEntries_Confirmation, 0);
  AddSceneConfigToQueue(VisualizationEntries::VisualizationEntries_MixerMergeTransition, VisualizationEntries::VisualizationEntries_SoundDetectionTester, 10000);
  AddSceneConfigToQueue(VisualizationEntries::VisualizationEntries_InstantSwitch, VisualizationEntries_WaterFallFireStreamer, 0);
  AddSceneConfigToQueue(VisualizationEntries::VisualizationEntries_MixerMergeTransition, VisualizationEntries::VisualizationEntries_SoundDetectionTester, 10000);
  AddSceneConfigToQueue(VisualizationEntries::VisualizationEntries_InstantSwitch, VisualizationEntries_SolidColorTower, 0);
  AddSceneConfigToQueue(VisualizationEntries::VisualizationEntries_MixerMergeTransition, VisualizationEntries::VisualizationEntries_SoundDetectionTester, 10000);
  AddSceneConfigToQueue(VisualizationEntries::VisualizationEntries_InstantSwitch, VisualizationEntries_FadingSolidColorTower, 0);
  AddSceneConfigToQueue(VisualizationEntries::VisualizationEntries_MixerMergeTransition, VisualizationEntries::VisualizationEntries_SoundDetectionTester, 10000);
  AddSceneConfigToQueue(VisualizationEntries::VisualizationEntries_InstantSwitch, VisualizationEntries_PowerBarWithBassSprite, 0);
  AddSceneConfigToQueue(VisualizationEntries::VisualizationEntries_MixerMergeTransition, VisualizationEntries::VisualizationEntries_SoundDetectionTester, 10000);
  AddSceneConfigToQueue(VisualizationEntries::VisualizationEntries_InstantSwitch, VisualizationEntries_RandomFrequencySprites, 0);
  AddSceneConfigToQueue(VisualizationEntries::VisualizationEntries_MixerMergeTransition, VisualizationEntries::VisualizationEntries_SoundDetectionTester, 10000);
  AddSceneConfigToQueue(VisualizationEntries::VisualizationEntries_InstantSwitch, VisualizationEntries_FFTAmplitudes, 0);
  AddSceneConfigToQueue(VisualizationEntries::VisualizationEntries_MixerMergeTransition, VisualizationEntries::VisualizationEntries_SoundDetectionTester, 10000);
  AddSceneConfigToQueue(VisualizationEntries::VisualizationEntries_InstantSwitch, VisualizationEntries_FrequencySpriteSpiral, 0);
  AddSceneConfigToQueue(VisualizationEntries::VisualizationEntries_MixerMergeTransition, VisualizationEntries::VisualizationEntries_SoundDetectionTester, 10000);
  AddSceneConfigToQueue(VisualizationEntries::VisualizationEntries_InstantSwitch, VisualizationEntries_RandomHighLowFrequencyAmplitudeStreamer, 0);
  AddSceneConfigToQueue(VisualizationEntries::VisualizationEntries_MixerMergeTransition, VisualizationEntries::VisualizationEntries_SoundDetectionTester, 10000);
  AddSceneConfigToQueue(VisualizationEntries::VisualizationEntries_InstantSwitch, VisualizationEntries_OutwardAmplitudeWithFloatingBassSprites, 0);
  AddSceneConfigToQueue(VisualizationEntries::VisualizationEntries_MixerMergeTransition, VisualizationEntries::VisualizationEntries_SoundDetectionTester, 10000);
  AddSceneConfigToQueue(VisualizationEntries::VisualizationEntries_InstantSwitch, VisualizationEntries_VerticalFFTAmplitudeTower, 0);
  AddSceneConfigToQueue(VisualizationEntries::VisualizationEntries_MixerMergeTransition, VisualizationEntries::VisualizationEntries_SoundDetectionTester, 10000);
  AddSceneConfigToQueue(VisualizationEntries::VisualizationEntries_InstantSwitch, VisualizationEntries_MultiRangeAmplitudeTower, 0);
  AddSceneConfigToQueue(VisualizationEntries::VisualizationEntries_MixerMergeTransition, VisualizationEntries::VisualizationEntries_SoundDetectionTester, 10000);
  AddSceneConfigToQueue(VisualizationEntries::VisualizationEntries_InstantSwitch, VisualizationEntries_SimultaneousFrequencyStreamer, 0);
  AddSceneConfigToQueue(VisualizationEntries::VisualizationEntries_MixerMergeTransition, VisualizationEntries::VisualizationEntries_SoundDetectionTester, 10000);
  AddSceneConfigToQueue(VisualizationEntries::VisualizationEntries_InstantSwitch, VisualizationEntries_MinMaxAmplitude, 0);
  AddSceneConfigToQueue(VisualizationEntries::VisualizationEntries_MixerMergeTransition, VisualizationEntries::VisualizationEntries_SoundDetectionTester, 10000);
  AddSceneConfigToQueue(VisualizationEntries::VisualizationEntries_InstantSwitch, VisualizationEntries_ChasingSprites, 0);
  AddSceneConfigToQueue(VisualizationEntries::VisualizationEntries_MixerMergeTransition, VisualizationEntries::VisualizationEntries_SoundDetectionTester, 10000);
  AddSceneConfigToQueue(VisualizationEntries::VisualizationEntries_InstantSwitch, VisualizationEntries_FrequencyColorStreamer, 0);
  AddSceneConfigToQueue(VisualizationEntries::VisualizationEntries_MixerMergeTransition, VisualizationEntries::VisualizationEntries_SoundDetectionTester, 10000);
  AddSceneConfigToQueue(VisualizationEntries::VisualizationEntries_InstantSwitch, VisualizationEntries_FrequencyColorSpinningTower, 0);
  AddSceneConfigToQueue(VisualizationEntries::VisualizationEntries_MixerMergeTransition, VisualizationEntries::VisualizationEntries_SoundDetectionTester, 10000);
  AddSceneConfigToQueue(VisualizationEntries::VisualizationEntries_InstantSwitch, VisualizationEntries_UpDownFrequencyColorStreamer, 0);
  AddSceneConfigToQueue(VisualizationEntries::VisualizationEntries_MixerMergeTransition, VisualizationEntries::VisualizationEntries_SoundDetectionTester, 10000);
  AddSceneConfigToQueue(VisualizationEntries::VisualizationEntries_InstantSwitch, VisualizationEntries_UpDownMaxFrequencyStreamer, 0);
  AddSceneConfigToQueue(VisualizationEntries::VisualizationEntries_MixerMergeTransition, VisualizationEntries::VisualizationEntries_SoundDetectionTester, 10000);
  AddSceneConfigToQueue(VisualizationEntries::VisualizationEntries_InstantSwitch, VisualizationEntries_ScrollingRainbow, 0);
  AddSceneConfigToQueue(VisualizationEntries::VisualizationEntries_MixerMergeTransition, VisualizationEntries::VisualizationEntries_SoundDetectionTester, 10000);
  AddSceneConfigToQueue(VisualizationEntries::VisualizationEntries_InstantSwitch, VisualizationEntries_FadingColors2, 0);
  AddSceneConfigToQueue(VisualizationEntries::VisualizationEntries_MixerMergeTransition, VisualizationEntries::VisualizationEntries_SoundDetectionTester, 10000);
  AddSceneConfigToQueue(VisualizationEntries::VisualizationEntries_InstantSwitch, VisualizationEntries_ScrollingFrequencyColorRectangles, 0);
  AddSceneConfigToQueue(VisualizationEntries::VisualizationEntries_MixerMergeTransition, VisualizationEntries::VisualizationEntries_SoundDetectionTester, 10000);
  AddSceneConfigToQueue(VisualizationEntries::VisualizationEntries_InstantSwitch, VisualizationEntries_ScrollingFrequencySprites, 0);
  AddSceneConfigToQueue(VisualizationEntries::VisualizationEntries_MixerMergeTransition, VisualizationEntries::VisualizationEntries_SoundDetectionTester, 10000);
  AddSceneConfigToQueue(VisualizationEntries::VisualizationEntries_InstantSwitch, VisualizationEntries_ScrollingSpeedFrequencySprites, 0);
  AddSceneConfigToQueue(VisualizationEntries::VisualizationEntries_MixerMergeTransition, VisualizationEntries::VisualizationEntries_SoundDetectionTester, 10000);
  AddSceneConfigToQueue(VisualizationEntries::VisualizationEntries_InstantSwitch, VisualizationEntries_ScrollingAmplitudeSprite, 0);
  AddSceneConfigToQueue(VisualizationEntries::VisualizationEntries_MixerMergeTransition, VisualizationEntries::VisualizationEntries_SoundDetectionTester, 10000);
  AddSceneConfigToQueue(VisualizationEntries::VisualizationEntries_InstantSwitch, VisualizationEntries_Opposites, 0);
  AddSceneConfigToQueue(VisualizationEntries::VisualizationEntries_MixerMergeTransition, VisualizationEntries::VisualizationEntries_SoundDetectionTester, 10000);
  AddSceneConfigToQueue(VisualizationEntries::VisualizationEntries_InstantSwitch, VisualizationEntries_Snake, 0);
  AddSceneConfigToQueue(VisualizationEntries::VisualizationEntries_MixerMergeTransition, VisualizationEntries::VisualizationEntries_SoundDetectionTester, 10000);
  */
  GetNextTransition();
  m_gainAdjustModeActive = false;
  if(true == debugMode && debugLevel >= 0) Serial << "Active Visualization Controller: Setup Complete\n";
}

void ActiveVisualizationController::HandleInterrupt()
{
  m_statisticalEngine.HandleInterrupt();
}

Visualizations * ActiveVisualizationController::GetCurrentVisualizationPtr()
{
  return m_currentVisualization;
}

Visualizations * ActiveVisualizationController::GetPreviousVisualizationPtr()
{
  return m_previousVisualization;  
}

CRGB ActiveVisualizationController::GetConfirmationColor()
{
  return m_activeSceneConfig.confirmationColor;
}

void ActiveVisualizationController::VisualizationStarted(Visualizations *visualization)
{
  if(visualization == m_currentVisualization)
  {
    if(true == debugMode && debugLevel >= 2) Serial << "Callback: Active Visualization Started\n";
  }
}

void ActiveVisualizationController::VisualizationEnded(Visualizations *visualization)
{
  if(visualization == m_currentVisualization)
  {
    if(true == m_automaticMode)
    {
      if(true == debugMode && debugLevel >= 2) Serial << "Callback: Active Visualization Ended\n";
      GetNextTransition();
    }
  }
}

void ActiveVisualizationController::TransitionStarted(Visualizations *visualization)
{
  if(visualization == m_activeTransition)
  {
    if(true == debugMode && debugLevel >= 2) Serial << "Callback: Active Transition Started\n"; 
  }
}

void ActiveVisualizationController::TransitionEnded(Visualizations *visualization)
{
  if(visualization == m_activeTransition)
  {
    if(true == debugMode && debugLevel >= 2) Serial << "Callback: Active Transition Ended\n";
    if(true == m_automaticMode)
    {
      GetNextTransition();
    }
  }
}

void ActiveVisualizationController::ConfirmationVisualizationStarted(Visualizations *visualization)
{
  if(visualization == m_currentVisualization)
  {
    if(true == debugMode && debugLevel >= 2) Serial << "Callback: Confirmation Started\n"; 
  }
}

void ActiveVisualizationController::ConfirmationVisualizationEnded(Visualizations *visualization)
{
  if(visualization == m_currentVisualization)
  {
    if(true == debugMode && debugLevel >= 2) Serial << "Callback: Active Confirmation Ended\n";
    GetNextTransition();
  }
}

void ActiveVisualizationController::AddSceneConfigToQueue(VisualizationEntries transition, VisualizationEntries visualization, unsigned long duration, CRGB confirmationColor)
{
  int count = (sizeof(m_sceneConfigQueue)/sizeof(*m_sceneConfigQueue));
  if(m_sceneConfigQueueHeadIndex - m_sceneConfigQueueTailIndex < count - 1)
  {
    SceneConfig item = {transition, visualization, duration, confirmationColor};
    m_sceneConfigQueue[m_sceneConfigQueueHeadIndex % count] = item;
    ++m_sceneConfigQueueHeadIndex;
  }
  if(true == debugMode && debugLevel >= 0) Serial << "Adding Scene Config with Confirmation Color.  Head: " << m_sceneConfigQueueHeadIndex << "\tTail: " << m_sceneConfigQueueTailIndex << "\tDuration: " << duration << "\n";
}

SceneConfig ActiveVisualizationController::GetNextSceneConfigFromQueue()
{
  int count = (sizeof(m_sceneConfigQueue)/sizeof(*m_sceneConfigQueue));
  SceneConfig result;
  if(m_sceneConfigQueueHeadIndex - m_sceneConfigQueueTailIndex >= 0)
  {
    result = m_sceneConfigQueue[m_sceneConfigQueueTailIndex % count];
    ++m_sceneConfigQueueTailIndex;
  }
  if(true == debugMode && debugLevel >= 0) Serial << "Getting Next Scene Config.  Head: " << m_sceneConfigQueueHeadIndex << "\tTail: " << m_sceneConfigQueueTailIndex << "\tDuration: " << result.duration << "\n";
  return result;
}

void ActiveVisualizationController::MicrophoneStateChange(SoundState state)  
{
  switch(state)
  {
    case SoundDetected:
      if(true == debugMode && debugLevel >= 0) Serial << "Sound Detected Callback\n";
      Illuminate(100);
      if(false == m_1stSoundDetected)
      {
        m_1stSoundDetected = true;
        if(m_sceneConfigQueueHeadIndex - m_sceneConfigQueueTailIndex == 0) AddSceneConfigToQueue(VisualizationEntries::VisualizationEntries_FadeTransition, VisualizationEntries_GetRandom, RANDOM_TIME);
        GetNextTransition();
      }
      else if(false == m_gainAdjustModeActive)
      {
        if(m_sceneConfigQueueHeadIndex - m_sceneConfigQueueTailIndex == 0) AddSceneConfigToQueue(VisualizationEntries::VisualizationEntries_FadeTransition, VisualizationEntries_GetRandom, RANDOM_TIME);
        GetNextTransition();
      }
    break;
    case SilenceDetected:
      if(true == debugMode && debugLevel >= 0) Serial << "Silence Detected Callback\n";
      Illuminate(50);
      if(false == m_gainAdjustModeActive)
      {
        if(m_sceneConfigQueueHeadIndex - m_sceneConfigQueueTailIndex == 0)
        {
          AddSceneConfigToQueue(VisualizationEntries::VisualizationEntries_InstantSwitch, m_visualizationsFactory.GetRandomStaticVisualizationEntry(), 0);
          AddSceneConfigToQueue(VisualizationEntries::VisualizationEntries_MixerMergeTransition, VisualizationEntries::VisualizationEntries_SoundDetectionTester, ULONG_MAX);
        }
        GetNextTransition(); 
      }
    break;
    case LastingSilenceDetected:
      if(true == debugMode && debugLevel >= 0) Serial << "Lasting Silence Detected Callback\n";
      if(false == m_gainAdjustModeActive)
      {
        Deluminate();
      }
    break;
    default:
    break;
  }
}

void ActiveVisualizationController::TestSequenceComplete()
{
  if(true == m_automaticMode)
  {
    AddSceneConfigToQueue(VisualizationEntries::VisualizationEntries_InstantSwitch, VisualizationEntries::VisualizationEntries_GetNext, 60000);
    GetNextTransition(); 
  }
}

void ActiveVisualizationController::StoreActiveVisualization()
{
  if(m_currentVisualization != NULL)
  {
    m_previousVisualization = m_currentVisualization;
    m_previousVisualizationEntry = m_activeSceneConfig.visualizationEntry;
    m_currentVisualization = NULL;
  }
}
void ActiveVisualizationController::StartVisualization()
{
  if(m_currentVisualization != NULL)
  {
    m_currentVisualization->Start();
  }
  else
  {
    if(true == debugMode && debugLevel >= 2) Serial << "Failed to create visualization\n";
  }
}

void ActiveVisualizationController::TrashTransition()
{
  if(m_activeTransition != NULL) 
  {
    m_garbageTransition = m_activeTransition;
    m_previousTransitionEntry = m_activeSceneConfig.transitionEntry;
    m_activeTransition = NULL;
  }
}
void ActiveVisualizationController::EmptyTransitionTrash()
{
  if(m_garbageTransition != NULL)
  {
    if(true == debugMode && debugLevel >= 2) Serial << "Emptying Trash\n";
    if(m_garbageTransition->getCurrentVisualization() != m_currentVisualization && m_garbageTransition->getCurrentVisualization() != m_previousVisualization)
    {
      if(true == debugMode && debugLevel >= 2) Serial << "Deleting Current Visualization\n";
      m_garbageTransition->DeleteCurrentVisualization();
    }
    if(m_garbageTransition->getPreviousVisualization() != m_currentVisualization && m_garbageTransition->getPreviousVisualization() != m_previousVisualization)
    {
      if(true == debugMode && debugLevel >= 2) Serial << "Deleting Previous Visualization\n";
      m_garbageTransition->DeletePreviousVisualization();      
    }
    delete m_garbageTransition;
    m_garbageTransition = NULL;
  }
}
void ActiveVisualizationController::PrintFreeMemory(String text)
{
  if(true == debugMode && debugLevel >= 1) 
  {
    if(true == debugMode && debugLevel >= 1) Serial << text << freeMemory() << "\n";
  }
}

void ActiveVisualizationController::GetNextTransition()
{
  bool usingSceneConfig = false;
  if(true == debugMode && debugLevel >= 1) Serial << "********* NEXT TRANSITION *********\n";
  if(true == debugMode && debugLevel >= 1) Serial << "Test Mode: " << m_statisticalEngine.GetTestMode() << "\tAuto Mode: " << m_automaticMode << "\n";
  PrintFreeMemory("Free Memory Before: ");
  EmptyTransitionTrash();
  TrashTransition();

  VisualizationType visualizationType = VisualizationType::TRANSITION;
  if(m_sceneConfigQueueHeadIndex - m_sceneConfigQueueTailIndex > 0)
  {
    usingSceneConfig = true;
    m_activeSceneConfig = GetNextSceneConfigFromQueue();
    if(VisualizationEntries::VisualizationEntries_CommandsStart < m_activeSceneConfig.transitionEntry && m_activeSceneConfig.transitionEntry < VisualizationEntries::VisualizationEntries_CommandsEnd)
    {
      switch(m_activeSceneConfig.transitionEntry)
      {
        case VisualizationEntries::VisualizationEntries_GetNext:
          {
            m_currentTransitionEntry = m_visualizationsFactory.GetNextVisualizationConfigForTypeAfterIndex(visualizationType, m_activeSceneConfig.transitionEntry).visualizationEntry;
            if(true == debugMode && debugLevel >= 2) Serial << "Getting Next Transition: " << m_currentTransitionEntry << "\n";
          }
        break;
        case VisualizationEntries::VisualizationEntries_GetPrevious:
          {
            m_currentTransitionEntry = m_previousTransitionEntry;
            if(true == debugMode && debugLevel >= 2) Serial << "Getting Previous Transition: " << m_currentTransitionEntry << "\n";    
          }
        break;
        case VisualizationEntries::VisualizationEntries_GetRandom:
          {
            m_currentTransitionEntry = m_visualizationsFactory.GetVisualizationConfigForTypeAtIndex(visualizationType, random(0, m_visualizationsFactory.GetNumberOfVisualizationType(visualizationType))).visualizationEntry;
            if(true == debugMode && debugLevel >= 2) Serial << "Getting Random Transitionization: " << m_currentTransitionEntry << "\n";
          }
        break;
        default:
          {
            m_currentTransitionEntry = VisualizationEntries::VisualizationEntries_InstantSwitch;
            if(true == debugMode && debugLevel >= 2) Serial << "Getting Default Transition: " << m_currentTransitionEntry << "\n";
          }
        break;
      }
    }
    else if(VisualizationEntries::VisualizationEntries_TransitionStart < m_activeSceneConfig.transitionEntry && m_activeSceneConfig.transitionEntry < VisualizationEntries::VisualizationEntries_TransitionEnd)
    {
      m_currentTransitionEntry = m_activeSceneConfig.transitionEntry;
      if(true == debugMode && debugLevel >= 2) Serial << "Get Specific Transition: " << m_currentTransitionEntry <<"\n";
    }
    else
    {
      m_currentTransitionEntry = VisualizationEntries::VisualizationEntries_InstantSwitch;
      if(true == debugMode && debugLevel >= 2) Serial << "Getting Default Transition: " << m_currentTransitionEntry << "\n";
    }
  }
  else if(true == m_statisticalEngine.GetTestMode() || false == m_automaticMode)
  {
    m_currentTransitionEntry = VisualizationEntries::VisualizationEntries_InstantSwitch;
    if(true == debugMode && debugLevel >= 2) Serial << "Get Instant Switch Transition\n";
  }
  else
  {
    m_currentTransitionEntry = m_visualizationsFactory.GetVisualizationConfigForTypeAtIndex(visualizationType, random(0, m_visualizationsFactory.GetNumberOfVisualizationType(visualizationType))).visualizationEntry;
    if(true == debugMode && debugLevel >= 2) Serial << "Getting Random Transitionization: " << m_currentTransitionEntry << "\n";
  }
  GetNextVisualizer(usingSceneConfig);
  m_activeTransition = m_visualizationsFactory.GetVisualizationConfig(m_currentTransitionEntry).getTransitionInstancePtr(m_statisticalEngine, this);
  PrintFreeMemory("Free Memory After: ");
}

void ActiveVisualizationController::GetNextVisualizer(bool usingSceneConfig)
{
  VisualizationType visualizationType = VisualizationType::VISUALIZATION;
  PrintFreeMemory("Free Memory Before: ");
  StoreActiveVisualization();
  if(true == usingSceneConfig)
  {
    if(true == debugMode && debugLevel >= 2) Serial << "Active Scene Config Duration: " << m_activeSceneConfig.duration << "\n";
    if(VisualizationEntries::VisualizationEntries_VisualizationStart < m_activeSceneConfig.visualizationEntry && m_activeSceneConfig.visualizationEntry < VisualizationEntries::VisualizationEntries_VisualizationEnd)
    {
      m_currentVisualizationEntry = m_activeSceneConfig.visualizationEntry;
      if(true == debugMode && debugLevel >= 2) Serial << "Getting Specific Visualization: " << m_currentVisualizationEntry << "\n";
      m_currentVisualization = m_visualizationsFactory.GetVisualizationConfig(m_currentVisualizationEntry).getVisualizationInstancePtr(m_activeSceneConfig.duration, m_statisticalEngine, this);
    }
    else if(VisualizationEntries::VisualizationEntries_CommandsStart < m_activeSceneConfig.visualizationEntry && m_activeSceneConfig.visualizationEntry < VisualizationEntries::VisualizationEntries_CommandsEnd)
    {
      switch(m_activeSceneConfig.visualizationEntry)
      {
        case VisualizationEntries::VisualizationEntries_GetNext:
          {
            m_currentVisualizationEntry = m_visualizationsFactory.GetNextVisualizationConfigForTypeAfterIndex(visualizationType, m_currentVisualizationEntry).visualizationEntry;
            if(true == debugMode && debugLevel >= 2) Serial << "Getting Next Visualization: " << m_currentVisualizationEntry << "\n";
            m_currentVisualization = m_visualizationsFactory.GetVisualizationConfig(m_currentVisualizationEntry).getVisualizationInstancePtr(m_activeSceneConfig.duration, m_statisticalEngine, this);
          }
        break;
        case VisualizationEntries::VisualizationEntries_GetPrevious:
          {
            m_currentVisualizationEntry = m_previousVisualizationEntry;
            if(true == debugMode && debugLevel >= 2) Serial << "Getting Previous Visualization: " << m_currentVisualizationEntry << "\n";
            m_currentVisualization = m_visualizationsFactory.GetVisualizationConfig(m_currentVisualizationEntry).getVisualizationInstancePtr(m_activeSceneConfig.duration, m_statisticalEngine, this);
          }
        break;
        case VisualizationEntries::VisualizationEntries_GetRandom:
          {
            m_currentVisualizationEntry = m_visualizationsFactory.GetVisualizationConfigForTypeAtIndex(visualizationType, random(0, m_visualizationsFactory.GetNumberOfVisualizationType(visualizationType))).visualizationEntry;
            if(true == debugMode && debugLevel >= 2) Serial << "Getting Random Visualization: " << m_currentVisualizationEntry << "\n";
            m_currentVisualization = m_visualizationsFactory.GetVisualizationConfig(m_currentVisualizationEntry).getVisualizationInstancePtr(m_activeSceneConfig.duration, m_statisticalEngine, this);
          }
        break;
        case VisualizationEntries::VisualizationEntries_GetForeground:
          {
            m_currentVisualizationEntry = m_visualizationsFactory.GetRandomForegroundVisualizationConfig().visualizationEntry;
            if(true == debugMode && debugLevel >= 2) Serial << "Getting Random Foreground Visualization: " << m_currentVisualizationEntry << "\n";
            m_currentVisualization = m_visualizationsFactory.GetVisualizationConfig(m_currentVisualizationEntry).getVisualizationInstancePtr(m_activeSceneConfig.duration, m_statisticalEngine, this);
          }
        break;
        case VisualizationEntries::VisualizationEntries_GetBackground:
          {
            m_currentVisualizationEntry = m_visualizationsFactory.GetRandomBackgroundVisualizationConfig().visualizationEntry;
            if(true == debugMode && debugLevel >= 2) Serial << "Getting Random Background Visualization: " << m_currentVisualizationEntry << "\n";
            m_currentVisualization = m_visualizationsFactory.GetVisualizationConfig(m_currentVisualizationEntry).getVisualizationInstancePtr(m_activeSceneConfig.duration, m_statisticalEngine, this);
          }
        break;
        default:
          {
            m_currentVisualizationEntry = VisualizationEntries::VisualizationEntries_WaterFallFireStreamer;
            if(true == debugMode && debugLevel >= 2) Serial << "Getting Default Visualization: " << m_currentVisualizationEntry << "\n";
            m_currentVisualization = m_visualizationsFactory.GetVisualizationConfig(m_currentVisualizationEntry).getVisualizationInstancePtr(m_activeSceneConfig.duration, m_statisticalEngine, this);
          }
        break;
      }
    }
    else
    {
      m_currentVisualizationEntry = m_visualizationsFactory.GetVisualizationConfigForTypeAtIndex(visualizationType, random(0, m_visualizationsFactory.GetNumberOfVisualizationType(visualizationType))).visualizationEntry;
      if(true == debugMode && debugLevel >= 2) Serial << "Getting Random Visualization: " << m_currentVisualizationEntry << "\n";
      m_currentVisualization = m_visualizationsFactory.GetVisualizationConfig(m_currentVisualizationEntry).getVisualizationInstancePtr(RANDOM_TIME, m_statisticalEngine, this);
    }
  }
  else if(true == m_visualizationsFactory.GetVisualizationConfig(m_currentTransitionEntry).isBackground && true == m_visualizationsFactory.GetVisualizationConfig(m_currentTransitionEntry).isForeground)
  {
    m_previousVisualizationEntry = m_visualizationsFactory.GetRandomBackgroundVisualizationConfig().visualizationEntry;
    if(true == debugMode && debugLevel >= 2) Serial << "Getting Random Foreground Visualization: " << m_currentVisualizationEntry << "\n";
    m_previousVisualization = m_visualizationsFactory.GetVisualizationConfig(m_previousVisualizationEntry).getVisualizationInstancePtr(RANDOM_TIME, m_statisticalEngine, this);
  }
  else
  {
    m_currentVisualizationEntry = m_visualizationsFactory.GetVisualizationConfigForTypeAtIndex(visualizationType, random(0, m_visualizationsFactory.GetNumberOfVisualizationType(visualizationType))).visualizationEntry;
    if(true == debugMode && debugLevel >= 2) Serial << "Getting Random Visualization: " << m_currentVisualizationEntry << "\n";
    m_currentVisualization = m_visualizationsFactory.GetVisualizationConfig(m_currentVisualizationEntry).getVisualizationInstancePtr(RANDOM_TIME, m_statisticalEngine, this);
  }
  PrintFreeMemory("Free Memory After: ");
}

void ActiveVisualizationController::Loop() 
{
  if(true == debugMode && debugLevel >= 3) Serial << "ActiveVisualizationController::Loop: \tAutomatic Mode: " << m_automaticMode << "    Gain Adjust Mode: " << m_gainAdjustModeActive << "    Silent Mode: " << m_silentModeActive << "    Test Mode: " << m_statisticalEngine.GetTestMode() << "\n";
  ProcessVisualizations();
  ProcessButtons();
}

void ActiveVisualizationController::ProcessVisualizations()
{
  bool update = false;
  if(m_activeTransition != NULL) update = m_activeTransition->Loop();
  for(int i = 0; i < NUMSTRIPS; ++i)
  {
    if(m_activeTransition != NULL) m_strips[i] = ( m_activeTransition->GetStrips())[i]; 
  }
  if(true == update)
  {
    FastLED.show();
  }
}
void ActiveVisualizationController::ProcessButtons()
{
  bool buttonIsPressed = (0 == digitalRead(2));
  if(true == buttonIsPressed)
  {
    unsigned long currentTime = millis();
    if(false == m_silentModeActive)
    {
      if(false == m_buttonIsPressed)
      {
        if(true == debugMode) Serial << "Button Pressed\n";
        m_buttonIsPressed = true;
        m_HoldTimeStart = currentTime; 
      }
      else
      {
        if(currentTime - m_HoldTimeStart >= 10000 && false == m_held10000)
        {
          m_held10000 = true;
          if(true == debugMode) Serial << "Button Held for >= 10000\n";
          if(true == m_statisticalEngine.GetTestMode())
          {
            m_statisticalEngine.DisableTestMode();
            AddSceneConfigToQueue(VisualizationEntries::VisualizationEntries_InstantSwitch, VisualizationEntries::VisualizationEntries_Confirmation, 2000, CRGB::Purple);
            GetNextTransition();
          }
          else
          {
            m_statisticalEngine.EnableTestMode();
            AddSceneConfigToQueue(VisualizationEntries::VisualizationEntries_InstantSwitch, VisualizationEntries::VisualizationEntries_Confirmation, 2000, CRGB::Purple);
            GetNextTransition();
          }
        }
        else if(currentTime - m_HoldTimeStart >= 5000 && false == m_held5000)
        {
          m_held5000 = true;
          if(true == debugMode) Serial << "Button Held for >= 5000\n";
          if(true == m_gainAdjustModeActive)
          {
            m_gainAdjustModeActive = false;
            m_automaticMode = true;
            AddSceneConfigToQueue(VisualizationEntries::VisualizationEntries_InstantSwitch, VisualizationEntries::VisualizationEntries_Confirmation, 2000, CRGB::Purple);
            GetNextTransition();
          }
          else
          {
            m_gainAdjustModeActive = true;
            m_automaticMode = false;
            AddSceneConfigToQueue(VisualizationEntries::VisualizationEntries_InstantSwitch, VisualizationEntries::VisualizationEntries_Confirmation, 2000, CRGB::Purple);
            AddSceneConfigToQueue(VisualizationEntries::VisualizationEntries_InstantSwitch, VisualizationEntries::VisualizationEntries_SoundDetectionTester, 60000);
            GetNextTransition();
          }
        }
        else if(currentTime - m_HoldTimeStart >= 2000 && false == m_held2000)
        {
          m_held2000 = true;
          m_automaticMode = true;
          if(true == debugMode) Serial << "Button Held for >= 2000.  Automatic Mode: " << m_automaticMode << "\n";
          if(true == debugMode) Serial << "Entering Automatic Mode:  Automatic Mode: " << m_automaticMode << "\n";
          AddSceneConfigToQueue(VisualizationEntries::VisualizationEntries_InstantSwitch, VisualizationEntries::VisualizationEntries_Confirmation, 2000, CRGB::Green);
          GetNextTransition();
        }
      }
    }
    else
    {
      if(false == m_buttonIsPressed)
      {
        if(true == debugMode) Serial << "Button Pressed\n";
        m_buttonIsPressed = true;
        m_HoldTimeStart = currentTime; 
      }
    }
  }
  if(false == buttonIsPressed && true == m_buttonIsPressed)
  {
    m_buttonIsPressed = false;
    if(true == debugMode) Serial << "Button Released\n";
    if(true == m_silentModeActive)
    {
      Illuminate(100);
    }
    else
    {
      if(true == m_held10000)
      {
        if(true == debugMode) Serial << "Released Button Held for >= 10000\n";
      }
      if(true == m_held5000)
      {
        if(true == debugMode) Serial << "Released Button Held for >= 5000\n";
      }
      if(true == m_held2000)
      {
        if(true == debugMode) Serial << "Released Button Held for >= 2000\n";
      }
      else
      {
        m_automaticMode = false;
        if(true == debugMode) Serial << "Release Button Held for < 2000.  Automatic Mode: " << m_automaticMode << "\n";
        if(false == m_automaticMode && true == m_automaticModeOld)
        {
          if(true == debugMode) Serial << "Entering Manual Mode:  Automatic Mode: " << m_automaticMode << "\n";
          AddSceneConfigToQueue(VisualizationEntries::VisualizationEntries_InstantSwitch, VisualizationEntries::VisualizationEntries_Confirmation, 2000, CRGB::Yellow);
          GetNextTransition();
        }
        else
        {
          AddSceneConfigToQueue(VisualizationEntries::VisualizationEntries_InstantSwitch, VisualizationEntries::VisualizationEntries_GetNext, RANDOM_TIME);
          GetNextTransition();
        }
      }
      m_held10000 = false;
      m_held5000 = false;
      m_held2000 = false;
    }
  }
  m_automaticModeOld = m_automaticMode;
}

void ActiveVisualizationController::Illuminate(unsigned int level)
{
  if(true == m_silentModeActive)
  {
    TurnOnLEDs(level);
    m_silentModeActive = false;   
  }
}

void ActiveVisualizationController::Deluminate()
{
  if(false == m_silentModeActive)
  {
    TurnOffLEDs();
    m_silentModeActive = true;
  }
}
