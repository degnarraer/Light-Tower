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

#ifndef Tunes_H
#define Tunes_H

#include <math.h>

//Output Debug Messages
const bool   debugMode = true;
const int    debugLevel = 1;

const bool   debugNanInf = false;
const bool   debugPlotMic = false;
const bool   debugPlotFFT = false;
const bool   debugFPS = true;
const bool   debugTasks = false;
const bool   debugModelNotifications = false;
const bool   debugModelNewValueProcessor = true;
const bool   debugSoundPower = false;
const bool   debugVisualization = false;
const bool   debugView = false;
const bool   debugLEDs = false;
const bool   debugModels = false;
const bool   debugSetBandValueStatisticalEngine = true;
const bool   debugGetBandValueStatisticalEngine = false;
const bool   debugGravitationalModel = false;
const bool   debugMemory = false;
const bool   debugFreeMemory = false;
const bool   debugAssertions = false;
const bool   debugRequired = ( debugMode || 
                               debugNanInf || 
                               debugPlotMic || 
                               debugPlotFFT || 
                               debugFPS || 
                               debugTasks || 
                               debugModelNotifications ||
                               debugModelNewValueProcessor ||
                               debugSoundPower ||
                               debugVisualization ||
                               debugView ||
                               debugLEDs ||
                               debugModels ||
                               debugSetBandValueStatisticalEngine ||
                               debugGetBandValueStatisticalEngine ||
                               debugGravitationalModel ||
                               debugMemory ||
                               debugFreeMemory ||
                               debugAssertions);

// LED SETUP
static const unsigned int NUMLEDS = 50;
static const unsigned int NUMSTRIPS = 4;
static const unsigned int SCREEN_WIDTH = NUMSTRIPS;
static const unsigned int SCREEN_HEIGHT = NUMLEDS;

//TBD Update This:
static const float LEDS_PER_METER = 60.0;
const unsigned int DATA_PIN_STRIP1 = 25;  //MATRIX1 PIN ASSIGNMENT
const unsigned int DATA_PIN_STRIP2 = 26;  //MATRIX2 PIN ASSIGNMENT
const unsigned int DATA_PIN_STRIP3 = 27;  //MATRIX3 PIN ASSIGNMENT
const unsigned int DATA_PIN_STRIP4 = 33;  //MATRIX4 PIN ASSIGNMENT

// VISUALIZATION TUNES
const float MAX_DISPLAYED_FREQ = 22050.0;

//FFT Tunes
const int FFT_MAX = 512;
const unsigned int SAMPLE_RATE = 44100;
const int SAMPLE_TIME_US = 1000000.0/SAMPLE_RATE;

//VU
const unsigned int BAND_SAVE_LENGTH = 10;
const unsigned int POWER_SAVE_LENGTH = 10;

//Trigger Level
const float SILENCE_THRESHOLD = 0.05;
const float triggerLevelGain = 1.1;

//Sound Detection
const float   SOUND_DETECT_THRESHOLD = 0.05;
const int     silenceIntegratorMax = 50000;
const int     silenceDetectedThreshold = 0.1*silenceIntegratorMax;
const int     soundDetectedThreshold = 0.9*silenceIntegratorMax;
const int     soundAdder = 1000;
const int     silenceSubtractor = -10;

//CALCULATED TUNES
const int FFT_M = (int)log2(FFT_MAX);
const int BINS = FFT_MAX / 2;
const float BINS_DOUBLE = FFT_MAX / 2.0;
const int ADC_RESOLUTION = 16;
const int ADDBITS = pow(2,ADC_RESOLUTION);
const int FFT_GAIN = 1000;
const int POWER_GAIN = 20;
const int MAX_POWER = ADDBITS * POWER_GAIN;
const float MAX_DB = 20*log10(ADDBITS);

#endif
