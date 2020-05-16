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
 * @file Visualization.cpp
 *
 *
 */

#include "Visualization.h"


//********* TRANSITION 1 **********
Transitions* InstantSwitch::GetInstance(  StatisticalEngine &statisticalEngine
                                        , VisualizationsCalleeInterface *callee )
{
  if(true == debugMode && debugLevel >= 2) Serial << "New InstantSwitch\n";
  InstantSwitch *newTransition = new InstantSwitch(statisticalEngine, callee);
  newTransition->SetCurrentVisualization(callee->GetCurrentVisualizationPtr());
  newTransition->SetPreviousVisualization(callee->GetPreviousVisualizationPtr());
  return newTransition;
}

void InstantSwitch::Start()
{
  Started();
}

bool InstantSwitch::Loop()
{
  if(false == m_visualizationStarted) {Start(); m_visualizationStarted = true;}
  if(true == debugMode && debugLevel >= 3) Serial << "InstantSwitch Loop\n";
  bool updateRequired = false;
  unsigned long currentTime = millis();
  if(currentTime-m_timers[0] >= 0) {ResetTimer(0); updateRequired |= Tick1();}
  return updateRequired;
}

bool InstantSwitch::Tick1()
{
  m_statisticalEngine.UpdateSoundData();
  bool updateRequired = false;
  updateRequired |= mp_currentVisualization->Loop();
  for(int i = 0; i < NUMSTRIPS; ++i)
  { 
    m_strips[i] = mp_currentVisualization->GetStrips()[i];
  }
  return updateRequired;
}

void InstantSwitch::End()
{
  Ended();
}


//********* TRANSITION 1 **********

Transitions* FadeTransition::GetInstance( StatisticalEngine &statisticalEngine
                                        , VisualizationsCalleeInterface *callee )
{
  if(true == debugMode && debugLevel >= 2) Serial << "New FadeTransition\n";
  FadeTransition *newTransition = new FadeTransition(statisticalEngine, callee);
  newTransition->SetCurrentVisualization(callee->GetCurrentVisualizationPtr());
  newTransition->SetPreviousVisualization(callee->GetPreviousVisualizationPtr());
  return newTransition;
}

void FadeTransition::Start()
{
  m_fadeCount = 0;
  Started();
}

bool FadeTransition::Loop()
{
  if(false == m_visualizationStarted) {Start(); m_visualizationStarted = true;}
  if(true == debugMode && debugLevel >= 3) Serial << "FadeTransition Loop\n";
  bool updateRequired = false;
  unsigned long currentTime = millis();
  if(currentTime-m_timers[0] >= 0) {ResetTimer(0); updateRequired |= Tick1();}
  if(currentTime-m_timers[1] >= 10) {ResetTimer(1); Tick2(); updateRequired = true;}
  return updateRequired;
}

bool FadeTransition::Tick1()
{
  m_statisticalEngine.UpdateSoundData();
  bool updateRequired = false;
  if(mp_currentVisualization != NULL && mp_previousVisualization != NULL) 
  {
    updateRequired |= mp_currentVisualization->Loop();
    updateRequired |= mp_previousVisualization->Loop(); 
    for(int i = 0; i < NUMSTRIPS; ++i)
    {
      m_strips[i] = AddLayer(FadeOutLayerBy(mp_previousVisualization->GetStrips()[i], m_fadeCount), FadeInLayerBy(mp_currentVisualization->GetStrips()[i], m_fadeCount));
    }
  }
  else if(mp_currentVisualization != NULL)
  {   
    updateRequired |= mp_currentVisualization->Loop();
    for(int i = 0; i < NUMSTRIPS; ++i)
    { 
      m_strips[i] = FadeInLayerBy(mp_currentVisualization->GetStrips()[i], m_fadeCount);
    }
  }
  return updateRequired;
}

void FadeTransition::Tick2()
{
  if(m_fadeCount<255)
  {
    m_fadeCount+=2;
    if(m_fadeCount > 255)
    {
      m_fadeCount = 255;
    }
    //if(true == debugMode && debugLevel >= 1) Serial << "Fade Count: " << m_fadeCount << "\n";
  }
}

void FadeTransition::End()
{
  Ended();
}


//********* TRANSITION 2 **********

Transitions* MixerTransition::GetInstance( StatisticalEngine &statisticalEngine
                                         , VisualizationsCalleeInterface *callee )
{
  if(true == debugMode && debugLevel >= 2) Serial << "New MixerTransition\n";
  MixerTransition *newTransition = new MixerTransition(statisticalEngine, callee);
  newTransition->SetCurrentVisualization(callee->GetCurrentVisualizationPtr());
  newTransition->SetPreviousVisualization(callee->GetPreviousVisualizationPtr());
  return newTransition;
}  

void MixerTransition::Start()
{
  m_blendMode = random(2);
  Started();
}
bool MixerTransition::Loop()
{
  if(false == m_visualizationStarted) {Start(); m_visualizationStarted = true;}
  if(true == debugMode && debugLevel >= 3) Serial << "MixerTransition Loop\n";
  bool updateRequired = false;
  unsigned long currentTime = millis();
  if(currentTime-m_timers[0] >= 0) {ResetTimer(0); updateRequired |= Tick1();}
  return updateRequired;
}

bool MixerTransition::Tick1()
{
  m_statisticalEngine.UpdateSoundData();
  bool updateRequired = false;
  if(mp_currentVisualization != NULL && mp_previousVisualization != NULL) 
  {
    updateRequired |= mp_currentVisualization->Loop();
    updateRequired |= mp_previousVisualization->Loop(); 
    for(int i = 0; i < NUMSTRIPS; ++i)
    {
      switch(m_blendMode)
      {
        case 0:
          m_strips[i] = AddLayer(mp_currentVisualization->GetStrips()[i], mp_previousVisualization->GetStrips()[i]);
        break;
        case 1:
          m_strips[i] = MergeLayer(mp_currentVisualization->GetStrips()[i], mp_previousVisualization->GetStrips()[i]);
        break;
        default:
          m_strips[i] = MergeLayer(mp_currentVisualization->GetStrips()[i], mp_previousVisualization->GetStrips()[i]);
        break;
      }
    }
  }
  else if(mp_currentVisualization != NULL)
  {   
    updateRequired |= mp_currentVisualization->Loop();
    for(int i = 0; i < NUMSTRIPS; ++i)
    {
      m_strips[i] = mp_currentVisualization->GetStrips()[i];
    }
  }
  return updateRequired;
}

void MixerTransition::End()
{
  Ended();
}



//********* TRANSITION 3 **********

Transitions* SlideUpTransition::GetInstance( StatisticalEngine &statisticalEngine
                                           , VisualizationsCalleeInterface *callee )
{
  if(true == debugMode && debugLevel >= 2) Serial << "New SlideUpTransition\n";
  SlideUpTransition *newTransition = new SlideUpTransition(statisticalEngine, callee);
  newTransition->SetCurrentVisualization(callee->GetCurrentVisualizationPtr());
  newTransition->SetPreviousVisualization(callee->GetPreviousVisualizationPtr());
  return newTransition;
}

void SlideUpTransition::Start()
{
  m_fadeCount = 0;
  Started();
}

bool SlideUpTransition::Loop()
{
  if(false == m_visualizationStarted) {Start(); m_visualizationStarted = true;}
  if(true == debugMode && debugLevel >= 3) Serial << "SlideUpTransition Loop\n";
  bool updateRequired = false;
  unsigned long currentTime = millis();
  if(currentTime-m_timers[0] >= 0) {ResetTimer(0); updateRequired |= Tick1();}
  if(currentTime-m_timers[1] >= 10) {ResetTimer(1); Tick2(); updateRequired = true;}
  return updateRequired;
}

bool SlideUpTransition::Tick1()
{
  m_statisticalEngine.UpdateSoundData();
  bool updateRequired = false;
  if(mp_currentVisualization != NULL && mp_previousVisualization != NULL) 
  {
    updateRequired |= mp_currentVisualization->Loop();
    updateRequired |= mp_previousVisualization->Loop(); 
    for(int i = 0; i < NUMSTRIPS; ++i)
    {
      for(int j = 0; j < NUMLEDS; ++j)
      { 
        if(j > m_fadeCount - 1)
        {
          m_strips[i][j] = mp_previousVisualization->GetStrips()[i][j - m_fadeCount];
        }
        else if(m_fadeCount - 1 == j)
        {
          m_strips[i][m_fadeCount - 1] = CRGB::Green;
        }
        else
        {
          m_strips[i][j] = mp_currentVisualization->GetStrips()[i][((NUMLEDS-1) - m_fadeCount + 2) + j];
        }
      }
    }
  }
  else if(mp_currentVisualization != NULL)
  {   
    updateRequired |= mp_currentVisualization->Loop();
    for(int i = 0; i < NUMSTRIPS; ++i)
    {
      m_strips[i] = mp_currentVisualization->GetStrips()[i];
    }
  }
  return updateRequired;
}

void SlideUpTransition::Tick2()
{
  if(m_fadeCount <= NUMLEDS)
  {
    ++m_fadeCount;
    if(true == debugMode && debugLevel >= 3) Serial << "Fade Count: " << m_fadeCount << "\n";
  }
}

void SlideUpTransition::End()
{
  Ended();
}


//********* TRANSITION 4 **********

 Transitions* SlideDownTransition::GetInstance( StatisticalEngine &statisticalEngine
                                              , VisualizationsCalleeInterface *callee )
{
  if(true == debugMode && debugLevel >= 2) Serial << "New SlideDownTransition\n";
  SlideDownTransition *newTransition = new SlideDownTransition(statisticalEngine, callee);
  newTransition->SetCurrentVisualization(callee->GetCurrentVisualizationPtr());
  newTransition->SetPreviousVisualization(callee->GetPreviousVisualizationPtr());
  return newTransition;
} 

void SlideDownTransition::Start()
{
  m_fadeCount = 0;
  Started();
}

bool SlideDownTransition::Loop()
{
  if(false == m_visualizationStarted) {Start(); m_visualizationStarted = true;}
  if(true == debugMode && debugLevel >= 3) Serial << "SlideDownTransition Loop\n";
  bool updateRequired = false;
  unsigned long currentTime = millis();
  if(currentTime-m_timers[0] >= 0) {ResetTimer(0); updateRequired |= Tick1();}
  if(currentTime-m_timers[1] >= 10) {ResetTimer(1); Tick2(); updateRequired = true;}
  return updateRequired;
}

bool SlideDownTransition::Tick1()
{
  m_statisticalEngine.UpdateSoundData();
  bool updateRequired = false;
  if(mp_currentVisualization != NULL && mp_previousVisualization != NULL) 
  {
    updateRequired |= mp_currentVisualization->Loop();
    updateRequired |= mp_previousVisualization->Loop(); 
    for(int i = 0; i < NUMSTRIPS; ++i)
    {
      for(int j = 0; j < NUMLEDS; ++j)
      { 
        int fadeCount = (NUMLEDS - m_fadeCount);
        if(j < fadeCount)
        {
            m_strips[i][j] = mp_previousVisualization->GetStrips()[i][j + (m_fadeCount)];
        }
        else if(fadeCount == j)
        {
          m_strips[i][fadeCount] = CRGB::Green;
        }
        else
        {
          m_strips[i][j] = mp_currentVisualization->GetStrips()[i][j - (NUMLEDS - (m_fadeCount-1))];
        }
      }
    }
  }
  else if(mp_currentVisualization != NULL)
  {   
    updateRequired |= mp_currentVisualization->Loop();
    for(int i = 0; i < NUMSTRIPS; ++i)
    {
      m_strips[i] = mp_currentVisualization->GetStrips()[i];
    }
  }
  return updateRequired;
}

void SlideDownTransition::Tick2()
{
  if(m_fadeCount <= NUMLEDS)
  {
    ++m_fadeCount;
    if(true == debugMode && debugLevel >= 3) Serial << "Fade Count: " << m_fadeCount << "\n";
  }
}

void SlideDownTransition::End()
{
  Ended();
}



//********* TRANSITION 5 **********
 Transitions* SplitTransition::GetInstance( StatisticalEngine &statisticalEngine
                                          , VisualizationsCalleeInterface *callee )
{
  if(true == debugMode && debugLevel >= 2) Serial << "New SplitTransition\n";
  SplitTransition *newTransition = new SplitTransition(statisticalEngine, callee);
  newTransition->SetCurrentVisualization(callee->GetCurrentVisualizationPtr());
  newTransition->SetPreviousVisualization(callee->GetPreviousVisualizationPtr());
  return newTransition;
}

void SplitTransition::Start()
{
  m_fadeCount = 0;
  Started();
}

bool SplitTransition::Loop()
{
  if(false == m_visualizationStarted) {Start(); m_visualizationStarted = true;}
  if(true == debugMode && debugLevel >= 3) Serial << "SplitTransition Loop\n";
  bool updateRequired = false;
  unsigned long currentTime = millis();
  if(currentTime-m_timers[0] >= 0) {ResetTimer(0); updateRequired |= Tick1();}
  if(currentTime-m_timers[1] >= 10) {ResetTimer(1); Tick2(); updateRequired = true;}
  return updateRequired;
}

bool SplitTransition::Tick1()
{
  m_statisticalEngine.UpdateSoundData();
  bool updateRequired = false;
  if(mp_currentVisualization != NULL && mp_previousVisualization != NULL) 
  {
    updateRequired |= mp_currentVisualization->Loop();
    updateRequired |= mp_previousVisualization->Loop(); 
    for(int i = 0; i < NUMSTRIPS; ++i)
    {
      for(int j = 0; j < NUMLEDS; ++j)
      {
        if(0==i%2)
        {
          int fadeCount = m_fadeCount - 1;
          if(j > fadeCount)
          {
            m_strips[i][j] = mp_previousVisualization->GetStrips()[i][j - m_fadeCount];
          }
          else if(fadeCount == j)
          {
            m_strips[i][fadeCount] = CRGB::Green;
          }
          else
          {
            m_strips[i][j] = mp_currentVisualization->GetStrips()[i][((NUMLEDS-1) - m_fadeCount + 2) + j];
          }
        }
        else
        {
          int fadeCount = (NUMLEDS - m_fadeCount);
          if(j < fadeCount)
          {
              m_strips[i][j] = mp_previousVisualization->GetStrips()[i][j + (m_fadeCount)];
          }
          else if(fadeCount == j)
          {
            m_strips[i][fadeCount] = CRGB::Green;
          }
          else
          {
            m_strips[i][j] = mp_currentVisualization->GetStrips()[i][j - (NUMLEDS - (m_fadeCount-1))];
          }
        }
      }
    }
  }
  else if(mp_currentVisualization != NULL)
  {   
    updateRequired |= mp_currentVisualization->Loop();
    for(int i = 0; i < NUMSTRIPS; ++i)
    {
      m_strips[i] = mp_currentVisualization->GetStrips()[i];
    }
  }
  return updateRequired;
}

void SplitTransition::Tick2()
{
  if(m_fadeCount <= NUMLEDS)
  {
    ++m_fadeCount;
    if(true == debugMode && debugLevel >= 3) Serial << "Fade Count: " << m_fadeCount << "\n";
  }
}

