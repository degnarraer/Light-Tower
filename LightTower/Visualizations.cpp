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
 * @file Visualizations.cpp
 *
 *
 */

#include "Visualizations.h"
#include "Statistical_Engine.h"
#include "VisualizationFactory.h"

void Visualizations::Started()
{
  m_visualizationState = VisualizationState::VISUALIZATION_STATE_STARTED;
  if(VISUALIZATION == m_visualizationType)
  {
    m_callee->VisualizationStarted(this);
  }
  else if(CONFIRMATION == m_visualizationType)
  {
    m_callee->ConfirmationVisualizationStarted(this);
  }
  else if(TRANSITION == m_visualizationType)
  {
    m_callee->TransitionStarted(this);
  }
}
void Visualizations::Ended()
{
  m_visualizationState = VISUALIZATION_STATE_STOPPED;
  if(VISUALIZATION == m_visualizationType)
  {
    m_callee->VisualizationEnded(this);
  }
  else if(CONFIRMATION == m_visualizationType)
  {
    m_callee->ConfirmationVisualizationEnded(this);
  }
  else if(TRANSITION == m_visualizationType)
  {
    m_callee->TransitionEnded(this);
  }
}
void Visualizations::ResetTimer(int timer)
{
  m_timers[timer] = millis();
}

void Visualizations::ResetAllTimers()
{
  unsigned long currentTime = millis();
  for(int i = 0; i < NUMBER_OF_TICK_TIMERS; ++i)
  {
    m_timers[i] = currentTime;
  }
  m_resetTimer = currentTime;
}

bool Visualizations::AllStripsAreBlack()
{
  bool result = ((m_strips)[0] == CRGB::Black && (m_strips)[1] == CRGB::Black && (m_strips)[2] == CRGB::Black && (m_strips)[3] == CRGB::Black);
  return result;
}

void Visualizations::SetAllLayersToColor(CRGB color)
{
  for(int i = 0; i < NUMBER_OF_LAYERS; ++i)
  {
    m_layer[i] = color;
  }
}

CRGB Visualizations::GetColor(int index, int maximum)
{
  int colorCount = 7.0;
  float calculation = ((float)((float)index/(float)maximum)*colorCount);
  double whole;
  double fractional = modf(calculation, &whole);
  CRGB value;
  CRGB value1;
  CRGB value2;

  int switchValue1 = (int)whole;
  int switchValue2 = (int)whole + 1;
  if(switchValue2 > colorCount - 1) switchValue2 = colorCount - 1;
  
  switch(switchValue1)
  {
    case 0:
      value1 = CRGB::Red;
    break;
    case 1:
      value1 = CRGB::Orange;
    break;
    case 2:
      value1 = CRGB::Yellow;
    break;
    case 3:
      value1 = CRGB::Green;
    break;
    case 4:
      value1 = CRGB::Blue;
    break;
    case 5:
      value1 = CRGB::Indigo;
    break;
    case 6:
      value1 = CRGB::Violet;
    break;
    default:
      value1 = CRGB::Violet;
    break;
  }
  switch(switchValue2)
  {
    case 0:
      value2 = CRGB::Red;
    break;
    case 1:
      value2 = CRGB::Orange;
    break;
    case 2:
      value2 = CRGB::Yellow;
    break;
    case 3:
      value2 = CRGB::Green;
    break;
    case 4:
      value2 = CRGB::Blue;
    break;
    case 5:
      value2 = CRGB::Indigo;
    break;
    case 6:
      value2 = CRGB::Violet;
    break;
    default:
      value2 = CRGB::Violet;
    break;
  }
  value.red = value1.red + fractional*(value2.red - value1.red);
  value.green = value1.green + fractional*(value2.green - value1.green);
  value.blue = value1.blue + fractional*(value2.blue - value1.blue);
  return value;
}

void Visualizations::ShiftStrip(LEDStrip &strip, ShiftDirection direction, int spaces)
{
  switch(direction)
  {
    case Down:
      for(int i = 0; i < NUMLEDS - 1; ++i)
      {
        strip(i, i) = strip(i+1, i+1);
      }
      strip(NUMLEDS-1, NUMLEDS-1) = CRGB::Black;
    break;
    case Up:
      for(int i = NUMLEDS - 1; i > 0; --i)
      {
        strip(i, i) = strip(i-1, i-1);
      }
      strip(0, 0) = CRGB::Black;
    break;
    default:
    break;
  }
}

