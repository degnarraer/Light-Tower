
#include "Views.h"

void View::Setup()
{
  if(true == debugLEDs) Serial << "Setup View\n";
  m_PixelArray = new PixelArray(m_X, m_Y, m_W, m_H);
  SetupView();
}

void View::MergeSubViews()
{
  for(int v = 0; v < m_SubViews.size(); ++v)
  {
    View *aView = m_SubViews.get(v);
    for(int x = m_X; x < m_X+m_W; ++x)
    {
      for(int y = m_Y; y < m_Y+m_H; ++y)
      {
        if(true == debugLEDs) Serial << "Pixel Value " << "\tR:" << aView->GetPixel(x, y).red << "\tG:" << aView->GetPixel(x, y).green << "\tB:" << aView->GetPixel(x, y).blue << "\n";
        if( aView->GetPixel(x, y).red != 0 || aView->GetPixel(x, y).green != 0 || aView->GetPixel(x, y).blue != 0 )
        {
          switch(aView->GetMergeType())
          {
            case MergeType_Layer:
              if(true == debugLEDs) Serial << "Set Pixel " << x << "|" << y << " to: " << "\tR:" << aView->GetPixel(x, y).red << "\tG:" << aView->GetPixel(x, y).green << "\tB:" << aView->GetPixel(x, y).blue << "\n";
              m_PixelArray->SetPixel(x, y, aView->GetPixel(x, y));
            break;
            case MergeType_Add:
            {
              if(true == debugLEDs) Serial << "Set Pixel " << x << "|" << y << " to: " << "\tR:" << aView->GetPixel(x, y).red << "\tG:" << aView->GetPixel(x, y).green << "\tB:" << aView->GetPixel(x, y).blue << "\n";
              CRGB pixel;
              pixel.red = qadd8(aView->GetPixel(x, y).red, m_PixelArray->GetPixel(x, y).red);
              pixel.blue = qadd8(aView->GetPixel(x, y).blue, m_PixelArray->GetPixel(x, y).blue);
              pixel.green = qadd8(aView->GetPixel(x, y).green, m_PixelArray->GetPixel(x, y).green);
              m_PixelArray->SetPixel(x, y, pixel);
            }
            break;
            default:
            break;
          }
        }
      }
    }
  }
}

void VerticalBarView::RunViewTask()
{
  m_ScaledHeight = (m_Y + round(m_HeightScalar*(float)m_H));
  if(m_ScaledHeight > m_Y + m_H) m_ScaledHeight = m_Y + m_H;
  if(true == debugLEDs) Serial << "Coords: " << m_X << "|" << m_Y << "|" << m_W << "|" << m_H << " Scaled Height: " << m_ScaledHeight << "\n";
  for(int x = m_X; x<m_X+m_W; ++x)
  {
    for(int y = 0; y<m_Y+m_H; ++y)
    {
      if( (x >= m_X) && (x < (m_X + m_W)) && (y >= m_Y) && (y < m_ScaledHeight) )
      {
        m_PixelArray->SetPixel(x, y, m_Color);
      }
      else
      {
        m_PixelArray->SetPixel(x, y, CRGB::Black);
      }
    }
  }
}
void ScrollingView::RunViewTask()
{
  switch(m_ScrollDirection)
  {
    case ScrollDirection_Up:
      if(true == debugView) Serial << "Scroll Up\n";
      for(int x = m_X; x < m_X+m_W; ++x)
      {
        for(int y = m_Y+m_H-1; y >= m_Y; --y)
        {
          if((x >= m_X) && (x < (m_X + m_W)) && (y > m_Y) && (y < (m_Y + m_H)))
          {
            if(true == debugView) Serial << x << "|" << y << ":S ";
            //Shift screen
            m_PixelArray->SetPixel(x, y, m_PixelArray->GetPixel(x, y-1));
          }
          else
          {
            if(true == debugView) Serial << x << "|" << y << ":N ";
            //DO nothing
          }
        }
        if(true == debugView) Serial << "\n";
      }
      break;
    case ScrollDirection_Down:
      if(true == debugView) Serial << "Scroll Down\n";
      for(int x = m_X; x < m_X+m_W; ++x)
      {
        for(int y = m_Y; y < m_Y+m_H; ++y)
        {
          if((x >= m_X) && (x < (m_X + m_W)) && (y >= m_Y) && (y < (m_Y + m_H - 1)))
          {
            if(true == debugView) Serial << x << "|" << y << ":S ";
            //Shift screen
            m_PixelArray->SetPixel(x, y, m_PixelArray->GetPixel(x, y+1));
          }
          else
          {
            if(true == debugView) Serial << x << "|" << y << ":N ";
            //Do nothing
          }
        }
        if(true == debugView) Serial << "\n";
      }
      break;
    default:
    break;
  }
}
void ColorSpriteView::RunViewTask()
{
  if(true == debugView) Serial << "Coords: " << m_X << "|" << m_Y << "|" << m_W << "|" << m_H << "\n";
  for(int x = m_X; x<m_X+m_W; ++x)
  {
    for(int y = m_Y; y<m_Y+m_H; ++y)
    {
        m_PixelArray->SetPixel(x, y, CRGB::Black);
      if( 
          (x >= m_X) && 
          (x < (m_X + m_W)) &&
          (y >= m_Y) && 
          (y < (m_Y + m_H))
        )
      {
        m_PixelArray->SetPixel(x, y, m_MyColor);
      }
    }
  }
}
