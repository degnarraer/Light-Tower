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
 * @file Visualizations.h
 *
 *
 */

#ifndef Visualizations_H
#define Visualizations_H
#include "LEDControllerInterface.h"
#include "Statistical_Engine.h"

enum VisualizationEntries
{ 
  VisualizationEntries_Nothing,
 
  VisualizationEntries_CommandsStart,
  VisualizationEntries_GetNext,
  VisualizationEntries_GetPrevious,
  VisualizationEntries_GetRandom,
  VisualizationEntries_GetForeground,
  VisualizationEntries_GetBackground,
  VisualizationEntries_GetStandby,
  VisualizationEntries_GetConfirmation,
  VisualizationEntries_CommandsEnd,
  
  VisualizationEntries_TransitionStart,
  VisualizationEntries_InstantSwitch,
  VisualizationEntries_FadeTransition,
  VisualizationEntries_MixerAddTransition,
  VisualizationEntries_MixerMergeTransition,
  VisualizationEntries_SlideUpTransition,
  VisualizationEntries_SlideDownTransition,
  VisualizationEntries_SplitTransition,
  VisualizationEntries_TransitionEnd,
  
  VisualizationEntries_VisualizationStart,
  VisualizationEntries_SoundDetectionTester,
  VisualizationEntries_ColorFadingTower,
  VisualizationEntries_Confirmation,
  VisualizationEntries_WaterFallFireStreamer,
  VisualizationEntries_SolidColorTower,
  VisualizationEntries_FadingSolidColorTower,
  VisualizationEntries_PowerBarWithBassSprite,
  VisualizationEntries_RandomFrequencySprites,
  VisualizationEntries_FFTAmplitudes,
  VisualizationEntries_FrequencySpriteSpiral,
  VisualizationEntries_RandomHighLowFrequencyAmplitudeStreamer,
  VisualizationEntries_OutwardAmplitudeWithFloatingBassSprites,
  VisualizationEntries_VerticalFFTAmplitudeTower,
  VisualizationEntries_MultiRangeAmplitudeTower,
  VisualizationEntries_SimultaneousFrequencyStreamer,
  VisualizationEntries_MinMaxAmplitude,
  VisualizationEntries_ChasingSprites,
  VisualizationEntries_FrequencyColorStreamer,
  VisualizationEntries_FrequencyColorSpinningTower,
  VisualizationEntries_UpDownFrequencyColorStreamer,
  VisualizationEntries_UpDownMaxFrequencyStreamer,
  VisualizationEntries_ScrollingRainbow,
  VisualizationEntries_FadingColors2,
  VisualizationEntries_ScrollingFrequencyColorRectangles,
  VisualizationEntries_ScrollingFrequencySprites,
  VisualizationEntries_ScrollingSpeedFrequencySprites,
  VisualizationEntries_ScrollingAmplitudeSprite,
  VisualizationEntries_Opposites,
  VisualizationEntries_Snake,
  VisualizationEntries_VisualizationEnd
};

enum VisualizationType
{
  TRANSITION,
  VISUALIZATION,
  BACKGROUND,
  FOREGROUND,
  CONFIRMATION,
  TUNER,
  STANDBY
};

enum ShiftDirection
{
  Down,
  Up
};

enum VisualizationState
{
  VISUALIZATION_STATE_STARTED,
  VISUALIZATION_STATE_STOPPED
};

enum Direction
{
  DIRECTION_UP = 0,
  DIRECTION_DOWN,
  DIRECTION_COUNT
};

struct Sprite
{
  CRGB color = CRGB::Black;
  float power = 0;
  float normalizedPower = 0;
  int position = 0;
  unsigned long timer = 0;
  Direction direction = DIRECTION_DOWN;
};

class Visualizations;
class VisualizationsCalleeInterface
{
  public:
    virtual Visualizations *GetCurrentVisualizationPtr() = 0;
    virtual Visualizations *GetPreviousVisualizationPtr() = 0;
    virtual CRGB GetConfirmationColor() = 0;
  
    virtual void VisualizationStarted(Visualizations *visualization) = 0;
    virtual void VisualizationEnded(Visualizations *visualization) = 0;
    virtual void TransitionStarted(Visualizations *visualization) = 0;
    virtual void TransitionEnded(Visualizations *visualization) = 0;
    virtual void ConfirmationVisualizationStarted(Visualizations *visualization) = 0;
    virtual void ConfirmationVisualizationEnded(Visualizations *visualization) = 0;  
};

class VisualizationsCaller
{
  public:
    VisualizationsCaller( VisualizationsCalleeInterface *callee )
                        : m_callee(callee){};
    virtual ~VisualizationsCaller(){}
    VisualizationsCalleeInterface *m_callee;
};