LEDStrip Visualizations::AddLayer(LEDStrip layer1, LEDStrip layer2)
{
  LEDStrip result;
  for (int i = 0; i < NUMLEDS; ++i)
  {
    result[i] = layer1[i] + layer2[i];
  }
  return result;
}
LEDStrip Visualizations::MergeLayer(LEDStrip &layer1, LEDStrip &layer2)
{
  LEDStrip result;
  for (int i = 0; i < NUMLEDS; ++i)
  {
    if(layer1(i, i) != CRGB::Black)
    {
      result(i, i) = layer1(i, i);
    }
    else
    {
      result(i, i) = layer2(i, i);
    }
  }
  return result;
}
LEDStrip Visualizations::MergeLayerFlipZOrder(LEDStrip &layer1, LEDStrip &layer2)
{
  LEDStrip result;
  for (int i = 0; i < NUMLEDS; ++i)
  {
    if(i < NUMLEDS/2-1)
    {
      if(layer1(i, i) != CRGB::Black)
      {
        result(i, i) = layer1(i, i);
      }
      else
      {
        result(i, i) = layer2(i, i);
      }
    }
    else
    {
      if(layer2(i, i) != CRGB::Black)
      {
        result(i, i) = layer2(i, i);
      }
      else
      {
        result(i, i) = layer1(i, i);
      }
      
    }
  }
  return result;
}
LEDStrip Visualizations::FadeInLayerBy(LEDStrip layer, byte fadeAmount)
{
  LEDStrip result;
  result = CRGB::Black;
  for(int i = 0; i < NUMLEDS; ++i)
  {
    result[i] += CRGB {fadeAmount, fadeAmount, fadeAmount};
    if(result[i].red > layer[i].red) result[i].red = layer[i].red;
    if(result[i].green > layer[i].green) result[i].green = layer[i].green;
    if(result[i].blue > layer[i].blue) result[i].blue = layer[i].blue;
  }
  return result;
}
LEDStrip Visualizations::FadeOutLayerBy(LEDStrip layer, byte fadeAmount)
{
  LEDStrip result;
  result = layer;
  for(int i = 0; i < NUMLEDS; ++i)
  {
    result[i] -= CRGB {fadeAmount, fadeAmount, fadeAmount};
  }
  return result;
}

void Visualizations::SpiralTowerUpwards()
{
  for(int led = NUMLEDS-1; led >= 0; --led)
    {
      for(int strip = NUMSTRIPS-1; strip >= 0; --strip)
      {
        if(led >= 0 && strip > 0)
        {
          (m_strips)[strip](led, led) = (m_strips)[strip-1](led, led);
        }
        else
        {
          (m_strips)[strip](led, led) = (m_strips)[NUMSTRIPS-1](led-1, led-1);
        }
    }
  }
}

void Visualizations::SpiralTowerDownwards()
{
  for(int i = 0; i < NUMLEDS; ++i)
  {
    for(int j = 0; j <= NUMSTRIPS-1; ++j)
    {
      if(j >= NUMSTRIPS-1 && i <= NUMLEDS-1)
      {
        (m_strips)[j](i, i) = (m_strips)[0](i+1, i+1);
      }
      else
      {
        (m_strips)[j](i, i) = (m_strips)[j+1](i, i);
      }
    }
  }
}

void FadeController::ConfigureFadeController(CRGB startColor, CRGB endColor, unsigned int duration)
{
  m_startColor = startColor;
  m_endColor = endColor;
  m_duration = duration;
  m_currentTickOfDuration = 0;
}

CRGB FadeController::IncrementFade(unsigned int incrementValue)
{
  float normalization = 1.0;
  m_currentTickOfDuration = m_currentTickOfDuration + incrementValue;
  if(m_currentTickOfDuration > m_duration) m_currentTickOfDuration = m_duration;
  normalization = ((float)m_currentTickOfDuration / (float)m_duration);
  if(true == debugMode && debugLevel >= 5) Serial << "FadeController::IncrementFade: " << normalization << "\n";
  m_currentColor.red = (byte)(m_startColor.red + (((float)m_endColor.red - (float)m_startColor.red) * normalization));
  m_currentColor.green = (byte)(m_startColor.green + (((float)m_endColor.green - (float)m_startColor.green) * normalization));
  m_currentColor.blue = (byte)(m_startColor.blue + (((float)m_endColor.blue - (float)m_startColor.blue) * normalization));
  return m_currentColor;
}
void FadeController::ResetFade()
{
  m_currentTickOfDuration = 0;
}

CRGB Visualizations::GetRandomColor(byte minRed, byte maxRed, byte minGreen, byte maxGreen, byte minBlue, byte maxBlue)
{
  byte red = random(minRed, maxRed);
  byte green = random(minGreen, maxGreen);
  byte blue = random(minBlue, maxBlue);
  return {red, green, blue};
}


CRGB Visualizations::GetRandomNonGrayColor()
{
  byte threshold = 25;
  byte red = random(0, 255);
  byte green = random(0, 255);
  byte blue = random(0, 255);

  int redGreenDiff = abs(red - green);
  int blueRedDiff = abs(blue - red);
  int greenBlueDiff = abs(green - blue);
  
  if(redGreenDiff < threshold && blueRedDiff < threshold && greenBlueDiff < threshold)
  {
    if(redGreenDiff < threshold)
    {
      red += threshold;
      if(red > 255) red-= 255;
    }
    else if(blueRedDiff < threshold)
    {
      blue += threshold;
      if(blue > 255) blue-= 255;
    }
    else if(greenBlueDiff < threshold)
    {
      green += threshold;
      if(green > 255) green-= 255;
    }
  }
  return {red, green, blue};
}

float Visualizations::GetTriggerGainForFrequency(float frequency)
{
  return 1.5 - 0.5*(frequency/MAX_DISPLAYED_FREQ);
}
