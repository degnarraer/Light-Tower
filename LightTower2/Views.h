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

#include "LEDControllerInterface.h"
#include "TaskInterface.h"
#include "Streaming.h"
#include "Tunes.h"
#include "Models.h"
#include <LinkedList.h>

enum MergeType
{
  MergeType_Layer,
  MergeType_Add,
  MergeType_Swap
};
enum RotationType
{
  RotationType_Static,
  RotationType_Scroll
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
                                                              , m_H(H)
    {
      if(true == debugMemory) Serial << "New: View\n";
    }
    View(String Title, position X, position Y, size W, size H, MergeType MergeType): Task(Title)
                                                              , m_X(X)
                                                              , m_Y(Y)
                                                              , m_W(W)
                                                              , m_H(H)
                                                              , m_MergeType(MergeType)
    {
      if(true == debugMemory) Serial << "New: View\n";
    }
    virtual ~View()
    {
      if(true == debugMemory) Serial << "Delete: View\n";
      delete m_PixelArray;
    }
    void SetPosition(position X, position Y);
    void SetSize(size W, size H);
    void AddSubView(View &SubView);
    CRGB GetPixel(int X, int Y);
    bool RemoveSubView(View &SubView);
    MergeType GetMergeType();
    PixelArray* GetPixelArray();
    void RemoveAllSubViews(){}
  protected:
    PixelArray *m_PixelArray;
    position m_X;
    position m_Y;
    size m_W;
    size m_H;
  protected:
    LinkedList<View*> m_SubViews = LinkedList<View*>();
    void MergeSubViews(bool clearViewBeforeMerge);
  private:
    View *m_ParentView;
    MergeType m_MergeType = MergeType_Layer;
    
    //Task
    void Setup();
    bool CanRunMyTask();
    void RunMyTask();

    //View
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
                                 , ModelWithNewValueNotification<Position>(title)
    {
      if(true == debugMemory) Serial << "New: VerticalBarView\n";
    }
    VerticalBarView(String title, position X, position Y, size W, size H): View(title, X, Y, W, H)
                                                                         , ModelWithNewValueNotification<Position>(title)
    {
      if(true == debugMemory) Serial << "New: VerticalBarView\n";
    }
    VerticalBarView(String title, position X, position Y, size W, size H, MergeType MergeType): View(title, X, Y, W, H, MergeType)
                                                                                              , ModelWithNewValueNotification<Position>(title)
    {
      if(true == debugMemory) Serial << "New: VerticalBarView\n";
    }
    virtual ~VerticalBarView()
    {
      if(true == debugMemory) Serial << "Delete: VerticalBarView\n";  
    }
    void ConnectBarHeightModel(ModelEventNotificationCaller<float> &caller) { caller.RegisterForNotification(*this); }
    void ConnectBarColorModel(ModelEventNotificationCaller<CRGB> &caller) { caller.RegisterForNotification(*this); }
  private:
    CRGB m_Color = CRGB::Green;
    float m_HeightScalar;
    int m_ScaledHeight;
    Position m_Peak;

    //ModelEventNotificationCallee
    void NewValueNotification(float value);
    void NewValueNotification(CRGB value);
    
  private:
    //View
    void SetupView();
    bool CanRunViewTask();
    void RunViewTask();

    //Model
    void SetupModel();
    bool CanRunModelTask();
    void RunModelTask();
    void UpdateValue();
};

