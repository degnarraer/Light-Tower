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

#include "EventSystem.h"
#include "I2S_Device.h"

class I2S_EventHandler: public Task, EventSystemCallee
{
  public:
    I2S_EventHandler(String Title, DataManager &DataManager): m_DataManager(DataManager)
                                                            , Task(Title, m_DataManager){}
    virtual ~I2S_EventHandler()
    {
      delete m_Mic;
    }
    //Task
    void Setup()
    {
      m_Mic = new I2S_Device( "Mic"
                            , m_DataManager
                            , I2S_NUM_0
                            , i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX)
                            , 44100
                            , I2S_BITS_PER_SAMPLE_32BIT
                            , I2S_CHANNEL_FMT_RIGHT_LEFT
                            , i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB)
                            , I2S_CHANNEL_STEREO
                            , 8
                            , 500
                            , 12
                            , 13
                            , 14
                            , I2S_PIN_NO_CHANGE );
      AddTask(*m_Mic);
      m_Mic->RegisterForEventNotification(this, m_Mic->MicrophoneNotification);
      m_Mic->StartDevice();
    }
    bool CanRunMyTask() { return true; }
    void RunMyTask(){}
    //Event System Callee
    void EventSystemNotification(String context)
    {
      if(m_Mic->MicrophoneNotification == context)
      {
      }
    }
  private:
    I2S_Device *m_Mic;
    DataManager &m_DataManager;
};

#endif
