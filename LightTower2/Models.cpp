/*
    Light Tower by Rob Shockency
    Copyright (C) 2020 Rob Shockency degnarraer@yahoo.com

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version of the License, or
    (at your option) any later version. 3

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
 
#include "Models.h"

void StatisticalEngineModelInterface::Setup()
{ 
  m_StatisticalEngine.ConnectCallback(this);
  AddTask(m_StatisticalEngine);
}
bool StatisticalEngineModelInterface::CanRunMyTask()
{ 
  m_StatisticalEngine.SetProcessFFTStatus(UsersRequireFFT());
  return true; 
}
void StatisticalEngineModelInterface::RunMyTask()
{
}

void RandomColorFadingModel::UpdateValue()
{
  if(true == debugModels) Serial << "RandomColorFadingModel value:  R:" << m_CurrentColor.red << "\tG:" << m_CurrentColor.green << "\tB:" << m_CurrentColor.blue << "\n";
  SetCurrentValue( m_CurrentColor );
}
void RandomColorFadingModel::StartFadingNextColor()
{
  m_StartColor = m_CurrentColor;
  m_EndColor = GetRandomNonGrayColor();
  m_StartTime = millis();
}
CRGB RandomColorFadingModel::GetRandomNonGrayColor()
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


void RandomColorFadingModel::SetupModel()
{
  m_StartColor = GetRandomNonGrayColor();
  m_EndColor = GetRandomNonGrayColor();
  m_StartTime = millis();
}
bool RandomColorFadingModel::CanRunModelTask()
{ 
  return true;
}
void RandomColorFadingModel::RunModelTask()
{
    float normalization = 1.0;
    m_CurrentTime = millis();
    m_CurrentDuration = m_CurrentTime - m_StartTime;
    if(m_CurrentDuration > m_Duration)
    {
      StartFadingNextColor();
    }
    else
    {
      normalization = (float)m_CurrentDuration / (float)m_Duration;
      if(true == debugModels) Serial << "FadeController: Current Duration: " << m_CurrentDuration << " Duration: " << m_Duration << " Fade Normalizatilon:" << normalization << "\n";
      m_CurrentColor.red = (byte)(m_StartColor.red + (((float)m_EndColor.red - (float)m_StartColor.red) * normalization));
      m_CurrentColor.green = (byte)(m_StartColor.green + (((float)m_EndColor.green - (float)m_StartColor.green) * normalization));
      m_CurrentColor.blue = (byte)(m_StartColor.blue + (((float)m_EndColor.blue - (float)m_StartColor.blue) * normalization)); 
    }
}

CRGB RainbowColorModel::GetColor(unsigned int numerator, unsigned int denominator)
{
  const int colorCount = 7.0;
  float calculation = ((float)((float)numerator/(float)denominator)*colorCount);
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
