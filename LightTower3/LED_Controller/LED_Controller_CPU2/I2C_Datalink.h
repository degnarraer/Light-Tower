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
#include <DataTypes.h>
#include <Helpers.h>

class I2C_Datalink: public NamedItem
                  , public CommonUtils
{
  private:
    TwoWire I2C_0 = TwoWire(0);
  public:
    I2C_Datalink( String Title  ) : NamedItem(Title){}
    virtual ~I2C_Datalink(){}
    void Setup()
    {
      I2C_0.begin(I2C_SDA_PIN, I2C_SCL_PIN, I2C_SLAVE_FREQ);
    }
    void Loop() 
    {
      WireSlaveRequest slaveReq(I2C_0, I2C_SLAVE_ADDR, MAX_SLAVE_RESPONSE_LENGTH);
      slaveReq.setRetryDelay(5);
      bool success = slaveReq.request();
      if (success) 
      {
          while (1 < slaveReq.available()) 
          {
            char c = slaveReq.read();
            Serial.print(c);
          }   
          int x = slaveReq.read();
          Serial.println(x);
      }
      else 
      {
          // if something went wrong, print the reason
          Serial.println(slaveReq.lastStatusToString());
      } 
    }
};


#endif
