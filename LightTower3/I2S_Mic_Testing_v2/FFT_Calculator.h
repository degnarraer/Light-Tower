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

#include "EventSystem.h"

class FFT_Calculator: public Task, EventSystemCallee
{
  public:
    FFT_Calculator(String Title, DataManager &DataManager): m_DataManager(DataManager)
                                                            , Task(Title, m_DataManager){}
    virtual ~FFT_Calculator()
    {
    }
    CalculateFFT()
    {
    }

    //EventSystemCallee Interface
    EventSystemNotification(String context)
    {
      if(MicrophoneDataReady == context)
      {
        
      }
      if(MicrophoneRightDataReady == context)
      {
        
      }
      if(MicrophoneLeftDataReady == context)
      {
        
      }
    }
};



#endif
