
#include "Views.h"

//*************** View ***************
void View::SetPosition(position x, position y)
{ 
  m_X = x; 
  m_Y = y;
  m_PixelArray->SetPosition(x, y);
}
void View::SetSize(size w, size h){ m_W = w; m_H = h; }
void View::AddSubView(View &subView)
{ 
  SubViewWithProperties_t sVWP;
  sVWP.SubView = &subView;
  m_SubViewWithProperties.add(0, sVWP);
  AddTask(subView);
}
void View::AddSubView(View &subView, bool clearViewBeforeMerge)
{ 
  SubViewWithProperties_t sVWP;
  sVWP.SubView = &subView;
  sVWP.ClearViewBeforeMerge = clearViewBeforeMerge;
  m_SubViewWithProperties.add(0, sVWP);
  AddTask(subView);
}
void View::AddSubView(View &subView, bool clearViewBeforeMerge, position x, position y, size width, size height )
{
  SubViewWithProperties_t sVWP;
  sVWP.SubView = &subView;
  sVWP.ClearViewBeforeMerge = clearViewBeforeMerge;
  sVWP.SpecifiedClearArea = true;
  sVWP.X_To_Clear = x;
  sVWP.Y_To_Clear = y;
  sVWP.W_To_Clear = width;
  sVWP.H_To_Clear = height;
  m_SubViewWithProperties.add(0, sVWP);
  AddTask(subView);
}
CRGB View::GetPixel(int x, int y)
{
  return m_PixelArray->GetPixel(x, y);
}
bool View::RemoveSubView(View &subView)
{
  bool viewFound = false;
  for(int i = 0; i < m_SubViewWithProperties.size(); ++i)
  {
    if(m_SubViewWithProperties.get(i).SubView == &subView)
    {
      viewFound = true;
      m_SubViewWithProperties.remove(i);
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
  m_PixelArray->Clear();
  m_PixelArray->SetPosition(m_X, m_Y);
  SetupMyView();
}
void View::RunMyPreTask()
{
  RunMyViewPreTask();
}
bool View::CanRunMyScheduledTask()
{
  return CanRunMyViewScheduledTask();
}
void View::RunMyScheduledTask()
{
  MergeSubViews();
  RunMyViewScheduledTask();
}
void View::RunMyPostTask()
{
  RunMyViewPostTask();
}
void View::MergeSubViews()
{
  //Z Order is 1st subview added on top, last subview added on bottom
  for(int v = 0; v < m_SubViewWithProperties.size(); ++v)
  {
    View *subView = m_SubViewWithProperties.get(v).SubView;
    position sub_X = subView->GetPixelArray()->GetX();
    position sub_Y = subView->GetPixelArray()->GetY();
    size sub_Width = subView->GetPixelArray()->GetWidth();
    size sub_Height = subView->GetPixelArray()->GetHeight();
    for(int x = sub_X; x < sub_X+sub_Width; ++x)
    {
      for(int y = sub_Y; y < sub_Y+sub_Height; ++y)
      {
        SubViewWithProperties_t sVWP = m_SubViewWithProperties.get(v);
        if( true == sVWP.ClearViewBeforeMerge )
        {
          if( false == sVWP.SpecifiedClearArea )
          {
            m_PixelArray->SetPixel(x, y, CRGB::Black);
          }
          else if( x >= sVWP.X_To_Clear && x < sVWP.X_To_Clear + sVWP.W_To_Clear && y >= sVWP.Y_To_Clear && y < sVWP.Y_To_Clear + sVWP.H_To_Clear )
          {
            m_PixelArray->SetPixel(x, y, CRGB::Black);
          }
        }
        if( true == debugLEDs ) Serial << "Pixel Value " << "\tR:" << subView->GetPixel(x, y).red << "\tG:" << subView->GetPixel(x, y).green << "\tB:" << subView->GetPixel(x, y).blue << "\n";
        switch(subView->GetMergeType())
        {
          default:
          case MergeType_Layer:
          {
            if(true == debugLEDs) Serial << "Set Pixel " << x << "|" << y << " to: " << "\tR:" << subView->GetPixel(x, y).red << "\tG:" << subView->GetPixel(x, y).green << "\tB:" << subView->GetPixel(x, y).blue << "\n";
            m_PixelArray->SetPixel(x, y, subView->GetPixel(x, y));
          }
          break;
          case MergeType_Add:
          {
            if(true == debugLEDs) Serial << "Set Pixel " << x << "|" << y << " to: " << "\tR:" << subView->GetPixel(x, y).red << "\tG:" << subView->GetPixel(x, y).green << "\tB:" << subView->GetPixel(x, y).blue << "\n";
            CRGB pixel;
            pixel.red = qadd8(subView->GetPixel(x, y).red, m_PixelArray->GetPixel(x, y).red);
            pixel.blue = qadd8(subView->GetPixel(x, y).blue, m_PixelArray->GetPixel(x, y).blue);
            pixel.green = qadd8(subView->GetPixel(x, y).green, m_PixelArray->GetPixel(x, y).green);
            m_PixelArray->SetPixel(x, y, pixel);
          }
          break;
        }
      }
    }
  }
}

//*************** VerticalBarView ***************
void VerticalBarView::SetupMyView() {}
bool VerticalBarView::CanRunMyViewScheduledTask() { return true; }
void VerticalBarView::RunMyViewScheduledTask()
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

void VerticalBarView::NewValueNotification(float value, String context) { m_HeightScalar = value; }
void VerticalBarView::NewValueNotification(CRGB value, String context) { m_Color = value; }
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

//*************** BassSpriteView ***************
void BassSpriteView::NewValueNotification(CRGB value, String context)
{
  m_MyColor = value;
}
void BassSpriteView::NewValueNotification(float value, String context)
{
  m_Scaler = value;
}
void BassSpriteView::NewValueNotification(Position value, String context) 
{ 
  SetPosition(value.X, value.Y, context);
}
void BassSpriteView::SetupMyView()
{
  SetPosition(m_X, m_Y, "Position");
}
bool BassSpriteView::CanRunMyViewScheduledTask(){ return true; }
void BassSpriteView::RunMyViewScheduledTask()
{
  m_CurrentWidth = round((m_Scaler*((float)m_MaxWidth - (float)m_MinWidth)) + m_MinWidth);
  m_CurrentHeight = round((m_Scaler*((float)m_MaxHeight - (float)m_MinHeight)) + m_MinHeight);
  for(int x = m_BottomX; x <= m_TopX; ++x)
  {
    for(int y = m_BottomY; y <= m_TopY; ++y)
    {
      if( (x == m_CenterX && true == m_ShowCenterX) || (y == m_CenterY && true == m_ShowCenterY) )
      {
        m_PixelArray->SetPixel(x, y, m_MyCenterColor);
      }
      else if( (x >= m_CenterX - m_CurrentWidth) && (x <= m_CenterX + m_CurrentWidth) && (y >= m_CenterY - m_CurrentHeight) && (y <= m_CenterY + m_CurrentHeight) )
      {
        m_PixelArray->SetPixel(x, y, m_MyColor);
      }
      else
      {
        m_PixelArray->SetPixel(x, y, CRGB::Black);
      }
    }
  }
}
void BassSpriteView::SetPosition(position x, position y, String context)
{
  m_CenterX = x;
  m_CenterY = y;
  m_TopX = m_CenterX + m_MaxWidth;
  m_BottomX = m_CenterX - m_MaxWidth;
  m_TopY = m_CenterY + m_MaxHeight;
  m_BottomY = m_CenterY - m_MaxHeight;
  if(context == "Position")
  {
    m_X = m_BottomX;
    m_Y = m_BottomY;
  }
  else if(context == "X")
  {
    m_X = m_BottomX;
  }
  else if(context == "Y")
  {
    m_Y = m_BottomY;
  }
  m_PixelArray->SetPosition(m_X, m_Y);
}

//*************** ScrollingView ***************
void ScrollingView::SetupMyView(){}
bool ScrollingView::CanRunMyViewScheduledTask()
{
  return true; 
}
void ScrollingView::RunMyViewScheduledTask()
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

//*************** ColorSpriteView ***************
void ColorSpriteView::ConnectColorModel(ModelEventNotificationCaller<CRGB> &caller) { caller.RegisterForNotification(*this, ""); }
void ColorSpriteView::ConnectPositionModel(ModelEventNotificationCaller<Position> &caller) { caller.RegisterForNotification(*this, "Position"); }
void ColorSpriteView::ConnectXPositionModel(ModelEventNotificationCaller<Position> &caller) { caller.RegisterForNotification(*this, "X"); }
void ColorSpriteView::ConnectYPositionModel(ModelEventNotificationCaller<Position> &caller) { caller.RegisterForNotification(*this, "Y"); }
void ColorSpriteView::ConnectBandPowerModel(ModelEventNotificationCaller<BandData> &caller) { caller.RegisterForNotification(*this, ""); }
void ColorSpriteView::NewValueNotification(CRGB value, String context) { m_MyColor = value; }
void ColorSpriteView::NewValueNotification(Position value, String context) 
{
  if(context == "Position")
  {
    m_X = value.X;
    m_Y = value.Y;
  }
  if(context == "X")
  {
    m_X = value.X;
  }
  if(context == "Y")
  {
    m_Y = value.Y;
  }
  m_PixelArray->SetPosition(m_X, m_Y);
}
void ColorSpriteView::NewValueNotification(BandData value, String context)
{
  m_MyColor = FadeColor(value.Color, value.Power);
}
void ColorSpriteView::SetupMyView(){}
bool ColorSpriteView::CanRunMyViewScheduledTask(){ return true; }
void ColorSpriteView::ColorSpriteView::RunMyViewScheduledTask()
{
  if(true == debugView) Serial << "Coords: " << m_X << "|" << m_Y << "|" << m_W << "|" << m_H << "\n";
  for(int x = m_X; x < m_X+m_W; ++x)
  {
    for(int y = m_Y; y < m_Y+m_H; ++y)
    {
      m_PixelArray->SetPixel(x, y, m_MyColor);
    }
  }
}

//*************** FadingView ***************
void FadingView::SetupMyView(){}
bool FadingView::CanRunMyViewScheduledTask(){ return true; }
void FadingView::RunMyViewScheduledTask()
{
  if(m_FadeLength > 0)
  {
    switch(m_Direction)
    {
      case Direction_Up:
        for(int x = m_X; x<m_X+m_W; ++x)
        {
          for(int y = m_Y; y<m_Y+m_H; ++y)
          {
            if((x >= m_X) && (x < (m_X + m_W)) && (y > m_Y) && (y < (m_Y + m_H)))
            {
              PerformFade(x, y, y);
            }
          }
        }
        break;
      case Direction_Down:
        for(int x = m_X; x < m_X+m_W; ++x)
        {
          unsigned int index = 0;
          for(int y = m_Y+m_H-1; y >= m_Y; --y)
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
void RotatingView::SetupMyView()
{
  m_ResultingPixelArray = new PixelArray(m_X, m_Y, m_W, m_H);
  m_ResultingPixelArray->Clear();
  m_PixelArray->Clear();
  m_startMillis = millis();
}
void RotatingView::RunMyViewPreTask()
{
  switch(m_RotationType)
  {
    case RotationType_Rotate:
      m_ResultingPixelArray->Clear();
      m_PixelArray->Clear();
    break;
    case RotationType_Scroll:
    break;
    default:
    break;
  }
}
bool RotatingView::CanRunMyViewScheduledTask()
{ 
  return true;
}
void RotatingView::RunMyViewScheduledTask()
{
  m_currentMillis = millis();
  m_lapsedTime = m_currentMillis - m_startMillis;
  switch(m_RotationType)
  {
    case RotationType_Rotate:
      if(m_lapsedTime >= m_updatePeriodMillis)
      {
        ++m_Count;
      }
      RotateView();
    break;
    case RotationType_Scroll:
      if(m_lapsedTime >= m_updatePeriodMillis)
      {
        ScrollView();
      }
    break;
    default:
    break;
  }
  if(m_lapsedTime >= m_updatePeriodMillis)
  {
    m_startMillis = millis();
  }
  *m_PixelArray = *m_ResultingPixelArray;
}
void RotatingView::ScrollView()
{
  for(int x = m_X; x < m_X+m_W; ++x)
  {
    for(int y = m_Y; y < m_Y+m_H; ++y)
    {
      switch(m_Direction)
      {
        case Direction_Up:
        {
          int source = y-1;
          if(source < m_Y) { source = m_Y; }
          m_ResultingPixelArray->SetPixel(x, y, m_PixelArray->GetPixel(x, source));
        }
        break;
        case Direction_Down:
        {
          int source = y+1;
          if(source >= m_Y + m_H) { source = m_Y+m_H-1; }
          m_ResultingPixelArray->SetPixel(x, y, m_PixelArray->GetPixel(x, source));
        }
        break;
        case Direction_Right:
        {
          int source = x-1;
          if(source < m_X) { source = m_X; }
          m_ResultingPixelArray->SetPixel(x, y, m_PixelArray->GetPixel(source, y));
        }
        break;
        case Direction_Left:
        {
          int source = x+1;
          if(source >= m_X + m_W - 1) { source = m_X + m_W - 1; }
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
  for(int x = m_X; x < m_X+m_W; ++x)
  {
    for(int y = m_Y; y < m_Y+m_H; ++y)
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
