#pragma once

//LED OUT PINS
#define DATA_PIN_STRIP1_PIN       0
#define DATA_PIN_STRIP2_PIN       2
#define DATA_PIN_STRIP3_PIN       4
#define DATA_PIN_STRIP4_PIN       5

//I2S IN PINS
#define I2S1_SCLCK_PIN            25
#define I2S1_WD_PIN               26
#define I2S1_SDIN_PIN             27
#define I2S1_SDOUT_PIN            I2S_PIN_NO_CHANGE

//I2S OUT PINS
#define I2S2_SCLCK_PIN            21
#define I2S2_WD_PIN               32
#define I2S2_SDIN_PIN             I2S_PIN_NO_CHANGE
#define I2S2_SDOUT_PIN            33

//CPU2&3 UART
#define CPU2_RX                   12
#define CPU2_TX                   13
#define CPU3_RX                   14
#define CPU3_TX                   15

//THREAD CORE ASSIGNMENTS
#define BLUETOOTH_TASK_CORE                 tskNO_AFFINITY
#define BLUETOOTH_TASK_PRIORITY             THREAD_PRIORITY_HIGH

#define DATALINK_TASK_CORE                  tskNO_AFFINITY
#define DATALINK_TASK_PRIORITY              THREAD_PRIORITY_MEDIUM

#define VISUALIZATION_TASK_CORE             tskNO_AFFINITY
#define MICROPHONE_DATE_REQUEST_TASK_CORE   tskNO_AFFINITY
#define STATISTICAL_ENGINE_TASK_CORE        tskNO_AFFINITY

//App Tunes
#define I2S_SAMPLE_RATE 44100

#define I2S_BUFFER_COUNT 100
#define I2S_SAMPLE_COUNT 64
#define ANALOG_GAIN 1


//App Debugging
#define STARTUP_DEBUG false
#define TASK_STACK_SIZE_DEBUG false
#define TASK_LOOP_COUNT_DEBUG false
#define HEAP_SIZE_DEBUG false

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
const bool   debugSilenceIntegrator = false;
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
const float SILENCE_THRESHOLD = 0.01;
const float triggerLevelGain = 1.1;

//Sound Detection
const float   SOUND_DETECT_THRESHOLD = 0.01;
const int     lastingSilenceTImeout = 5000;
const int     silenceIntegratorMax = 50000;
const int     silenceDetectedThreshold = 0.3*silenceIntegratorMax;
const int     soundDetectedThreshold = 0.7*silenceIntegratorMax;
const int     soundAdder = 5000000;
const int     silenceSubtractor = -5000000;

//CALCULATED TUNES
const int FFT_M = (int)log2(FFT_MAX);
const int BINS = FFT_MAX / 2;
const float BINS_DOUBLE = FFT_MAX / 2.0;
const int FFT_RESOLUTION = 16;
const int FFT_GAIN = 1000;
const int POWER_GAIN = 20;
