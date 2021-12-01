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

#ifndef I2S_EventHander_H
#define I2S_EventHander_H

#define EVENT_HANDLER_DEBUG false

#include <I2S_Device.h>
#include "FFT_Calculator.h"
#include <DataTypes.h>

class Manager: NamedItem
{
  public:
    Manager(String Title, FFT_Calculator &fftCalculator);
    virtual ~Manager();
    void Setup();
    void RunTask();
    void ProcessEventQueue();
  private:
    FFT_Calculator &m_FFT_Calculator;
    I2S_Device *m_Mic;
    I2S_Device *m_Speaker;
};

#endif