class BassSpriteView: public View
                    , public ModelEventNotificationCallee<float>
{
  public:
    BassSpriteView(String title, position X, position Y, size W, size H): View(title, X, Y, W, H)
    {
      if(true == debugMemory) Serial << "New: BassSpriteView\n";
    }
    BassSpriteView(String title, position X, position Y, size W, size H, MergeType MergeType): View(title, X, Y, W, H, MergeType)
    {
      if(true == debugMemory) Serial << "New: BassSpriteView\n";
    }
    virtual ~BassSpriteView()
    {
      if(true == debugMemory) Serial << "Delete: BassSpriteView\n";  
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
                 , m_ScrollDirection(scrollDirection)
    {
      if(true == debugMemory) Serial << "New: ScrollingView\n";  
    }
    ScrollingView( String title
                 , ScrollDirection scrollDirection
                 , position X
                 , position Y
                 , size W
                 , size H
                 , MergeType MergeType)
                 : View(title, X, Y, W, H, MergeType)
                 , m_ScrollDirection(scrollDirection)
    {
      if(true == debugMemory) Serial << "New: ScrollingView\n";  
    }
    virtual ~ScrollingView()
    {
      if(true == debugMemory) Serial << "Delete: ScrollingView\n";  
    }
    
  private:
    //View
    void SetupView();
    bool CanRunViewTask();
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
                   : View(title, X, Y, W, H)
    {
      if(true == debugMemory) Serial << "New: ColorSpriteView\n";  
    }
    ColorSpriteView( String title
                   , position X
                   , position Y
                   , size W
                   , size H
                   , CRGB Color)
                   : View(title, X, Y, W, H)
                   , m_MyColor(Color)
    {
      if(true == debugMemory) Serial << "New: ColorSpriteView\n";  
    }
    ColorSpriteView( String title
                   , position X
                   , position Y
                   , size W
                   , size H
                   , CRGB Color
                   , MergeType MergeType)
                   : View(title, X, Y, W, H, MergeType)
                   , m_MyColor(Color)
    {
      if(true == debugMemory) Serial << "New: ColorSpriteView\n";  
    }
    ColorSpriteView( String title
                   , position X
                   , position Y
                   , size W
                   , size H
                   , MergeType MergeType)
                   : View(title, X, Y, W, H, MergeType)
    {
      if(true == debugMemory) Serial << "New: ColorSpriteView\n";  
    }
    virtual ~ColorSpriteView() { if(true == debugMemory) Serial << "Delete: ColorSpriteView\n"; }
    void ConnectColorModel(ModelEventNotificationCaller<CRGB> &caller);
    void ConnectPositionModel(ModelEventNotificationCaller<Position> &caller);
    
    //ModelEventNotificationCallee
    void NewValueNotification(CRGB value);
    void NewValueNotification(Position value);
    
  private:
    CRGB m_MyColor = CRGB::Black;
    
    //View
    void SetupView();
    bool CanRunViewTask();
    void RunViewTask();
};

class FadingView: public View
{
  public:
    FadingView( String title
              , unsigned int FadeLength
              , Direction Direction
              , position x
              , position y
              , size w
              , size h)
              : View(title, x, y, w, h)
              , m_FadeLength(FadeLength)
              , m_Direction(Direction)
    {
      if(true == debugMemory) Serial << "New: FadingView\n";  
    }
    FadingView( String title
              , unsigned int FadeLength
              , Direction Direction
              , position x
              , position y
              , size w
              , size h
              , MergeType MergeType)
              : View(title, x, y, w, h, MergeType)
              , m_FadeLength(FadeLength)
              , m_Direction(Direction)
    {
      if(true == debugMemory) Serial << "New: FadingView\n";  
    }
    virtual ~FadingView()
    {
      if(true == debugMemory) Serial << "Delete: FadingView\n";  
    }
    
  private:
    Direction m_Direction = Direction_Down;
    unsigned int m_FadeLength = 0;
    void PerformFade(unsigned int x, unsigned int y, unsigned int i);

    //View
    void SetupView();
    bool CanRunViewTask();
    void RunViewTask();
};

class RotatingView: public View
{
  public:
    RotatingView( String title
                , Direction Direction
                , unsigned int updatePeriodMillis
                , RotationType rotationType
                , position X
                , position Y
                , size W
                , size H)
                : View(title, X, Y, W, H)
                , m_Direction(Direction)
                , m_updatePeriodMillis(updatePeriodMillis)
                , m_RotationType(rotationType)
    {
      if(true == debugMemory) Serial << "New: RotatingView\n";  
    }
    RotatingView( String title
                , Direction Direction
                , unsigned int updatePeriodMillis
                , RotationType rotationType
                , position X
                , position Y
                , size W
                , size H
                , MergeType MergeType)
                : View(title, X, Y, W, H, MergeType)
                , m_Direction(Direction)
                , m_updatePeriodMillis(updatePeriodMillis)
                , m_RotationType(rotationType)
    {
      if(true == debugMemory) Serial << "New: RotatingView\n";  
    }
    virtual ~RotatingView()
    {
      if(true == debugMemory) Serial << "Delete: RotatingView\n";
      delete m_ResultingPixelArray; 
    }
    
  private:
    unsigned long m_startMillis;
    unsigned long m_currentMillis;
    unsigned int m_updatePeriodMillis;
    unsigned long m_lapsedTime;
    PixelArray *m_ResultingPixelArray;
    Direction m_Direction = Direction_Right;
    RotationType m_RotationType;
    unsigned int m_Count = 0;
    //View
    void SetupView();
    bool CanRunViewTask();
    void RunViewTask();
};
#endif
