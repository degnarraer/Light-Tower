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

#ifndef Views_H
#define Views_H


#include <assert.h>
#include "LEDControllerInterface.h"
#include "TaskInterface.h"
#include "Streaming.h"
#include "Tunes.h"
#include "Models.h"
#include <LinkedList.h>



typedef int position;
typedef int size;


enum MergeType
{
  MergeType_Layer,
  MergeType_Add
};

enum Direction
{
  Direction_Up,
  Direction_Down,
  Direction_Right,
  Direction_Left
};

class View: public Task
{
  public:
    View(String Title, position X, position Y, size W, size H): Task(Title)
                                                              , m_X(X)
                                                              , m_Y(Y)
                                                              , m_W(W)
                                                              , m_H(H){}
    View(String Title, position X, position Y, size W, size H, MergeType MergeType): Task(Title)
                                                              , m_X(X)
                                                              , m_Y(Y)
                                                              , m_W(W)
                                                              , m_H(H)
                                                              , m_MergeType(MergeType){}
    virtual ~View()
    {
      if(true == debugMode && debugLevel >= 1) Serial << "Delete View\n";
    }
    void SetPosition(position X, position Y){ m_X = X; m_Y = Y; }
    void SetSize(size W, size H){ m_W = W; m_H = H; }
    void AddSubView(View &SubView)
    { 
      m_SubViews.add(&SubView);
      AddTask(SubView);
    }
    bool RemoveSubView(View &SubView)
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
    MergeType GetMergeType(){ return m_MergeType; }
    void RemoveAllSubViews(){}
    PixelStruct& GetPixelStruct() { return m_MyPixelStruct; }
  protected:
    PixelStruct m_MyPixelStruct;
    position m_X;
    position m_Y;
    size m_W;
    size m_H;
  private:
    LinkedList<View*> m_SubViews = LinkedList<View*>();
    View *m_ParentView;
    MergeType m_MergeType = MergeType_Layer;
    
    //Task
    void Setup();
    bool CanRunMyTask(){ return CanRunViewTask(); }
    void RunMyTask()
    {
      MergeSubViews();
      RunViewTask();
    }

    //View
    void MergeSubViews();
    virtual void SetupView() = 0;
    virtual bool CanRunViewTask() = 0;
    virtual void RunViewTask() = 0;
};

class VerticalBarView: public View
                     , public ModelWithNewValueNotification<Position>
                     , public ModelEventNotificationCallee<float>
                     , public ModelEventNotificationCallee<CRGB>
{
  public:
    VerticalBarView(String title): View(title, 0, 0, 0, 0)
                                 , ModelWithNewValueNotification<Position>(title){}
    VerticalBarView(String title, position X, position Y, size W, size H): View(title, X, Y, W, H)
                                                                         , ModelWithNewValueNotification<Position>(title){}
    VerticalBarView(String title, position X, position Y, size W, size H, MergeType MergeType): View(title, X, Y, W, H, MergeType)
                                                                                              , ModelWithNewValueNotification<Position>(title){}
    virtual ~VerticalBarView()
    {
      if(true == debugMemory) Serial << "Delete VerticalBarView\n";  
    }
    void ConnectBarHeightModel(ModelEventNotificationCaller<float> &caller) { caller.RegisterForNotification(*this); }
    void ConnectBarColorModel(ModelEventNotificationCaller<CRGB> &caller) { caller.RegisterForNotification(*this); }
  private:
    CRGB m_Color = CRGB::Green;
    float m_HeightScalar;
    int m_ScaledHeight;
    Position m_Peak;

    //ModelEventNotificationCallee
    void NewValueNotification(float value) { m_HeightScalar = value; }
    void NewValueNotification(CRGB value) { m_Color = value; }
    
  private:
    //View
    void SetupView() {}
    bool CanRunViewTask() { return true; }
    void RunViewTask();

    //Model
    void SetupModel(){}
    bool CanRunModelTask(){ return true;}
    void RunModelTask()
    {
      m_Peak.X = m_X;
      m_Peak.Y = m_ScaledHeight;
    }
    void UpdateValue(){ SetCurrentValue(m_Peak); }
};

