#ifndef TUNES_H
#define TUNES_H

//PIN ASSIGNMENTS
#define I2S1_SCLCK_PIN          12
#define I2S1_WD_PIN             13
#define I2S1_SDIN_PIN           14
#define I2S1_SDOUT_PIN          I2S_PIN_NO_CHANGE
#define HARDWARE_SERIAL_RX_PIN  22
#define HARDWARE_SERIAL_TX_PIN  23
#define I2S2_SCLCK_PIN          25
#define I2S2_WD_PIN             26
#define I2S2_SDIN_PIN           I2S_PIN_NO_CHANGE
#define I2S2_SDOUT_PIN          33

//App Tunes
#define I2S_SAMPLE_RATE 44100
#define MAX_VISUALIZATION_FREQUENCY 4000.0
#define I2S_BUFFER_COUNT 10
#define I2S_SAMPLE_COUNT 64
#define NUMBER_OF_BANDS 32
#define FFT_SIZE 512

#endif TUNES_H
