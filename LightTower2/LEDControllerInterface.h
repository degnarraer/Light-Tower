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
