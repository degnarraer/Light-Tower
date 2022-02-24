#ifndef TUNES_H
#define TUNES_H

//App Tunes
#define I2S_SAMPLE_RATE 44100
#define DAC_MUTE_PIN 22

#define I2S_BUFFER_COUNT 30
#define I2S_CHANNEL_SAMPLE_COUNT 128
#define ANALOG_GAIN 1


//App Debugging
#define STARTUP_DEBUG false
#define TASK_STACK_SIZE_DEBUG false
#define HEAP_SIZE_DEBUG false

//Output Debug Messages
const bool   debugMode = true;
const int    debugLevel = 1;

const bool   debugNanInf = false;
const bool   debugPlotMic = false;
const bool   debugPlotFFT = false;
const bool   debugFPS = false;
const bool   debugTasks = false;
const bool   debugModelNotifications = false;
const bool   debugModelNewValueProcessor = false;
const bool   debugSoundPower = false;
const bool   debugVisualization = false;
const bool   debugView = false;
const bool   debugLEDs = false;
const bool   debugModels = false;
const bool   debugSetBandValueStatisticalEngine = false;
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
const unsigned int DATA_PIN_STRIP1 = 18;  //MATRIX1 PIN ASSIGNMENT
const unsigned int DATA_PIN_STRIP2 = 19;  //MATRIX2 PIN ASSIGNMENT
const unsigned int DATA_PIN_STRIP3 = 22;  //MATRIX3 PIN ASSIGNMENT
const unsigned int DATA_PIN_STRIP4 = 23;  //MATRIX4 PIN ASSIGNMENT


//FFT Tunes
const int FFT_MAX = 2048;
const unsigned int SAMPLE_RATE = 44100;
const int SAMPLE_TIME_US = 1000000.0/SAMPLE_RATE;

// VISUALIZATION TUNES
const float MAX_DISPLAYED_FREQ = 10000;

//VU
const unsigned int BAND_SAVE_LENGTH = 20;
const unsigned int POWER_SAVE_LENGTH = 20;

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
const int FFT_RESOLUTION = 16;
const int FFT_GAIN = 1000;
const int POWER_GAIN = 20;
#endif TUNES_H
