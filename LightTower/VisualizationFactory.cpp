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

#ifndef VisualizationFactory_C
#define VisualizationFactory_C

#include "VisualizationFactory.h"

int VisualizationsFactory::GetNumberOfVisualizationType(VisualizationType visualizationType)
{
  int numberOfConfigs = (sizeof(visualizationConfig)/sizeof(*visualizationConfig));
  int count = 0;
  bool value = false;
  for(int i = 0; i < numberOfConfigs; ++i)
  {
    switch(visualizationType)
    {
      case TRANSITION:
        value = visualizationConfig[i].isTransition;
      break;
      case VISUALIZATION:
        value = visualizationConfig[i].isVisualization;
      break;
      case BACKGROUND:
        value = visualizationConfig[i].isBackground;
      break;
      case FOREGROUND:
        value = visualizationConfig[i].isForeground;
      break;
      case CONFIRMATION:
        value = visualizationConfig[i].isConfirmation;
      break;
      case TUNER:
        value = visualizationConfig[i].isTuner;
      break;
      default:
      break;
    }
    if(true == value) ++count;
  }
  return count;
}
VisualizationConfig VisualizationsFactory::GetVisualizationConfigForTypeAtIndex(VisualizationType visualizationType, int index)
{
  VisualizationConfig resultingConfig;
  int numberOfConfigs = (sizeof(visualizationConfig)/sizeof(*visualizationConfig));
  int count = 0;
  bool value = false;
  for(int i = 0; i < numberOfConfigs; ++i)
  {
    switch(visualizationType)
    {
      case TRANSITION:
        value = visualizationConfig[i].isTransition;
      break;
      case VISUALIZATION:
        value = visualizationConfig[i].isVisualization;
      break;
      case BACKGROUND:
        value = visualizationConfig[i].isBackground;
      break;
      case FOREGROUND:
        value = visualizationConfig[i].isForeground;
      break;
      case CONFIRMATION:
        value = visualizationConfig[i].isConfirmation;
      break;
      case TUNER:
        value = visualizationConfig[i].isTuner;
      break;
      default:
      break;
    }
    if(true == value)
    {
      value = false;
      if(count == index) resultingConfig = visualizationConfig[i];
      ++count;
    }
  }
  return resultingConfig;
}

VisualizationConfig VisualizationsFactory::GetNextVisualizationConfigForTypeAfterIndex(VisualizationType visualizationType, VisualizationEntries entry)
{
  int numberOfConfigs = (sizeof(visualizationConfig)/sizeof(*visualizationConfig));
  int firstConfigIndex = -1;
  int foundConfigIndex = -1;
  bool value = false;
  for(int i = 0; i < numberOfConfigs; ++i)
  {
    switch(visualizationType)
    {
      case TRANSITION:
        value = visualizationConfig[i].isTransition;
      break;
      case VISUALIZATION:
        value = visualizationConfig[i].isVisualization;
      break;
      case BACKGROUND:
        value = visualizationConfig[i].isBackground;
      break;
      case FOREGROUND:
        value = visualizationConfig[i].isForeground;
      break;
      case CONFIRMATION:
        value = visualizationConfig[i].isConfirmation;
      break;
      case TUNER:
        value = visualizationConfig[i].isTuner;
      break;
      default:
        value = false;
      break;
    }
    if(true == value)
    {
      value = false;
      if(firstConfigIndex < 0 && visualizationConfig[i].visualizationEntry != entry) 
      {
        firstConfigIndex = i;
      }
      else if(visualizationConfig[i].visualizationEntry == entry)
      {
        foundConfigIndex = i;
      }
      else if(foundConfigIndex >= 0)
      {
        return visualizationConfig[i];
      }
    }
  }
  if(firstConfigIndex > 0)
  {
    return visualizationConfig[firstConfigIndex];
  }
  else
  {
    return visualizationConfig[foundConfigIndex];
  }
}


VisualizationConfig VisualizationsFactory::GetVisualizationConfig(VisualizationEntries visualizationEntry)
{
  VisualizationConfig resultingConfig;
  int numberOfConfigs = (sizeof(visualizationConfig)/sizeof(*visualizationConfig));
  int count = 0;
  bool value = false;
  for(int i = 0; i < numberOfConfigs; ++i)
  {
    if(visualizationEntry == visualizationConfig[i].visualizationEntry) return visualizationConfig[i];
  }
  return resultingConfig; 
}

VisualizationConfig VisualizationsFactory::GetRandomBackgroundVisualizationConfig()
{
  int count = GetNumberOfVisualizationType(VisualizationType::BACKGROUND);
  return GetVisualizationConfigForTypeAtIndex(VisualizationType::BACKGROUND, random(0, count));
}

VisualizationConfig VisualizationsFactory::GetRandomForegroundVisualizationConfig()
{
  int count = GetNumberOfVisualizationType(VisualizationType::FOREGROUND);
  return GetVisualizationConfigForTypeAtIndex(VisualizationType::FOREGROUND, random(0, count));
}

VisualizationEntries VisualizationsFactory::GetRandomStaticVisualizationEntry()
{
  switch(random(2))
  {
    case 0: 
      //return VisualizationEntries::VisualizationEntries_ColorFadingTower;
      return VisualizationEntries::VisualizationEntries_ScrollingRainbow;
      
    break;
    case 1:
      //return VisualizationEntries::VisualizationEntries_ColorFadingTower;
      return VisualizationEntries::VisualizationEntries_ScrollingRainbow;
    break;
    default:
      return VisualizationEntries::VisualizationEntries_ColorFadingTower;
    break;
  }
}
#endif
