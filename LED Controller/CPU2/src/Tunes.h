#ifndef TUNES_H
#define TUNES_H

//I2S IN PINS
#define I2S1_SCLCK_PIN            25
#define I2S1_WD_PIN               26
#define I2S1_SDIN_PIN             27
#define I2S1_SDOUT_PIN            I2S_PIN_NO_CHANGE

//CPU2&3 UART
#define CPU1_RX                   12
#define CPU1_TX                   13
#define CPU3_RX                   14
#define CPU3_TX                   15

//App Tunes
#define I2S_SAMPLE_RATE                 44100
#define MAX_VISUALIZATION_FREQUENCY     4000.0
#define I2S_BUFFER_COUNT                10
#define I2S_SAMPLE_COUNT                512
#define NUMBER_OF_BANDS                 32
#define FFT_SIZE                        512
#define AMPLITUDE_BUFFER_FRAME_COUNT    100
#define AUDIO_BUFFER_SIZE               2048

#define TASK_STACK_SIZE_DEBUG           false
#define TASK_LOOP_COUNT_DEBUG           false

#endif