class BassSpriteView: public View
                    , public ModelEventNotificationCallee<float>
{
  public:
    BassSpriteView(String title, position X, position Y, size W, size H): View(title, X, Y, W, H){}
    BassSpriteView(String title, position X, position Y, size W, size H, MergeType MergeType): View(title, X, Y, W, H, MergeType){}
    virtual ~BassSpriteView()
    {
      if(true == debugMemory) Serial << "Delete BassSpriteView\n";  
    }
    void ConnectModel(ModelEventNotificationCaller<float> &caller) { caller.RegisterForNotification(*this); }

    //ModelEventNotificationCallee
    void NewValueNotification(float Value);
    
  private:
    //View
    void SetupView(){}
    bool CanRunViewTask(){ return true; }
    void RunViewTask(){}
};


enum ScrollDirection
{
  ScrollDirection_Up,
  ScrollDirection_Down,
  ScrollDirection_Left,
  ScrollDirection_Right
};
class ScrollingView: public View
{
  public:
    ScrollingView( String title
                 , ScrollDirection scrollDirection
                 , position X
                 , position Y
                 , size W
                 , size H)
                 : View(title, X, Y, W, H)
                 , m_ScrollDirection(scrollDirection){}
    ScrollingView( String title
                 , ScrollDirection scrollDirection
                 , position X
                 , position Y
                 , size W
                 , size H
                 , MergeType MergeType)
                 : View(title, X, Y, W, H, MergeType)
                 , m_ScrollDirection(scrollDirection){}
    virtual ~ScrollingView()
    {
      if(true == debugMemory) Serial << "Delete ScrollingView\n";  
    }
    
  private:
    //View
    void SetupView(){}
    bool CanRunViewTask(){ return true; }
    void RunViewTask();
    enum ScrollDirection m_ScrollDirection = ScrollDirection_Up;
};


class ColorSpriteView: public View
                     , public ModelEventNotificationCallee<CRGB>
                     , public ModelEventNotificationCallee<Position>
{
  public:
    ColorSpriteView( String title
                   , position X
                   , position Y
                   , size W, size H)
                   : View(title, X, Y, W, H){}
    ColorSpriteView( String title
                   , position X
                   , position Y
                   , size W
                   , size H
                   , CRGB Color)
                   : View(title, X, Y, W, H)
                   , m_MyColor(Color){}
    ColorSpriteView( String title
                   , position X
                   , position Y
                   , size W
                   , size H
                   , CRGB Color
                   , MergeType MergeType)
                   : View(title, X, Y, W, H, MergeType)
                   , m_MyColor(Color){}
    ColorSpriteView( String title
                   , position X
                   , position Y
                   , size W
                   , size H
                   , MergeType MergeType)
                   : View(title, X, Y, W, H, MergeType){}
    virtual ~ColorSpriteView() { if(true == debugMemory) Serial << "Delete ColorSpriteView\n"; }
    void ConnectColorModel(ModelEventNotificationCaller<CRGB> &caller) { caller.RegisterForNotification(*this); }
    void ConnectPositionModel(ModelEventNotificationCaller<Position> &caller) { caller.RegisterForNotification(*this); }
    
    //ModelEventNotificationCallee
    void NewValueNotification(CRGB value) { m_MyColor = value; }
    void NewValueNotification(Position value) 
    { 
      m_X = value.X;
      m_Y = value.Y;
    }
    
  private:
    //View
    void SetupView(){}
    bool CanRunViewTask(){ return true; }
    void RunViewTask();
    CRGB m_MyColor = CRGB::Black;
};

class FadingView: public View
{
  public:
    FadingView( String title
              , unsigned int FadeLength
              , Direction Direction
              , position X
              , position Y
              , size W
              , size H)
              : View(title, X, Y, W, H)
              , m_FadeLength(FadeLength)
              , m_Direction(Direction){}
    FadingView( String title
              , unsigned int FadeLength
              , Direction Direction
              , position X
              , position Y
              , size W
              , size H
              , MergeType MergeType)
              : View(title, X, Y, W, H, MergeType)
              , m_FadeLength(FadeLength)
              , m_Direction(Direction){}
    virtual ~FadingView()
    {
      if(true == debugMemory) Serial << "Delete FadingView\n";  
    }
    