void SplitTransition::End()
{
  Ended();
}


//********* SoundDetectionTester *********
 Visualizations* SoundDetectionTester::GetInstance( int duration
                                                  , StatisticalEngine &statisticalEngine
                                                  , VisualizationsCalleeInterface *callee )
{
  if(true == debugMode && debugLevel >= 2) Serial << "New SoundDetectionTester\n";
  if(true == debugMode && debugLevel >= 2) Serial << "Duration: " << duration << "\n";
  return new SoundDetectionTester(duration, statisticalEngine, callee);
} 
void SoundDetectionTester::Start()
{
  if(true == debugMode && debugLevel >= 1) Serial << "SoundDetectionTester Start\n";
  Started();
}
bool SoundDetectionTester::Loop() 
{
  if(false == m_visualizationStarted) {Start(); m_visualizationStarted = true;}
  bool updateRequired = false;
  unsigned long currentTime = millis();
  if(true == debugMode && debugLevel >= 3) Serial << "SoundDetectionTester Loop: " << currentTime-m_resetTimer << " | " << m_duration << "\n";
  if(currentTime-m_timers[0] >= 0) {ResetTimer(0); Tick1(); updateRequired = true;}
  return updateRequired;
}
void SoundDetectionTester::Tick1()
{
  m_layer[0] = CRGB::Black;
  CRGB spriteColor;
  if(m_statisticalEngine.power >= SOUND_DETECT_THRESHOLD)
  {
    spriteColor = CRGB::Blue;
  }
  else
  {
    spriteColor = CRGB::Yellow;
  }
  m_layer[0][((float )m_statisticalEngine.m_silenceDetectedThreshold/(float )m_statisticalEngine.m_silenceIntegratorMax) * (NUMLEDS-1)] = CRGB::Red;
  m_layer[0][((float )m_statisticalEngine.m_silenceIntegrator/(float )m_statisticalEngine.m_silenceIntegratorMax) * (NUMLEDS-1)] = spriteColor;
  m_layer[0][((float )m_statisticalEngine.m_soundDetectedThreshold/(float )m_statisticalEngine.m_silenceIntegratorMax) * (NUMLEDS-1)] = CRGB::Green;
  for(int i=0; i<NUMSTRIPS; ++i)
  {
     m_strips[i] = m_layer[0];
  }
}
void SoundDetectionTester::End()
{
  Ended();
}

//********* ColorFadingTower *********
 Visualizations* ColorFadingTower::GetInstance( int duration
                                              , StatisticalEngine &statisticalEngine
                                              , VisualizationsCalleeInterface *callee )
{
  if(true == debugMode && debugLevel >= 2) Serial << "New ColorFadingTower\n";
  if(true == debugMode && debugLevel >= 2) Serial << "Duration: " << duration << "\n";
  return new ColorFadingTower(duration, statisticalEngine, callee);
}
void ColorFadingTower::Start()
{
  if(true == debugMode && debugLevel >= 1) Serial << "ColorFadingTower Start\n";
  Tick2();
  for(int i=0; i<NUMSTRIPS; ++i)
  {
     m_strips[i] = m_color;
  }
  Started();
}
bool ColorFadingTower::Loop() 
{
  if(false == m_visualizationStarted) {Start(); m_visualizationStarted = true;}
  bool updateRequired = false;
  unsigned long currentTime = millis();
  if(true == debugMode && debugLevel >= 4) Serial << "ColorFadingTower Loop: " << currentTime-m_resetTimer << " | " << m_duration << "\n";
  if(currentTime-m_timers[0] >= 0) {ResetTimer(0); Tick1(); updateRequired = true;}
  if(currentTime-m_timers[1] >= m_randomTime) {ResetTimer(1); Tick2(); updateRequired = true;}
  if(currentTime-m_resetTimer >= m_duration) {End(); updateRequired = true;}
  return updateRequired;
}
void ColorFadingTower::Tick1()
{
  m_color = FadeColor(m_fadeController);
  for(int i=0; i<NUMSTRIPS; ++i)
  {
     m_strips[i] = m_color;
  }
}
void ColorFadingTower::Tick2()
{
  m_randomTime = random(1000, m_maxRandomTime);
  m_fadeToColor = GetRandomNonGrayColor();
  SetFadeToColor(m_fadeController, m_color, m_fadeToColor, m_fadeLength);
}
void ColorFadingTower::End()
{
  Ended();
}

//********* Confirmation *********
Visualizations* Confirmation::GetInstance( int duration
                                         , StatisticalEngine &statisticalEngine
                                         , VisualizationsCalleeInterface *callee )
{
  if(true == debugMode && debugLevel >= 2) Serial << "New Confirmation\n";
  Confirmation *newVisualization = new Confirmation(duration, statisticalEngine, callee);
  newVisualization->SetConfirmationColor(callee->GetConfirmationColor());
  return newVisualization;
}
void Confirmation::Start()
{
  if(true == debugMode && debugLevel >= 1) Serial << "Confirmation Start\n";
  Started();
}
bool Confirmation::Loop()
{
  if(false == m_visualizationStarted) {Start(); m_visualizationStarted = true;}
  bool updateRequired = false;
  unsigned long currentTime = millis();
  if(true == debugMode && debugLevel >= 3) Serial << "Confirmation Loop: " << currentTime-m_resetTimer << " | " << m_duration << "\n";
  if(currentTime - m_timers[0] >= 100) {ResetTimer(0); Tick1(); updateRequired = true;}
  if(m_count % 10 == 0) {End(); updateRequired = true;}
  return updateRequired;
}
void Confirmation::Tick1()
{
  for(int i = 0; i < NUMSTRIPS; ++i)
  {
    if(m_count%2==0)
    {
      m_strips[i] = m_confirmationColor; 
    }
    else
    {
      m_strips[i] = CRGB::Black;
    }
  }
  ++m_count;
}
void Confirmation::End()
{
  Ended();
}

//********* WaterFallFireStreamer *********
 Visualizations* WaterFallFireStreamer::GetInstance( int duration
                                                         , StatisticalEngine &statisticalEngine
                                                         , VisualizationsCalleeInterface *callee )
{
  if(true == debugMode && debugLevel >= 2) Serial << "New WaterFallFireStreamer\n";
  return new WaterFallFireStreamer(duration, statisticalEngine, callee);
}
void WaterFallFireStreamer::Start()
{
  if(true == debugMode && debugLevel >= 1) Serial << "WaterFallFireStreamer Start\n";
  m_layer[0] = m_layer[1] = m_layer[2] = m_layer[3] = CRGB::Black;
  m_layer[4] = m_layer[5] = m_layer[6] = m_layer[7] = CRGB::Black;
  Randomize();
  Started();
}
bool WaterFallFireStreamer::Loop()
{
  if(false == m_visualizationStarted) {Start(); m_visualizationStarted = true;}  
  bool updateRequired = false;
  unsigned long currentTime = millis();
  if(true == debugMode && debugLevel >= 3) Serial << "WaterFallFireStreamer Loop: " << currentTime-m_resetTimer << " | " << m_duration << "\n";
  if(currentTime-m_timers[0] >= 0) {ResetTimer(0); Tick1(); updateRequired = true;}
  if(currentTime-m_timers[1] >= m_randomTime) {ResetTimer(1); Randomize(); updateRequired = true;}
  if(currentTime-m_resetTimer >= m_duration) {End(); updateRequired = true;}
  return updateRequired;
}
void WaterFallFireStreamer::Tick1()
{
  m_BottomColor = FadeColor(m_bottomFadeController);
  m_TopColor = FadeColor(m_topFadeController);
  m_BottomMiddleColor = FadeColor(m_bottomMiddleFadeController);
  m_TopMiddleColor = FadeColor(m_topMiddleFadeController);
  
  ShiftStrip(m_layer[0], Up, 1);
  ShiftStrip(m_layer[1], Down, 1);
  ShiftStrip(m_layer[2], Down, 1);
  ShiftStrip(m_layer[3], Up, 1);
  
  m_layer[0](0, 0) = CRGB((byte)(dim8_raw(m_BottomColor.red * m_statisticalEngine.powerDb)), (byte)(dim8_raw(m_BottomColor.green * m_statisticalEngine.powerDb)), (byte)(dim8_raw(m_BottomColor.blue * m_statisticalEngine.powerDb)));
  m_layer[1](NUMLEDS-1, NUMLEDS-1) = CRGB((byte)(dim8_raw(m_TopColor.red * m_statisticalEngine.powerDb)), (byte)(dim8_raw(m_TopColor.green * m_statisticalEngine.powerDb)), (byte)(dim8_raw(m_TopColor.blue * m_statisticalEngine.powerDb)));
  
  m_layer[2]((NUMLEDS/2-1), (NUMLEDS/2-1)) = CRGB((byte)(dim8_raw(m_BottomMiddleColor.red * m_statisticalEngine.powerDb)), (byte)(dim8_raw(m_BottomMiddleColor.green * m_statisticalEngine.powerDb)), (byte)(dim8_raw(m_BottomMiddleColor.blue * m_statisticalEngine.powerDb)));
  m_layer[3](NUMLEDS/2, NUMLEDS/2) = CRGB((byte)(dim8_raw(m_TopMiddleColor.red * m_statisticalEngine.powerDb)), (byte)(dim8_raw(m_TopMiddleColor.green * m_statisticalEngine.powerDb)), (byte)(dim8_raw(m_TopMiddleColor.blue * m_statisticalEngine.powerDb)));
  
  m_layer[4] = m_layer[0];
  m_layer[5] = m_layer[1];
  m_layer[6] = m_layer[2];
  m_layer[7] = m_layer[3];

  for(int i = 0; i < NUMLEDS; ++i)
  {
    m_layer[4][i].subtractFromRGB(i*2);
    m_layer[5][NUMLEDS-i-1].subtractFromRGB(i*2);
  }
  for(int i = 0; i < NUMLEDS/2; ++i)
  {
    m_layer[6][((NUMLEDS/2)-1)-i].subtractFromRGB(i*0.5);
    m_layer[7][(NUMLEDS/2)+i].subtractFromRGB(i*0.5);
  }
  for(int i = 0; i < NUMSTRIPS; ++i)
  {    
    switch(m_streamerType)
    {
      case StreamerType_Outward_In:
        m_strips[i] = AddLayer(m_layer[4], m_layer[5]);
      break;
      case StreamerType_Inward_Out:
        m_strips[i] = AddLayer(m_layer[6], m_layer[7]);
      break;
      case StreamerType_Both:
        m_strips[i] = AddLayer(m_layer[4], m_layer[5]);
        m_strips[i] = AddLayer(m_strips[i], m_layer[6]);
        m_strips[i] = AddLayer(m_strips[i], m_layer[7]);
      break;
      default:
      break;
    }
  }
}

void WaterFallFireStreamer::Randomize()
{
  m_randomTime = random(10001);
  m_BottomFadeToColor = GetRandomNonGrayColor();
  m_TopFadeToColor = GetRandomNonGrayColor();
  m_BottomMiddleFadeToColor = GetRandomNonGrayColor();
  m_TopMiddleFadeToColor = GetRandomNonGrayColor();
  SetFadeToColor(m_bottomFadeController, m_BottomColor, m_BottomFadeToColor, m_fadeLength);
  SetFadeToColor(m_topFadeController, m_TopColor, m_TopFadeToColor, m_fadeLength);
  SetFadeToColor(m_bottomMiddleFadeController, m_BottomMiddleColor, m_BottomMiddleFadeToColor, m_fadeLength);
  SetFadeToColor(m_topMiddleFadeController, m_TopMiddleColor, m_TopMiddleFadeToColor, m_fadeLength);
}
void WaterFallFireStreamer::End()
{
  Ended();
}

//********* SolidColorTower *********
 Visualizations* SolidColorTower::GetInstance( int duration
                                             , StatisticalEngine &statisticalEngine
                                             , VisualizationsCalleeInterface *callee )
{
  if(true == debugMode && debugLevel >= 2) Serial << "New SolidColorTower\n";
  return new SolidColorTower(duration, statisticalEngine, callee);
}
void SolidColorTower::Start()
{
  if(true == debugMode && debugLevel >= 1) Serial << "SolidColorTower Start\n";
  Started();
}
bool SolidColorTower::Loop() 
{
  if(false == m_visualizationStarted) {Start(); m_visualizationStarted = true;}
  bool updateRequired = false;
  unsigned long currentTime = millis();
  if(true == debugMode && debugLevel >= 3) Serial << "SolidColorTower Loop: " << currentTime-m_resetTimer << " | " << m_duration << "\n";
  if(currentTime-m_timers[0] >= 100) {ResetTimer(0); Tick1(); updateRequired = true;}
  if(currentTime-m_resetTimer >= m_duration) {End(); updateRequired = true;}
  return updateRequired;
}
void SolidColorTower::Tick1()
{
  int maxBinSave = 0;
  float  maxLevelSave = 0.0;
  if(true == debugMode && debugLevel >= 4) Serial << "SolidColorTower Tick1: \t";
  for(int i = 0; i < BINS; ++i)
  {
    float  level = GetNormalizedSoundLevelForBin(i, 3, SoundLevelOutputType_Beat);
    if(level > maxLevelSave)
    {
      maxLevelSave = level;
      maxBinSave = i;
    }
    if(true == debugMode && debugLevel >= 4) Serial << i << "|" << level << "\t";
  }
  if(true == debugMode && debugLevel >= 4) Serial << "\n";
  CRGB color = GetColor(maxBinSave, m_statisticalEngine.GetFFTBinIndexForFrequency(MAX_DISPLAYED_FREQ));
  color.red = color.red * maxLevelSave;
  color.blue = color.blue * maxLevelSave;
  color.green = color.green * maxLevelSave;
  m_layer[0] = color;
  if (m_statisticalEngine.power >= SILENCE_THRESHOLD)
  {
    m_layer[0] = color;
  }
  else
  {
    m_layer[0] = CRGB::Black;
  }
  for(int i=0; i<NUMSTRIPS; ++i)
  {
     m_strips[i] = m_layer[0];
  }
}
void SolidColorTower::End()
{
  Ended();
}

//********* PowerBarWithBassSprite *********
 Visualizations* PowerBarWithBassSprite::GetInstance( int duration
                                                    , StatisticalEngine &statisticalEngine
                                                    , VisualizationsCalleeInterface *callee )
{
  if(true == debugMode && debugLevel >= 2) Serial << "New PowerBarWithBassSprite\n";
  return new PowerBarWithBassSprite(duration, statisticalEngine, callee);
}
void PowerBarWithBassSprite::Start()
{
  if(true == debugMode && debugLevel >= 1) Serial << "PowerBarWithBassSprite Start\n";
  m_powerBarColor = GetRandomColor(0, 100, 150, 255, 0, 100);
  SetFadeToColor(m_powerBarFadeController, m_powerBarColor, GetRandomColor(0, 100, 150, 255, 0, 100), 50);
  m_bassPowerBarColor = GetRandomColor(150, 255, 0, 100, 0, 100);
  SetFadeToColor(m_bassPowerBarFadeController, m_bassPowerBarColor, GetRandomColor(150, 255, 0, 100, 0, 100), 50);
  m_maxBassSpriteColor = GetRandomColor(0, 100, 0, 100, 150, 255);
  SetFadeToColor(m_maxBassSpriteFadeController, m_maxBassSpriteColor, GetRandomColor(0, 100, 0, 100, 150, 255), 50);
  m_sprites[0].position = 0;
  m_sprites[0].color = m_bassPowerBarColor;
  m_sprites[1].position = 0;
  m_sprites[1].color = m_maxBassSpriteColor;
  m_sprites[2].position = 0;
  m_sprites[2].color = m_maxBassSpriteColor;
  m_powerBarChangeColorTime = random(5001);
  m_bassPowerBarChangeColorTime = random(5001);
  m_bassPowerMaxChangeColorTime = random(5001);
  Started();
}

