#ifndef TUNES_H
#define TUNES_H



//I2S IN PINS
#define I2S1_SCLCK_PIN            25
#define I2S1_WD_PIN               26
#define I2S1_SDIN_PIN             27
#define I2S1_SDOUT_PIN            I2S_PIN_NO_CHANGE

//SPI HSPI PINS
#define HSPI_PIN_SCK              14
#define HSPI_PIN_MISO             12
#define HSPI_PIN_MOSI             13
#define HSPI_PIN_SS               15

//SPI VSPI PINS
#define VSPI_PIN_SCK              18
#define VSPI_PIN_MISO             19
#define VSPI_PIN_MOSI             23
#define VSPI_PIN_SS               5

//App Tunes
#define I2S_SAMPLE_RATE                 44100
#define MAX_VISUALIZATION_FREQUENCY     4000.0
#define I2S_BUFFER_COUNT                20
#define I2S_SAMPLE_COUNT                128
#define NUMBER_OF_BANDS                 32
#define FFT_SIZE                        512
#define AMPLITUDE_BUFFER_FRAME_COUNT    100
#define AUDIO_BUFFER_SIZE               2048

#define TASK_STACK_SIZE_DEBUG           false
#define TASK_LOOP_COUNT_DEBUG           false

#endif TUNES_H
