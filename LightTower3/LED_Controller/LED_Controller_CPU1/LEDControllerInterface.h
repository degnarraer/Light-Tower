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

#ifndef LEDControllerInterface_H
#define LEDControllerInterface_H

#define FASTLED_ALLOW_INTERRUPTS 1
#define FASTLED_INTERRUPT_RETRY_COUNT 10
#include <FastLED.h>
#include "Tunes.h"
#include "Streaming.h"

typedef CRGBArray<NUMLEDS> LEDStrip;
typedef int position;
typedef int size;

class PixelArray
{
  public:
    PixelArray(position x, position y, size w, size h): m_X(x), m_Y(y),  m_W(w), m_H(h)
    {
      m_Pixels = Create2DArray<CRGB>(m_W, m_H, CRGB::Black);
    }
    virtual ~PixelArray()
    {
      if(true == debugMemory) Serial << "Delete: PixelArray\n";
      Delete2DArray(m_Pixels);
    }
    void Clear()
    {
      for(int x = 0; x <= m_W-1; ++x)
      {
        for(int y = 0; y <= m_H-1; ++y)
        {
          m_Pixels[x][y] = CRGB::Black;
        }
      }
    }
    void SetPosition(position x, position y)
    {
      m_X = x;
      m_Y = y;
    }
    CRGB GetPixel(position x, position y)
    {
      if( (x >= m_X) && (x <= m_X + m_W - 1) && (y >= m_Y) && (y <= m_Y + m_H - 1) )
      {
        return m_Pixels[x - m_X][y - m_Y];
      }
      else
      {
        return CRGB::Black;
      }
    }
    void SetPixel(position x, position y, CRGB value)
    {
      if( (x >= m_X) && (x <= m_X + m_W - 1) && (y >= m_Y) && (y <= m_Y + m_H - 1) )
      {
        m_Pixels[x - m_X][y - m_Y] = value;
      }
    }
    void operator = (PixelArray &pa)
    {
      m_X = pa.GetX();
      m_Y = pa.GetY();
      m_W = pa.GetWidth();
      m_H = pa.GetHeight();
      for(position x = m_X; x <= m_X + m_W - 1; ++x)
      {
        for(position y = m_Y; y <= m_Y + m_H - 1; ++y)
        {
          m_Pixels[x][y] = pa.GetPixel(x, y);
        }
      }
    }
    size GetWidth(){ return m_W; }
    size GetHeight(){ return m_H; }
    position GetX(){ return m_X; }
    position GetY(){ return m_Y; }
  private:
    CRGB** m_Pixels;
    position m_X = 0;
    position m_Y = 0;
    size m_W = 0;
    size m_H = 0;
    
    template <typename T>
    T** Create2DArray(unsigned nrows, unsigned ncols, const T& val = T())
    {
       T** ptr = NULL;
       T* pool = NULL;
    
       ptr = new T*[nrows];  // allocate pointers (can throw here)
       pool = new T[nrows*ncols]{val};  // allocate pool (can throw here)
       for (unsigned i = 0; i < nrows; ++i, pool += ncols )
           ptr[i] = pool;

       return ptr;
    }
    
    template <typename T>
    void Delete2DArray(T** arr)
    {
      delete [] arr[0];  // remove the pool
      delete [] arr;     // remove the pointers 
    }
};

class LEDController
{
  public:
    LEDController()
    {
      FastLED.addLeds<WS2812B, DATA_PIN_STRIP1, RGB>(m_LEDStrip[0], NUMLEDS);
      FastLED.addLeds<WS2812B, DATA_PIN_STRIP2, RGB>(m_LEDStrip[1], NUMLEDS);
      FastLED.addLeds<WS2812B, DATA_PIN_STRIP3, RGB>(m_LEDStrip[2], NUMLEDS);
      FastLED.addLeds<WS2812B, DATA_PIN_STRIP4, RGB>(m_LEDStrip[3], NUMLEDS);
      FastLED.setBrightness(255);
    }
    void Setup()
    {
    }
    void UpdateLEDs(PixelArray *pixelArray)
    {
      if(true == debugLEDs) Serial << "******LED Controller LEDs******\n";
      for(int y = 0; y < SCREEN_HEIGHT; ++ y)
      {
        for(int x = 0; x < SCREEN_WIDTH; ++x)
        {
          CRGB bufColor = pixelArray->GetPixel(x, y);
          if(bufColor.red <= 1) bufColor.red = 0;
          if(bufColor.green <= 1) bufColor.green = 0;
          if(bufColor.blue <= 1) bufColor.blue = 0;
          m_LEDStrip[x][y].red = bufColor.red; //(byte)dim8_raw(bufColor.red);
          m_LEDStrip[x][y].green = bufColor.green; //(byte)dim8_raw(bufColor.green);
          m_LEDStrip[x][y].blue = bufColor.blue; //(byte)dim8_raw(bufColor.blue);
          if(true == debugLEDs) Serial << "\tR:" << bufColor.red << "\tG:" << bufColor.green << "\tB:" << bufColor.blue << " \t";
        }
        if(true == debugLEDs) Serial << "\n";
      }
      FastLED.show();
    }
    void TurnOffLEDs()
    {
      FastLED.setBrightness(0);
      if(true == debugMode && debugLevel >= 2) Serial << "Brightness set to 0.\n";
    }
    void TurnOnLEDs(unsigned int level)
    {
      FastLED.setBrightness(255*(double)level/(double)100);
      if(true == debugMode && debugLevel >= 2) Serial << "Brightness set to " << level << ".\n";
    }
  private:
    LEDStrip m_LEDStrip[NUMSTRIPS];
};

class LEDControllerInterface: public LEDController
{
  public:
    LEDControllerInterface(){}
};

#endif
