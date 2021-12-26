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

//Task Interface
void StatisticalEngineModelInterface::Setup()
{ 
  Serial << GetTitle() << " Setup\n";
  m_StatisticalEngine.ConnectMicrophoneMeasureCallerInterfaceCallback(this);
  AddTask(m_StatisticalEngine);
}
bool StatisticalEngineModelInterface::CanRunMyScheduledTask()
{ 
  m_StatisticalEngine.SetProcessFFTStatus(UsersRequireFFT());
  return true; 
}
void StatisticalEngineModelInterface::RunMyScheduledTask()
{
}

bool BandData::operator == (const BandData& a)
{
  return (true == ((a.Power == Power) && (a.Band == Band) && (a.Color == Color))) ? true : false;
}
bool BandData::operator != (const BandData& a)
{
  return (true == ((a.Power == Power) && (a.Band == Band) && (a.Color == Color))) ? false : true;
}
Print& operator << (Print& os, const BandData& bd)
{
  os << bd.Power << "|" << bd.Power << "|" << bd.Color.red << "|" << bd.Color.green << "|" << bd.Color.blue;
  return os;
}

bool Position::operator==(const Position& a)
{
  return (true == ((a.X == X) && (a.Y == Y))) ? true : false;
}
bool Position::operator!=(const Position& a)
{
  return (true == ((a.X == X) && (a.Y == Y))) ? false : true;
}
Print& operator<<(Print& os, const Position& pos)
{
  os << pos.X << "|" << pos.Y;
  return os;
}

bool Size::operator==(const Size& a)
{
  return (true == ((a.W == W) && (a.H == H))) ? true : false;
}
bool Size::operator!=(const Size& a)
{
  return (true == ((a.W == W) && (a.H == H))) ? false : true;
}

bool Coordinates::operator==(const Coordinates& a)
{
  return (true == ((a.Position.X == Position.X) && (a.Position.Y == Position.Y) && (a.Size.W == Size.W) && (a.Size.H == Size.H))) ? true : false;
}
bool Coordinates::operator!=(const Coordinates& a)
{
  return (true == ((a.Position.X == Position.X) && (a.Position.Y == Position.Y) && (a.Size.W == Size.W) && (a.Size.H == Size.H))) ? false : true;
}

//StatisticalEngine Getters
unsigned int StatisticalEngineModelInterface::GetNumberOfBands() {
  return m_StatisticalEngine.GetNumberOfBands();
}
float StatisticalEngineModelInterface::GetNormalizedSoundPower() {
  return m_StatisticalEngine.GetNormalizedSoundPower();
}
float StatisticalEngineModelInterface::GetBandAverage(unsigned int band, unsigned int depth) {
  return m_StatisticalEngine.GetBandAverage(band, depth);
}
float StatisticalEngineModelInterface::GetBandAverageForABandOutOfNBands(unsigned int band, unsigned int depth, unsigned int totalBands) {
  return m_StatisticalEngine.GetBandAverageForABandOutOfNBands(band, depth, totalBands);
}
float StatisticalEngineModelInterface::GetBandValue(unsigned int band, unsigned int depth) {
  return m_StatisticalEngine.GetBandValue(band, depth);
}

void Model::Setup()
{
  SetupModel();
}
bool Model::CanRunMyScheduledTask()
{
  return CanRunModelTask();
}
void Model::RunMyScheduledTask()
{
  RunModelTask();
  UpdateValue();
}
CRGB Model::FadeColor(CRGB color, float scalar)
{
  CHSV hsv = rgb2hsv_approximate(color);
  CRGB rgb;
  hsv.value = (uint8_t )floor(hsv.value*scalar);
  hsv2rgb_rainbow(hsv, rgb);
  return rgb;
}
CRGB Model::GetColor(unsigned int numerator, unsigned int denominator)
{
  CHSV hsv;
  CRGB rgb;
  uint8_t  hue = (uint8_t )floor(((float)numerator/(float)denominator*(float)255));
  uint8_t  saturation = 255;
  uint8_t  value = 255;
  hsv = CHSV(hue, saturation, value);
  hsv2rgb_rainbow(hsv, rgb);
  return rgb;
}
CRGB Model::GetRandomNonGrayColor()
{ 
  CHSV hsv;
  CRGB rgb;
  uint8_t  hue = (uint8_t )random(0, 256);
  uint8_t  saturation = 255;
  uint8_t  value = 255;
  hsv = CHSV(hue, saturation, value);
  hsv2rgb_rainbow(hsv, rgb);
  return rgb;
}

void StatisticalEngineModelInterfaceUserTracker::RegisterAsUser(StatisticalEngineModelInterfaceUsers &user)
{
  m_MyUsers.add(&user);
}
void StatisticalEngineModelInterfaceUserTracker::DeRegisterAsUser(StatisticalEngineModelInterfaceUsers &user)
{
  for (int i = 0; i < m_MyUsers.size(); ++i)
  {
    if (m_MyUsers.get(i) == &user)
    {
      m_MyUsers.remove(i);
      break;
    }
  }
}
bool StatisticalEngineModelInterfaceUserTracker::UsersRequireFFT()
{
  bool result = false;
  for (int u = 0; u < m_MyUsers.size(); ++u)
  {
    if (true == m_MyUsers.get(u)->RequiresFFT())
    {
      result = true;
      break;
    }
  }
  return result;
}

