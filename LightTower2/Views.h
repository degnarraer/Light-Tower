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

#include "LEDControllerInterface.h"
#include "TaskInterface.h"
#include "Streaming.h"
#include "Tunes.h"
#include <LinkedList.h>
#include <assert.h>



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
    Pixels &GetPixels() { return m_Pixels; }

    void SetModel(Model &Model) 
    { 
      Model.RegisterForNotification(*this);
    }
    Pixels m_Pixels = {{{CRGB::Red},{ CRGB::Red}}};
    position m_X;
    position m_Y;
    size m_W;
    size m_H;
  private:
    LinkedList<View*> ChildViews = LinkedList<View*>();
    View *ParentView;
    
    //Task
    void Setup(){ SetupView(); }
    bool CanRunMyTask(){ return CanRunViewTask(); }
    void RunMyTask(){ RunViewTask(); }

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
    void SetColor(CRGB Color){ m_Color = Color; }
    void SetNormalizedHeight(float Height) { assert (Height <= 1.0); m_Height = Height; }

  private:
    CRGB m_Color = CRGB::Red;
    float m_Height;

    
    //ModelEventNotificationCallerInterface
    void NewFloatValueNotificationFrom(float Value, ModelEventNotificationCallerInterface &source)
    {
      SetNormalizedHeight(Value);
    }

    //View
    void SetupView()
    {
      for(int w = 0; w<SCREEN_WIDTH; ++w)
      {
        for(int h = 0; h<SCREEN_HEIGHT; ++h)
        {
          m_Pixels.Pixel[w][h] = CRGB::Black;
        }
      }
    }
    bool CanRunViewTask(){ return true; }
    void RunViewTask()
    {
      
      if(true == debugLEDs) Serial << "Coords: " << m_X << "|" << m_Y << "|" << m_W << "|" << m_H << "\n";
      for(int w = 0; w<SCREEN_WIDTH; ++w)
      {
        for(int h = 0; h<SCREEN_HEIGHT; ++h)
        {
          m_Pixels.Pixel[w][h] = CRGB::Black;
          if(w >= m_X && w < m_X + m_W && h >= m_Y && h < m_Height * (m_Y + m_H))
          {
            if(true == debugLEDs) Serial << "R";
            m_Pixels.Pixel[w][h] = CRGB::Red;
          }
        }
      }
      if(true == debugLEDs) Serial << "\n";
      if(true == debugLEDs) Serial << "******View LEDs******\n";
      for(int h = 0; h < SCREEN_HEIGHT; ++ h)
      {
        for(int w = 0; w < SCREEN_WIDTH; ++w)
        {
          CRGB bufColor = m_Pixels.Pixel[w][h];
          if(true == debugLEDs) Serial << bufColor[0] << ":" << bufColor[1] << ":" << bufColor[2] << " \t";
        }
        if(true == debugLEDs) Serial << "\n";
      }
    }
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
