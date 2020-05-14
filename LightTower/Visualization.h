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
 * @file Visualization.h
 *
 *
 */

#ifndef Visualization_H
#define Visualization_H
#include "Visualizations.h"
#include "Streaming.h"
#include "Tunes.h"

class InstantSwitch: public Transitions
{
  public:
    InstantSwitch( StatisticalEngine &statisticalEngine
                 , VisualizationsCalleeInterface *callee )
                 : Transitions( statisticalEngine
                              , callee )
    {
    }
    virtual ~InstantSwitch()
    {
      if(true == debugMode && debugLevel >= 2) Serial << "Delete InstantSwitch\n";
    }
    static Transitions* GetInstance( StatisticalEngine &statisticalEngine
                                   , VisualizationsCalleeInterface *callee );
    virtual void Start();
    virtual bool Loop();
    virtual void End();
    static int Add(int input1, int input2)
    {
      return input1 + input2;
    }
  private:
    bool Tick1();
};

class FadeTransition: public Transitions
{
  public:
    FadeTransition( StatisticalEngine &statisticalEngine
                  , VisualizationsCalleeInterface *callee )
                  : Transitions( statisticalEngine
                               , callee )
    {
    }
    virtual ~FadeTransition()
    {
      if(true == debugMode && debugLevel >= 2) Serial << "Delete FadeTransition\n";
    }
    static Transitions* GetInstance( StatisticalEngine &statisticalEngine
                                   , VisualizationsCalleeInterface *callee );
    virtual void Start();
    virtual bool Loop();
    virtual void End();
  private:
    bool Tick1();
    void Tick2();
    int m_fadeCount = 0;
};

class MixerTransition: public Transitions
{
  public:
    MixerTransition( StatisticalEngine &statisticalEngine
                   , VisualizationsCalleeInterface *callee )
                   : Transitions( statisticalEngine
                                , callee )
    {
    }
    virtual ~MixerTransition()
    {
      if(true == debugMode && debugLevel >= 2) Serial << "Delete MixerTransition\n";
    }
    static Transitions* GetInstance( StatisticalEngine &statisticalEngine
                                   , VisualizationsCalleeInterface *callee );       
    virtual void Start();
    virtual bool Loop();
    virtual void End();
  private:
    bool Tick1();
    int m_blendMode;
};

class SlideUpTransition: public Transitions
{
  public:
    SlideUpTransition( StatisticalEngine &statisticalEngine
                     , VisualizationsCalleeInterface *callee )
                     : Transitions( statisticalEngine
                                  , callee  )
    {
    }
    virtual ~SlideUpTransition()
    {
      if(true == debugMode && debugLevel >= 2) Serial << "Delete SlideUpTransition\n";
    }
    static Transitions* GetInstance( StatisticalEngine &statisticalEngine
                                   , VisualizationsCalleeInterface *callee );        
    virtual void Start();
    virtual bool Loop();
    virtual void End();
  private:
    bool Tick1();
    void Tick2();
    int m_fadeCount = 0;
};

class SlideDownTransition: public Transitions
{
  public:
    SlideDownTransition( StatisticalEngine &statisticalEngine
                       , VisualizationsCalleeInterface *callee )
                       : Transitions( statisticalEngine
                                    , callee )
    {
    }
    virtual ~SlideDownTransition()
    {
      if(true == debugMode && debugLevel >= 2) Serial << "Delete SlideDownTransition\n";
    }
    static Transitions* GetInstance( StatisticalEngine &statisticalEngine
                                   , VisualizationsCalleeInterface *callee );              
    virtual void Start();
    virtual bool Loop();
    virtual void End();
  private:
    bool Tick1();
    void Tick2();
    int m_fadeCount = 0;
};


class SplitTransition: public Transitions
{
  public:
    SplitTransition( StatisticalEngine &statisticalEngine
                   , VisualizationsCalleeInterface *callee )
                   : Transitions( statisticalEngine
                                , callee )
    {
    }
    virtual ~SplitTransition()
    {
      if(true == debugMode && debugLevel >= 2) Serial << "Delete SplitTransition\n";
    }
    static Transitions* GetInstance( StatisticalEngine &statisticalEngine
                                   , VisualizationsCalleeInterface *callee );            
    virtual void Start();
    virtual bool Loop();
    virtual void End();
  private:
    bool Tick1();
    void Tick2();
    int m_fadeCount = 0;
};