class Visualizations: public VisualizationsCaller
                    , public LEDControllerInterface
{
  public:
    Visualizations( int duration
                  , VisualizationType visualizationType
                  , StatisticalEngine &statisticalEngine
                  , VisualizationsCalleeInterface *callee )
                  : m_duration(duration)
                  , m_visualizationType( visualizationType )
                  , m_statisticalEngine( statisticalEngine )
                  , VisualizationsCaller(callee)
    {
      //if(true == debugMode && debugLevel >= 0) Serial << "Visualizations: Instantiate\n";
      SetAllLayersToColor(CRGB::Black);
      ResetAllTimers();
      for(int i = 0; i < NUMSTRIPS; ++i)
      {
        m_strips[i] = CRGB::Black;
      }
    }
    virtual ~Visualizations()
    {
      if(true == debugMode && debugLevel >= 2) Serial << "Delete Visualizations\n";
    }
    virtual void Start() = 0;
    virtual bool Loop() = 0;
    virtual void End() = 0;
    virtual void Started();
    virtual void Ended();
    LEDStrip* GetStrips() { return m_strips; }
    
    enum SoundLevelOutputType
    {
      SoundLevelOutputType_Level,
      SoundLevelOutputType_Beat,
    };
    
    float GetNormalizedSoundLevelDbForBin(int aBin, int aDepth, SoundLevelOutputType aLevelType)
    {
      float amp = 0.0;
      float triggerLevel = 0.0;
      float maximum = 0.0;
      float ampDb = 0.0;
      float triggerLevelDb = 0.0;
      float maximumDb = 0.0;
      float normalizedLevel = 0.0;
      
      switch(aLevelType)
      {
        case SoundLevelOutputType_Level:
          amp = m_statisticalEngine.GetFFTBinRMS(aBin, aDepth, BinDataType::INSTANT);
        break;
        case SoundLevelOutputType_Beat:
          amp = m_statisticalEngine.GetFFTBinAverage(aBin, aDepth, BinDataType::INSTANT);
          triggerLevel = m_statisticalEngine.GetFFTBinAverage(aBin, BIN_SAVE_LENGTH-1, BinDataType::AVERAGE)*triggerLevelGain;
          maximum = m_statisticalEngine.GetFFTBinMax(aBin, BIN_SAVE_LENGTH-1, BinDataType::AVERAGE);
        break;
      }
      if(amp > 0)
      {
        ampDb = 20*log10(amp);
      }
      else
      {
        ampDb = 0.0;
      }
      if(triggerLevel > 0)
      {
        triggerLevelDb = 20*log10(triggerLevel);
      }
      else
      {
        triggerLevelDb = 0.0;
      }
      if(maximum > 0)
      {
        maximumDb = 20*log10(maximum);
      }
      else
      {
        maximumDb = 0.0;
      }
      switch(aLevelType)
      {
        case SoundLevelOutputType_Level:
          normalizedLevel = (ampDb) / MAX_DB;
        break;
        case SoundLevelOutputType_Beat:
          if(ampDb > triggerLevelDb) 
          {
            normalizedLevel = ((ampDb-triggerLevelDb) / maximumDb);
          }
          else 
          {
            normalizedLevel = 0.0;
          }
        break;
      }
      if(normalizedLevel < 0.0) normalizedLevel = 0.0;
      if(normalizedLevel > 1.0) normalizedLevel = 1.0;
      if((true == debugMode && debugLevel >= 5) || (true == debugNanInf && ( isnan(normalizedLevel) || isinf(normalizedLevel)))) Serial << "GetNormalizedSoundLevelDbForBin: \tBin: " << aBin << "\tDepth: " << aDepth << "\tLevel:" << ampDb << "\Average: " << triggerLevelDb << "\tNormalized Level: " << normalizedLevel << "\n";
      return normalizedLevel;
    }
    
    float GetNormalizedSoundLevelForBin(int aBin, int aDepth, SoundLevelOutputType aLevelType)
    {
      float normalizedLevel = 0.0;
      float amp = 0.0;
      float triggerLevel = 0.0;
      float maximum = 0.0;
      switch(aLevelType)
      {
        case SoundLevelOutputType_Level:
          amp = m_statisticalEngine.GetFFTBinRMS(aBin, aDepth, BinDataType::INSTANT);
          normalizedLevel = (amp) / ADDBITS;
        break;
        case SoundLevelOutputType_Beat:
          amp = m_statisticalEngine.GetFFTBinRMS(aBin, aDepth, BinDataType::INSTANT);
          triggerLevel = m_statisticalEngine.GetFFTBinAverage(aBin, BIN_SAVE_LENGTH-1, BinDataType::AVERAGE)*triggerLevelGain;
          maximum = m_statisticalEngine.GetFFTBinMax(aBin, BIN_SAVE_LENGTH-1, BinDataType::AVERAGE);
          if(amp > triggerLevel) 
          {
            normalizedLevel = (amp-triggerLevel) / maximum;
          }
          else 
          {
            normalizedLevel = 0.0;
          }
        break;
      }
      if(normalizedLevel < 0.0) normalizedLevel = 0.0;
      if(normalizedLevel > 1.0) normalizedLevel = 1.0;
      if((true == debugMode && debugLevel >= 5) || (true == debugNanInf && ( isnan(normalizedLevel) || isinf(normalizedLevel)))) Serial << "GetNormalizedSoundLevelForBin: \tBin: " << aBin << "\tDepth: " << aDepth << "\tLevel:" << amp << "\tTrigger Level: " << triggerLevel << "\tNormalized Level: " << normalizedLevel << "\n";
      return normalizedLevel;
    }

    float GetNormalizedSoundLevelDbForFrequencyRange(float aStartFrequency, float aStopFrequency, int aDepth, SoundLevelOutputType aLevelType)
    {
      float normalizedLevel = 0.0;
      db maximumDb = 0.0;
      db ampDb = 0.0;
      db triggerLevelDb = 0.0;
      switch(aLevelType)
      {
        case SoundLevelOutputType_Level:
          ampDb = m_statisticalEngine.GetRMSDbOfFreqRange(aStartFrequency, aStopFrequency, aDepth, BinDataType::INSTANT);
          triggerLevelDb = m_statisticalEngine.GetAverageDbOfFreqRange(aStartFrequency, aStopFrequency, BIN_SAVE_LENGTH-1, BinDataType::AVERAGE)*triggerLevelGain;
          normalizedLevel = ((ampDb) / MAX_DB);
        break;
        case SoundLevelOutputType_Beat:
          ampDb = m_statisticalEngine.GetRMSDbOfFreqRange(aStartFrequency, aStopFrequency, aDepth, BinDataType::INSTANT);
          float triggerLevel = m_statisticalEngine.GetAverageOfFreqRange(aStartFrequency, aStopFrequency, BIN_SAVE_LENGTH-1, BinDataType::AVERAGE)*triggerLevelGain;
          maximumDb = m_statisticalEngine.GetMaxDbOfFreqRange(aStartFrequency, aStopFrequency, BIN_SAVE_LENGTH-1, BinDataType::AVERAGE);
          if(triggerLevel > 0)
          {
            triggerLevelDb = 20*log10(triggerLevel);
          }
          else
          {
            triggerLevelDb = 0.0;
          }
          if(ampDb > triggerLevelDb) 
          {
          normalizedLevel = ((ampDb-triggerLevelDb) / maximumDb);
          }
          else 
          {
            normalizedLevel = 0.0;
          }
        break;
      }
      if(normalizedLevel < 0.0) normalizedLevel = 0.0;
      if(normalizedLevel > 1.0) normalizedLevel = 1.0;
      if((true == debugMode && debugLevel >= 5) || (true == debugNanInf && ( isnan(normalizedLevel) || isinf(normalizedLevel)))) Serial << "GetNormalizedSoundLevelDbForFrequencyRange: \tStart Freq: " << aStartFrequency << "\tStop Freq: " << aStopFrequency << "\tDepth: " << aDepth << "\tLevel: " << ampDb << "\tAvrtage: " << triggerLevelDb << "\tNormalized Level: " << normalizedLevel << "\n";
      return normalizedLevel;
    }
    
    float GetNormalizedSoundLevelForFrequencyRange(float aStartFrequency, float aStopFrequency, int aDepth, SoundLevelOutputType aLevelType)
    {
      float normalizedLevel = 0.0;
      float amp = 0.0;
      float triggerLevel = 0.0;
      float maximum = 0.0;
      switch(aLevelType)
      {
        case SoundLevelOutputType_Level:
          amp = m_statisticalEngine.GetRMSOfFreqRange(aStartFrequency, aStopFrequency, aDepth, BinDataType::INSTANT);
          triggerLevel = m_statisticalEngine.GetAverageOfFreqRange(aStartFrequency, aStopFrequency, BIN_SAVE_LENGTH-1, BinDataType::AVERAGE)*triggerLevelGain;
          normalizedLevel = ((amp) / ADDBITS);
        break;
        case SoundLevelOutputType_Beat:
          amp = m_statisticalEngine.GetRMSOfFreqRange(aStartFrequency, aStopFrequency, aDepth, BinDataType::INSTANT);
          triggerLevel = m_statisticalEngine.GetAverageOfFreqRange(aStartFrequency, aStopFrequency, BIN_SAVE_LENGTH-1, BinDataType::AVERAGE)*triggerLevelGain;
          maximum = m_statisticalEngine.GetMaxOfFreqRange(aStartFrequency, aStopFrequency, BIN_SAVE_LENGTH-1, BinDataType::AVERAGE);
          if(amp > triggerLevel) 
          {
            normalizedLevel = ((amp-triggerLevel) / maximum);
          }
          else 
          {
            normalizedLevel = 0.0;
          }
        break;
      }
      if(normalizedLevel < 0.0) normalizedLevel = 0.0;
      if(normalizedLevel > 1.0) normalizedLevel = 1.0;
      if((true == debugMode && debugLevel >= 5) || (true == debugNanInf && ( isnan(normalizedLevel) || isinf(normalizedLevel)))) Serial << "GetNormalizedSoundLevelForFrequencyRange: \tStart Freq: " << aStartFrequency << "\tStop Freq: " << aStopFrequency << "\tDepth: " << aDepth << "\tLevel: " << amp << "\tTrigger Level: " << triggerLevel << "\tNormalized Level: " << normalizedLevel << "\n";
      return normalizedLevel;
    }
    
  protected:
    VisualizationState GetVisualizationState()
    {
      return m_visualizationState;
    }
    unsigned long m_timers[NUMBER_OF_TICK_TIMERS];
    void ResetTimer(int timer);
    void ResetAllTimers();
    unsigned long m_resetTimer;
    int m_duration;
    bool m_visualizationStarted = false;
    
    //********** Helpers **********
    bool AllStripsAreBlack();
    void SetAllLayersToColor(CRGB color);
    CRGB GetColor(int index, int maximum);
    void ShiftStrip(LEDStrip &strip, ShiftDirection direction, int spaces);
    LEDStrip AddLayer(LEDStrip layer1, LEDStrip layer2);
    LEDStrip MergeLayer(LEDStrip &layer1, LEDStrip &layer2);
    LEDStrip MergeLayerFlipZOrder(LEDStrip &layer1, LEDStrip &layer2);
    LEDStrip FadeOutLayerBy(LEDStrip layer, byte fadeAmount);
    LEDStrip FadeInLayerBy(LEDStrip layer, byte fadeAmount);
    void SpiralTowerUpwards();
    void SpiralTowerDownwards();
    CRGB GetRandomColor(byte minRed, byte maxRed, byte minGreen, byte maxGreen, byte minBlue, byte maxBlue);
    CRGB GetRandomNonGrayColor();
    float GetTriggerGainForFrequency(float frequency);
    StatisticalEngine &m_statisticalEngine;
    LEDStrip m_layer[8];
    LEDStrip m_strips[NUMSTRIPS];
    
  private:
    int m_tick;
    VisualizationState m_visualizationState = VISUALIZATION_STATE_STOPPED;
    VisualizationType m_visualizationType;
};