  private:
    //View
    void SetupView(){}
    bool CanRunViewTask(){ return true; }
    void RunViewTask()
    {
      if(m_FadeLength > 0)
      {
        switch(m_Direction)
        {
          case Direction_Up:
            for(int x = 0; x < SCREEN_WIDTH; ++x)
            {
              for(int y = 0; y < SCREEN_HEIGHT; ++y)
              {
                if((x >= m_X) && (x < (m_X + m_W)) && (y > m_Y) && (y < (m_Y + m_H)))
                {
                  PerformFade(x, y, y);
                }
              }
            }
            break;
          case Direction_Down:
            for(unsigned int x = 0; x < SCREEN_WIDTH; ++x)
            {
              unsigned int index = 0;
              for(unsigned int y = SCREEN_HEIGHT-1; y > 0; --y)
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
    void PerformFade(unsigned int x, unsigned int y, unsigned int i)
    {
      float normalizedFade = 1.0 - ((float)i / (float)m_FadeLength);
      m_MyPixelStruct.Pixel[x][y].red = m_MyPixelStruct.Pixel[x][y].red * normalizedFade;
      m_MyPixelStruct.Pixel[x][y].green = m_MyPixelStruct.Pixel[x][y].green * normalizedFade;
      m_MyPixelStruct.Pixel[x][y].blue = m_MyPixelStruct.Pixel[x][y].blue * normalizedFade;
    }
    Direction m_Direction = Direction_Down;
    unsigned int m_FadeLength = 0;
};

class RotatingView: public View
{
  public:
    RotatingView( String title
                , Direction Direction
                , position X
                , position Y
                , size W
                , size H)
                : View(title, X, Y, W, H)
                , m_Direction(Direction){}
    RotatingView( String title
                , Direction Direction
                , position X
                , position Y
                , size W
                , size H
                , MergeType MergeType)
                : View(title, X, Y, W, H, MergeType)
                , m_Direction(Direction){}
    virtual ~RotatingView()
    {
      if(true == debugMemory) Serial << "Delete RotatingView\n";  
    }
    
  private:
    //View
    void SetupView(){}
    bool CanRunViewTask()
    { 
      //m_MyPixelStruct.Clear(); 
      return true; 
    }
    void RunViewTask()
    {
      ++m_Count;
      for(int x = 0; x < SCREEN_WIDTH; ++x)
      {
        for(int y = 0; y < SCREEN_HEIGHT; ++y)
        {
          if((x >= m_X) && (x < (m_X + m_W)) && (y >= m_Y) && (y < (m_Y + m_H)))
          {
            switch(m_Direction)
            {
              case Direction_Up:
              {
                unsigned int rotation = m_Count % m_H;
                int source = y-rotation;
                if(source < m_Y) { source = m_H - (rotation - y); }
                m_ResultingPixels.Pixel[x][y] = m_MyPixelStruct.Pixel[x][source];
              }
              break;
              case Direction_Down:
              {
                unsigned int rotation = m_Count % m_H;
                int source = y+rotation;
                if(source >= m_Y + m_H) { source = m_Y - (m_H - (y + rotation)); }
                m_ResultingPixels.Pixel[x][y] = m_MyPixelStruct.Pixel[x][source];
              }
              break;
              case Direction_Right:
              {
                unsigned int rotation = m_Count % m_W;
                int source = x+rotation;
                if(source >= m_X + m_W) { source = m_X - (m_W - (x + rotation)); }
                m_ResultingPixels.Pixel[x][y] = m_MyPixelStruct.Pixel[source][y];
              }
              break;
              case Direction_Left:
              {
                unsigned int rotation = m_Count % m_W;
                int source = x-rotation;
                if(source < m_X) { source = m_W - (rotation - x); }
                m_ResultingPixels.Pixel[x][y] = m_MyPixelStruct.Pixel[source][y];
              }
              break;
              default:
              break;
            }
          }
        }
      }
      m_MyPixelStruct = m_ResultingPixels;
    }
    PixelStruct m_ResultingPixels;
    Direction m_Direction = Direction_Right;
    unsigned int m_Count = 0;
};
#endif