class SoundDetectionTester: public Visualizations
{
  public:
    SoundDetectionTester( int duration
                        , StatisticalEngine &statisticalEngine
                        , VisualizationsCalleeInterface *callee )
                        : Visualizations( duration
                                        , VisualizationType::VISUALIZATION
                                        , statisticalEngine
                                        , callee ){}
    virtual ~SoundDetectionTester()
    {
      if(true == debugMode && debugLevel >= 2) Serial << "Delete SoundDetectionTester\n";
    }
    static Visualizations* GetInstance( int duration
                                      , StatisticalEngine &statisticalEngine
                                      , VisualizationsCalleeInterface *callee );  
    virtual void Start();
    virtual bool Loop();
    virtual void End();
  private:
    void Tick1();
};

class Confirmation: public Visualizations
{
  public:
    Confirmation( int duration
                , StatisticalEngine &statisticalEngine
                , VisualizationsCalleeInterface *callee )
                : Visualizations( duration
                                , VisualizationType::CONFIRMATION
                                , statisticalEngine
                                , callee ){}
    virtual ~Confirmation()
    {
      if(true == debugMode && debugLevel >= 2) Serial << "Delete Confirmation\n";
    }
    static Visualizations* GetInstance( int duration
                                      , StatisticalEngine &statisticalEngine
                                      , VisualizationsCalleeInterface *callee );
    virtual void Start();
    virtual bool Loop();
    virtual void End();
    void SetConfirmationColor(CRGB color)
    {
      m_confirmationColor = color;
    }
  private:
    void Tick1();
    int m_count = 1;
    CRGB m_confirmationColor;
};

class ColorFadingTower: public Visualizations
{
  public:
    ColorFadingTower( int duration
                    , StatisticalEngine &statisticalEngine
                    , VisualizationsCalleeInterface *callee )
                    : Visualizations( duration
                                    , VisualizationType::VISUALIZATION
                                    , statisticalEngine
                                    , callee ){}
    virtual ~ColorFadingTower()
    {
      if(true == debugMode && debugLevel >= 2) Serial << "Delete ColorFadingTower\n";
    }
    static Visualizations* GetInstance( int duration
                                      , StatisticalEngine &statisticalEngine
                                      , VisualizationsCalleeInterface *callee );
    virtual void Start();
    virtual bool Loop();
    virtual void End();
  private:
    int m_randomTime;
    CRGB m_color = GetRandomNonGrayColor();
    CRGB m_fadeToColor;
    const int m_maxRandomTime = 10000;
    const int m_fadeLength = 500;
    struct FadeController m_fadeController;
    void Tick1();
    void Tick2();
};

class WaterFallFireStreamer: public Visualizations
{
  public:
    WaterFallFireStreamer( int duration
                         , StatisticalEngine &statisticalEngine
                         , VisualizationsCalleeInterface *callee )
                         : Visualizations( duration
                                         , VisualizationType::VISUALIZATION
                                         , statisticalEngine
                                         , callee ){}
    virtual ~WaterFallFireStreamer()
    {
      if(true == debugMode && debugLevel >= 2) Serial << "Delete WaterFallFireStreamer\n";
    }
    static Visualizations* GetInstance( int duration
                                      , StatisticalEngine &statisticalEngine
                                      , VisualizationsCalleeInterface *callee );  
    virtual void Start();
    virtual bool Loop();
    virtual void End();
  private:
    void Tick1();
    void Randomize();
    CRGB m_BottomColor;
    CRGB m_TopColor;
    CRGB m_BottomMiddleColor;
    CRGB m_TopMiddleColor;
    CRGB m_BottomFadeToColor;
    CRGB m_TopFadeToColor;
    CRGB m_BottomMiddleFadeToColor;
    CRGB m_TopMiddleFadeToColor;
    
