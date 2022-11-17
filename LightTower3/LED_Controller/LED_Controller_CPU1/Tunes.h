#ifndef TUNES_H
#define TUNES_H

//PIN Assignments
#define DATA_PIN_STRIP1_PIN       0
#define DATA_PIN_STRIP2_PIN       2
#define DATA_PIN_STRIP3_PIN       4
#define DATA_PIN_STRIP4_PIN       5

//I2S MIC IN PINS
#define I2S1_SCLCK_PIN            25
#define I2S1_WD_PIN               26
#define I2S1_SDIN_PIN             33
#define I2S1_SDOUT_PIN            I2S_PIN_NO_CHANGE

//I2S OUT PINS
#define I2S2_SCLCK_PIN            12
#define I2S2_WD_PIN               13
#define I2S2_SDIN_PIN             I2S_PIN_NO_CHANGE
#define I2S2_SDOUT_PIN            14

#define SPI1_PIN_SCK               15
#define SPI1_PIN_MISO              17
#define SPI1_PIN_MOSI              18
#define SPI1_PIN_SS                19


//App Tunes
#define I2S_SAMPLE_RATE 44100

#define I2S_BUFFER_COUNT 10
#define I2S_CHANNEL_SAMPLE_COUNT 512
#define ANALOG_GAIN 1


//App Debugging
#define STARTUP_DEBUG false
#define TASK_STACK_SIZE_DEBUG true
#define HEAP_SIZE_DEBUG true

//Visualization Debug Messages
const bool   debugMode = false;
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

//FFT Tunes
const int FFT_MAX = 512;
const unsigned int SAMPLE_RATE = 44100;
const int SAMPLE_TIME_US = 1000000.0/SAMPLE_RATE;

// VISUALIZATION TUNES
const float MAX_DISPLAYED_FREQ = 4000;

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
const int FFT_RESOLUTION = 16;
const int FFT_GAIN = 1000;
const int POWER_GAIN = 20;
#endif TUNES_H
