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

class View: public Task
{
  public:
    View(String Title, position X, position Y, size W, size H): Task(Title)
                                                              , m_X(X)
                                                              , m_Y(Y)
                                                              , m_W(W)
                                                              , m_H(H){}
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
    void RemoveAllSubViews(){}
    PixelStruct& GetPixelStruct() { return m_MyPixelStruct; }
    PixelStruct m_MyPixelStruct;
    position m_X;
    position m_Y;
    size m_W;
    size m_H;
  private:
    LinkedList<View*> m_SubViews = LinkedList<View*>();
    View *m_ParentView;
    
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
                     , public ModelEventNotificationCallee<float>
                     , public ModelEventNotificationCallee<CRGB>
{
  public:
    VerticalBarView(String Title): View(Title, 0, 0, 0, 0){}
    VerticalBarView(String Title, position X, position Y, size W, size H): View(Title, X, Y, W, H){}
    virtual ~VerticalBarView()
    {
      if(true == debugMemory) Serial << "Delete VerticalBarView\n";  
    }
    void ConnectBarHeightModel(ModelEventNotificationCaller<float> &Caller) { Caller.RegisterForNotification(*this); }
    void ConnectBarColorModel(ModelEventNotificationCaller<CRGB> &Caller) { Caller.RegisterForNotification(*this); }
    void SetColor(CRGB Color) { m_Color = Color; }
    void SetNormalizedHeight(float Height) { m_HeightScalar = Height; }
  private:
    CRGB m_Color = CRGB::Green;
    float m_HeightScalar;

    //ModelEventNotificationCallee
    void NewValueNotification(float Value) { SetNormalizedHeight(Value); }
    void NewValueNotification(CRGB Value) { m_Color = Value; }
    
  private:
    //View
    void SetupView() {}
    bool CanRunViewTask() { return true; }
    void RunViewTask();
};

class BassSpriteView: public View
                    , public ModelEventNotificationCallee<float>
{
  public:
    BassSpriteView(String Title, position X, position Y, size L, size W): View(Title, X, Y, L, W){}
    virtual ~BassSpriteView()
    {
      if(true == debugMemory) Serial << "Delete BassSpriteView\n";  
    }
    void ConnectModel(ModelEventNotificationCaller<float> &Caller) { Caller.RegisterForNotification(*this); }

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
  ScrollDirection_Down
};
class ScrollingView: public View
                   , public ModelEventNotificationCallee<bool>
{
  public:
  
    ScrollingView(String Title, ScrollDirection ScrollDirection, position X, position Y, size L, size W): View(Title, X, Y, L, W){}
    virtual ~ScrollingView()
    {
      if(true == debugMemory) Serial << "Delete ScrollingView\n";  
    }
    void ConnectModel(ModelEventNotificationCaller<bool> &Caller) { Caller.RegisterForNotification(*this); }

    //ModelEventNotificationCallee
    void NewValueNotification(bool Value){ }
    
  private:
    //View
    void SetupView(){}
    bool CanRunViewTask(){ return true; }
    void RunViewTask();
    enum ScrollDirection m_ScrollDirection = ScrollDirection_Up;
};


class ColorSpriteView: public View
                     , public ModelEventNotificationCallee<CRGB>
{
  public:
    ColorSpriteView(String Title, position X, position Y, size L, size W): View(Title, X, Y, L, W){}
    virtual ~ColorSpriteView()
    {
      if(true == debugMemory) Serial << "Delete ColorSpriteView\n";  
    }
    void ConnectColorModel(ModelEventNotificationCaller<CRGB> &Caller) { Caller.RegisterForNotification(*this); }

    //ModelEventNotificationCallee
    void NewValueNotification(CRGB Value) { m_MyColor = Value; }
    
  private:
    //View
    void SetupView(){}
    bool CanRunViewTask(){ return true; }
    void RunViewTask();
    CRGB m_MyColor = CRGB::Black;
};
#endif