    int m_randomTime;
    const int m_fadeLength = 500;
    struct FadeController m_topFadeController;
    struct FadeController m_bottomFadeController;
    struct FadeController m_topMiddleFadeController;
    struct FadeController m_bottomMiddleFadeController;
    enum StreamerType
    {
      StreamerType_Outward_In,
      StreamerType_Inward_Out,
      StreamerType_Both,
      StreamerType_Total
    };
    const StreamerType m_streamerType = (StreamerType)random(0, StreamerType::StreamerType_Total);
};

class SolidColorTower: public Visualizations
{
  public:
    SolidColorTower( int duration
                   , StatisticalEngine &statisticalEngine
                   , VisualizationsCalleeInterface *callee )
                   : Visualizations( duration
                                   , VisualizationType::VISUALIZATION
                                   , statisticalEngine
                                   , callee ){}
    virtual ~SolidColorTower()
    {
      if(true == debugMode && debugLevel >= 2) Serial << "Delete SolidColorTower\n";
    }
    static Visualizations* GetInstance( int duration
                                      , StatisticalEngine &statisticalEngine
                                      , VisualizationsCalleeInterface *callee );
    virtual void Start();
    virtual bool Loop();
    virtual void End();
  private:
    void Tick1();
        
};


class PowerBarWithBassSprite: public Visualizations
{
  public:
    PowerBarWithBassSprite( int duration
                          , StatisticalEngine &statisticalEngine
                          , VisualizationsCalleeInterface *callee )
                          : Visualizations( duration
                                          , VisualizationType::VISUALIZATION
                                          , statisticalEngine
                                          , callee ){}
    virtual ~PowerBarWithBassSprite()
    {
      if(true == debugMode && debugLevel >= 2) Serial << "Delete PowerBarWithBassSprite\n";
    }
    static Visualizations* GetInstance( int duration
                                      , StatisticalEngine &statisticalEngine
                                      , VisualizationsCalleeInterface *callee );
    virtual void Start();
    virtual bool Loop();
    virtual void End();
  private:
    void Tick1();
    void Tick2();
    void Tick3();
    void Tick4();
    void Tick5();
    void Tick6();
    void Random();
    struct Sprite m_sprites[3];
    struct FadeController m_fadeControllers[2];
    CRGB m_powerBarColor;
    struct FadeController m_powerBarFadeController;
    int m_powerBarChangeColorTime;
    CRGB m_bassPowerBarColor;
    struct FadeController m_bassPowerBarFadeController;
    int m_bassPowerBarChangeColorTime;
    CRGB m_maxBassSpriteColor;
    struct FadeController m_maxBassSpriteFadeController;
    int m_bassPowerMaxChangeColorTime;
        
};

class RandomFrequencySprites: public Visualizations
{
  public:
    RandomFrequencySprites( int duration
                          , StatisticalEngine &statisticalEngine
                          , VisualizationsCalleeInterface *callee )
                          : Visualizations( duration
                                          , VisualizationType::VISUALIZATION
                                          , statisticalEngine
                                          , callee){}
    virtual ~RandomFrequencySprites()
    {
      if(true == debugMode && debugLevel >= 2) Serial << "Delete RandomFrequencySprites\n";
    }
    static Visualizations* GetInstance( int duration
                                      , StatisticalEngine &statisticalEngine
                                      , VisualizationsCalleeInterface *callee );
    virtual void Start();
    virtual bool Loop();
    virtual void End();
  private:
    void Tick1();
    const int m_fadeAmount = random (1,51);
     
};

class FFTAmplitudes: public Visualizations
{
  public:
    FFTAmplitudes( int duration
                 , StatisticalEngine &statisticalEngine
                 , VisualizationsCalleeInterface *callee )
                 : Visualizations( duration
                                 , VisualizationType::VISUALIZATION
                                 , statisticalEngine
                                 , callee){}
    virtual ~FFTAmplitudes()
    {
      if(true == debugMode && debugLevel >= 2) Serial << "Delete FFTAmplitudes\n";
    }
    static Visualizations* GetInstance( int duration
                                      ,  StatisticalEngine &statisticalEngine
                                      , VisualizationsCalleeInterface *callee );
    virtual void Start();
    virtual bool Loop();
    virtual void End();
  private:
    void Tick1();
    void Random();
    unsigned long m_randomTimes[1];
    const int m_gainLimitMax = 1000;
    const int m_gainLimitMin = 1;
    const int m_gainmultiplier = 10;
    int m_gainIntigrator = 100;
        
};

