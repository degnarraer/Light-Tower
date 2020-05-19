    /*
    Light Tower by Rob Shockency
    Copyright (C) 2019 Rob Shockency degnarraer@yahoo.com

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
    */
/**
 * @file LEDControllerInterface.h
 *
 *
 */

#ifndef LEDControllerInterface_H
#define LEDControllerInterface_H

#define FASTLED_ALLOW_INTERRUPTS 1
#define FASTLED_INTERRUPT_RETRY_COUNT 1
#include <FastLED.h>
#include "Tunes.h"
#include "Streaming.h"

typedef CRGBArray<NUMLEDS> LEDStrip;

class LEDControllerInterface
{
  public:
    LEDControllerInterface(){}
    virtual void Start() = 0;
    virtual bool Loop() = 0;
    virtual void End() = 0;
};


class LEDController
{
  public:
    LEDController()
    {
      FastLED.addLeds<WS2812, DATA_PIN_STRIP1, GRB>(m_strips[0], NUMLEDS);
      FastLED.addLeds<WS2812, DATA_PIN_STRIP2, GRB>(m_strips[1], NUMLEDS);
      FastLED.addLeds<WS2812, DATA_PIN_STRIP3, GRB>(m_strips[2], NUMLEDS);
      FastLED.addLeds<WS2812, DATA_PIN_STRIP4, GRB>(m_strips[3], NUMLEDS);
      FastLED.setBrightness(255);
    }
    void Setup()
    {
      
    }
    void TurnOffLEDs()
    {
      FastLED.setBrightness(0);
      if(true == debugMode && debugLevel >= 2) Serial << "Brightness set to 0.\n";
    }
    void TurnOnLEDs(unsigned int level)
    {
      FastLED.setBrightness(255*level/100);
      if(true == debugMode && debugLevel >= 2) Serial << "Brightness set to " << level << ".\n";
    }
  protected:
    LEDStrip m_strips[NUMSTRIPS];
};

#endif
