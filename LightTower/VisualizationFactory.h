
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
 * @file VisualizationFactory.h
 *
 *
 */

#ifndef VisualizationFactory_H
#define VisualizationFactory_H
#include "Visualization.h"

typedef Visualizations* (* VisualizationFactoryPtr)( int, StatisticalEngine &, VisualizationsCalleeInterface *);
typedef Transitions* (* TransitionFactoryPtr)( StatisticalEngine &, VisualizationsCalleeInterface *);

struct VisualizationConfig
{
  VisualizationEntries visualizationEntry;
  VisualizationFactoryPtr getVisualizationInstancePtr;
  TransitionFactoryPtr getTransitionInstancePtr;
  bool isTransition;
  bool isVisualization;
  bool isForeground;
  bool isBackground;
  bool isConfirmation;
  bool isTuner;
};
    
const VisualizationConfig visualizationConfig[36] PROGMEM =
{
  //visualizationEntry                                                                    Visualization Instantiation                            Transition Instantiation           isTransition   isVisualization   isForeground  isBackground  isConfirmation  isTuner
{ VisualizationEntries::VisualizationEntries_InstantSwitch,                               NULL,                                                  InstantSwitch::GetInstance,        true,          false,            true,         false,        false,          false },
{ VisualizationEntries::VisualizationEntries_FadeTransition,                              NULL,                                                  FadeTransition::GetInstance,       true,          false,            true,         false,        false,          false },
{ VisualizationEntries::VisualizationEntries_MixerAddTransition,                          NULL,                                                  MixerAddTransition::GetInstance,   false,         false,            true,         true,         false,          false },
{ VisualizationEntries::VisualizationEntries_MixerMergeTransition,                        NULL,                                                  MixerMergeTransition::GetInstance, false,         false,            true,         true,         false,          false },
{ VisualizationEntries::VisualizationEntries_SlideUpTransition,                           NULL,                                                  SlideUpTransition::GetInstance,    true,          false,            true,         false,        false,          false },
{ VisualizationEntries::VisualizationEntries_SlideDownTransition,                         NULL,                                                  SlideDownTransition::GetInstance,  true,          false,            true,         false,        false,          false },
{ VisualizationEntries::VisualizationEntries_SplitTransition,                             NULL,                                                  SplitTransition::GetInstance,      true,          false,            true,         false,        false,          false },
{ VisualizationEntries::VisualizationEntries_SoundDetectionTester,                        SoundDetectionTester::GetInstance,                     NULL,                              false,         false,            false,        false,        false,          true  },
{ VisualizationEntries::VisualizationEntries_ColorFadingTower,                            ColorFadingTower::GetInstance,                         NULL,                              false,         false,            true,         true,         false,          false },
{ VisualizationEntries::VisualizationEntries_Confirmation,                                Confirmation::GetInstance,                             NULL,                              false,         false,            false,        false,        true,           false },
{ VisualizationEntries::VisualizationEntries_WaterFallFireStreamer,                       WaterFallFireStreamer::GetInstance,                    NULL,                              false,         true,             true,         false,        false,          false },
{ VisualizationEntries::VisualizationEntries_SolidColorTower,                             SolidColorTower::GetInstance,                          NULL,                              false,         true,             true,         true,         false,          false },
{ VisualizationEntries::VisualizationEntries_FadingSolidColorTower,                       FadingSolidColorTower::GetInstance,                    NULL,                              false,         true,             true,         true,         false,          false },
{ VisualizationEntries::VisualizationEntries_PowerBarWithBassSprite,                      PowerBarWithBassSprite::GetInstance,                   NULL,                              false,         true,             true,         false,        false,          false },
{ VisualizationEntries::VisualizationEntries_RandomFrequencySprites,                      RandomFrequencySprites::GetInstance,                   NULL,                              false,         true,             true,         false,        false,          false },
{ VisualizationEntries::VisualizationEntries_FFTAmplitudes,                               FFTAmplitudes::GetInstance,                            NULL,                              false,         true,             true,         false,        false,          false },
{ VisualizationEntries::VisualizationEntries_FrequencySpriteSpiral,                       FrequencySpriteSpiral::GetInstance,                    NULL,                              false,         true,             true,         false,        false,          false },
{ VisualizationEntries::VisualizationEntries_RandomHighLowFrequencyAmplitudeStreamer,     RandomHighLowFrequencyAmplitudeStreamer::GetInstance,  NULL,                              false,         true,             true,         false,        false,          false },
{ VisualizationEntries::VisualizationEntries_OutwardAmplitudeWithFloatingBassSprites,     OutwardAmplitudeWithFloatingBassSprites::GetInstance,  NULL,                              false,         true,             true,         false,        false,          false },
{ VisualizationEntries::VisualizationEntries_VerticalFFTAmplitudeTower,                   VerticalFFTAmplitudeTower::GetInstance,                NULL,                              false,         true,             true,         false,        false,          false },
{ VisualizationEntries::VisualizationEntries_MultiRangeAmplitudeTower,                    MultiRangeAmplitudeTower::GetInstance,                 NULL,                              false,         true,             true,         false,        false,          false },
{ VisualizationEntries::VisualizationEntries_SimultaneousFrequencyStreamer,               SimultaneousFrequencyStreamer::GetInstance,            NULL,                              false,         true,             true,         false,        false,          false },
{ VisualizationEntries::VisualizationEntries_MinMaxAmplitude,                             MinMaxAmplitude::GetInstance,                          NULL,                              false,         true,             true,         false,        false,          false },
{ VisualizationEntries::VisualizationEntries_ChasingSprites,                              ChasingSprites::GetInstance,                           NULL,                              false,         true,             true,         false,        false,          false },
{ VisualizationEntries::VisualizationEntries_FrequencyColorStreamer,                      FrequencyColorStreamer::GetInstance,                   NULL,                              false,         true,             true,         false,        false,          false },
{ VisualizationEntries::VisualizationEntries_FrequencyColorSpinningTower,                 FrequencyColorSpinningTower::GetInstance,              NULL,                              false,         true,             true,         true,         false,          false },
{ VisualizationEntries::VisualizationEntries_UpDownFrequencyColorStreamer,                UpDownFrequencyColorStreamer::GetInstance,             NULL,                              false,         true,             true,         false,        false,          false },
{ VisualizationEntries::VisualizationEntries_UpDownMaxFrequencyStreamer,                  UpDownMaxFrequencyStreamer::GetInstance,               NULL,                              false,         true,             true,         false,        false,          false },
{ VisualizationEntries::VisualizationEntries_ScrollingRainbow,                            ScrollingRainbow::GetInstance,                         NULL,                              false,         false,            true,         true,         false,          false },
{ VisualizationEntries::VisualizationEntries_FadingColors2,                               FadingColors2::GetInstance,                            NULL,                              false,         false,            true,         true,         false,          false },
{ VisualizationEntries::VisualizationEntries_ScrollingFrequencyColorRectangles,           ScrollingFrequencyColorRectangles::GetInstance,        NULL,                              false,         true,             true,         false,        false,          false },
{ VisualizationEntries::VisualizationEntries_ScrollingFrequencySprites,                   ScrollingFrequencySprites::GetInstance,                NULL,                              false,         true,             true,         false,        false,          false },
{ VisualizationEntries::VisualizationEntries_ScrollingSpeedFrequencySprites,              ScrollingSpeedFrequencySprites::GetInstance,           NULL,                              false,         true,             true,         false,        false,          false },
{ VisualizationEntries::VisualizationEntries_ScrollingAmplitudeSprite,                    ScrollingAmplitudeSprite::GetInstance,                 NULL,                              false,         true,             true,         false,        false,          false },
{ VisualizationEntries::VisualizationEntries_Opposites,                                   Opposites::GetInstance,                                NULL,                              false,         true,             true,         false,        false,          false },
{ VisualizationEntries::VisualizationEntries_Snake,                                       Snake::GetInstance,                                    NULL,                              false,         true,             true,         false,        false,          false }
};


class VisualizationsFactory
{
  public:
    VisualizationsFactory(){}
    int GetNumberOfVisualizationType(VisualizationType visualizationType);
    VisualizationConfig GetVisualizationConfigForTypeAtIndex(VisualizationType visualizationType, int index);
    VisualizationConfig GetNextVisualizationConfigForTypeAfterIndex(VisualizationType visualizationType, VisualizationEntries entry);
    VisualizationConfig GetRandomBackgroundVisualizationConfig();
    VisualizationConfig GetRandomForegroundVisualizationConfig();
    VisualizationEntries GetRandomStaticVisualizationEntry();
    VisualizationConfig GetVisualizationConfig(VisualizationEntries visualization);
};


#endif