class FrequencySpriteSpiral: public Visualizations
{
  public:
    FrequencySpriteSpiral( int duration
                         , StatisticalEngine &statisticalEngine
                         , VisualizationsCalleeInterface *callee )
                         : Visualizations( duration
                                         , VisualizationType::VISUALIZATION
                                         , statisticalEngine
                                         , callee ){}
    virtual ~FrequencySpriteSpiral()
    {
      if(true == debugMode && debugLevel >= 2) Serial << "Delete FrequencySpriteSpiral\n";
    }
    static Visualizations* GetInstance( int duration
                                      , StatisticalEngine &statisticalEngine
                                      , VisualizationsCalleeInterface *callee );
    virtual void Start();
    virtual bool Loop();
    virtual void End();
  private:
    void Tick1();
    unsigned long m_randomTimes[3];
        
};

class RandomHighLowFrequencyAmplitudeStreamer: public Visualizations
{
  public:
    RandomHighLowFrequencyAmplitudeStreamer( int duration
                                           , StatisticalEngine &statisticalEngine
                                           , VisualizationsCalleeInterface *callee )
                                           : Visualizations( duration
                                                           , VisualizationType::VISUALIZATION
                                                           , statisticalEngine
                                                           , callee ){}
    virtual ~RandomHighLowFrequencyAmplitudeStreamer()
    {
      if(true == debugMode && debugLevel >= 2) Serial << "Delete RandomHighLowFrequencyAmplitudeStreamer\n";
    }
    static Visualizations* GetInstance( int duration
                                      , StatisticalEngine &statisticalEngine
                                      , VisualizationsCalleeInterface *callee );
    virtual void Start();
    virtual bool Loop();
    virtual void End();
  private:
    void Tick1();
    void Random();
    CRGB m_randomColors[20];
    unsigned long m_randomTime;
    float  m_triggerFrequencyLow;
    float  m_triggerFrequencyHigh;
        
};

class OutwardAmplitudeWithFloatingBassSprites: public Visualizations
{
  public:
    OutwardAmplitudeWithFloatingBassSprites( int duration
                                           , StatisticalEngine &statisticalEngine
                                           , VisualizationsCalleeInterface *callee )
                                           : Visualizations( duration
                                                           , VisualizationType::VISUALIZATION
                                                           , statisticalEngine
                                                           , callee ){}
    virtual ~OutwardAmplitudeWithFloatingBassSprites()
    {
      if(true == debugMode && debugLevel >= 2) Serial << "Delete OutwardAmplitudeWithFloatingBassSprites\n";
    }
    static Visualizations* GetInstance( int duration
                                      , StatisticalEngine &statisticalEngine
                                      , VisualizationsCalleeInterface *callee );
    virtual void Start();
    virtual bool Loop();
    virtual void End();
  private:
    void Tick1();
    void Tick2();
    void Random();
    
    unsigned long m_randomTimes[2];
    struct Sprite m_sprites[2];
    CRGB m_BottomColor;
    CRGB m_TopColor;
    CRGB m_BottomFadeToColor;
    CRGB m_TopFadeToColor;
    int m_fadeLength;
    struct FadeController m_topFadeController;
    struct FadeController m_bottomFadeController;
        
};

class VerticalFFTAmplitudeTower: public Visualizations
{
  public:
    VerticalFFTAmplitudeTower( int duration
                             , StatisticalEngine &statisticalEngine
                             , VisualizationsCalleeInterface *callee )
                             : Visualizations( duration
                                             , VisualizationType::VISUALIZATION
                                             , statisticalEngine
                                             , callee ){}
    virtual ~VerticalFFTAmplitudeTower()
    {
      if(true == debugMode && debugLevel >= 2) Serial << "Delete VerticalFFTAmplitudeTower\n";
    }
    static Visualizations* GetInstance( int duration
                                      , StatisticalEngine &statisticalEngine
                                      , VisualizationsCalleeInterface *callee );
    virtual void Start();
    virtual bool Loop();
    virtual void End();
  private:
    void Tick1();
    unsigned long m_randomTimes[1];
        
};

