

#ifndef TUNES_H
#define TUNES_H

//App Tunes
#define DAC_MUTE_PIN 22
#define DAC_SF0_PIN 32
#define DAC_SF1_PIN 33
#define I2S_BUFFER_COUNT 10
#define I2S_CHANNEL_SAMPLE_COUNT 128
#define NUMBER_OF_BANDS 32
#define FFT_LARGE_SIZE 2048
#define FFT_SMALL_SIZE 512
#define ANALOG_GAIN 1


//App Debugging
#define TASK_STACK_SIZE_DEBUG false
#define HEAP_SIZE_DEBUG false
#define SOUND_PROCESSOR_DEBUG false
#define SOUND_PROCESSOR_QUEUE_DEBUG false
#define SOUND_PROCESSOR_LOOPS_DEBUG false
#define SOUND_PROCESSOR_INPUTDATA_R_DEBUG false
#define SOUND_PROCESSOR_INPUTDATA_L_DEBUG false
#define SOUND_PROCESSOR_INPUT_COMPLETE_DATA_R_DEBUG false
#define SOUND_PROCESSOR_INPUT_COMPLETE_DATA_L_DEBUG false
#define SOUND_PROCESSOR_MAX_BAND_INPUT_COMPLETE_DATA_R_DEBUG true
#define SOUND_PROCESSOR_MAX_BAND_INPUT_COMPLETE_DATA_L_DEBUG false
#define SOUND_PROCESSOR_OUTPUTDATA_R_DEBUG false
#define SOUND_PROCESSOR_OUTPUTDATA_L_DEBUG false
#define SOUND_PROCESSOR_OUTPUT_R_BANDDATA_DEBUG false
#define SOUND_PROCESSOR_OUTPUT_L_BANDDATA_DEBUG false
#define SOUND_PROCESSOR_OUTPUT_R_MAXBAND_DEBUG false
#define SOUND_PROCESSOR_OUTPUT_L_MAXBAND_DEBUG false
#define EVENT_HANDLER_DEBUG false
#define PRINT_DATA_DEBUG false
#define PRINT_BYTE_MANIPULATION_DEBUG false
#define PRINT_RIGHT_CHANNEL_DATA_DEBUG false
#define PRINT_LEFT_CHANNEL_DATA_DEBUG false
#define SAWTOOTH_OUTPUT_DATA_DEBUG false


#endif TUNES_H
