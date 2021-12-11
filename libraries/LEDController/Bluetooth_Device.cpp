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

#include "Bluetooth_Device.h"

Bluetooth_Sink::Bluetooth_Sink ( String Title
							   , i2s_port_t i2S_PORT
							   , int BufferCount
							   , int BufferSize
							   , int SerialClockPin
							   , int WordSelectPin
							   , int SerialDataInPin
							   , int SerialDataOutPin ): NamedItem(Title)
													   , m_i2S_PORT(i2S_PORT)
													   , m_BufferCount(BufferCount)
													   , m_BufferSize(BufferSize)
                                                       , m_SerialClockPin(SerialClockPin)
                                                       , m_WordSelectPin(WordSelectPin)
													   , m_SerialDataInPin(SerialDataInPin)
                                                       , m_SerialDataOutPin(SerialDataOutPin)
{
}
Bluetooth_Sink::~Bluetooth_Sink()
{
}