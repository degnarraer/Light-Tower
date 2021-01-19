
#include "Views.h"

//*************** View ***************
void View::SetPosition(position X, position Y)
{ 
  m_X = X; 
  m_Y = Y;
  m_PixelArray->SetPosition(m_X, m_Y);
}
void View::SetSize(size W, size H){ m_W = W; m_H = H; }
void View::AddSubView(View &SubView)
{ 
  m_SubViews.add(&SubView);
  AddTask(SubView);
}
CRGB View::GetPixel(int X, int Y)
{
  return m_PixelArray->GetPixel(X, Y);
}
bool View::RemoveSubView(View &SubView)
{
  bool viewFound = false;
  for(int i = 0; i < m_SubViews.size(); ++i)
  {
    if(m_SubViews.get(i) == &SubView)
    {
      viewFound = true;
      m_SubViews.remove(i);
      break;
    }
  }
  if(true == viewFound)
  {
    if(true == debugTasks || true == debugMemory) Serial << "View Successfully Removed Subview.\n";
    return true;
  }
  else
  {
    if(true == debugTasks || true == debugMemory) Serial << "View failed to Remove Subview.\n";
    return false;
  }
}
MergeType View::GetMergeType()
{ 
  return m_MergeType;
}
PixelArray* View::GetPixelArray()
{ 
  return m_PixelArray; 
}
void View::Setup()
{
  if(true == debugLEDs) Serial << "Setup View\n";
  m_PixelArray = new PixelArray(m_X, m_Y, m_W, m_H);
  SetupView();
}
bool View::CanRunMyTask()
{
  MergeSubViews(false);
  return CanRunViewTask(); 
}
void View::RunMyTask()
{
  RunViewTask();
}
void View::MergeSubViews(bool clearViewBeforeMerge)
{
  if(true == clearViewBeforeMerge) m_PixelArray->Clear();
  for(int v = 0; v < m_SubViews.size(); ++v)
  {
    View *aView = m_SubViews.get(v);
    int aX = aView->GetPixelArray()->GetX();
    int aY = aView->GetPixelArray()->GetY();
    int aWidth = aView->GetPixelArray()->GetWidth();
    int aHeight = aView->GetPixelArray()->GetHeight();
    for(int x = aX; x <= aX + aWidth - 1; ++x)
    {
      for(int y = aY; y <= aY + aHeight - 1; ++y)
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

//*************** VerticalBarView ***************
void VerticalBarView::SetupView() {}
bool VerticalBarView::CanRunViewTask() { return true; }
void VerticalBarView::RunViewTask()
{
  m_ScaledHeight = (m_Y + round(m_HeightScalar*(float)m_H));
  if(m_ScaledHeight > m_Y + m_H) m_ScaledHeight = m_Y + m_H;
  if(true == debugLEDs) Serial << "Coords: " << m_X << "|" << m_Y << "|" << m_W << "|" << m_H << " Scaled Height: " << m_ScaledHeight << "\n";
  for(int x = m_X; x<m_X+m_W; ++x)
  {
    for(int y = m_Y; y<m_Y+m_H; ++y)
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

void VerticalBarView::NewValueNotification(float value) { m_HeightScalar = value; }
void VerticalBarView::NewValueNotification(CRGB value) { m_Color = value; }
//Model
void VerticalBarView::SetupModel()
{
  m_Peak.X = m_X;
  m_Peak.Y = m_ScaledHeight;
}
bool VerticalBarView::CanRunModelTask()
{
  return true;
}
void VerticalBarView::RunModelTask()
{
  m_Peak.X = m_X;
  m_Peak.Y = m_ScaledHeight;
}
void VerticalBarView::UpdateValue()
{ 
  SetCurrentValue(m_Peak); 
}


//*************** ScrollingView ***************
void ScrollingView::SetupView(){}
bool ScrollingView::CanRunViewTask()
{
  return true; 
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

//*************** ScrollingView ***************
void ColorSpriteView::ConnectColorModel(ModelEventNotificationCaller<CRGB> &caller) { caller.RegisterForNotification(*this); }
void ColorSpriteView::ConnectPositionModel(ModelEventNotificationCaller<Position> &caller) { caller.RegisterForNotification(*this); }
void ColorSpriteView::NewValueNotification(CRGB value) { m_MyColor = value; }
void ColorSpriteView::NewValueNotification(Position value) 
{ 
  m_X = value.X;
  m_Y = value.Y;
  m_PixelArray->SetPosition(m_X, m_Y);
}
void ColorSpriteView::SetupView(){}
bool ColorSpriteView::CanRunViewTask(){ return true; }
void ColorSpriteView::ColorSpriteView::RunViewTask()
{
  if(true == debugView) Serial << "Coords: " << m_X << "|" << m_Y << "|" << m_W << "|" << m_H << "\n";
  for(int x = m_X; x <= m_X + m_W - 1; ++x)
  {
    for(int y = m_Y; y <= m_Y + m_H - 1; ++y)
    {
      m_PixelArray->SetPixel(x, y, m_MyColor);
    }
  }
}

//*************** FadingView ***************
void FadingView::SetupView(){}
bool FadingView::CanRunViewTask(){ return true; }
void FadingView::RunViewTask()
{
  if(m_FadeLength > 0)
  {
    switch(m_Direction)
    {
      case Direction_Up:
        for(int x = m_X; x < m_X+m_W; ++x)
        {
          for(int y = m_Y; y < m_Y+m_H; ++y)
          {
            if((x >= m_X) && (x < (m_X + m_W)) && (y > m_Y) && (y < (m_Y + m_H)))
            {
              PerformFade(x, y, y);
            }
          }
        }
        break;
      case Direction_Down:
        for(unsigned int x = m_X; x < m_X+m_W; ++x)
        {
          unsigned int index = 0;
          for(unsigned int y = m_Y+m_H-1; y > m_Y; --y)
          {
            if((x >= m_X) && (x < (m_X + m_W)) && (y >= m_Y) && (y < (m_Y + m_H - 1)))
            {
              PerformFade(x, y, index);
            }
            ++index;
          }
        }
        break;
      default:
      break;
    }
  }
}
void FadingView::PerformFade(unsigned int x, unsigned int y, unsigned int i)
{
  float normalizedFade = 1.0 - ((float)i / (float)m_FadeLength);
  CRGB pixel;
  pixel.red = m_PixelArray->GetPixel(x, y).red * normalizedFade;
  pixel.green = m_PixelArray->GetPixel(x, y).green * normalizedFade;
  pixel.blue = m_PixelArray->GetPixel(x, y).blue * normalizedFade;
  m_PixelArray->SetPixel(x, y, pixel);
}

//*************** RotatingView ***************
void RotatingView::SetupView()
{
  m_ResultingPixelArray = new PixelArray(m_X, m_Y, m_W, m_H);
  m_ResultingPixelArray->Clear();
  m_PixelArray->Clear();
  m_startMillis = millis();
}
bool RotatingView::CanRunViewTask()
{
  switch(m_RotationType)
  {
    case RotationType_Static:
      RotateView();
    break;
    case RotationType_Scroll:
    break;
    default:
    break;
  }
  *m_PixelArray = *m_ResultingPixelArray;
   
  m_currentMillis = millis();
  m_lapsedTime = m_currentMillis - m_startMillis;
  if(m_lapsedTime >= m_updatePeriodMillis)
  {
    return true;
  }
  else
  {
    return false;
  }
}
void RotatingView::RunViewTask()
{
  m_startMillis = millis();
  switch(m_RotationType)
  {
    case RotationType_Static:
      ++m_Count;
      RotateView();
    break;
    case RotationType_Scroll:
      ScrollView();
    break;
    default:
    break;
  }
  *m_PixelArray = *m_ResultingPixelArray;
}
void RotatingView::ScrollView()
{
  for(int x = m_X; x <= m_X+m_W-1; ++x)
  {
    for(int y = m_Y; y <= m_Y+m_H-1; ++y)
    {
      switch(m_Direction)
      {
        case Direction_Up:
        {
          int source = y-1;
          if(source < m_Y) { source = m_Y + m_H - 1; }
          m_ResultingPixelArray->SetPixel(x, y, m_PixelArray->GetPixel(x, source));
        }
        break;
        case Direction_Down:
        {
          int source = y+1;
          if(source >= m_Y + m_H - 1) { source = m_Y; }
          m_ResultingPixelArray->SetPixel(x, y, m_PixelArray->GetPixel(x, source));
        }
        break;
        case Direction_Right:
        {
          int source = x-1;
          if(source < m_X) { source = m_X + m_W - 1; }
          m_ResultingPixelArray->SetPixel(x, y, m_PixelArray->GetPixel(source, y));
        }
        break;
        case Direction_Left:
        {
          int source = x+1;
          if(source >= m_X + m_W - 1) { source = m_X; };
          m_ResultingPixelArray->SetPixel(x, y, m_PixelArray->GetPixel(source, y));
        }
        break;
        default:
        break;
      }
    }
  }
}
void RotatingView::RotateView()
{
  for(int x = m_X; x <= m_X+m_W-1; ++x)
  {
    for(int y = m_Y; y <= m_Y+m_H-1; ++y)
    {
      switch(m_Direction)
      {
        case Direction_Up:
        {
          int offset = m_Count%m_H;
          int source = y-offset;
          if(source < m_Y) { source = m_Y + m_H + source; }
          m_ResultingPixelArray->SetPixel(x, y, m_PixelArray->GetPixel(x, source));
        }
        break;
        case Direction_Down:
        {
          int offset = m_Count%m_H;
          int source = y+offset;
          if(source > m_Y + m_H - 1) { source = source - (m_Y + m_H); }
          m_ResultingPixelArray->SetPixel(x, y, m_PixelArray->GetPixel(x, source));
        }
        break;
        case Direction_Right:
        {
          int offset = m_Count%m_W;
          int source = x-offset;
          if(source < m_X) { source = m_X + m_W + source; }
          m_ResultingPixelArray->SetPixel(x, y, m_PixelArray->GetPixel(source, y));
        }
        break;
        case Direction_Left:
        {
          int offset = m_Count%m_W;
          int source = x+offset;
          if(source > m_X + m_W - 1) { source = source - (m_X + m_W); };
          m_ResultingPixelArray->SetPixel(x, y, m_PixelArray->GetPixel(source, y));
        }
        break;
        default:
        break;
      }
    }
  }
}