class MultiRangeAmplitudeTower: public Visualizations
{
  public:
    MultiRangeAmplitudeTower( int duration
                            , StatisticalEngine &statisticalEngine
                            , VisualizationsCalleeInterface *callee )
                            : Visualizations( duration
                                            , VisualizationType::VISUALIZATION
                                            , statisticalEngine
                                            , callee ){}
    virtual ~MultiRangeAmplitudeTower()
    {
      if(true == debugMode && debugLevel >= 2) Serial << "Delete MultiRangeAmplitudeTower\n";
    }
    static Visualizations* GetInstance( int duration
                                      , StatisticalEngine &statisticalEngine
                                      , VisualizationsCalleeInterface *callee );
    virtual void Start();
    virtual bool Loop();
    virtual void End();
  private:
    void Tick1();
    void Random();
    
    unsigned long m_randomTimes[1];
    int m_randomNumber;
    const int m_maxLEDsPerRange = 4;
    CRGB m_powerBarColor;
    CRGB m_bottomColor;
    CRGB m_topColor;
    bool m_tooLoud;
    unsigned long m_tooLoudTime; 
};

class SimultaneousFrequencyStreamer: public Visualizations
{
  public:
    SimultaneousFrequencyStreamer( int duration
                                 , StatisticalEngine &statisticalEngine
                                 , VisualizationsCalleeInterface *callee )
                                 : Visualizations( duration
                                                 , VisualizationType::VISUALIZATION
                                                 , statisticalEngine
                                                 , callee ){}
    virtual ~SimultaneousFrequencyStreamer()
    {
      if(true == debugMode && debugLevel >= 2) Serial << "Delete SimultaneousFrequencyStreamer\n";
    }
    static Visualizations* GetInstance( int duration
                                      , StatisticalEngine &statisticalEngine
                                      , VisualizationsCalleeInterface *callee );
    virtual void Start();
    virtual bool Loop();
    virtual void End();

  private:
    void Tick1();
    void Tick2();
    unsigned long m_randomTimes[1];
    CRGB m_randomColors[20];
    enum MergeType
    {
      MERGE_TYPE_ADD,
      MERGE_TYPE_MERGE,
      MERGE_TYPE_SIZE
    };
    enum FadeType
    {
      FADE_TYPE_ON,
      FADE_TYPE_OFF,
      FADE_TYPE_SIZE
    };
    MergeType mergeType;
    enum RotationDirection
    {
      ROTATION_DIRECTION_LEFT,
      ROTATION_DIRECTION_RIGHT,
      ROTATION_DIRECTION_SIZE
    };
    RotationDirection m_rotationDirection = (RotationDirection)random(0, RotationDirection::ROTATION_DIRECTION_SIZE);
    FadeType m_fadeType = (FadeType)random(0, FadeType::FADE_TYPE_SIZE);
    unsigned int m_rotationCount = 0;
    const int m_bottomLayers[4] = { 0, 1, 2, 3 };
    const int m_topLayers[4] = { 4, 5, 6, 7 };
};

class MinMaxAmplitude: public Visualizations
{
  public:
    MinMaxAmplitude( int duration
                   , StatisticalEngine &statisticalEngine
                   , VisualizationsCalleeInterface *callee )
                   : Visualizations( duration
                                   , VisualizationType::VISUALIZATION
                                   , statisticalEngine
                                   , callee ){}
    virtual ~MinMaxAmplitude()
    {
      if(true == debugMode && debugLevel >= 2) Serial << "Delete MinMaxAmplitude\n";
    }
    static Visualizations* GetInstance( int duration
                                      , StatisticalEngine &statisticalEngine
                                      , VisualizationsCalleeInterface *callee );
    virtual void Start();
    virtual bool Loop();
    virtual void End();
  private:
    void Tick1();
    void Random();
    unsigned long m_randomTime;
    int m_fadeAmount;
    const int m_numBands = 10;
    int m_activeBand = 0;  
};

