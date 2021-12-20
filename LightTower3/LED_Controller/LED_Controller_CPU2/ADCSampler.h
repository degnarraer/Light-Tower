/*
    Light Tower by Rob Shockency
    Copyright (C) 2020 Rob Shockency degnarraer@yahoo.com

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version of the License, or
    (at your option) any later version. 3

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
 
#ifndef ADCSAMPLER_H
#define ADCSAMPLER_H

#include "Tunes.h"
#include "Streaming.h"
#include <Arduino.h>


class ADCInterruptHandler
{
public:
    virtual void HandleADCInterrupt() = 0;
};

class SampledDataInterface
{
  public:
    virtual void SetSampleRateAndStart(unsigned int samplingRate) = 0;
    virtual void End() = 0;
    virtual void HandleADCInterrupt() = 0;
    virtual bool IsAvailable() = 0;
    virtual unsigned int GetSamplingRate() = 0;
    virtual uint16_t* GetData(int *bufferLength) = 0;
    virtual unsigned int GetNumberOfReadings() = 0;
    virtual void SetReadCompleted() = 0;
};

class ADCSampler: public SampledDataInterface
                , ADCInterruptHandler {
  public:
    ADCSampler();
    void SetSampleRateAndStart(unsigned int samplingRate){}
    void End(){}
    void HandleADCInterrupt(){}
    bool IsAvailable(){ return false; }
    unsigned int GetSamplingRate() { return 1; }
    uint16_t* GetData(int *bufferLength) { return NULL; }
    unsigned int GetNumberOfReadings() { return 0; }
    void SetReadCompleted(){}
  private:
    void StartNextBuffer(){}
    unsigned int adcDMAIndex;        //!< This hold the index of the next DMA buffer
    unsigned int adcTransferIndex;   //!< This hold the last filled buffer
    unsigned int samplingRate;
    volatile bool dataReady;
    volatile bool bufferOverflow;
    uint16_t adcBuffer[NUMBER_OF_BUFFERS][BUFFER_SIZE];
};

#endif
