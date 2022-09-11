#ifndef TUNES_H
#define TUNES_H

#define I2C_SDA_PIN                     21
#define I2C_SCL_PIN                     22
#define I2C_SLAVE_ADDR                  0x04
#define MAX_SLAVE_RESPONSE_LENGTH       124
#define I2C_MASTER_FREQ                 1000000
#define I2C_MASTER_REQUEST_TIMEOUT      3
#define I2C_MASTER_REQUEST_RETRY_COUNT  3

//PIN ASSIGNMENTS
#define HARDWARE_SERIAL_RX_PIN          25
#define HARDWARE_SERIAL_TX_PIN          26
#define I2S2_SCLCK_PIN                  12
#define I2S2_WD_PIN                     13
#define I2S2_SDIN_PIN                   I2S_PIN_NO_CHANGE
#define I2S2_SDOUT_PIN                  14

//App Tunes
#define I2S_SAMPLE_RATE                 44100
#define MAX_VISUALIZATION_FREQUENCY     4000.0
#define I2S_BUFFER_COUNT                20
#define I2S_SAMPLE_COUNT                128
#define NUMBER_OF_BANDS                 32
#define FFT_SIZE                        512

#endif TUNES_H
