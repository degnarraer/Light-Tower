#ifndef TUNES_H
#define TUNES_H

//I2S IN PINS
#define I2S1_SCLCK_PIN      25
#define I2S1_WD_PIN         26
#define I2S1_SDIN_PIN       27
#define I2S1_SDOUT_PIN      I2S_PIN_NO_CHANGE

//CPU2&3 UART
#define CPU1_RX     12
#define CPU1_TX     13
#define CPU3_RX     14
#define CPU3_TX     15

//THREAD CORE ASSIGNMENTS
#define BLUETOOTH_TASK_CORE             tskNO_AFFINITY
#define BLUETOOTH_TASK_PRIORITY         THREAD_PRIORITY_HIGH

#define DATALINK_TASK_CORE              tskNO_AFFINITY
#define DATALINK_TASK_PRIORITY          THREAD_PRIORITY_HIGH

#define AMPLITUDE_TASK_CORE             tskNO_AFFINITY
#define AMPLITUDE_TASK_PRIORITY         THREAD_PRIORITY_LOW

#define FFT_COMPUTE_TASK_CORE           tskNO_AFFINITY
#define FFT_COMPUTE_TASK_PRIORITY       THREAD_PRIORITY_MEDIUM
#define FFT_COMPUTE_TASK_DELAY          50

#define FFT_BANDS_QUEUE_SIZE            5
#define FFT_BANDS_RECEIVE_WAIT          5
#define FFT_BANDS_SEND_WAIT             5
#define FFT_BANDS_TASK_PRIORITY         THREAD_PRIORITY_MEDIUM
#define FFT_BANDS_TASK_DELAY            50
    
//App Tunes 
#define I2S_SAMPLE_RATE                     44100
#define MAX_VISUALIZATION_FREQUENCY         4000.0
#define I2S_BUFFER_COUNT                    5
#define I2S_SAMPLE_COUNT                    1024
#define NUMBER_OF_BANDS                     32
#define FFT_SIZE                            256
#define HOP_SIZE                            256
#define AMPLITUDE_BUFFER_FRAME_COUNT        128
#define AMPLITUDE_AUDIO_BUFFER_SIZE         128
    
#define TASK_STACK_SIZE_DEBUG               false
#define TASK_LOOP_COUNT_DEBUG               false

#endif
