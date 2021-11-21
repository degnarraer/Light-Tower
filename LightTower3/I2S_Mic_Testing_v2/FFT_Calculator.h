/*
    Light Tower by Rob Shockency
    Copyright (C) 2021 Rob Shockency degnarraer@yahoo.com

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

#ifndef I2S_FFT_CALCULATOR_H
#define I2S_FFT_CALCULATOR_H
#define FFT_LENGTH 1024
#define DEBUG_FFT_CALCULATOR false
#define DEBUG_FFT_CALCULATOR_LOOPS false
#define DEBUG_FFT_CALCULATOR_INPUTDATA false
#define DEBUG_FFT_CALCULATOR_OUTPUTDATA false

#include "EventSystem.h"
#include "Task.h"
#include <Adafruit_ZeroFFT.h>

class FFT_Calculator: public Task, EventSystemCallee
{
  public:
    FFT_Calculator(String Title, DataManager &DataManager); //
    virtual ~FFT_Calculator();

    //Task Interface
    void Setup();
    bool CanRunMyTask();
    void RunMyTask();
    
    //EventSystemCallee Interface
    void EventSystemNotification(String context);
    
  private:
    int16_t m_FFT_Right_Data[FFT_LENGTH];
    int32_t m_FFT_Right_Buffer_Data[FFT_LENGTH];
    int m_FFT_Right_Buffer_Index = 0;
    int16_t m_FFT_Left_Data[FFT_LENGTH];
    int32_t m_FFT_Left_Buffer_Data[FFT_LENGTH];
    int m_FFT_Left_Buffer_Index = 0;
};



#endif
