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
          , public ModelEventNotificationCalleeInterface
{
  public:
    View(String Title, position X, position Y, size W, size H): Task(Title)
                                                              , m_X(X)
                                                              , m_Y(Y)
                                                              , m_W(W)
                                                              , m_H(H){}
    ~View()
    {
      if(true == debugMode && debugLevel >= 1) Serial << "Delete View\n";
    }
    void SetPosition(position X, position Y){ m_X = X; m_Y = Y; }
    void SetSize(size W, size H){ m_W = W; m_H = H; }
    void AddChildView(View &Child){}
    void RemoveChildView(View &Child){}
    void RemoveAllChildrenViews(){}
    PixelStruct& GetPixelStruct() { return m_MyPixelStruct; }

    void SetModel(Model &Model) 
    { 
      Model.RegisterForNotification(*this);
    }
    PixelStruct m_MyPixelStruct;
    position m_X;
    position m_Y;
    size m_W;
    size m_H;
  private:
    LinkedList<View*> m_ChildViews = LinkedList<View*>();
    View *m_ParentView;
    
    //Task
    void Setup();
    bool CanRunMyTask();
    void RunMyTask();

    //ModelEventNotificationCallerInterface
    virtual void NewFloatValueNotificationFrom(float Value, ModelEventNotificationCallerInterface &source) = 0;

    //View
    virtual void SetupView() = 0;
    virtual bool CanRunViewTask() = 0;
    virtual void RunViewTask() = 0;
};

class VerticalBarView: public View
{
  public:
    VerticalBarView(String Title): View(Title, 0, 0, 0, 0){}
    VerticalBarView(String Title, position X, position Y, size W, size H): View(Title, X, Y, W, H){}
    ~VerticalBarView(){}
    void SetColor(CRGB Color);
    void SetNormalizedHeight(float Height);

  private:
    CRGB m_Color = CRGB::Green;
    float m_HeightScalar;

    //ModelEventNotificationCallerInterface
    void NewFloatValueNotificationFrom(float Value, ModelEventNotificationCallerInterface &source);

    //View
    void SetupView();
    bool CanRunViewTask();
    void RunViewTask();
};

class BassSpriteView: public View
{
  public:
    BassSpriteView(String Title, position X, position Y, size L, size W): View(Title, X, Y, L, W){}
    ~BassSpriteView(){}

  private:
    //View
    void SetupView(){}
    bool CanRunViewTask(){ return true; }
    void RunViewTask(){}
};

#endif