bool PowerBarWithBassSprite::Loop() 
{  
  if(false == m_visualizationStarted) {Start(); m_visualizationStarted = true;}
  bool updateRequired = false;
  unsigned long currentTime = millis();
  if(true == debugMode && debugLevel >= 3) Serial << "PowerBarWithBassSprite Loop: " << currentTime-m_resetTimer << " | " << m_duration << "\n";
  if(currentTime-m_timers[0] >= 0) {ResetTimer(0); Tick1(); updateRequired = true;}
  if(currentTime-m_timers[1] >= 500) {ResetTimer(1); Tick2(); updateRequired = true;}
  if(currentTime-m_timers[2] >= 1) {ResetTimer(2); Tick3(); updateRequired = true;}
  if(currentTime-m_timers[3] >= m_powerBarChangeColorTime) {ResetTimer(3); Tick4(); updateRequired = true;}
  if(currentTime-m_timers[4] >= m_bassPowerBarChangeColorTime) {ResetTimer(4); Tick5(); updateRequired = true;}
  if(currentTime-m_timers[5] >= m_bassPowerMaxChangeColorTime) {ResetTimer(5); Tick6(); updateRequired = true;}
  if(currentTime-m_resetTimer >= m_duration) {End(); updateRequired = true;}
  return updateRequired;
}

void PowerBarWithBassSprite::Tick1()
{
  m_layer[0] = CRGB::Black;
  int bassMin = 0;
  int bassMax = 0;
  m_powerBarColor = FadeColor(m_powerBarFadeController);
  m_bassPowerBarColor = FadeColor(m_bassPowerBarFadeController);
  m_maxBassSpriteColor = FadeColor(m_maxBassSpriteFadeController);
  float  level1 = GetNormalizedSoundLevelForFrequencyRange(0.0, MAX_DISPLAYED_FREQ, 5, SoundLevelOutputType_Level);
  float  level2 = GetNormalizedSoundLevelForFrequencyRange(0.0, 500.0, 2, SoundLevelOutputType_Beat);
  int power = level1 * (NUMLEDS - 2);
  int bassSize = level2 * (NUMLEDS*0.10);
  if(m_sprites[0].position <= power + 1)
  {
    int offset = power + 1 - m_sprites[0].position;
    m_sprites[0].position += offset;
    m_sprites[1].position += offset;
    m_sprites[2].position += offset;
  }
  bassMax = m_sprites[0].position + bassSize;
  bassMin = m_sprites[0].position - bassSize;
  if (bassMin < 1) bassMin = 1;
  if (bassMax > NUMLEDS - 2) bassMax = NUMLEDS - 2;
  if(m_sprites[2].position > bassMin)
  {
    m_sprites[2].position = bassMin;
  }
  if(m_sprites[1].position < bassMax)
  {
    m_sprites[1].position = bassMax;
  }
  if (m_sprites[2].position < 1) m_sprites[2].position = 1;
  if (m_sprites[1].position > NUMLEDS-1) m_sprites[1].position = NUMLEDS - 1;
  
  m_layer[0](0, power) = m_powerBarColor;
  m_sprites[0].color = m_bassPowerBarColor;
  m_sprites[1].color = m_sprites[2].color = m_maxBassSpriteColor;
  
  m_layer[0](bassMin, bassMax) = m_sprites[0].color;
  m_layer[0](m_sprites[1].position, m_sprites[1].position) = m_sprites[1].color;
  m_layer[0](m_sprites[2].position, m_sprites[2].position) = m_sprites[2].color;
  for(int i = 0; i < NUMSTRIPS; ++i)
  {
     m_strips[i] = m_layer[0];
  }
}

void PowerBarWithBassSprite::Tick2()
{
  if(m_sprites[0].position > 0)
  {
    m_sprites[0].position -= 1;
    m_sprites[1].position -= 1;
    m_sprites[2].position -= 1;
  }
}

void PowerBarWithBassSprite::Tick3()
{
  if(m_sprites[1].position > m_sprites[0].position)
  {
    m_sprites[1].position -= 1;
  }
  if(m_sprites[2].position < m_sprites[0].position)
  {
    m_sprites[2].position += 1;
  }
}

void PowerBarWithBassSprite::Tick4()
{
  m_powerBarChangeColorTime = random(5001);
  SetFadeToColor(m_powerBarFadeController, m_powerBarColor, GetRandomColor(0, 100, 150, 255, 0, 100), 50);
}

void PowerBarWithBassSprite::Tick5()
{
  m_bassPowerBarChangeColorTime = random(5001);
  SetFadeToColor(m_bassPowerBarFadeController, m_bassPowerBarColor, GetRandomColor(150, 255, 0, 100, 0, 100), 50);
}

void PowerBarWithBassSprite::Tick6()
{
  m_bassPowerMaxChangeColorTime = random(5001); 
  SetFadeToColor(m_maxBassSpriteFadeController, m_maxBassSpriteColor, GetRandomColor(0, 100, 0, 100, 150, 255), 50);
}

void PowerBarWithBassSprite::End()
{
  Ended();
}

//********* RandomFrequencySprites *********
 Visualizations* RandomFrequencySprites::GetInstance( int duration
                                                    , StatisticalEngine &statisticalEngine
                                                    , VisualizationsCalleeInterface *callee )
{
  if(true == debugMode && debugLevel >= 2) Serial << "New RandomFrequencySprites\n";
  return new RandomFrequencySprites(duration, statisticalEngine, callee);
}
void RandomFrequencySprites::Start()
{
  if(true == debugMode && debugLevel >= 1) Serial << "RandomFrequencySprites Start\n";
  Started();
}

bool RandomFrequencySprites::Loop() 
{
  if(false == m_visualizationStarted) {Start(); m_visualizationStarted = true;}
  bool updateRequired = false;
  unsigned long currentTime = millis();
  if(true == debugMode && debugLevel >= 3) Serial << "RandomFrequencySprites Loop: " << currentTime-m_resetTimer << " | " << m_duration << "\n";
  if(currentTime-m_timers[0] >= 0) {ResetTimer(0); Tick1(); updateRequired = true;}
  if(currentTime-m_resetTimer >= m_duration) {End();  updateRequired = true;}
  return updateRequired;
}

void RandomFrequencySprites::Tick1()
{
  int maxBinSave = 0;
  float  level = 0.0;
  float  maxLevelSave = 0.0;
  for(int i = 0; i < NUMSTRIPS; ++i)
  {
    m_strips[i] = FadeOutLayerBy(m_strips[i], m_fadeAmount);
  }
  for(int i = 0; i < BINS; ++i)
  {
    level = GetNormalizedSoundLevelForBin(i, 0, SoundLevelOutputType_Beat);
    if(true == debugMode && debugLevel >= 5) Serial << i << "|" << level << "\t";
    if(level > maxLevelSave)
    {
      maxLevelSave = level;
      maxBinSave = i;
    }
  }
  if(true == debugMode && debugLevel >= 5) Serial << "\n";
  CRGB color = GetColor(maxBinSave, m_statisticalEngine.GetFFTBinIndexForFrequency(MAX_DISPLAYED_FREQ));
  color.red = color.red * maxLevelSave;
  color.blue = color.blue * maxLevelSave;
  color.green = color.green * maxLevelSave;
  struct Sprite aSprite;
  aSprite.position = random(0, NUMLEDS);
  if(m_statisticalEngine.power >= SILENCE_THRESHOLD)
  {
    aSprite.color = color;
    m_layer[0] = CRGB::Black;
    m_layer[0](aSprite.position, aSprite.position) = aSprite.color;
  }
  if(true == debugMode && debugLevel >= 2) Serial << "RandomFrequencySprites: Max Bin: " << maxBinSave << "\tMax Level: " << maxLevelSave << "\n";
  for(int i = 0; i < NUMSTRIPS; ++i)
  {
    m_strips[i] = AddLayer(m_strips[i], m_layer[0]);
  }
}

void RandomFrequencySprites::End()
{
  Ended();
}

//********* FFTAmplitudes *********
 Visualizations* FFTAmplitudes::GetInstance( int duration
                                           , StatisticalEngine &statisticalEngine
                                           , VisualizationsCalleeInterface *callee )
{
  if(true == debugMode && debugLevel >= 2) Serial << "New FFTAmplitudes\n";
  return new FFTAmplitudes(duration, statisticalEngine, callee);
}
void FFTAmplitudes::Start()
{
  if(true == debugMode && debugLevel >= 1) Serial << "FFTAmplitudes Start\n";
  Started();
}

bool FFTAmplitudes::Loop() 
{
  if(false == m_visualizationStarted) {Start(); m_visualizationStarted = true;}
  bool updateRequired = false;
  unsigned long currentTime = millis();
  if(true == debugMode && debugLevel >= 3) Serial << "FFTAmplitudes Loop: " << currentTime-m_resetTimer << " | " << m_duration << "\n";
  if(currentTime-m_timers[0] >= 0) {ResetTimer(0); Tick1(); updateRequired = true;}
  if(currentTime-m_resetTimer >= m_duration) {End(); updateRequired = true;}
  return updateRequired;
}

void FFTAmplitudes::Tick1()
{  
  int numBands = 7;
  int freqWidth =  MAX_DISPLAYED_FREQ / numBands;
  int maxLEDIlluminated = 0;
  for(int i = 0; i < NUMLEDS; ++i)
  {
    m_layer[0][i].subtractFromRGB(50);
  }
  for(int i = 0; i < numBands; ++i)
  {
    float lowerFrequencyRange = i*freqWidth;
    float upperFrequencyRange = i*freqWidth+freqWidth;
    float  level = GetNormalizedSoundLevelForFrequencyRange(lowerFrequencyRange, upperFrequencyRange, 20, SoundLevelOutputType_Level);
    level = m_gainmultiplier*m_gainIntigrator*level/m_gainLimitMax;
    if(level < 0.0) level = 0.0;
    if(level > 1.0) level = 1.0;
    float  amplitude = level * NUMLEDS;
    if(amplitude >= NUMLEDS-1)
    {
      amplitude = NUMLEDS-1;
    }
    if(amplitude > maxLEDIlluminated) maxLEDIlluminated = amplitude;
    m_layer[0]((int)amplitude, (int)amplitude) = GetColor(i, numBands);
    if(true == debugMode && debugLevel >= 3) Serial << "\n";
  }
  if(maxLEDIlluminated < 0.6*NUMLEDS && m_gainIntigrator < m_gainLimitMax) ++m_gainIntigrator;
  if(maxLEDIlluminated > 0.8*NUMLEDS && m_gainIntigrator > m_gainLimitMin) --m_gainIntigrator;
  for(int j = 0; j < NUMSTRIPS; ++j)
  {
     m_strips[j] = m_layer[0];
  }
}

void FFTAmplitudes::End()
{
  Ended();
}

//********* FrequencySpriteSpiral *********
 Visualizations* FrequencySpriteSpiral::GetInstance( int duration
                                                         , StatisticalEngine &statisticalEngine
                                                         , VisualizationsCalleeInterface *callee )
{
  if(true == debugMode && debugLevel >= 2) Serial << "New FrequencySpriteSpiral\n";
  return new FrequencySpriteSpiral(duration, statisticalEngine, callee);
}
void FrequencySpriteSpiral::Start()
{
  if(true == debugMode && debugLevel >= 1) Serial << "FrequencySpriteSpiral Start\n";
  m_randomTimes[0] = random(0, 100);
  m_randomTimes[1] = random(1001);
  for(int i = 0; i < NUMSTRIPS; ++i)
  {
    m_strips[i] = CRGB::Black;
    m_layer[i] = CRGB::Black;
  }
  Started();
}
bool FrequencySpriteSpiral::Loop() 
{
  if(false == m_visualizationStarted) {Start(); m_visualizationStarted = true;}
  bool updateRequired = false;
  unsigned long currentTime = millis();
  if(true == debugMode && debugLevel >= 3) Serial << "FrequencySpriteSpiral Loop: " << currentTime-m_resetTimer << " | " << m_duration << "\n";
  if(currentTime-m_timers[0] >= m_randomTimes[0]) {ResetTimer(0); Tick1(); updateRequired = true;}
  if(currentTime-m_resetTimer >= m_duration) {End(); updateRequired = true;}
  return updateRequired;
}
void FrequencySpriteSpiral::Tick1()
{
  for(int i = 0; i < NUMLEDS; ++i)
  {
    m_layer[0][i].subtractFromRGB(i*0.1);
    m_layer[1][i].subtractFromRGB(i*0.1);
    m_layer[2][i].subtractFromRGB(i*0.1);
    m_layer[3][i].subtractFromRGB(i*0.1);
    m_layer[4][NUMLEDS-i-1].subtractFromRGB(i*0.1);
    m_layer[5][NUMLEDS-i-1].subtractFromRGB(i*0.1);
    m_layer[6][NUMLEDS-i-1].subtractFromRGB(i*0.1);
    m_layer[7][NUMLEDS-i-1].subtractFromRGB(i*0.1);
  }
  for(int i = 0; i < NUMSTRIPS; ++i)
  {
    m_strips[i] = CRGB::Black;
    m_strips[i] = m_layer[i];
  }
  SpiralTowerUpwards();
  for(int i = 0; i < NUMSTRIPS; ++i)
  {
    m_layer[i] = m_strips[i];
    m_strips[i] = CRGB::Black;
  }
  for(int i = 0; i < NUMSTRIPS; ++i)
  {
    m_strips[i] = m_layer[i+4];
  }
  SpiralTowerDownwards();
  for(int i = 0; i < NUMSTRIPS; ++i)
  {
    m_layer[i+4] = m_strips[i];
    m_strips[i] = CRGB::Black;
  } 
  
  struct Sprite aSprite;
  int maxBinSave = 0;
  float  maxLevelSave = 0.0;
  for(int i = 0; i < BINS; ++i)
  {
    float  level = GetNormalizedSoundLevelForBin(i, 0, SoundLevelOutputType_Beat);
    if(level > maxLevelSave)
    {
      maxLevelSave = level;
      maxBinSave = i;
    }
  }    
  CRGB color = GetColor(maxBinSave, m_statisticalEngine.GetFFTBinIndexForFrequency(MAX_DISPLAYED_FREQ));
  color.red = color.red * maxLevelSave;
  color.blue = color.blue * maxLevelSave;
  color.green = color.green * maxLevelSave;  
  
  if (m_statisticalEngine.power >= SILENCE_THRESHOLD)
  {
    aSprite.color = color;    
  }
  else
  {
    aSprite.color = CRGB::Black;
  }
  m_layer[0](0, 0) = aSprite.color;
  m_layer[1](0, 0) = aSprite.color;
  m_layer[2](0, 0) = aSprite.color;
  m_layer[3](0, 0) = aSprite.color;
  m_layer[4](NUMLEDS-1, NUMLEDS-1) = aSprite.color;
  m_layer[5](NUMLEDS-1, NUMLEDS-1) = aSprite.color;
  m_layer[6](NUMLEDS-1, NUMLEDS-1) = aSprite.color;
  m_layer[7](NUMLEDS-1, NUMLEDS-1) = aSprite.color;
  
  for(int i = 0; i < NUMSTRIPS; ++i)
  {
    m_strips[i] = AddLayer(m_layer[i], m_layer[i+4]);
  }
}
void FrequencySpriteSpiral::End()
{
  Ended();
}

