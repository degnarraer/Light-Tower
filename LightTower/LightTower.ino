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

long startMillis;
long currentMillis;
int frameCount = 0;

void setup()
{
  if( true == debugMode ||
      true == debugNanInf ||
      true == debugPlotMic ||
      true == debugPlotFFT)
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
  long lapsedTime = currentMillis - startMillis;
  if(lapsedTime >= 1000)
  {
    startMillis = millis();
    if(true == debugMode && debugLevel >= 2) Serial << "FPS: " << frameCount / (lapsedTime/1000.0) << "\n";
    frameCount = 0;
  }
}


/*
#include "Tunes.h"
#include "ADCSampler.h"


ADCSampler sampler;
unsigned int samplingRate = 26000;


void setup()
{
  
  Serial.begin(115200);  // To print debugging messages.
  Serial.print("Start ... Sampling Rate is set to ");
  Serial.print(samplingRate);
  Serial.println("Hz");

  Serial.print("The DMA uses ");;
  Serial.print(NUMBER_OF_BUFFERS);
  Serial.print(" buffers with ");
  Serial.print(BUFFER_SIZE);
  Serial.println(" elements each");
  Serial.print("A serial stream will be send every ");
  double interval = (((double) BUFFER_SIZE) / samplingRate) / NUM_CHANNELS;
  Serial.print(interval);
  Serial.println(" seconds");
  sampler.begin(samplingRate);
}

void loop()
{
  if (sampler.available()) {
    int bufferLength = 0;
    uint16_t* cBuf = sampler.getFilledBuffer(&bufferLength);
    for (int i = 0; i < bufferLength; i=i+2)
    {
      Serial.print(cBuf[i]);
      Serial.print(",");
      Serial.println(cBuf[i+1]);
    }
    Serial.println("----");
    sampler.readBufferDone();
  }
}

void ADC_Handler()
{
  sampler.handleInterrupt();
}
*/
