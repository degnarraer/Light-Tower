#ifndef TUNES_H
#define TUNES_H



//PIN ASSIGNMENTS
#define HARDWARE_SERIAL_RX_PIN          22
#define HARDWARE_SERIAL_TX_PIN          23

#define I2S1_SCLCK_PIN                  12
#define I2S1_WD_PIN                     13
#define I2S1_SDIN_PIN                   14
#define I2S1_SDOUT_PIN                  I2S_PIN_NO_CHANGE

#define SPI1_PIN_SCK               15
#define SPI1_PIN_MISO              17
#define SPI1_PIN_MOSI              18
#define SPI1_PIN_SS                19

//App Tunes
#define I2S_SAMPLE_RATE                 44100
#define MAX_VISUALIZATION_FREQUENCY     4000.0
#define I2S_BUFFER_COUNT                10
#define I2S_SAMPLE_COUNT                256
#define NUMBER_OF_BANDS                 32
#define FFT_SIZE                        512
#define AMPLITUDE_BUFFER_FRAME_COUNT    100
#define AUDIO_BUFFER_SIZE               2048


#define TASK_STACK_SIZE_DEBUG           false

#endif TUNES_H