//********* RandomHighLowFrequencyAmplitudeStreamer *********
 Visualizations* RandomHighLowFrequencyAmplitudeStreamer::GetInstance( int duration
                                                                           , StatisticalEngine &statisticalEngine
                                                                           , VisualizationsCalleeInterface *callee )
{
  if(true == debugMode && debugLevel >= 2) Serial << "New RandomHighLowFrequencyAmplitudeStreamer\n";
  return new RandomHighLowFrequencyAmplitudeStreamer(duration, statisticalEngine, callee);
}
void RandomHighLowFrequencyAmplitudeStreamer::Start()
{
  if(true == debugMode && debugLevel >= 1) Serial << "RandomHighLowFrequencyAmplitudeStreamer Start\n";
  Random();
  Started();
}
bool RandomHighLowFrequencyAmplitudeStreamer::Loop() 
{
  if(false == m_visualizationStarted) {Start(); m_visualizationStarted = true;}
  bool updateRequired = false;
  unsigned long currentTime = millis();
  if(true == debugMode && debugLevel >= 3) Serial << "RandomHighLowFrequencyAmplitudeStreamer Loop: " << currentTime-m_resetTimer << " | " << m_duration << "\n";
  if(currentTime-m_timers[0] >= 0) {ResetTimer(0); Tick1(); updateRequired = true;}
  if(currentTime-m_timers[1] >= m_randomTime) {ResetTimer(1); Random(); updateRequired = true;}
  if(currentTime-m_resetTimer >= m_duration) {End(); updateRequired = true;}
  return updateRequired; 
}
void RandomHighLowFrequencyAmplitudeStreamer::Tick1()
{
  ShiftStrip(m_layer[0], Up, 1);
  ShiftStrip(m_layer[1], Down, 1);
  float  level1 = GetNormalizedSoundLevelForFrequencyRange(0.0, m_triggerFrequencyLow, 0, SoundLevelOutputType_Beat); 
  CRGB color = GetColor(m_triggerFrequencyLow, MAX_DISPLAYED_FREQ);
  color.red = color.red * level1;
  color.blue = color.blue * level1;
  color.green = color.green * level1; 
  m_layer[0](0, 0) = color;
  float  level2 = GetNormalizedSoundLevelForFrequencyRange(m_triggerFrequencyHigh, MAX_DISPLAYED_FREQ, 0, SoundLevelOutputType_Beat);  
  CRGB color2 = GetColor(m_triggerFrequencyHigh, MAX_DISPLAYED_FREQ);
  color2.red = color2.red * level2;
  color2.blue = color2.blue * level2;
  color2.green = color2.green * level2; 
  m_layer[1](NUMLEDS-1, NUMLEDS-1) = color2;
  for(int i = 0; i < NUMLEDS; ++i)
  {
    m_layer[0][i].subtractFromRGB(i*0.25);
    m_layer[1][NUMLEDS-i-1].subtractFromRGB(i*0.25);
  }
  for(int i = 0; i < NUMSTRIPS; ++i)
  {
    m_strips[i] = AddLayer(m_layer[0], m_layer[1]);
  }
}
void RandomHighLowFrequencyAmplitudeStreamer::Random()
{
  m_randomTime = random(10000);
  m_triggerFrequencyLow = random(0, MAX_DISPLAYED_FREQ/3 + 1);
  m_triggerFrequencyHigh = random(2*MAX_DISPLAYED_FREQ/3, MAX_DISPLAYED_FREQ + 1);
}
void RandomHighLowFrequencyAmplitudeStreamer::End()
{
  Ended();
}

//********* OutwardAmplitudeWithFloatingBassSprites *********
 Visualizations* OutwardAmplitudeWithFloatingBassSprites::GetInstance( int duration
                                                                           , StatisticalEngine &statisticalEngine
                                                                           , VisualizationsCalleeInterface *callee )
{
  if(true == debugMode && debugLevel >= 2) Serial << "New OutwardAmplitudeWithFloatingBassSprites\n";
  return new OutwardAmplitudeWithFloatingBassSprites(duration, statisticalEngine, callee);
}
void OutwardAmplitudeWithFloatingBassSprites::Start()
{
  if(true == debugMode && debugLevel >= 1) Serial << "OutwardAmplitudeWithFloatingBassSprites Start\n";
  m_sprites[0].color = CRGB::Red;
  m_sprites[1].color = CRGB::Red;
  m_sprites[0].position = NUMLEDS/2;
  m_sprites[1].position = NUMLEDS/2;
  m_randomTimes[0] = random(0, 201);
  m_randomTimes[1] = random(5001);
  Random();
  Started();
}
bool OutwardAmplitudeWithFloatingBassSprites::Loop() 
{
  if(false == m_visualizationStarted) {Start(); m_visualizationStarted = true;}
  bool updateRequired = false;
  unsigned long currentTime = millis();
  if(true == debugMode && debugLevel >= 3) Serial << "OutwardAmplitudeWithFloatingBassSprites Loop: " << currentTime-m_resetTimer << " | " << m_duration << "\n";
  if(currentTime-m_timers[0] >= 0) {ResetTimer(0); Tick1(); updateRequired = true;}
  if(currentTime-m_timers[1] >= m_randomTimes[0]) {ResetTimer(1); Tick2(); updateRequired = true;}
  if(currentTime-m_timers[2] >= m_randomTimes[1]) {ResetTimer(2); Random(); updateRequired = true;}
  if(currentTime-m_resetTimer >= m_duration) {End(); updateRequired = true;}
  return updateRequired;
}
void OutwardAmplitudeWithFloatingBassSprites::Tick1()
{
  int bottomCenter = NUMLEDS/2-1;
  int topCenter = NUMLEDS/2;
  float  level1 = GetNormalizedSoundLevelForFrequencyRange(0.0, MAX_DISPLAYED_FREQ, 5, SoundLevelOutputType_Level);
  float  level2 = GetNormalizedSoundLevelForFrequencyRange(0.0, 500, 2, SoundLevelOutputType_Beat);
  int barHeight = ((NUMLEDS/2-1))*level1;
  int bassSize = level2 * (0.1 * NUMLEDS);
  int bottomPeak = bottomCenter;
  int topPeak = topCenter;
  m_BottomColor = FadeColor(m_bottomFadeController);
  m_TopColor = FadeColor(m_topFadeController);
  m_layer[0] = CRGB::Black;
  if(barHeight >= ((NUMLEDS/2-1)))
  {
    barHeight = ((NUMLEDS/2-1));
  }
  bottomPeak = bottomCenter - barHeight;
  topPeak = topCenter + barHeight;
  m_layer[0](bottomCenter, bottomPeak) = m_BottomColor;
  m_layer[0](topCenter, topPeak) = m_TopColor;

  int bassMin0 = 0;
  int bassMax0 = 0;
  if(m_sprites[0].position > bottomPeak -1)
  {
    m_sprites[0].position = bottomPeak - 1;
  }
  bassMin0 = m_sprites[0].position - bassSize;
  bassMax0 = m_sprites[0].position;
  if (bassMin0 < 0) bassMin0 = 0;
  if (bassMax0 < 0) bassMax0 = 0;
  m_layer[1](bassMin0, bassMax0) = m_sprites[0].color;      
  
  int bassMin1 = 0;
  int bassMax1 = 0;
  if(m_sprites[1].position < topPeak + 1)
  {
    m_sprites[1].position = topPeak + 1;
  }  
  bassMin1 = m_sprites[1].position;
  bassMax1 = m_sprites[1].position + bassSize;
  if (bassMax1 > NUMLEDS-1) bassMax1 = NUMLEDS-1;
  if (bassMin1 > NUMLEDS-1) bassMin1 = NUMLEDS-1;
  m_layer[1](bassMin1, bassMax1) = m_sprites[1].color;
  
  for(int i = 0; i < NUMSTRIPS; ++i)
  {
    m_strips[i] = MergeLayer(m_layer[1], m_layer[0]);
  }
}
void OutwardAmplitudeWithFloatingBassSprites::Tick2()
{
  byte bottomCenter = NUMLEDS/2;
  byte topCenter = NUMLEDS/2 - 1;
  byte result = 0;
  byte bottomPeak = bottomCenter - result;
  byte topPeak = topCenter + result;
  m_layer[1] = CRGB::Black;
  if(m_sprites[0].position < bottomCenter)
  {
    m_sprites[0].position = m_sprites[0].position + 1;
  }
  
  if(m_sprites[1].position > topCenter)
  {
    m_sprites[1].position = m_sprites[1].position - 1;
  }
}
void OutwardAmplitudeWithFloatingBassSprites::Random()
{
  m_randomTimes[1] = random(5001);
  m_BottomFadeToColor = GetRandomNonGrayColor();
  m_TopFadeToColor = GetRandomNonGrayColor();
  m_fadeLength = random(0, 2001);
  SetFadeToColor(m_bottomFadeController, m_BottomColor, m_BottomFadeToColor, m_fadeLength);
  SetFadeToColor(m_topFadeController, m_TopColor, m_TopFadeToColor, m_fadeLength);
}
void OutwardAmplitudeWithFloatingBassSprites::End()
{
  Ended();
}


//********* VerticalFFTAmplitudeTower *********
 Visualizations* VerticalFFTAmplitudeTower::GetInstance( int duration
                                                             , StatisticalEngine &statisticalEngine
                                                             , VisualizationsCalleeInterface *callee )
{
  if(true == debugMode && debugLevel >= 2) Serial << "New VerticalFFTAmplitudeTower\n";
  return new VerticalFFTAmplitudeTower(duration, statisticalEngine, callee);
}
void VerticalFFTAmplitudeTower::Start()
{
  if(true == debugMode && debugLevel >= 1) Serial << "VerticalFFTAmplitudeTower Start\n";
  for(int i = 0; i < NUMSTRIPS; ++i)
  {
    m_strips[i] = CRGB::Black;
  }
  Started();
}
bool VerticalFFTAmplitudeTower::Loop() 
{
  if(false == m_visualizationStarted) {Start(); m_visualizationStarted = true;}
  bool updateRequired = false;
  unsigned long currentTime = millis();
  if(true == debugMode && debugLevel >= 3) Serial << "VerticalFFTAmplitudeTower Loop: " << currentTime-m_resetTimer << " | " << m_duration << "\n";
  if(currentTime-m_timers[0] >= 0) {ResetTimer(0); Tick1(); updateRequired = true;}
  if(currentTime-m_resetTimer >= m_duration) {End(); updateRequired = true;}
  return updateRequired;  
}
void VerticalFFTAmplitudeTower::Tick1()
{
  
  float  endBin = m_statisticalEngine.GetFFTBinIndexForFrequency(MAX_DISPLAYED_FREQ);
  float  leds = NUMLEDS;
  float  binsPerLED = endBin/leds;
  float  ledsPerBin = leds/endBin;
  float  maxAmplitude = 0;
  float  minAmplitude = INT_MAX;
  float  value = 0;
  
  if(binsPerLED >= 1)
  {    
    for(int i = 0; i < NUMLEDS; ++i)
    { 
      float  levelSave = 0.0;
      for(int j = 0; ((j < binsPerLED) && (i+j < endBin)); ++j)
      {
        float  level = GetNormalizedSoundLevelForBin(i+j, 0, SoundLevelOutputType_Beat);
        if(level > levelSave) levelSave = level;
      }
      if(true == debugMode && debugLevel >= 5) Serial << i << " | " << levelSave << "\n";
      CRGB color = GetColor(i, NUMLEDS);
      m_layer[0][i] = CRGB((byte)(dim8_raw(color.red * levelSave)), (byte)(dim8_raw(color.green * levelSave)), (byte)(dim8_raw(color.blue * levelSave)));
    }
    if(true == debugMode && debugLevel >= 5) Serial << "**********\n";
    for(int i = 0; i < NUMSTRIPS; ++i)
    {
      m_strips[i] = m_layer[0];
    } 
  }
  else if(ledsPerBin >= 1)
  {
    for(int i = 0; i < NUMLEDS; ++i)
    {
      float  level = GetNormalizedSoundLevelForBin(i/ledsPerBin, 0, SoundLevelOutputType_Beat);
      if(true == debugMode && debugLevel >= 5) Serial << i << " | " << level << "\n";
      CRGB color = GetColor(i, NUMLEDS);
      m_layer[0][i] = CRGB((byte)(dim8_raw(color.red * level)), (byte)(dim8_raw(color.green * level)), (byte)(dim8_raw(color.blue * level)));
    }
    if(true == debugMode && debugLevel >= 5) Serial << "**********\n";
    for(int i = 0; i < NUMSTRIPS; ++i)
    {
      m_strips[i] = m_layer[0];
    } 
  }
}
void VerticalFFTAmplitudeTower::End()
{
  Ended();
}

