
#ifndef ADCSAMPLER_H
#define ADCSAMPLER_H

#include "Tunes.h"
#include "Streaming.h"
#include <Arduino.h>

class ADCSampler {
  public:
    ADCSampler();
    void begin(unsigned int samplingRate);
    void end();
    void handleInterrupt();
    bool available();
    unsigned int getSamplingRate();
    uint16_t* getFilledBuffer(int *bufferLength);
    unsigned int numberOfReadings();
    void StartNextBuffer();
    void readBufferDone();
  private:
    unsigned int samplingRate;
    volatile bool dataReady;
    volatile bool bufferOverflow;
    uint16_t adcBuffer[NUMBER_OF_BUFFERS][BUFFER_SIZE];
  public:
    unsigned int adcDMAIndex;        //!< This hold the index of the next DMA buffer
    unsigned int adcTransferIndex;   //!< This hold the last filled buffer
    unsigned int adcReadIndex;       //!< This hold the last read buffer

};

#endif /* ADCSAMPLER_H */
