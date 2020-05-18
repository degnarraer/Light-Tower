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
 * @file ActiveVisualizationController.h
 *
 *
 */

#ifndef ActiveVisualizationController_H
#define ActiveVisualizationController_H
#include "LEDControllerInterface.h"
#include "VisualizationFactory.h"
#include "Statistical_Engine.h"

#define RANDOM_TIME random(10000, 120000)

struct SceneConfig
{
  VisualizationEntries transitionEntry;
  VisualizationEntries visualizationEntry;
  unsigned long duration;
  CRGB confirmationColor;
};
    
class ActiveVisualizationController: public LEDController
                                   , public MicrophoneMeasureCalleeInterface
                                   , public VisualizationsCalleeInterface
{
  public:
    ActiveVisualizationController()
    {
      //if(true == debugMode && debugLevel >= 0) Serial << "ActiveVisualizationController: Instantiate\n";
    }

    //Visualizations
    void HandleInterrupt();
    void Setup();
    void Loop();
    
    //MicrophoneMeasureCallbackInterface
    void MicrophoneStateChange(SoundState state);
    void TestSequenceComplete();
    
    //VisualizationsCalleeInterface
    Visualizations *GetCurrentVisualizationPtr();
    Visualizations *GetPreviousVisualizationPtr();
    CRGB GetConfirmationColor();
    VisualizationsFactory m_visualizationsFactory;
    void VisualizationStarted(Visualizations *visualization);
    void VisualizationEnded(Visualizations *visualization);
    void TransitionStarted(Visualizations *visualization);
    void TransitionEnded(Visualizations *visualization);
    void ConfirmationVisualizationStarted(Visualizations *visualization);
    void ConfirmationVisualizationEnded(Visualizations *visualization);
  
  private:  
    StatisticalEngine m_statisticalEngine;
    //Modes
    bool m_silentModeActive = false;
    bool m_gainAdjustModeActive = true;
    bool m_1stSoundDetected = false;
    bool m_automaticMode = true;
    bool m_automaticModeOld = true;

    //SceneConfig Queue
    SceneConfig m_sceneConfigQueue[60];
    SceneConfig m_activeSceneConfig;
    int m_sceneConfigQueueHeadIndex = 0;
    int m_sceneConfigQueueTailIndex = 0;
    void AddSceneConfigToQueue(VisualizationEntries transition, VisualizationEntries visualization, unsigned long duration, CRGB confirmationColor = CRGB::Black);
    SceneConfig GetNextSceneConfigFromQueue();

    //Visualizations
    VisualizationEntries m_previousVisualizationEntry;
    VisualizationEntries m_currentVisualizationEntry;
    VisualizationEntries m_previousTransitionEntry;
    VisualizationEntries m_currentTransitionEntry;
    Visualizations *m_currentVisualization = NULL;
    Visualizations *m_previousVisualization = NULL;
    Transitions *m_activeTransition = NULL;
    Transitions *m_garbageTransition = NULL;
    
    unsigned long m_HoldTimeStart = 0;
    bool m_buttonIsPressed = false;
    bool m_held2000 = false;
    bool m_held5000 = false;
    bool m_held10000 = false;
    void TrashPreviousVisualization();
    void StoreActiveVisualization();
    void EmptyVisualizationTrash();
    void StartVisualization();
    void TrashTransition();
    void EmptyTransitionTrash();
    void PrintFreeMemory(String text);
    void GetNextTransition();
    void GetNextVisualizer(bool usingSceneConfig);
    void ProcessVisualizations();
    void ProcessButtons();
    void ProcessSilence();
    void Illuminate();
    void Deluminate();
};
#endif
