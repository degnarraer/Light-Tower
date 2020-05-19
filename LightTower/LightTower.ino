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
 * @file LightTower.ino
 *
 *
 */

#include <Arduino.h>
#include "ActiveVisualizationController.h"
#include "Statistical_Engine.h"

ActiveVisualizationController activeVisualizationController;

unsigned long startMillis;
unsigned long currentMillis;
unsigned int frameCount = 0;

void setup()
{
  if( true == debugMode ||
      true == debugNanInf ||
      true == debugPlotMic ||
      true == debugPlotFFT ||
      true == debugFPS )
  {
    Serial.begin(115200);
  }
  else
  {
    pinMode(0, OUTPUT); 
    pinMode(1, OUTPUT); 
    digitalWrite(0, HIGH);
    digitalWrite(1, HIGH);  
  }
  delay(3000);
  randomSeed(analogRead(4)*1000);
  if(true == debugMode && debugLevel >= 0) Serial.println("Setup");
  if(true == debugMode && debugLevel >= 0) Serial << "SAMPLE_RATE: " << SAMPLE_RATE << "\n";
  if(true == debugMode && debugLevel >= 0) Serial << "FFT_MAX: " << FFT_MAX << "\n";
  activeVisualizationController.Setup();
  startMillis = millis();
}

void ADC_Handler()
{
  activeVisualizationController.HandleInterrupt();
}

void loop()
{
  activeVisualizationController.Loop();
  ++frameCount;
  currentMillis = millis();
  unsigned long lapsedTime = currentMillis - startMillis;
  if(lapsedTime >= 1000)
  {
    startMillis = millis();
    if(true == debugFPS) Serial << "FPS: " << frameCount / (lapsedTime/1000.0) << "\n";
    frameCount = 0;
  }
}