void DataModel::Setup()
{
  SetupModel();
}
bool DataModel::CanRunMyScheduledTask()
{
  return CanRunModelTask();
}
void DataModel::RunMyScheduledTask()
{
  RunModelTask();
  UpdateValue();
}

void RandomColorFadingModel::UpdateValue()
{
  if(true == debugModels) Serial << "RandomColorFadingModel value:  R:" << m_CurrentColor.red << "\tG:" << m_CurrentColor.green << "\tB:" << m_CurrentColor.blue << "\n";
  SetCurrentValue( m_CurrentColor );
}
void RandomColorFadingModel::StartFadingNextColor()
{
  m_StartTime = millis();
  m_StartColor = m_CurrentColor;
  m_EndColor = GetRandomNonGrayColor();
}
void RandomColorFadingModel::SetupModel()
{
  m_CurrentColor = m_StartColor = GetRandomNonGrayColor();
  m_EndColor = GetRandomNonGrayColor();
  m_CurrentTime = m_StartTime = millis();
}
bool RandomColorFadingModel::CanRunModelTask()
{ 
  return true;
}
void RandomColorFadingModel::RunModelTask()
{
  m_CurrentTime = millis();
  m_CurrentDuration = m_CurrentTime - m_StartTime;
  if(m_CurrentDuration > m_Duration)
  {
    StartFadingNextColor();
  }
  else
  {
    float normalization = (float)m_CurrentDuration / (float)m_Duration;
    if(true == debugModels) Serial << "FadeController: Current Duration: " << m_CurrentDuration << " Duration: " << m_Duration << " Fade Normalizatilon:" << normalization << "\n";
    m_CurrentColor.red = (byte)(m_StartColor.red + (((float)m_EndColor.red - (float)m_StartColor.red)*normalization));
    m_CurrentColor.green = (byte)(m_StartColor.green + (((float)m_EndColor.green - (float)m_StartColor.green)*normalization));
    m_CurrentColor.blue = (byte)(m_StartColor.blue + (((float)m_EndColor.blue - (float)m_StartColor.blue)*normalization));
  }
}

void ColorFadingModel::ColorFadingModel::UpdateValue()
{
  if(true == debugModels) Serial << "RandomColorFadingModel value:  R:" << m_CurrentColor.red << "\tG:" << m_CurrentColor.green << "\tB:" << m_CurrentColor.blue << "\n";
  SetCurrentValue( m_CurrentColor );
}

void ColorFadingModel::NewValueNotification(CRGB value, String context) 
{
  m_CurrentTime = millis();
  m_CurrentDuration = m_CurrentTime - m_StartTime;
  if(m_CurrentDuration > m_MinimumUpdateTime)
  {
    m_StartTime = millis();
    m_StartColor = m_CurrentColor;
    m_EndColor = value;
  }
}

void ColorFadingModel::NewValueNotification(BandData value, String context) 
{
  m_CurrentTime = millis();
  m_CurrentDuration = m_CurrentTime - m_StartTime;
  if(m_CurrentDuration > m_MinimumUpdateTime)
  {
    m_StartTime = millis();
    m_StartColor = m_CurrentColor;
    m_EndColor = FadeColor(value.Color, value.Power); 
  }
}
void ColorFadingModel::SetupModel()
{
  m_StartColor = CRGB::Black;
  m_CurrentColor = CRGB::Black;
  m_EndColor = CRGB::Black;
}
bool ColorFadingModel::CanRunModelTask()
{ 
  return true;
}
void ColorFadingModel::RunModelTask()
{;
  m_CurrentTime = millis();
  m_CurrentDuration = m_CurrentTime - m_StartTime;
  float normalization = (float)m_CurrentDuration / (float)m_Duration;
  if(normalization <= 1.0)
  {
    if(true == debugModels) Serial << "FadeController: Current Duration: " << m_CurrentDuration << "Normalizatilon:" << normalization << "\t" << m_CurrentColor.red << "|" << m_CurrentColor.green << "|" << m_CurrentColor.blue << "\t" << m_EndColor.red << "|" << m_EndColor.green << "|" << m_EndColor.blue << "\n";
    m_CurrentColor.red = (byte)(m_StartColor.red + (((float)m_EndColor.red - (float)m_StartColor.red)*normalization));
    m_CurrentColor.green = (byte)(m_StartColor.green + (((float)m_EndColor.green - (float)m_StartColor.green)*normalization));
    m_CurrentColor.blue = (byte)(m_StartColor.blue + (((float)m_EndColor.blue - (float)m_StartColor.blue)*normalization));
  }
  else
  {
    m_CurrentColor = m_EndColor;
  }
}