class Transitions: public Visualizations
{
  public:
    Transitions( StatisticalEngine &statisticalEngine
               , VisualizationsCalleeInterface *callee )
               : Visualizations( INT_MAX
                               , VisualizationType::TRANSITION
                               , statisticalEngine
                               , callee ) 
    {
      //if(true == debugMode && debugLevel >= 0) Serial << "Transitions: Instantiate\n";
    }
    virtual ~Transitions()
    {
      if(true == debugMode && debugLevel >= 2) Serial << "Delete Transition\n";
    }    
    Visualizations *getCurrentVisualization()
    {
      return mp_currentVisualization;
    }
    void DeleteCurrentVisualization()
    {
      delete mp_currentVisualization;
      mp_currentVisualization = NULL;
    }
    Visualizations *getPreviousVisualization()
    {
      return mp_previousVisualization;
    }
    void DeletePreviousVisualization()
    {
      delete mp_previousVisualization;
      mp_previousVisualization = NULL;
    }
    void SetCurrentVisualization(Visualizations *currentVisualization)
    {
      mp_currentVisualization = currentVisualization;
    }
    void SetPreviousVisualization(Visualizations *previousVisualization)
    {
      mp_previousVisualization = previousVisualization;
    }    
  protected:
    Visualizations *mp_currentVisualization;
    Visualizations *mp_previousVisualization;
};

class FadeController
{
  public:
  FadeController()
  {
  }
  virtual ~FadeController(){}
  void ConfigureFadeController(CRGB start, CRGB endColor, unsigned int duration);
  CRGB IncrementFade(unsigned int incrementValue);
  void ResetFade();
  CRGB GetCurrentColor() { return m_currentColor; }
  CRGB SetCurrentColor(CRGB color) { m_currentColor = color; }
  unsigned int GetCurrentTickOfDuration() { return m_currentTickOfDuration; }
  private:
  CRGB m_startColor;
  CRGB m_currentColor;
  CRGB m_endColor;
  int m_duration;
  int m_currentTickOfDuration;
};

#endif
