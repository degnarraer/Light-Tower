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

#ifndef SerialDataLink_H
#define SerialDataLink_H

#include "Task.h"
#include <HardwareSerial.h>

class SerialDataLink: public Task
{
  public:
    SerialDataLink(String Title, DataManager &DataManager): Task(Title, DataManager){}
    virtual ~SerialDataLink(){}

    void Setup()
    {
      hSerial.begin(500000, SERIAL_8N1, 16, 17); // pins 16 rx2, 17 tx2, 19200 bps, 8 bits no parity 1 stop bit
    }
    bool CanRunMyTask(){return (hSerial.available());}
    void RunMyTask()
    {
      byte ch;
      ch = hSerial.read();
      m_StringData += (char)ch;
      if (ch=='\n') 
      {
        m_StringData.trim();
        Serial << "Data Received from CPU 2: " << m_StringData << "\n";
        m_StringData = "";
      }
    }
    
  private:
  HardwareSerial &hSerial = Serial2;
  String m_StringData = "";
};

#endif