class ChasingSprites: public Visualizations
{
  public:
    ChasingSprites( int duration
                  , StatisticalEngine &statisticalEngine
                  , VisualizationsCalleeInterface *callee )
                  : Visualizations( duration
                                  , VisualizationType::VISUALIZATION
                                  , statisticalEngine
                                  , callee ){}
    virtual ~ChasingSprites()
    {
      if(true == debugMode && debugLevel >= 2) Serial << "Delete ChasingSprites\n";
    }
    static Visualizations* GetInstance( int duration
                                      , StatisticalEngine &statisticalEngine
                                      , VisualizationsCalleeInterface *callee );
    virtual void Start();
    virtual bool Loop();
    virtual void End();
    
    void Tick1();
    void Tick2();
    void Tick3();
    void Tick4();
    void Random();

  private:
    int m_numberOfSprites;
    int GetNewBassTime();
    int GetNewMidTime();
    int GetNewHighTime();
    
    CRGB m_randomColors[20]; 
    CRGB bassColor;
    CRGB midColor;
    CRGB highColor;
    int m_bassTime;
    int m_bassOffset;
    int m_midTime;
    int m_midOffset;
    int m_highTime;
    int m_highOffset;
    int m_mergeType;
};

class FrequencyColorStreamer: public Visualizations
{
  public:
    FrequencyColorStreamer( int duration
                          , StatisticalEngine &statisticalEngine
                          , VisualizationsCalleeInterface *callee )
                          : Visualizations( duration
                                          , VisualizationType::VISUALIZATION
                                          , statisticalEngine
                                          , callee ){}
    virtual ~FrequencyColorStreamer()
    {
      if(true == debugMode && debugLevel >= 2) Serial << "Delete FrequencyColorStreamer\n";
    }
    static Visualizations* GetInstance( int duration
                                      , StatisticalEngine &statisticalEngine
                                      , VisualizationsCalleeInterface *callee );
    virtual void Start();
    virtual bool Loop();
    virtual void End();
    
    void Tick1();
    CRGB m_BottomColor;
    
    
};

class FrequencyColorSpinningTower: public Visualizations
{
  public:
    FrequencyColorSpinningTower( int duration
                               , StatisticalEngine &statisticalEngine
                               , VisualizationsCalleeInterface *callee )
                               : Visualizations( duration
                                               , VisualizationType::VISUALIZATION
                                               , statisticalEngine
                                               , callee ){}
    virtual ~FrequencyColorSpinningTower()
    {
      if(true == debugMode && debugLevel >= 2) Serial << "Delete FrequencyColorSpinningTower\n";
    }
    static Visualizations* GetInstance( int duration
                                      , StatisticalEngine &statisticalEngine
                                      , VisualizationsCalleeInterface *callee );
    virtual void Start();
    virtual bool Loop();
    virtual void End();
    
    void Tick1();
    void IncrementCount();
    int m_countTime;
    int m_rotationCount;
    
    
};

class UpDownFrequencyColorStreamer: public Visualizations
{
  public:
    UpDownFrequencyColorStreamer( int duration
                                , StatisticalEngine &statisticalEngine
                                , VisualizationsCalleeInterface *callee )
                                : Visualizations( duration
                                                , VisualizationType::VISUALIZATION
                                                , statisticalEngine
                                                , callee ){}
    virtual ~UpDownFrequencyColorStreamer()
    {
      if(true == debugMode && debugLevel >= 2) Serial << "Delete UpDownFrequencyColorStreamer\n";
    }
    static Visualizations* GetInstance( int duration
                                      , StatisticalEngine &statisticalEngine
                                      , VisualizationsCalleeInterface *callee );
    virtual void Start();
    virtual bool Loop();
    virtual void End();
    void Tick1();
    CRGB m_BottomColor;
    int m_Count;
    
    
};

