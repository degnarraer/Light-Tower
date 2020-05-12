
#ifndef Tunes_H
#define Tunes_H

#include <math.h>

//Mic Selection
enum MicType
{
  MIC_TYPE_AUTOGAIN,
  MIC_TYPE_FIXEDGAIN
};
static MicType micType = MicType::MIC_TYPE_FIXEDGAIN;

//Run Fixed Data Tables instead of microphone
static bool   testMode = false;

//Output Debug Messages
static bool   debugMode = true;
static bool   debugNanInf = false;
static bool   debugPlotMic = false;
static bool   debugPlotFFT = false;
static int    debugLevel = 1;

// LED SETUP
#define NUMLEDS 60
#define NUMSTRIPS 4
#define DATA_PIN_STRIP1 4
#define DATA_PIN_STRIP2 5
#define DATA_PIN_STRIP3 6
#define DATA_PIN_STRIP4 7

// VISUALIZATION TUNES
#define NUMSPRITES 8
#define MAX_DISPLAYED_FREQ 10000.0
#define NUMBER_OF_TICK_TIMERS 10
#define NUMBER_OF_LAYERS 10
#define NUMBER_OF_FADE_CONTROLLERS 10

//FFT Tunes
const int FFT_MAX = 128;
const unsigned int SAMPLE_RATE = 26000;
const int SAMPLE_TIME_US = 1000000.0/SAMPLE_RATE;

// Sampler
const unsigned int NUM_CHANNELS = 3;
/* ch7:A0 ch6:A1 ch5:A2 ch4:A3 ch3:A4 ch2:A5 ch1:A6 ch0:A7 */
#define ADC_CHANNELS ADC_CHER_CH7 | ADC_CHER_CH5 | ADC_CHER_CH4
const unsigned int CHANNEL_SIZE = FFT_MAX;
const unsigned int BUFFER_SIZE = CHANNEL_SIZE*NUM_CHANNELS;
const unsigned int NUMBER_OF_BUFFERS = 20;  /// Make this 3 or greater

//Sound Detection
const float   SOUND_DETECT_THRESHOLD = 0.05;
const int     silenceIntegratorMax = 50000;
const int     silenceDetectedThreshold = 0.1*silenceIntegratorMax;
const int     soundDetectedThreshold = 0.9*silenceIntegratorMax;
const int     soundAdder = 1000;
const int     silenceSubtractor = -100;

//Other Tunes
#define ADC_RESOLUTION 12
const float Vin = 3.3;
const int BIN_SAVE_LENGTH = 60;

//Trigger Level
const float SILENCE_THRESHOLD = 0.025;
static float triggerLevelGain = 1.0;

//CALCULATED TUNES
const int FFT_M = (int)log2(FFT_MAX);
const int BINS = FFT_MAX / 2;
const float BINS_DOUBLE = FFT_MAX / 2.0;
const int ADDBITS = pow(2,ADC_RESOLUTION);
const int FFT_GAIN = 200;
const int POWER_GAIN = 10;
const int MAX_POWER = ADDBITS * POWER_GAIN;
const float MAX_DB = 20*log10(ADDBITS);
#endif
