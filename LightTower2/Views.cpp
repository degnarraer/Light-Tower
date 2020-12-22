
#include "Views.h"

void View::Setup()
{
  if(true == debugView) Serial << "Setup View\n";
  for(int x = 0; x < SCREEN_WIDTH; ++x)
  {
    for(int y = 0; y < SCREEN_HEIGHT; ++y)
    {
      m_MyPixelStruct.Pixel[x][y] = CRGB::Black;
      if(true == debugView) Serial << "\tR: " << m_MyPixelStruct.Pixel[x][y].red << "\tG: " << m_MyPixelStruct.Pixel[x][y].green << "\tB: " << m_MyPixelStruct.Pixel[x][y].blue << "\n";
    }
  }
  SetupView();
}
bool View::CanRunMyTask()
{ 
  return CanRunViewTask(); 
}
void View::RunMyTask()
{
  RunViewTask();
}
void VerticalBarView::SetColor(CRGB Color)
{ 
  m_Color = Color; 
}
void VerticalBarView::SetNormalizedHeight(float Height) 
{ 
  m_HeightScalar = Height;
}
void VerticalBarView::NewValueNotification(float Value)
{
  SetNormalizedHeight(Value);
}
void VerticalBarView::NewValueNotification(CRGB Value)
{
  m_Color = Value; 
}
void VerticalBarView::SetupView()
{

}
bool VerticalBarView::CanRunViewTask()
{ 
  return true; 
}
void VerticalBarView::RunViewTask()
{
  int scaledHeight = (m_Y + round(m_HeightScalar*(float)m_H));
  if(scaledHeight > m_Y + m_H) scaledHeight = m_Y + m_H;
  if(true == debugLEDs) Serial << "Coords: " << m_X << "|" << m_Y << "|" << m_W << "|" << m_H << " Scaled Height: " << scaledHeight << "\n";
  for(int x = 0; x<SCREEN_WIDTH; ++x)
  {
    for(int y = 0; y<SCREEN_HEIGHT; ++y)
    {
        m_MyPixelStruct.Pixel[x][y] = CRGB::Black;
      if( 
          (x >= m_X) && 
          (x < (m_X + m_W)) &&
          (y >= m_Y) && 
          (y < scaledHeight)
        )
      {
        m_MyPixelStruct.Pixel[x][y] = m_Color;
      }
    }
  }
  if(true == debugLEDs) Serial << "\n";
  if(true == debugLEDs) Serial << "************\n";
  for(int y = 0; y < SCREEN_HEIGHT; ++y)
  {
    for(int x = 0; x < SCREEN_WIDTH; ++x)
    {
      CRGB bufColor = m_MyPixelStruct.Pixel[x][y];
      if(true == debugLEDs) Serial << bufColor.red << ":" << bufColor.green << ":" << bufColor.blue << " \t";
    }
    if(true == debugLEDs) Serial << "\n";
  }
}