//********** MultiRangeAmplitudeTower *************
 Visualizations* MultiRangeAmplitudeTower::GetInstance( int duration
                                                            , StatisticalEngine &statisticalEngine
                                                            , VisualizationsCalleeInterface *callee )
{
  if(true == debugMode && debugLevel >= 2) Serial << "New MultiRangeAmplitudeTower\n";
  return new MultiRangeAmplitudeTower(duration, statisticalEngine, callee);
}
void MultiRangeAmplitudeTower::Start()
{
  if(true == debugMode && debugLevel >= 1) Serial << "MultiRangeAmplitudeTower Start\n";
  Random();
  Started();
}
bool MultiRangeAmplitudeTower::Loop() 
{
  bool updateRequired = false;
  unsigned long currentTime = millis();
  if(false == m_visualizationStarted) {Start(); m_visualizationStarted = true;}
  if(true == debugMode && debugLevel >= 3) Serial << "MultiRangeAmplitudeTower Loop: " << currentTime-m_timers[1] << " | " << m_duration << "\n";
  if(currentTime-m_timers[0] >= 0) {ResetTimer(0); Tick1(); updateRequired = true;}
  if(currentTime-m_timers[2] >= m_duration) {End(); updateRequired = true;}
  return updateRequired;
}
void MultiRangeAmplitudeTower::Tick1()
{
  
  m_layer[0] = CRGB::Black;
  int numRanges = m_randomNumber;
  int rangeHeight = NUMLEDS / numRanges;
  int rangeFrequency = MAX_DISPLAYED_FREQ / numRanges;
  for(int i = 0; i < numRanges; ++i)
  {
    int lowerLedPosition = i*rangeHeight;
    int upperLedPosition = i*rangeHeight+rangeHeight-1;
    float lowerFrequencyRange = i*rangeFrequency;
    float upperFrequencyRange = i*rangeFrequency+rangeFrequency;
    float  level = GetNormalizedSoundLevelForFrequencyRange(lowerFrequencyRange, upperFrequencyRange, 5, SoundLevelOutputType_Level);   
    if(true == debugMode && debugLevel >= 3) Serial << i << "|" << level << "\t";
    int onLEDs = level*(upperLedPosition-lowerLedPosition - 1);
    m_layer[0](lowerLedPosition, lowerLedPosition + onLEDs) = m_powerBarColor;    
    m_layer[0](upperLedPosition, upperLedPosition) = m_topColor;
    m_layer[0](lowerLedPosition, lowerLedPosition) = m_bottomColor;
  }
  if(true == debugMode && debugLevel >= 3) Serial << "\n";
  for(int i = 0; i < NUMSTRIPS; ++i)
  {
    m_strips[i] = m_layer[0];
  } 
}

void MultiRangeAmplitudeTower::Random()
{
  m_topColor = GetRandomNonGrayColor();
  m_powerBarColor = GetRandomNonGrayColor();
  m_bottomColor = GetRandomNonGrayColor();
  m_randomNumber = random(1, NUMLEDS/m_maxLEDsPerRange + 1);
  while((NUMLEDS % m_randomNumber != 0))
  {
    m_randomNumber = random(1, NUMLEDS/m_maxLEDsPerRange + 1);
  }
}

void MultiRangeAmplitudeTower::End()
{
  Ended();
}


//********** SimultaneousFrequencyStreamer *************
 Visualizations* SimultaneousFrequencyStreamer::GetInstance( int duration
                                                                 , StatisticalEngine &statisticalEngine
                                                                 , VisualizationsCalleeInterface *callee )
{
  if(true == debugMode && debugLevel >= 2) Serial << "New SimultaneousFrequencyStreamer\n";
  return new SimultaneousFrequencyStreamer(duration, statisticalEngine, callee);
}
void SimultaneousFrequencyStreamer::Start()
{
  if(true == debugMode && debugLevel >= 1) Serial << "SimultaneousFrequencyStreamer Start\n";
  m_randomColors[0] = GetColor(0,7);
  m_randomColors[1] = GetColor(1,7);
  m_randomColors[2] = GetColor(2,7);
  m_randomColors[3] = GetColor(3,7);
  m_randomColors[4] = GetColor(4,7);
  m_randomColors[5] = GetColor(5,7);
  m_randomColors[6] = GetColor(6,7);
  m_randomColors[7] = GetColor(7,7);
  mergeType = (MergeType)random(MergeType::MERGE_TYPE_SIZE);
  Started();
}
bool SimultaneousFrequencyStreamer::Loop() 
{ 
  bool updateRequired = false;
  unsigned long currentTime = millis();
  if(false == m_visualizationStarted) {Start(); m_visualizationStarted = true;}
  if(true == debugMode && debugLevel >= 3) Serial << "SimultaneousFrequencyStreamer Loop: " << currentTime-m_resetTimer << " | " << m_duration << "\n";
  if(currentTime-m_timers[0] >= 0) {ResetTimer(0); Tick1(); updateRequired = true;}
  if(currentTime-m_timers[1] >= 10000) {ResetTimer(1); Tick2(); updateRequired = true;}
  if(currentTime-m_resetTimer >= m_duration) {End(); updateRequired = true;}
  return updateRequired;
}
void SimultaneousFrequencyStreamer::Tick1()
{ 
  float  level1 = GetNormalizedSoundLevelForFrequencyRange(0*MAX_DISPLAYED_FREQ/8, 1*MAX_DISPLAYED_FREQ/8, 0, SoundLevelOutputType_Beat);
  m_randomColors[8] = CRGB((byte)(dim8_raw(m_randomColors[0].red * level1))
                       , (byte)(dim8_raw(m_randomColors[0].green * level1))
                       , (byte)(dim8_raw(m_randomColors[0].blue * level1)));
                     
  float  level2 = GetNormalizedSoundLevelForFrequencyRange(1*MAX_DISPLAYED_FREQ/8, 2*MAX_DISPLAYED_FREQ/8, 0, SoundLevelOutputType_Beat);
  m_randomColors[9] = CRGB((byte)(dim8_raw(m_randomColors[1].red * level2))
                       , (byte)(dim8_raw(m_randomColors[1].green * level2))
                       , (byte)(dim8_raw(m_randomColors[1].blue * level2)));
                    
  float  level3 = GetNormalizedSoundLevelForFrequencyRange(2*MAX_DISPLAYED_FREQ/8, 3*MAX_DISPLAYED_FREQ/8, 0, SoundLevelOutputType_Beat);
  m_randomColors[10] = CRGB((byte)(dim8_raw(m_randomColors[2].red * level3))
                         , (byte)(dim8_raw(m_randomColors[2].green * level3))
                         , (byte)(dim8_raw(m_randomColors[2].blue * level3)));
                                            
  float  level4 = GetNormalizedSoundLevelForFrequencyRange(3*MAX_DISPLAYED_FREQ/8, 4*MAX_DISPLAYED_FREQ/8, 0, SoundLevelOutputType_Beat);
  m_randomColors[11] = CRGB((byte)(dim8_raw(m_randomColors[3].red * level4))
                         , (byte)(dim8_raw(m_randomColors[3].green * level4))
                         , (byte)(dim8_raw(m_randomColors[3].blue * level4)));
                     
  float  level5 = GetNormalizedSoundLevelForFrequencyRange(4*MAX_DISPLAYED_FREQ/8, 5*MAX_DISPLAYED_FREQ/8, 0, SoundLevelOutputType_Beat);
  m_randomColors[12] = CRGB((byte)(dim8_raw(m_randomColors[4].red * level5))
                         , (byte)(dim8_raw(m_randomColors[4].green * level5))
                         , (byte)(dim8_raw(m_randomColors[4].blue * level5)));
                   
  float  level6 = GetNormalizedSoundLevelForFrequencyRange(5*MAX_DISPLAYED_FREQ/8, 6*MAX_DISPLAYED_FREQ/8, 0, SoundLevelOutputType_Beat);
  m_randomColors[13] = CRGB((byte)(dim8_raw(m_randomColors[5].red * level6))
                         , (byte)(dim8_raw(m_randomColors[5].green * level6))
                         , (byte)(dim8_raw(m_randomColors[5].blue * level6)));
                    
  float  level7 = GetNormalizedSoundLevelForFrequencyRange(6*MAX_DISPLAYED_FREQ/8, 7*MAX_DISPLAYED_FREQ/8, 0, SoundLevelOutputType_Beat);
  m_randomColors[14] = CRGB((byte)(dim8_raw(m_randomColors[6].red * level7))
                         , (byte)(dim8_raw(m_randomColors[6].green * level7))
                         , (byte)(dim8_raw(m_randomColors[6].blue * level7)));
                     
  float  level8 = GetNormalizedSoundLevelForFrequencyRange(7*MAX_DISPLAYED_FREQ/8, 8*MAX_DISPLAYED_FREQ/8, 0, SoundLevelOutputType_Beat);
  m_randomColors[15] = CRGB((byte)(dim8_raw(m_randomColors[7].red * level8))
                         , (byte)(dim8_raw(m_randomColors[7].green * level8))
                         , (byte)(dim8_raw(m_randomColors[7].blue * level8)));
                           
  ShiftStrip(m_layer[0], Up, 1);
  m_layer[0](0, 0) = m_randomColors[8];
  
  ShiftStrip(m_layer[1], Up, 1);
  m_layer[1](0, 0) = m_randomColors[9];
  
  ShiftStrip(m_layer[2], Up, 1);
  m_layer[2](0, 0) = m_randomColors[10];
  
  ShiftStrip(m_layer[3], Up, 1);
  m_layer[3](0, 0) = m_randomColors[11];
  
  ShiftStrip(m_layer[4], Down, 1);
  m_layer[4](NUMLEDS-1, NUMLEDS-1) = m_randomColors[12];
  
  ShiftStrip(m_layer[5], Down, 1);
  m_layer[5](NUMLEDS-1, NUMLEDS-1) = m_randomColors[13];
  
  ShiftStrip(m_layer[6], Down, 1);
  m_layer[6](NUMLEDS-1, NUMLEDS-1) = m_randomColors[14];
  
  ShiftStrip(m_layer[7], Down, 1);
  m_layer[7](NUMLEDS-1, NUMLEDS-1) = m_randomColors[15];

  switch(m_fadeType)
  {
    default:
    case FADE_TYPE_ON:
    {
      /*
      int fadeAmount = (int)(255.0/((float )NUMLEDS));
      for(int i = 0; i < NUMLEDS; ++i)
      {
        m_layer[0][i].subtractFromRGB(fadeAmount);
        m_layer[1][i].subtractFromRGB(fadeAmount);
        m_layer[2][i].subtractFromRGB(fadeAmount);
        m_layer[3][i].subtractFromRGB(fadeAmount);
      }
      for(int i = NUMLEDS-1; i >= 0; --i)
      {
        m_layer[4][i].subtractFromRGB(fadeAmount);
        m_layer[5][i].subtractFromRGB(fadeAmount);
        m_layer[6][i].subtractFromRGB(fadeAmount);
        m_layer[7][i].subtractFromRGB(fadeAmount);
      }
      */
    }
    break;
    case FADE_TYPE_OFF:
    break;
  }

  switch(mergeType)
  {
    case MERGE_TYPE_ADD:
      m_strips[0] = AddLayer(m_layer[m_bottomLayers[(0 + m_rotationCount)%4]], m_layer[m_topLayers[(3 + m_rotationCount)%4]]);
      m_strips[1] = AddLayer(m_layer[m_bottomLayers[(1 + m_rotationCount)%4]], m_layer[m_topLayers[(2 + m_rotationCount)%4]]);
      m_strips[2] = AddLayer(m_layer[m_bottomLayers[(2 + m_rotationCount)%4]], m_layer[m_topLayers[(1 + m_rotationCount)%4]]);
      m_strips[3] = AddLayer(m_layer[m_bottomLayers[(3 + m_rotationCount)%4]], m_layer[m_topLayers[(0 + m_rotationCount)%4]]);
      break;
    case MERGE_TYPE_MERGE:    
      m_strips[0] = MergeLayerFlipZOrder(m_layer[m_bottomLayers[(0 + m_rotationCount)%4]], m_layer[m_topLayers[(3 + m_rotationCount)%4]]);
      m_strips[1] = MergeLayerFlipZOrder(m_layer[m_bottomLayers[(1 + m_rotationCount)%4]], m_layer[m_topLayers[(2 + m_rotationCount)%4]]);
      m_strips[2] = MergeLayerFlipZOrder(m_layer[m_bottomLayers[(2 + m_rotationCount)%4]], m_layer[m_topLayers[(1 + m_rotationCount)%4]]);
      m_strips[3] = MergeLayerFlipZOrder(m_layer[m_bottomLayers[(3 + m_rotationCount)%4]], m_layer[m_topLayers[(0 + m_rotationCount)%4]]);
      break;
    default:
      break;
  }
}
void SimultaneousFrequencyStreamer::Tick2()
{
  switch(m_rotationDirection)
  {
    case ROTATION_DIRECTION_RIGHT:
      ++m_rotationCount;
    break;
    case ROTATION_DIRECTION_LEFT:
      --m_rotationCount;
    break;
    default:
      ++m_rotationCount;
    break;
  }
}
void SimultaneousFrequencyStreamer::End()
{
  Ended();
}

//********* MinMaxAmplitude *********
 Visualizations* MinMaxAmplitude::GetInstance( int duration
                                             , StatisticalEngine &statisticalEngine
                                             , VisualizationsCalleeInterface *callee )
{
  if(true == debugMode && debugLevel >= 2) Serial << "New MinMaxAmplitude\n";
  return new MinMaxAmplitude(duration, statisticalEngine, callee);
}
void MinMaxAmplitude::Start()
{
  if(true == debugMode && debugLevel >= 1) Serial << "MinMaxAmplitude Start\n";
  m_randomTime = random(1000, 10001);
  Random();
  Started();
}
bool MinMaxAmplitude::Loop() 
{
  bool updateRequired = false;
  unsigned long currentTime = millis();
  if(false == m_visualizationStarted) {Start(); m_visualizationStarted = true;}
  if(true == debugMode && debugLevel >= 3) Serial << "MinMaxAmplitude Loop: " << currentTime-m_resetTimer << " | " << m_duration << "\n";
  if(currentTime-m_timers[0] >= 0) {ResetTimer(0); Tick1(); updateRequired = true;}
  if(currentTime-m_timers[1] >= m_randomTime) {ResetTimer(1); Random(); updateRequired = true;}
  if(currentTime-m_resetTimer >= m_duration) {End(); updateRequired = true;}
  return updateRequired;
}
void MinMaxAmplitude::Tick1()
{
  for(int i = 0; i < NUMSTRIPS; ++i)
  {
     m_strips[i] = CRGB::Black;
  }
  float freqWidth =  MAX_DISPLAYED_FREQ / m_numBands;
  struct Sprite aSprite;
  float lowerFrequencyRange = m_activeBand*freqWidth;
  float upperFrequencyRange = m_activeBand*freqWidth+freqWidth;
  aSprite.color = GetColor(m_activeBand, m_numBands);
  db amp = m_statisticalEngine.GetAverageDbOfFreqRange(lowerFrequencyRange, upperFrequencyRange, 0, BinDataType::INSTANT);
  db avg = m_statisticalEngine.GetAverageDbOfFreqRange(lowerFrequencyRange, upperFrequencyRange, BIN_SAVE_LENGTH-1, BinDataType::INSTANT);
  db maximum = m_statisticalEngine.GetMaxDbOfFreqRange(lowerFrequencyRange, upperFrequencyRange, BIN_SAVE_LENGTH-1, BinDataType::INSTANT);
  db minimum = m_statisticalEngine.GetMinDbOfFreqRange(lowerFrequencyRange, upperFrequencyRange, BIN_SAVE_LENGTH-1, BinDataType::INSTANT);
  MinMaxDb mm = m_statisticalEngine.GetFFTBinMinMaxAverageDbOfFreqRange(lowerFrequencyRange, upperFrequencyRange, BIN_SAVE_LENGTH-1, BinDataType::INSTANT);
  int powerAmp = (amp/MAX_DB) * (NUMLEDS - 1);
  int powerAvg = (avg/MAX_DB) * (NUMLEDS - 1);
  int powerMinMaxMax = (mm.max/MAX_DB) * (NUMLEDS - 1);
  int powerMinMaxMin = (mm.min/MAX_DB) * (NUMLEDS - 1);
  int powerMax = (maximum/MAX_DB) * (NUMLEDS - 1);
  int powerMin = (minimum/MAX_DB) * (NUMLEDS - 1);
  
  if(powerMinMaxMin < 0) powerMinMaxMin = 0;
  if(powerMinMaxMax < 0) powerMinMaxMax = 0;
  if(powerMax < 0) powerMax = 0;
  if(powerMax < 0) powerMax = 0;
  if(powerAvg < 0) powerAvg = 0;
  if(powerAmp < 0) powerAmp = 0;
  if(powerMinMaxMin > NUMLEDS-1) powerMinMaxMin = NUMLEDS-1;
  if(powerMinMaxMax > NUMLEDS-1) powerMinMaxMax = NUMLEDS-1;
  if(powerMax > NUMLEDS-1) powerMax = NUMLEDS-1;
  if(powerMin > NUMLEDS-1) powerMin = NUMLEDS-1;
  if(powerAvg > NUMLEDS-1) powerAvg = NUMLEDS-1;
  if(powerAmp > NUMLEDS-1) powerAmp = NUMLEDS-1;
  if(true == debugMode && debugLevel >= 5) Serial << "MinMaxAmplitude:\tLFR: " << lowerFrequencyRange << "    UFR: " << upperFrequencyRange << "    Average: " << avg << "    Amp: " << amp << "    Power Avg: " << powerAvg << "    Power Amp: " << powerAmp << "    Power Min: " << powerMinMaxMin << "    Power Max: " << powerMinMaxMax << "\n";
  for(int i = 0; i < NUMLEDS; ++i)
  {
    m_layer[0][i].subtractFromRGB(m_fadeAmount);
  }
  m_layer[0](powerAmp, powerAmp) = aSprite.color;  
  for(int i = 0; i < NUMSTRIPS; ++i)
  {
     m_strips[i](powerMinMaxMin, powerMinMaxMin) = -aSprite.color;
     m_strips[i](powerMinMaxMax, powerMinMaxMax) = -aSprite.color;
     m_strips[i](powerMax, powerMax) = CRGB::Red;
     m_strips[i](powerMin, powerMin) = CRGB::Red;
     m_strips[i](powerAvg, powerAvg) = CRGB::Green;
  }
  for(int i = 0; i < NUMSTRIPS; ++i)
  {
    m_strips[i] = MergeLayer(m_strips[i], m_layer[0]);
  }
}

