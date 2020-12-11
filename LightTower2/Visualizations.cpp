#include "Visualizations.h"

//********* BandAmplitudes *********
void BandAmplitudes::Start()
{
  if(true == debugMode && debugLevel >= 1) Serial << "BandAmplitudes Start\n";
  Started();
}

bool BandAmplitudes::Loop() 
{
  if(false == m_visualizationStarted) {Start(); m_visualizationStarted = true;}
  bool updateRequired = false;
  unsigned long currentTime = millis();
  if(true == debugMode && debugLevel >= 3) Serial << "BandAmplitudes Loop: " << currentTime-m_resetTimer << " | " << m_duration << "\n";
  if(currentTime-m_timers[0] >= 0) {ResetTimer(0); Tick1(); updateRequired = true;}
  if(currentTime-m_resetTimer >= m_duration) {End(); updateRequired = true;}
  return updateRequired;
}

void BandAmplitudes::Tick1()
{  
  int numBands = NUM_BANDS;
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
    float  level = GetNormalizedSoundLevelDbForFrequencyRange(lowerFrequencyRange, upperFrequencyRange, 5, SoundLevelOutputType_Level);
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
}

void BandAmplitudes::End()
{
  Ended();
}