class UpDownMaxFrequencyStreamer: public Visualizations
{
  public:
    UpDownMaxFrequencyStreamer( int duration
                              , StatisticalEngine &statisticalEngine
                              , VisualizationsCalleeInterface *callee )
                              : Visualizations( duration
                                              , VisualizationType::VISUALIZATION
                                              , statisticalEngine
                                              , callee ){}
    virtual ~UpDownMaxFrequencyStreamer()
    {
      if(true == debugMode && debugLevel >= 2) Serial << "Delete UpDownMaxFrequencyStreamer\n";
    }
    static Visualizations* GetInstance( int duration
                                      , StatisticalEngine &statisticalEngine
                                      , VisualizationsCalleeInterface *callee );
    virtual void Start();
    virtual bool Loop();
    virtual void End();
    
    void Tick1();
    void Tick2();
    CRGB m_Color;
    int m_Count;
    int m_maxBin;
    const int m_colorTime = 200;
    
};

class ScrollingRainbow: public Visualizations
{
  public:
    ScrollingRainbow( int duration
                    , StatisticalEngine &statisticalEngine
                    , VisualizationsCalleeInterface *callee )
                    : Visualizations( duration
                                    , VisualizationType::VISUALIZATION
                                    , statisticalEngine
                                    , callee ){}
    virtual ~ScrollingRainbow()
    {
      if(true == debugMode && debugLevel >= 2) Serial << "Delete ScrollingRainbow\n";
    }
    static Visualizations* GetInstance( int duration
                                      , StatisticalEngine &statisticalEngine
                                      , VisualizationsCalleeInterface *callee );
    virtual void Start();
    virtual bool Loop();
    virtual void End();
    
    void Tick1();
    void Tick2();
    void Tick3();
    CRGB m_currentColor;
    CRGB m_fadeToColor;
    int m_currentColorCount = 0;
    int m_renderCount = 0;
    const int m_numColors = 7;
    const int m_colorLength = random(1, NUMLEDS);
    struct FadeController m_fadeController;
    
};

class ScrollingFrequencyColorRectangles: public Visualizations
{
  public:
    ScrollingFrequencyColorRectangles( int duration
                                     , StatisticalEngine &statisticalEngine
                                     , VisualizationsCalleeInterface *callee )
                                     : Visualizations( duration
                                                     , VisualizationType::VISUALIZATION
                                                     , statisticalEngine
                                                     , callee ){}
    virtual ~ScrollingFrequencyColorRectangles()
    {
      if(true == debugMode && debugLevel >= 2) Serial << "Delete ScrollingFrequencyColorRectangles\n";
    }
    static Visualizations* GetInstance( int duration
                                      , StatisticalEngine &statisticalEngine
                                      , VisualizationsCalleeInterface *callee );
    virtual void Start();
    virtual bool Loop();
    virtual void End();
    
    void Tick1();
    void Tick2();
    CRGB m_currentColor;
    CRGB m_detectedColor;
    CRGB m_backgroundColor = GetRandomNonGrayColor();
    int m_currentColorCount = 0;
    int m_renderCount = 0;
    int m_direction = random(2);
    float  m_gain = 1.0;
    const int m_numColors = 2;
    const int m_colorLength = random(1, 11);
    
};

class ScrollingFrequencySprites: public Visualizations
{
  public:
    ScrollingFrequencySprites( int duration
                             , StatisticalEngine &statisticalEngine
                             , VisualizationsCalleeInterface *callee )
                             : Visualizations( duration
                                             , VisualizationType::VISUALIZATION
                                             , statisticalEngine
                                             , callee )
    {
      m_sprites = new struct Sprite[m_numberOfSprites];
    }
    virtual ~ScrollingFrequencySprites()
    {
      if(true == debugMode && debugLevel >= 2) Serial << "Delete ScrollingFrequencySprites\n";
      delete m_sprites;
    }
    static Visualizations* GetInstance( int duration
                                      , StatisticalEngine &statisticalEngine
                                      , VisualizationsCalleeInterface *callee );
    virtual void Start();
    virtual bool Loop();
    virtual void End();
  private:  
    void Tick1();
    void Tick2();
    const int m_numberOfSprites = random(1, NUMLEDS+1);
    struct Sprite *m_sprites;
    
};

