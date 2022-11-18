#include "Models_Color.h"

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
