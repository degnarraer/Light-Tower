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
 
#ifndef DataSampler_H
#define DataSampler_H

#include "Tunes.h"
#include "Streaming.h"

class SampledDataInterface
{
  public:
    virtual void End() = 0;
    virtual bool IsAvailable() = 0;
    virtual unsigned int GetSamplingRate() = 0;
    virtual uint16_t* GetData(int *bufferLength) = 0;
    virtual unsigned int GetNumberOfReadings() = 0;
};

class DataSampler: public SampledDataInterface {
  public:
    DataSampler(){}
    virtual ~DataSampler(){}
    void End(){}
    bool IsAvailable(){ return false; }
    void SetSampleRateAndStart(unsigned int samplingRate){}
    unsigned int GetSamplingRate() { return 0; };
    uint16_t* GetData(int *bufferLength){ return NULL; }
    unsigned int GetNumberOfReadings() { return 0; }
  private:

};

#endif
