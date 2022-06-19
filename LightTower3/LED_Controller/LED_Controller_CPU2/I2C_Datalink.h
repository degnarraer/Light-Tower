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

#ifndef I2C_Datalink_H
#define I2C_Datalink_H
#include <Wire.h>
#include <WireSlaveRequest.h>
#include <WireSlave.h>
#include <Helpers.h>

class I2C_Datalink: public NamedItem
{
  public:
    I2C_Datalink( String Title, TwoWire &TwoWire, uint8_t SDA_Pin, uint8_t SCL_Pin )
                : NamedItem(Title)
                , m_TwoWire(&TwoWire)
                , m_SDA_PIN(SDA_Pin)
                , m_SCL_PIN(SCL_Pin){}
    virtual ~I2C_Datalink(){}

    //Master Functions
    void SetupAsMaster(uint16_t MaxResponseLength, uint32_t Freq);
    char ReadDataFromSlave(uint8_t SlaveAddress);

    //Slave Functions
    void SetupAsSlave(uint8_t My_Address, uint16_t MaxResponseLength);
    void UpdateI2C();

    //Callback Functions
    static void ReceiveEvent(int howMany);
    static void RequestEvent();
  private:
    TwoWire *m_TwoWire;
    uint8_t m_SDA_PIN;
    uint8_t m_SCL_PIN;
    uint8_t m_Slave_Address;
    uint16_t m_MaxResponseLength;
    uint32_t m_Freq;
};
#endif