void MinMaxAmplitude::Random()
{
  m_randomTime = random(10000, 20001);
  m_fadeAmount = random(5, 51);
  m_activeBand = random(m_numBands);
}
void MinMaxAmplitude::End()
{
  Ended();
}

//********* ChasingSprites *********
 Visualizations* ChasingSprites::GetInstance( int duration
                                            , StatisticalEngine &statisticalEngine
                                            , VisualizationsCalleeInterface *callee )
{
  if(true == debugMode && debugLevel >= 2) Serial << "New ChasingSprites\n";
  return new ChasingSprites(duration, statisticalEngine, callee);
}
void ChasingSprites::Start()
{
  if(true == debugMode && debugLevel >= 1) Serial << "ChasingSprites Start\n";
  m_bassTime = 100;
  m_midTime = 100;
  m_highTime = 100;
  m_bassOffset = 0;
  m_midOffset = 0;
  m_highOffset = 0;
  Random();
  Started();
}
bool ChasingSprites::Loop() 
{
  bool updateRequired = false;
  unsigned long currentTime = millis();
  if(false == m_visualizationStarted) {Start(); m_visualizationStarted = true;}
  if(true == debugMode && debugLevel >= 3) Serial << "ChasingSprites Loop: " << currentTime-m_resetTimer << " | " << m_duration << "\n";
  if(currentTime-m_timers[0] >= 0) {ResetTimer(0); Tick1(); updateRequired = true;}
  if(currentTime-m_timers[1] >= m_bassTime) {ResetTimer(1); Tick2(); updateRequired = true;}
  if(currentTime-m_timers[2] >= m_midTime) {ResetTimer(2); Tick3(); updateRequired = true;}
  if(currentTime-m_timers[3] >= m_highTime) {ResetTimer(3); Tick4(); updateRequired = true;}
  if(currentTime-m_resetTimer >= m_duration) {End(); updateRequired = true;}
  return updateRequired;
}
void ChasingSprites::Tick1()
{
  
  bassColor = GetColor( m_statisticalEngine.GetMaxFFTBinFromRange( m_statisticalEngine.GetFFTBinIndexForFrequency(0.0), m_statisticalEngine.GetFFTBinIndexForFrequency(MAX_DISPLAYED_FREQ/3)), m_statisticalEngine.GetFFTBinIndexForFrequency(MAX_DISPLAYED_FREQ));
  midColor = GetColor( m_statisticalEngine.GetMaxFFTBinFromRange( m_statisticalEngine.GetFFTBinIndexForFrequency(MAX_DISPLAYED_FREQ/3)+1, m_statisticalEngine.GetFFTBinIndexForFrequency(2*MAX_DISPLAYED_FREQ/3)), m_statisticalEngine.GetFFTBinIndexForFrequency(MAX_DISPLAYED_FREQ));
  highColor = GetColor( m_statisticalEngine.GetMaxFFTBinFromRange( m_statisticalEngine.GetFFTBinIndexForFrequency(2*MAX_DISPLAYED_FREQ/3)+1, m_statisticalEngine.GetFFTBinIndexForFrequency(MAX_DISPLAYED_FREQ)), m_statisticalEngine.GetFFTBinIndexForFrequency(MAX_DISPLAYED_FREQ));
    
  m_bassTime = GetNewBassTime();
  m_midTime = GetNewMidTime();
  m_highTime = GetNewHighTime();
  if(true == debugMode && debugLevel >= 3) Serial << m_bassTime << "|" << m_midTime << "|" << m_highTime << "\n";
  switch(m_mergeType)
  {
    case 0:
      for(int i = 0; i < NUMSTRIPS; ++i)
      {
        m_strips[i] = MergeLayer(m_layer[0], m_layer[1]);
        m_strips[i] = MergeLayer(m_layer[2], m_strips[i]);
      }
      break;
    case 1:
      for(int i = 0; i < NUMSTRIPS; ++i)
      {
        m_strips[i] = MergeLayer(m_layer[0], m_strips[i]);
        m_strips[i] = MergeLayer(m_layer[1], m_strips[i]);
        m_strips[i] = MergeLayer(m_layer[2], m_strips[i]);
      }
      break;
    default:
    break;
  }
}
void ChasingSprites::Tick2()
{
  int numberOfBlanks = NUMLEDS/m_numberOfSprites;
  m_layer[0] = CRGB::Black;
  for(int j = 0; j < NUMLEDS; ++j)
  {
    if(j % numberOfBlanks == 0)
    {
      if(j + m_bassOffset < NUMLEDS)
      {
        m_layer[0][j + m_bassOffset] = bassColor;  
      }
    }
  }
  if(m_bassOffset < numberOfBlanks - 1)
  {
    m_bassOffset += 1; 
  }
  else
  {
    m_bassOffset = 0;
  }
}
void ChasingSprites::Tick3()
{
  int numberOfBlanks = NUMLEDS/m_numberOfSprites;  
  m_layer[1] = CRGB::Black;
  for(int j = 0; j < NUMLEDS; ++j)
  {
    if(j % numberOfBlanks == 0)
    {
      if(j + m_midOffset < NUMLEDS)
      {
        m_layer[1][j + m_midOffset] = midColor;  
      }
    }
  }
    if(m_midOffset < numberOfBlanks - 1)
    {
      m_midOffset += 1; 
    }
    else
    {
      m_midOffset = 0;
    }
  
}
void ChasingSprites::Tick4()
{
  int numberOfBlanks = NUMLEDS/m_numberOfSprites;
  m_layer[2] = CRGB::Black;
  for(int j = 0; j < NUMLEDS; ++j)
  {
    if(j % numberOfBlanks == 0)
    {
      if(j + m_highOffset < NUMLEDS)
      {
        m_layer[2][j + m_highOffset] = highColor;  
      }
    }
  }  
    if(m_highOffset < numberOfBlanks - 1)
    {
      m_highOffset += 1; 
    }
    else
    {
      m_highOffset = 0;
    }
  
}

void ChasingSprites::Random()
{
  m_numberOfSprites = random(1, NUMLEDS / 3 + 1);
  m_mergeType = random(2);
  while(!(NUMLEDS % m_numberOfSprites == 0) )
  {
    m_numberOfSprites = random(1, NUMLEDS / 3 + 1 );
  }
}
int ChasingSprites::GetNewBassTime()
{
  
  float  level = GetNormalizedSoundLevelForFrequencyRange(0*MAX_DISPLAYED_FREQ/3, 1*MAX_DISPLAYED_FREQ/3, 0, SoundLevelOutputType_Beat);
  int newTime;
  if(level >= SILENCE_THRESHOLD)
  {
    newTime = 5000 - (5000 * (1 - exp(-25*level)));
  }
  else
  {
    newTime = INT_MAX;
  }
  if(newTime < 0)
  {
    newTime = 0;
  }
  return newTime;
}
int ChasingSprites::GetNewMidTime()
{
  float  level = GetNormalizedSoundLevelForFrequencyRange(1*MAX_DISPLAYED_FREQ/3, 2*MAX_DISPLAYED_FREQ/3, 0, SoundLevelOutputType_Beat);
  int newTime;
  if(level >= SILENCE_THRESHOLD)
  {
    newTime = 5000 - (5000 * (1 - exp(-25*level)));
  }
  else
  {
    newTime = INT_MAX;
  }
  if(newTime < 0)
  {
    newTime = 0;
  }
  return newTime;
}
int ChasingSprites::GetNewHighTime()
{
  float  level = GetNormalizedSoundLevelForFrequencyRange(2*MAX_DISPLAYED_FREQ/3, 3*MAX_DISPLAYED_FREQ/3, 0, SoundLevelOutputType_Beat);
  int newTime;
  if(level >= SILENCE_THRESHOLD)
  {
    newTime = 5000 - (5000 * (1 - exp(-25*level)));
  }
  else
  {
    newTime = INT_MAX;
  }
  if(newTime < 0)
  {
    newTime = 0;
  }
  return newTime;
}
void ChasingSprites::End()
{
  Ended();
}

//********* FrequencyColorStreamer *********
 Visualizations* FrequencyColorStreamer::GetInstance( int duration
                                                    , StatisticalEngine &statisticalEngine
                                                    , VisualizationsCalleeInterface *callee )
{
  if(true == debugMode && debugLevel >= 2) Serial << "New FrequencyColorStreamer\n";
  return new FrequencyColorStreamer(duration, statisticalEngine, callee);
}
void FrequencyColorStreamer::Start()
{
  if(true == debugMode && debugLevel >= 1) Serial << "FrequencyColorStreamer Start\n";
  Started();
}
bool FrequencyColorStreamer::Loop() 
{
  if(false == m_visualizationStarted) {Start(); m_visualizationStarted = true;}
  bool updateRequired = false;
  unsigned long currentTime = millis();
  if(true == debugMode && debugLevel >= 3) Serial << "FrequencyColorStreamer Loop: " << currentTime-m_resetTimer << " | " << m_duration << "\n";
  if(currentTime-m_timers[0] >= 0) {ResetTimer(0); Tick1(); updateRequired = true;}
  if(currentTime-m_resetTimer >= m_duration) {End(); updateRequired = true;}
  return updateRequired;
}
void FrequencyColorStreamer::Tick1()
{
  int maxBinSave = 0;
  float  maxLevelSave = 0.0;
  for(int i = 0; i < BINS; ++i)
  {
    float  level = GetNormalizedSoundLevelForBin(i, 0, SoundLevelOutputType_Beat); 
    if(level > maxLevelSave)
    {
      maxLevelSave = level;
      maxBinSave = i;
    }
  }
  if(m_statisticalEngine.power >= SILENCE_THRESHOLD)
  {
    m_BottomColor = GetColor(maxBinSave, m_statisticalEngine.GetFFTBinIndexForFrequency(MAX_DISPLAYED_FREQ));
  }
  else
  {
    m_BottomColor = CRGB::Black;
  }
  ShiftStrip(m_layer[0], Up, 1);
  m_layer[0](0, 0) = CRGB((byte)(dim8_raw(m_BottomColor.red * maxLevelSave)), (byte)(dim8_raw(m_BottomColor.green * maxLevelSave)), (byte)(dim8_raw(m_BottomColor.blue * maxLevelSave)));
  for(int i = 0; i < NUMSTRIPS; ++i)
  {
    m_strips[i] = m_layer[0];
  }
}
void FrequencyColorStreamer::End()
{
  Ended();
}

//********* Visualization 17 *********
// 4 Freq Color spinning Tower
 Visualizations* FrequencyColorSpinningTower::GetInstance( int duration
                                                               , StatisticalEngine &statisticalEngine
                                                               , VisualizationsCalleeInterface *callee )
{
  if(true == debugMode && debugLevel >= 2) Serial << "New FrequencyColorSpinningTower\n";
  return new FrequencyColorSpinningTower(duration, statisticalEngine, callee);
}
void FrequencyColorSpinningTower::Start()
{
  if(true == debugMode && debugLevel >= 1) Serial << "FrequencyColorSpinningTower Start\n"; 
  m_countTime = 100;
  m_rotationCount = 0;
  Started();
}
bool FrequencyColorSpinningTower::Loop() 
{
  if(false == m_visualizationStarted) {Start(); m_visualizationStarted = true;}
  bool updateRequired = false;
  unsigned long currentTime = millis();
  if(true == debugMode && debugLevel >= 3) Serial << "FrequencyColorSpinningTower Loop: " << currentTime-m_resetTimer << " | " << m_duration << "\n";
  if(currentTime-m_timers[0] >= 0) {ResetTimer(0); Tick1(); updateRequired = true;}
  if(currentTime-m_timers[1] >= m_countTime) {ResetTimer(1); IncrementCount(); updateRequired = true;}
  if(currentTime-m_resetTimer >= m_duration) {End(); updateRequired = true;}
  return updateRequired;
}
void FrequencyColorSpinningTower::Tick1()
{
  float freqWidth =  MAX_DISPLAYED_FREQ / NUMSTRIPS;
  for(int i = 0; i < NUMSTRIPS; ++i)
  {
    int rotatedIndex = ((i+m_rotationCount)%NUMSTRIPS);
    float lowerFrequencyRange = rotatedIndex*freqWidth;
    float upperFrequencyRange = rotatedIndex*freqWidth+freqWidth;
    float  level = GetNormalizedSoundLevelForFrequencyRange(lowerFrequencyRange, upperFrequencyRange, 0, SoundLevelOutputType_Beat);    
    CRGB color = GetColor(rotatedIndex, NUMSTRIPS);
    if(true == debugMode && debugLevel >= 3) Serial << i << "|" << level << "\t";
    m_strips[i] = CRGB((byte)(dim8_raw(color.red*level)), (byte)(dim8_raw(color.green*level)), (byte)(dim8_raw(color.blue*level)));
  }
  if(true == debugMode && debugLevel >= 3) Serial << "\n";
}
void FrequencyColorSpinningTower::IncrementCount()
{
  ++m_rotationCount;
}
void FrequencyColorSpinningTower::End()
{
  Ended();
}

