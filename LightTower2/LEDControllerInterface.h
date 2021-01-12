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

template <typename T>
T** Create2DArray(unsigned nrows, unsigned ncols, const T& val = T())
{
   if (nrows == 0){}
        //throw std::invalid_argument("number of rows is 0");
   if (ncols == 0){}
        //throw std::invalid_argument("number of columns is 0");
   T** ptr = NULL;
   T* pool = NULL;

   ptr = new T*[nrows];  // allocate pointers (can throw here)
   pool = new T[nrows*ncols]{val};  // allocate pool (can throw here)

   // now point the row pointers to the appropriate positions in
   // the memory pool
   for (unsigned i = 0; i < nrows; ++i, pool += ncols )
       ptr[i] = pool;

   // Done.
   return ptr;
}

template <typename T>
void Delete2DArray(T** arr)
{
   delete [] arr[0];  // remove the pool
   delete [] arr;     // remove the pointers
}

struct PixelStruct
{ 
  CRGB (Pixel[SCREEN_WIDTH][SCREEN_HEIGHT]);
  public:
    PixelStruct()
    {
      Clear();
    }
    void Clear()
    {
      for(int x = 0; x < SCREEN_WIDTH; ++x)
      {
        for(int y = 0; y < SCREEN_HEIGHT; ++y)
        {
          Pixel[x][y] = CRGB::Black;
        }
      }
    }
};

class PixelArray
{
  public:
    PixelArray(int X, int Y, int W, int H): m_X(X), m_Y(Y),  m_W(W), m_H(H)
    {
      m_Pixel = Create2DArray<CRGB>(m_W, m_H, CRGB::Black);
    }
    virtual ~PixelArray()
    {
      if(true == debugMemory) Serial << "Delete: PixelArray\n";
      Delete2DArray(m_Pixel);
    }
    void Clear()
    {
      for(int x = 0; x < m_W; ++x)
      {
        for(int y = 0; y < m_H; ++y)
        {
          m_Pixel[x][y] = CRGB::Black;
        }
      }
    }
    void SetPosition(int X, int Y)
    {
      m_X = X;
      m_Y = Y;
    }
    CRGB GetPixel(int X, int Y)
    {
      if( (X >= m_X) && (X < m_X + m_W) && (Y >= m_Y) && (Y < m_Y + m_H) )
      {
        return m_Pixel[X - m_X][Y - m_Y];
      }
      else
      {
        return CRGB::Black;
      }
    }
    void SetPixel(int X, int Y, CRGB value)
    {
      if( (X >= m_X) && (X < m_X + m_W) && (Y >= m_Y) && (Y < m_Y + m_H) )
      {
        m_Pixel[X - m_X][Y - m_Y] = value;
      }
    }
    int GetWidth(){ return m_W; }
    int GetHeight(){ return m_H; }
  private:
    CRGB** m_Pixel;
    int m_X = 0;
    int m_Y = 0;
    int m_W = 0;
    int m_H = 0;
  
};

class LEDController
{
  public:
    LEDController()
    {
      FastLED.addLeds<WS2812, DATA_PIN_STRIP1, GRB>(m_LEDStrip[0], NUMLEDS);
      FastLED.addLeds<WS2812, DATA_PIN_STRIP2, GRB>(m_LEDStrip[1], NUMLEDS);
      FastLED.addLeds<WS2812, DATA_PIN_STRIP3, GRB>(m_LEDStrip[2], NUMLEDS);
      FastLED.addLeds<WS2812, DATA_PIN_STRIP4, GRB>(m_LEDStrip[3], NUMLEDS);
      FastLED.setBrightness(255);
    }
    void Setup()
    {
      
    }
    void UpdateLEDs(PixelStruct &pixelStruct)
    {
      if(true == debugLEDs) Serial << "******LED Controller LEDs******\n";
      for(int y = 0; y < SCREEN_HEIGHT; ++ y)
      {
        for(int x = 0; x < SCREEN_WIDTH; ++x)
        {
          CRGB bufColor = pixelStruct.Pixel[x][y];
          m_LEDStrip[x][y].red =(byte)dim8_raw(bufColor.red);
          m_LEDStrip[x][y].green = (byte)dim8_raw(bufColor.green);
          m_LEDStrip[x][y].blue = (byte)dim8_raw(bufColor.blue);
          if(true == debugLEDs) Serial << "\tR:" << bufColor.red << "\tG:" << bufColor.green << "\tB:" << bufColor.blue << " \t";
        }
        if(true == debugLEDs) Serial << "\n";
      }
      FastLED.show();
    }
    void UpdateLEDs(PixelArray *pixelArray)
    {
      if(true == debugLEDs) Serial << "******LED Controller LEDs******\n";
      for(int y = 0; y < SCREEN_HEIGHT; ++ y)
      {
        for(int x = 0; x < SCREEN_WIDTH; ++x)
        {
          CRGB bufColor = pixelArray->GetPixel(x, y);
          m_LEDStrip[x][y].red =(byte)dim8_raw(bufColor.red);
          m_LEDStrip[x][y].green = (byte)dim8_raw(bufColor.green);
          m_LEDStrip[x][y].blue = (byte)dim8_raw(bufColor.blue);
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
