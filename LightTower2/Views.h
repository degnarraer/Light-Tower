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
    int m_ID;
    
    //Task
    void Setup()
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
    void SetNormalizedHeight(float Height) { m_HeightScalar = Height; }

  private:
    CRGB m_Color = CRGB::Green;
    float m_HeightScalar;

    //ModelEventNotificationCallerInterface
    void NewFloatValueNotificationFrom(float Value, ModelEventNotificationCallerInterface &source)
    {
      SetNormalizedHeight(Value);
    }

    //View
    void SetupView(){}
    bool CanRunViewTask(){ return true; }
    void RunViewTask()
    {
      int scaledHeight = (m_Y + round(m_HeightScalar*(float)m_H));
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