//********* UpDownFrequencyColorStreamer *********
 Visualizations* UpDownFrequencyColorStreamer::GetInstance( int duration
                                                          , StatisticalEngine &statisticalEngine
                                                          , VisualizationsCalleeInterface *callee )
{
  if(true == debugMode && debugLevel >= 2) Serial << "New UpDownFrequencyColorStreamer\n";
  return new UpDownFrequencyColorStreamer(duration, statisticalEngine, callee);
}
void UpDownFrequencyColorStreamer::Start()
{
  if(true == debugMode && debugLevel >= 1) Serial << "UpDownFrequencyColorStreamer Start\n";
  Started();
}
bool UpDownFrequencyColorStreamer::Loop() 
{
  if(false == m_visualizationStarted) {Start(); m_visualizationStarted = true;}
  bool updateRequired = false;
  unsigned long currentTime = millis();
  if(true == debugMode && debugLevel >= 3) Serial << "UpDownFrequencyColorStreamer Loop: " << currentTime-m_resetTimer << " | " << m_duration << "\n";
  if(currentTime-m_timers[0] >= 0) {ResetTimer(0); Tick1(); updateRequired = true;}
  if(currentTime-m_resetTimer >= m_duration) {End(); updateRequired = true;}
  return updateRequired;
}
void UpDownFrequencyColorStreamer::Tick1()
{
  int maxBinSave = 0;
  float  maxLevelSave = 0.0;
  ShiftStrip(m_layer[0], Up, 1);
  ShiftStrip(m_layer[1], Down, 1);
  for(int i = 0; i < BINS; ++i)
  {
    float  level = GetNormalizedSoundLevelForBin(i, 0, SoundLevelOutputType_Beat); 
    if(level > maxLevelSave)
    {
      maxLevelSave = level;
      maxBinSave = i;
    }
  }
  if(m_statisticalEngine.power >= SILENCE_THRESHOLD)
  {
    m_BottomColor = GetColor(maxBinSave, m_statisticalEngine.GetFFTBinIndexForFrequency(MAX_DISPLAYED_FREQ));
  }
  else
  {
    m_BottomColor = CRGB::Black;
  }
  if(m_Count % 2 == 0)
  {
    m_layer[0](0, 0) = CRGB((byte)(dim8_raw(m_BottomColor.red * maxLevelSave)), (byte)(dim8_raw(m_BottomColor.green * maxLevelSave)), (byte)(dim8_raw(m_BottomColor.blue * maxLevelSave)));
    m_layer[1](NUMLEDS - 2, NUMLEDS - 2) = CRGB::Black; 
  }
  else
  {
    m_layer[1](NUMLEDS - 2, NUMLEDS - 2) = CRGB((byte)(dim8_raw(m_BottomColor.red * maxLevelSave)), (byte)(dim8_raw(m_BottomColor.green * maxLevelSave)), (byte)(dim8_raw(m_BottomColor.blue * maxLevelSave)));
    m_layer[0](0, 0) = CRGB::Black; 
  }
  for(int i = 0; i < NUMSTRIPS; ++i)
  {
    m_strips[i] = AddLayer(m_layer[0], m_layer[1]);
  }
  ++m_Count;
}
void UpDownFrequencyColorStreamer::End()
{
  Ended();
}


//********* UpDownMaxFrequencyStreamer *********
 Visualizations* UpDownMaxFrequencyStreamer::GetInstance( int duration
                                                              , StatisticalEngine &statisticalEngine
                                                              , VisualizationsCalleeInterface *callee )
{
  if(true == debugMode && debugLevel >= 2) Serial << "New UpDownMaxFrequencyStreamer\n";
  return new UpDownMaxFrequencyStreamer(duration, statisticalEngine, callee);
}
void UpDownMaxFrequencyStreamer::Start()
{
  if(true == debugMode && debugLevel >= 1) Serial << "UpDownMaxFrequencyStreamer Start\n";
  Tick2();
  Started();
}
bool UpDownMaxFrequencyStreamer::Loop() 
{
  if(false == m_visualizationStarted) {Start(); m_visualizationStarted = true;}
  bool updateRequired = false;
  unsigned long currentTime = millis();
  if(true == debugMode && debugLevel >= 3) Serial << "UpDownMaxFrequencyStreamer Loop: " << currentTime-m_resetTimer << " | " << m_duration << "\n";
  if(currentTime-m_timers[0] >= 0) {ResetTimer(0); Tick1(); updateRequired = true;}
  if(currentTime-m_timers[1] >= m_colorTime) {ResetTimer(1); Tick2(); updateRequired = true;}
  if(currentTime-m_resetTimer >= m_duration) {End(); updateRequired = true;}
  return updateRequired;
}
void UpDownMaxFrequencyStreamer::Tick1()
{
  ShiftStrip(m_layer[0], Up, 1);
  ShiftStrip(m_layer[1], Down, 1);
  float  level = GetNormalizedSoundLevelForBin(m_maxBin, 0, SoundLevelOutputType_Beat);
  if(m_Count % 2 == 0)
  {
    m_layer[0](0, 0) = CRGB((byte)(dim8_raw(m_Color.red * level)), (byte)(dim8_raw(m_Color.green * level)), (byte)(dim8_raw(m_Color.blue * level)));
    m_layer[1](NUMLEDS - 2, NUMLEDS - 2) = CRGB::Black; 
  }
  else
  {
    m_layer[1](NUMLEDS - 2, NUMLEDS - 2) = CRGB((byte)(dim8_raw(m_Color.red * level)), (byte)(dim8_raw(m_Color.green * level)), (byte)(dim8_raw(m_Color.blue * level)));
    m_layer[0](0, 0) = CRGB::Black; 
  }
  for(int i = 0; i < NUMSTRIPS; ++i)
  {
    m_strips[i] = AddLayer(m_layer[0], m_layer[1]);
  }
  ++m_Count;
}
void UpDownMaxFrequencyStreamer::Tick2()
{
  float  maxLevelSave = 0.0;
  m_maxBin = 0;
  if (m_statisticalEngine.power > SILENCE_THRESHOLD)
  {
    for(int i = 0; i < BINS; ++i)
    {
      float  level = GetNormalizedSoundLevelForBin(m_maxBin, 0, SoundLevelOutputType_Beat);
      if(level > maxLevelSave)
      {
        maxLevelSave = level;
        m_maxBin = i;
      }
    }
    m_Color = GetColor(m_maxBin, m_statisticalEngine.GetFFTBinIndexForFrequency(MAX_DISPLAYED_FREQ));
  }
  else
  {
    m_Color = CRGB::Black;
  }
}
void UpDownMaxFrequencyStreamer::End()
{
  Ended();
}

//********* ScrollingRainbow *********
 Visualizations* ScrollingRainbow::GetInstance( int duration
                                              , StatisticalEngine &statisticalEngine
                                              , VisualizationsCalleeInterface *callee )
{
  if(true == debugMode && debugLevel >= 2) Serial << "New ScrollingRainbow\n";
  return new ScrollingRainbow(duration, statisticalEngine, callee);
}
void ScrollingRainbow::Start()
{
  if(true == debugMode && debugLevel >= 1) Serial << "ScrollingRainbow Start\n";
  Tick3();
  Tick4();
  m_desiredRenderCount0 = NUMLEDS;
  m_desiredRenderCount1 = NUMLEDS;
  m_desiredRenderCount2 = NUMLEDS;
  Started();
}
bool ScrollingRainbow::Loop() 
{
  bool updateRequired = false;
  unsigned long currentTime = millis();
  if(false == m_visualizationStarted) {Start(); m_visualizationStarted = true;}
  if(true == debugMode && debugLevel >= 3) Serial << "ScrollingRainbow Loop: " << currentTime-m_resetTimer << " | " << m_duration << "\n";
  if(currentTime-m_timers[0] >= 0) {ResetTimer(0); Tick0(); updateRequired = true;}
  if(m_renderCount0 % m_desiredRenderCount0 == 0) {Tick1(); updateRequired = true;}
  if(m_renderCount0 % m_desiredRenderCount0 == 0) {Tick2(); updateRequired = true;}
  if(currentTime-m_timers[1] >= m_randomTime1) {ResetTimer(1); Tick3(); updateRequired = true;}
  if(currentTime-m_timers[2] >= m_randomTime2) {ResetTimer(2); Tick4(); updateRequired = true;}
  if(currentTime-m_resetTimer > m_duration) {End(); updateRequired = true;}
  return updateRequired;
}
void ScrollingRainbow::Tick0()
{
  SetFadeToColor(m_fadeController0, m_currentColor1, m_currentColor2, m_desiredRenderCount0);
  for(int i = 0; i < NUMLEDS; ++i)
  {
    m_layer[0][i] = FadeColor(m_fadeController0);
    ++m_renderCount0;
  }
  for(int i = 0; i < NUMSTRIPS; ++i)
  {
    m_strips[i] = m_layer[0]; 
  }
}
void ScrollingRainbow::Tick1()
{
  m_currentColor1 = FadeColor(m_fadeController1);
  ++m_renderCount1;
}
void ScrollingRainbow::Tick2()
{
  m_currentColor2 = FadeColor(m_fadeController2);
  ++m_renderCount2;
}
void ScrollingRainbow::Tick3()
{
  m_randomTime1 = random(0, m_maxTime);
  m_fadeToColor1 = GetColor(random(0,m_numColors), m_numColors);
  Tick5();
}
void ScrollingRainbow::Tick4()
{
  m_randomTime2 = random(0, m_maxTime);
  m_fadeToColor2 = GetColor(random(0,m_numColors), m_numColors);
  Tick5();
}
void ScrollingRainbow::Tick5()
{
  m_fadeToColor1 = GetColor(random(0,m_numColors), m_numColors);
  SetFadeToColor(m_fadeController1, m_currentColor1, m_fadeToColor1, m_desiredRenderCount1);
  m_fadeToColor2 = GetColor(random(0,m_numColors), m_numColors);
  SetFadeToColor(m_fadeController2, m_currentColor2, m_fadeToColor2, m_desiredRenderCount2);
}
void ScrollingRainbow::End()
{
  Ended();
}

//********* ScrollingFrequencyColorRectangles *********
 Visualizations* ScrollingFrequencyColorRectangles::GetInstance( int duration
                                                                     , StatisticalEngine &statisticalEngine
                                                                     , VisualizationsCalleeInterface *callee )
{
  if(true == debugMode && debugLevel >= 2) Serial << "New ScrollingFrequencyColorRectangles\n";
  return new ScrollingFrequencyColorRectangles(duration, statisticalEngine, callee);
}
void ScrollingFrequencyColorRectangles::Start()
{
  if(true == debugMode && debugLevel >= 1) Serial << "ScrollingFrequencyColorRectangles Start\n";
  Tick2();
  for(int i = 0; i < NUMLEDS; ++i)
  {
    Tick1();
    if(m_renderCount >= m_colorLength)
    {
      m_renderCount = 0;
      Tick2();
    }
  }
  Started();
}
bool ScrollingFrequencyColorRectangles::Loop() 
{
  if(false == m_visualizationStarted) {Start(); m_visualizationStarted = true;}
  bool updateRequired = false;
  unsigned long currentTime = millis();
  if(true == debugMode && debugLevel >= 3) Serial << "ScrollingFrequencyColorRectangles Loop: " << currentTime-m_resetTimer << " | " << m_duration << "\n";
  if(currentTime-m_timers[0] >= 25) {ResetTimer(0); Tick1(); updateRequired = true;}
  if(m_renderCount >= m_colorLength) {m_renderCount = 0; Tick2(); updateRequired = true;}
  if(currentTime-m_resetTimer > m_duration) {End(); updateRequired = true;}
  return updateRequired;
}
void ScrollingFrequencyColorRectangles::Tick1()
{
  m_gain = m_statisticalEngine.power;
  switch(m_direction)
  {
    case 0:
      ShiftStrip(m_layer[0], Up, 1);
      m_layer[0][0] = m_currentColor;
    break;
    case 1:
      ShiftStrip(m_layer[0], Down, 1);
      m_layer[0][NUMLEDS-1] = m_currentColor;
    break;
    default:
    break;
  }
  for(int i = 0; i < NUMLEDS; ++i)
  {
    m_layer[1][i] = CRGB {(byte)dim8_raw(m_backgroundColor.red * m_gain), (byte)dim8_raw(m_backgroundColor.green * m_gain), (byte)dim8_raw(m_backgroundColor.blue * m_gain)}; 
  }
  for(int i = 0; i < NUMSTRIPS; ++i)
  {
    m_strips[i] = MergeLayer(m_layer[0], m_layer[1]);
  }
  ++m_renderCount;
}
void ScrollingFrequencyColorRectangles::Tick2()
{
  int maxBinSave = 0;
  float  maxLevelSave = 0.0;
  if (m_statisticalEngine.power > SILENCE_THRESHOLD)
  {
    for(int i = 0; i < BINS; ++i)
    {
      float  level = GetNormalizedSoundLevelForBin(i, 0, SoundLevelOutputType_Beat);
      if(level > maxLevelSave)
      {
        maxLevelSave = level;
        maxBinSave = i;
      }
    }
    m_detectedColor = GetColor(maxBinSave, m_statisticalEngine.GetFFTBinIndexForFrequency(MAX_DISPLAYED_FREQ));
  }
  else
  {
    m_detectedColor = CRGB::Black;
  }
  int count = m_currentColorCount%m_numColors;
  switch(count)
  {
    case 0:
      m_currentColor = m_detectedColor;
    break;
    case 1:
      m_currentColor = CRGB::Black;
    break;
    default:
      m_currentColor = CRGB::Black;
    break;
  }
  ++m_currentColorCount;
}
void ScrollingFrequencyColorRectangles::End()
{
  Ended();
}


