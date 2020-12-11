#include "Statistical_Engine.h"
#include "Visualizations.h"
#include "LEDControllerInterface.h"

class VisualizationController: public TaskInterface
                             , MicrophoneMeasureCalleeInterface
                             , InterruptHandler
{
  public:
    VisualizationController(){}
    StatisticalEngine m_StatisticalEngine;
    
    void Setup()
    {
      m_StatisticalEngine.Setup();
      m_StatisticalEngine.ConnectCallback(this);
    }
    bool Loop()
    {
      m_StatisticalEngine.UpdateSoundData();
    }

    //MicrophoneMeasureCalleeInterface
    void MicrophoneStateChange(SoundState){}
    void TestSequenceComplete(){}
    void HandleInterrupt() 
    {
      m_StatisticalEngine.HandleInterrupt();
    }
  private:
    Visualization CreateNewVisualization() {};
    BandAmplitudes fFTAmplitudes;

};
