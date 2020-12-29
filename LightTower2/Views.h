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
                     , public ModelEventNotificationCallee<float>
                     , public ModelEventNotificationCallee<CRGB>
{
  public:
    VerticalBarView(String title): View(title, 0, 0, 0, 0){}
    VerticalBarView(String title, position X, position Y, size W, size H): View(title, X, Y, W, H){}
    virtual ~VerticalBarView()
    {
      if(true == debugMemory) Serial << "Delete VerticalBarView\n";  
    }
    void ConnectBarHeightModel(ModelEventNotificationCaller<float> &caller) { caller.RegisterForNotification(*this); }
    void ConnectBarColorModel(ModelEventNotificationCaller<CRGB> &caller) { caller.RegisterForNotification(*this); }
    void SetColor(CRGB color) { m_Color = color; }
    void SetNormalizedHeight(float height) { m_HeightScalar = height; }
  private:
    CRGB m_Color = CRGB::Green;
    float m_HeightScalar;

    //ModelEventNotificationCallee
    void NewValueNotification(float value) { SetNormalizedHeight(value); }
    void NewValueNotification(CRGB value) { m_Color = value; }
    
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
    BassSpriteView(String title, position X, position Y, size L, size W): View(title, X, Y, L, W){}
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
  
    ScrollingView(String title, ScrollDirection scrollDirection, position X, position Y, size L, size W): View(title, X, Y, L, W)
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
{
  public:
    ColorSpriteView(String title, position X, position Y, size L, size W): View(title, X, Y, L, W){}
    virtual ~ColorSpriteView()
    {
      if(true == debugMemory) Serial << "Delete ColorSpriteView\n";  
    }
    void ConnectColorModel(ModelEventNotificationCaller<CRGB> &caller) { caller.RegisterForNotification(*this); }
    
    //ModelEventNotificationCallee
    void NewValueNotification(CRGB value) { m_MyColor = value; }
    
  private:
    //View
    void SetupView(){}
    bool CanRunViewTask(){ return true; }
    void RunViewTask();
    CRGB m_MyColor = CRGB::Black;
};
#endif
