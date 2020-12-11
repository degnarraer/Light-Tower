
#ifndef ADCSAMPLER_H
#define ADCSAMPLER_H

#include "Tunes.h"
#include "Streaming.h"
#include <Arduino.h>


class InterruptHandler
{
public:
    virtual void HandleInterrupt() = 0;
};

class ADCSampler: public InterruptHandler {
  public:
    ADCSampler();
    void SetSampleRateAndStart(unsigned int samplingRate);
    void End();
    void HandleInterrupt();
    bool IsAvailable();
    unsigned int GetSamplingRate();
    uint16_t* GetFilledBuffer(int *bufferLength);
    unsigned int GetNumberOfReadings();
    void StartNextBuffer();
    void SetReadCompleted();
  private:
    unsigned int samplingRate;
    volatile bool dataReady;
    volatile bool bufferOverflow;
    uint16_t adcBuffer[NUMBER_OF_BUFFERS][BUFFER_SIZE];
  public:
    unsigned int adcDMAIndex;        //!< This hold the index of the next DMA buffer
    unsigned int adcTransferIndex;   //!< This hold the last filled buffer
};

#endif /* ADCSAMPLER_H */