//********* ScrollingFrequencySprites *********
 Visualizations* ScrollingFrequencySprites::GetInstance( int duration
                                                             , StatisticalEngine &statisticalEngine
                                                             , VisualizationsCalleeInterface *callee )
{
  if(true == debugMode && debugLevel >= 2) Serial << "New ScrollingFrequencySprites\n";
  return new ScrollingFrequencySprites(duration, statisticalEngine, callee);
}
void ScrollingFrequencySprites::Start()
{
  if(true == debugMode && debugLevel >= 1) Serial << "ScrollingFrequencySprites Start\n";
  for(int i = 0; i < m_numberOfSprites; ++i)
  {
    m_sprites[i].position = i*(NUMLEDS/m_numberOfSprites);
    m_sprites[i].color = GetColor(i, m_numberOfSprites);
  }
  Started();
}
bool ScrollingFrequencySprites::Loop() 
{
  if(false == m_visualizationStarted) {Start(); m_visualizationStarted = true;}
  bool updateRequired = false;
  unsigned long currentTime = millis();
  if(true == debugMode && debugLevel >= 3) Serial << "ScrollingFrequencySprites Loop: " << currentTime-m_resetTimer << " | " << m_duration << "\n";
  if(currentTime-m_timers[0] >= 0) {ResetTimer(0); Tick1(); updateRequired = true;}
  if(currentTime-m_timers[1] >= 25) {ResetTimer(1); Tick2(); updateRequired = true;}
  if(currentTime-m_resetTimer >= m_duration) {End(); updateRequired = true;}
  return updateRequired;
}
void ScrollingFrequencySprites::Tick1()
{
  float freqWidth =  MAX_DISPLAYED_FREQ / m_numberOfSprites;
  for(int i = 0; i < m_numberOfSprites; ++i)
  {
    float lowerFrequencyRange = i*freqWidth;
    float upperFrequencyRange = i*freqWidth+freqWidth;  
    m_sprites[i].normalizedPower = GetNormalizedSoundLevelForFrequencyRange(lowerFrequencyRange, upperFrequencyRange, 0, SoundLevelOutputType_Beat);
  }
  for(int i = 0; i < m_numberOfSprites; ++i)
  {
    m_layer[0][m_sprites[i].position] = CRGB { (byte)dim8_raw(m_sprites[i].color.red * m_sprites[i].normalizedPower)
                                             , (byte)dim8_raw(m_sprites[i].color.green * m_sprites[i].normalizedPower)
                                             , (byte)dim8_raw(m_sprites[i].color.blue * m_sprites[i].normalizedPower) };
  }
  for(int i = 0; i < NUMSTRIPS; ++i)
  {
    m_strips[i] = m_layer[0];
  }
}
void ScrollingFrequencySprites::Tick2()
{
  for(int i = 0; i < m_numberOfSprites; ++i)
  {
    m_sprites[i].position += 1;
    if(m_sprites[i].position >= NUMLEDS)
    {
      m_sprites[i].position = 0;
    }
  }
}
void ScrollingFrequencySprites::End()
{
  Ended();
}


//********* ScrollingSpeedFrequencySprites *********
 Visualizations* ScrollingSpeedFrequencySprites::GetInstance( int duration
                                                                  , StatisticalEngine &statisticalEngine
                                                                  , VisualizationsCalleeInterface *callee )
{
  if(true == debugMode && debugLevel >= 2) Serial << "New ScrollingSpeedFrequencySprites\n";
  return new ScrollingSpeedFrequencySprites(duration, statisticalEngine, callee);
}
void ScrollingSpeedFrequencySprites::Start()
{
  if(true == debugMode && debugLevel >= 1) Serial << "ScrollingSpeedFrequencySprites Start\n";
  for(int i = 0; i < m_numberOfSprites; ++i)
  {
    m_sprites[i].position = i*(NUMLEDS/m_numberOfSprites);
    m_sprites[i].color = GetColor(i, m_numberOfSprites);
  }
  Started();
}
bool ScrollingSpeedFrequencySprites::Loop() 
{
  if(false == m_visualizationStarted) {Start(); m_visualizationStarted = true;}
  bool updateRequired = false;
  unsigned long currentTime = millis();
  if(true == debugMode && debugLevel >= 3) Serial << "ScrollingSpeedFrequencySprites Loop: " << currentTime-m_resetTimer << " | " << m_duration << "\n";
  if(currentTime-m_timers[0] >= 0) {ResetTimer(0); Tick1(); updateRequired = true;}
  if(currentTime-m_resetTimer >= m_duration) {End(); updateRequired = true;}
  return updateRequired;
}
void ScrollingSpeedFrequencySprites::Tick1()
{
  float freqWidth =  MAX_DISPLAYED_FREQ / m_numberOfSprites;
  unsigned long currentTime = millis();
  for(int i = 0; i < m_numberOfSprites; ++i)
  {
    float lowerFrequencyRange = i*freqWidth;
    float upperFrequencyRange = i*freqWidth+freqWidth; 
    float  level = GetNormalizedSoundLevelForFrequencyRange(lowerFrequencyRange, upperFrequencyRange, 0, SoundLevelOutputType_Beat);
    if(i == 0 || i == 1)
    {
      if(level >= 1.0)
      {
        switch(m_sprites[i].direction)
        {
          case DIRECTION_UP:
          m_sprites[i].direction = DIRECTION_DOWN;
          break;
          case DIRECTION_DOWN:
          m_sprites[i].direction = DIRECTION_UP;
          break;
          default:
          m_sprites[i].direction = DIRECTION_DOWN;
          break;
        }
      }
    }
    if(level < 0.0) level = 0.0;
    if(level > 1.0) level = 1.0;
    m_sprites[i].normalizedPower = level;
    unsigned long newTime = currentTime + (500-500*level);
    if( newTime < m_sprites[i].timer )
    {
      m_sprites[i].timer = newTime;
    }
    if(level >= SILENCE_THRESHOLD)
    {      
      if(currentTime >= m_sprites[i].timer)
      {
        m_sprites[i].timer = newTime;
        switch(m_sprites[i].direction)
        {
          case DIRECTION_UP:
            m_sprites[i].position += 1;
            if(m_sprites[i].position >= NUMLEDS)
            {
              m_sprites[i].position = 0;
            }
          break;
          case DIRECTION_DOWN:
            m_sprites[i].position -= 1;
            if(m_sprites[i].position < 0)
            {
              m_sprites[i].position = NUMLEDS - 1;
            }
          break;
          default:
          break;
        }
      } 
    }
  }
  for(int i = m_numberOfSprites - 1; i >= 0; --i)
  {
    m_layer[0][m_sprites[i].position] = CRGB { (byte)dim8_raw(m_sprites[i].color.red * m_sprites[i].normalizedPower)
                                             , (byte)dim8_raw(m_sprites[i].color.green * m_sprites[i].normalizedPower)
                                             , (byte)dim8_raw(m_sprites[i].color.blue * m_sprites[i].normalizedPower) };
  }
  for(int i = 0; i < NUMSTRIPS; ++i)
  {
    m_strips[i] = m_layer[0];
  }
}
void ScrollingSpeedFrequencySprites::End()
{
  Ended();
}


//********* ScrollingAmplitudeSprite *********
 Visualizations* ScrollingAmplitudeSprite::GetInstance( int duration
                                                            , StatisticalEngine &statisticalEngine
                                                            , VisualizationsCalleeInterface *callee )
{
  if(true == debugMode && debugLevel >= 2) Serial << "New ScrollingAmplitudeSprite\n";
  return new ScrollingAmplitudeSprite(duration, statisticalEngine, callee);
}
void ScrollingAmplitudeSprite::Start()
{
  if(true == debugMode && debugLevel >= 1) Serial << "ScrollingAmplitudeSprite Start\n";
  Random();
  Started();
}

bool ScrollingAmplitudeSprite::Loop() 
{
  bool updateRequired = false;
  unsigned long currentTime = millis();
  if(false == m_visualizationStarted) {Start(); m_visualizationStarted = true;}
  if(true == debugMode && debugLevel >= 3) Serial << "ScrollingAmplitudeSprite Loop: " << currentTime-m_resetTimer << " | " << m_duration << "\n";
  if(currentTime-m_timers[0] >= 0) {ResetTimer(0); Tick1(); updateRequired = true;}
  if(currentTime-m_timers[1] >= m_randomTime) {ResetTimer(1); Random(); updateRequired = true;}
  if(currentTime-m_resetTimer >= m_duration) {End(); updateRequired = true;}
  return updateRequired;
}

void ScrollingAmplitudeSprite::Tick1()
{
  float  level = m_statisticalEngine.power;
  m_sprite.color = FadeColor(m_fadeController);
  if(m_count % m_waitCount == 0)
  {
    switch(m_sprite.direction)
    {
      case DIRECTION_UP:
        ++m_sprite.position;
        if(m_sprite.position >= NUMLEDS - 1)
        {
          m_sprite.position = NUMLEDS - 1;
          m_sprite.direction = DIRECTION_DOWN;
        }
      break;
      case DIRECTION_DOWN:
        --m_sprite.position;
        if(m_sprite.position <= 0)
        {
          m_sprite.position = 0;
          m_sprite.direction = DIRECTION_UP;
        }
      break;
      default:
      break;
    }
    m_layer[0][m_sprite.position, m_sprite.position] = CRGB { (byte)dim8_raw(m_sprite.color.red * level)
                                                            , (byte)dim8_raw(m_sprite.color.green * level)
                                                            , (byte)dim8_raw(m_sprite.color.blue * level) };    
  }
  for(int i = 0; i < NUMSTRIPS; ++i)
  {
    m_strips[i] = m_layer[0];
  }
  ++m_count;
  if(m_count >= m_waitCount) m_count = 0;
}

void ScrollingAmplitudeSprite::Random()
{
  m_waitCount = random(1,(BIN_SAVE_LENGTH/10) + 1);
  m_randomTime = random(5000, m_maxRandomTime);
  m_fadeToColor = GetRandomNonGrayColor();
  SetFadeToColor(m_fadeController, m_sprite.color, m_fadeToColor, m_fadeLength*m_waitCount);
  m_sprite.color = GetRandomNonGrayColor();
}

void ScrollingAmplitudeSprite::End()
{
  Ended();
}



//********* Opposites *********
 Visualizations* Opposites::GetInstance( int duration
                                       , StatisticalEngine &statisticalEngine
                                       , VisualizationsCalleeInterface *callee )
{
  if(true == debugMode && debugLevel >= 2) Serial << "New Opposites\n";
  return new Opposites(duration, statisticalEngine, callee);
}
void Opposites::Start()
{
  if(true == debugMode && debugLevel >= 1) Serial << "ScrollingAmplitudeSprite Start\n";
  Random();
  Started();
}

bool Opposites::Loop() 
{
  if(false == m_visualizationStarted) {Start(); m_visualizationStarted = true;}
  bool updateRequired = false;
  unsigned long currentTime = millis();
  if(true == debugMode && debugLevel >= 3) Serial << "Opposites Loop: " << currentTime-m_resetTimer << " | " << m_duration << "\n";
  if(currentTime-m_timers[0] >= 200) {ResetTimer(0); Tick1(); updateRequired = true;}
  if(currentTime-m_timers[1] >= m_randomTime) {ResetTimer(1); Random(); updateRequired = true;}
  if(currentTime-m_resetTimer >= m_duration) {End(); updateRequired = true;}
  return updateRequired;
}

void Opposites::Tick1()
{
  int maxBinSave = 0;
  int minBinSave = 0;
  float  maxLevelSave = 0.0;
  float  minLevelSave = 1.0;
  CRGB colorMax;
  CRGB colorMin;
  if (m_statisticalEngine.power >= SILENCE_THRESHOLD)
  {
    for(int i = 0; i < BINS; ++i)
    {
      float  level = GetNormalizedSoundLevelForBin(i, 0, SoundLevelOutputType_Beat);
      if(level >= maxLevelSave)
      {
        maxLevelSave = level;
        maxBinSave = i;
      }
      if(level <= minLevelSave)
      {
        minLevelSave = level;
        minBinSave = i;
      }
    }
    colorMax = GetColor(maxBinSave, m_statisticalEngine.GetFFTBinIndexForFrequency(MAX_DISPLAYED_FREQ));
    colorMin = GetColor(minBinSave, m_statisticalEngine.GetFFTBinIndexForFrequency(MAX_DISPLAYED_FREQ));
    int count = 0;
    for(int i = 0; i < NUMLEDS; i = i + m_barLength)
    {
      int bottom = i;
      int top = i + m_barLength;
      if(top > NUMLEDS - 1) top = NUMLEDS - 1;
      if(count % 2 == 0)
      {
        if(true == debugMode && debugLevel >= 3) Serial << "Even: " << bottom << " | " << top << "\t";
        m_layer[0](bottom, top) = colorMax;
      }
      else
      {
        if(true == debugMode && debugLevel >= 3) Serial << "Odd: " << bottom << " | " << top << "\t";
        m_layer[0](i, i + m_barLength) = colorMin;
      }
      ++count;
    }
  }
  else
  {
    m_layer[0] = CRGB::Black;
  }
  if(true == debugMode && debugLevel >= 3) Serial << "\n";
  for(int i = 0; i < NUMSTRIPS; ++i)
  {
    m_strips[i] = m_layer[0];
  }
}

void Opposites::Random()
{
  m_barLength = random(1, NUMLEDS/4);
  m_randomTime = random(1000,30000);
}

void Opposites::End()
{
  Ended();
}


//********* Snake *********
 Visualizations* Snake::GetInstance( int duration
                                   , StatisticalEngine &statisticalEngine
                                   , VisualizationsCalleeInterface *callee )
{
  if(true == debugMode && debugLevel >= 2) Serial << "New Snake\n";
  return new Snake(duration, statisticalEngine, callee);
}
void Snake::Start()
{
  if(true == debugMode && debugLevel >= 1) Serial << "Snake Start\n";
  Started();
}

bool Snake::Loop() 
{
  if(false == m_visualizationStarted) {Start(); m_visualizationStarted = true;}
  bool updateRequired = false;
  unsigned long currentTime = millis();
  if(true == debugMode && debugLevel >= 3) Serial << "Opposites Loop: " << currentTime-m_resetTimer << " | " << m_duration << "\n";
  if(currentTime-m_timers[0] >= 0) {ResetTimer(0); Tick1(); updateRequired = true;}
  if(currentTime-m_resetTimer >= m_duration) {End(); updateRequired = true;}
  return updateRequired;
}

void Snake::Tick1()
{
  int maxBinSave = 0;
  float  maxLevelSave = 0.0;
  ShiftStrip(m_layer[0], Up, 1);
  ShiftStrip(m_layer[1], Down, 1);
  ShiftStrip(m_layer[2], Up, 1);
  ShiftStrip(m_layer[3], Down, 1);
  m_layer[1](NUMLEDS-1,NUMLEDS-1) = m_layer[0](NUMLEDS-1,NUMLEDS-1);
  m_layer[2](0,0) = m_layer[1](0,0);
  m_layer[3](NUMLEDS-1,NUMLEDS-1) = m_layer[2](NUMLEDS-1,NUMLEDS-1);
  for(int i = 0; i < BINS; ++i)
  {
    float  level = GetNormalizedSoundLevelForBin(i, 0, SoundLevelOutputType_Beat);
    if(level > maxLevelSave)
    {
      maxLevelSave = level;
      maxBinSave = i;
    }
  }    
  CRGB color = GetColor(maxBinSave, m_statisticalEngine.GetFFTBinIndexForFrequency(MAX_DISPLAYED_FREQ));
  color.red = color.red * maxLevelSave;
  color.blue = color.blue * maxLevelSave;
  color.green = color.green * maxLevelSave;
  m_layer[0](0,0) = color;
  if (m_statisticalEngine.power>SILENCE_THRESHOLD)
  {
    m_layer[0](0,0) = color;
  }
  else
  {
    m_layer[0](0,0) = CRGB::Black;
  }
  m_strips[0] = m_layer[0];
  m_strips[1] = m_layer[1];
  m_strips[2] = m_layer[2];
  m_strips[3] = m_layer[3];
}

void Snake::End()
{
  Ended();
}