class ScrollingSpeedFrequencySprites: public Visualizations
{
  public:
    ScrollingSpeedFrequencySprites( int duration
                                  , StatisticalEngine &statisticalEngine
                                  , VisualizationsCalleeInterface *callee )
                                  : Visualizations( duration
                                                  , VisualizationType::VISUALIZATION
                                                  , statisticalEngine
                                                  , callee)
    {
      m_sprites = new struct Sprite[m_numberOfSprites];
    }
    virtual ~ScrollingSpeedFrequencySprites()
    {
      if(true == debugMode && debugLevel >= 2) Serial << "Delete ScrollingSpeedFrequencySprites\n";
      delete m_sprites;
    }
    static Visualizations* GetInstance( int duration
                                      , StatisticalEngine &statisticalEngine
                                      , VisualizationsCalleeInterface *callee );
    virtual void Start();
    virtual bool Loop();
    virtual void End();
  private:  
    void Tick1();
    const int m_numberOfSprites = random(3, 11);
    struct Sprite *m_sprites;
    bool m_tooLoud;
    unsigned long m_tooLoudTime;
    
};

class ScrollingAmplitudeSprite: public Visualizations
{
  public:
    ScrollingAmplitudeSprite( int duration
                            , StatisticalEngine &statisticalEngine
                            , VisualizationsCalleeInterface *callee )
                            : Visualizations( duration
                                            , VisualizationType::VISUALIZATION
                                            , statisticalEngine
                                            , callee )
    {
      m_sprite.color = GetRandomNonGrayColor();
    }
    virtual ~ScrollingAmplitudeSprite()
    {
      if(true == debugMode && debugLevel >= 2) Serial << "Delete ScrollingAmplitudeSprite\n";
    }
    static Visualizations* GetInstance( int duration
                                      , StatisticalEngine &statisticalEngine
                                      , VisualizationsCalleeInterface *callee );
    virtual void Start();
    virtual bool Loop();
    virtual void End();
    void Tick1();
    void Random();
    
  private:
    struct Sprite m_sprite;
    CRGB m_currentColor;
    unsigned long m_randomTime;
    int m_direction = random(2);
    int m_waitCount = 0;
    int m_count = 0;
    CRGB m_fadeToColor;
    const int m_maxRandomTime = 5000;
    const int m_fadeLength = NUMLEDS;
    struct FadeController m_fadeController;
    
};

class Opposites: public Visualizations
{
  public:
    Opposites( int duration
             , StatisticalEngine &statisticalEngine
             , VisualizationsCalleeInterface *callee )
             : Visualizations( duration
                             , VisualizationType::VISUALIZATION
                             , statisticalEngine
                             , callee ){}
    virtual ~Opposites()
    {
      if(true == debugMode && debugLevel >= 2) Serial << "Delete ScrollingAmplitudeSprite\n";
    }
    static Visualizations* GetInstance( int duration
                                      , StatisticalEngine &statisticalEngine
                                      , VisualizationsCalleeInterface *callee );
    virtual void Start();
    virtual bool Loop();
    virtual void End();
    void Tick1();
    void Random();
    
  private:
    int m_barLength;
    unsigned long m_randomTime;
    
};


class Snake: public Visualizations
{
  public:
        Snake( int duration
             , StatisticalEngine &statisticalEngine
             , VisualizationsCalleeInterface *callee )
             : Visualizations( duration
                             , VisualizationType::VISUALIZATION
                             , statisticalEngine
                             , callee ){}
    virtual ~Snake()
    {
      if(true == debugMode && debugLevel >= 2) Serial << "Delete Snake\n";
    }
    static Visualizations* GetInstance( int duration
                                      , StatisticalEngine &statisticalEngine
                                      , VisualizationsCalleeInterface *callee );
    virtual void Start();
    virtual bool Loop();
    virtual void End();
    void Tick1();
    void Random();
    
  private:
    
};

#endif
