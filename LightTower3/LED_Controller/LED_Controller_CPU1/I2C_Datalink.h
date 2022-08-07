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

class I2C_Datalink
{
  public:
    I2C_Datalink( uint8_t SDA_Pin, uint8_t SCL_Pin )
                : m_SDA_PIN(SDA_Pin)
                , m_SCL_PIN(SCL_Pin){}
    virtual ~I2C_Datalink(){}
  protected:
    uint8_t m_SDA_PIN;
    uint8_t m_SCL_PIN;
    uint8_t m_Slave_Address;
    uint16_t m_MaxResponseLength;
    uint32_t m_Freq;
};

class I2C_Datalink_Master: public NamedItem
                         , public I2C_Datalink
{
  public:
    I2C_Datalink_Master( String Title, TwoWire &TwoWire, uint8_t SDA_Pin, uint8_t SCL_Pin )
                       : NamedItem(Title)
                       , I2C_Datalink(SDA_Pin, SCL_Pin)
                       , m_TwoWire(&TwoWire){}
    virtual ~I2C_Datalink_Master(){}

    //Master Functions
    void SetupMaster(uint16_t MaxResponseLength, uint32_t Freq);
    void ReadDataFromSlave(uint8_t SlaveAddress, uint32_t count);
    void WriteDataToSlave(uint8_t SlaveAddress, String Data);
  private:
    TwoWire *m_TwoWire;
};

class I2C_Datalink_Slave: public NamedItem
                        , public I2C_Datalink
                        , public TwoWireSlaveNotifier
{
  public:
    I2C_Datalink_Slave( String Title, TwoWireSlave &TwoWireSlave, uint8_t SDA_Pin, uint8_t SCL_Pin )
                      : NamedItem(Title)
                      , I2C_Datalink(SDA_Pin, SCL_Pin)
                      , m_TwoWireSlave(&TwoWireSlave){}
    virtual ~I2C_Datalink_Slave(){}
    void SetupSlave(uint8_t My_Address, uint16_t MaxResponseLength);
    void UpdateI2C();

    //Callback Functions
    void RequestEvent();
    void ReceiveEvent(int HowMany);
  private:
    TwoWireSlave *m_TwoWireSlave;
};
#endif
