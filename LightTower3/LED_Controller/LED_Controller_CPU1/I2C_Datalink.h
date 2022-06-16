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
#include <WireSlave.h>
#include <DataTypes.h>
#include <Helpers.h>

class I2C_Datalink: public NamedItem
                  , public CommonUtils
{
  private:
  public:
    I2C_Datalink( String Title  ) : NamedItem(Title){}
    virtual ~I2C_Datalink(){}
    void Setup()
    {
      if (true == WireSlave.begin(I2C_SDA_PIN, I2C_SCL_PIN, I2C_SLAVE_ADDR)) 
      {
        WireSlave.onRequest(RequestEvent);
        Serial.println("I2C slave init success");
      }
      else
      {
        Serial.println("I2C slave init failed");
      }
    }
    void Loop() 
    {
      WireSlave.update();
    }
    static void RequestEvent()
    {
      static int y = 0;
      String result = "The value for y is " + String(++y) + ".\n";
      WireSlave.print(result);
      Serial.print(result);
    }
};


#endif
