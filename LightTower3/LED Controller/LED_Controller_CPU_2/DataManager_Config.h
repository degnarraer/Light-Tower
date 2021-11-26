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

#ifndef DataManagerConfig_H
#define DataManagerConfig_H 

#define I2S_BUFFER_SIZE 50
#define I2S_BUFFER_COUNT 10
#define FFT_LENGTH 2048

#include "DataManagerTypes.h"

class DataManagerConfig
{
  public:
    DataManagerConfig(){}
    virtual ~DataManagerConfig(){}
    
    int GetCount()
    {
      return sizeof(DataItems) / sizeof(DataItems[0]);
    }
    DataItemConfig_t* GetDataManagerConfig()
    {
      return DataItems;
    }
  private:
    DataItemConfig_t DataItems[4]
    {
      {"Sound_Buffer_Data",                   DataType_Int32_t,     I2S_BUFFER_SIZE * 2 },
      {"Right_Channel_Sound_Buffer_Data",     DataType_Int32_t,     I2S_BUFFER_SIZE     },
      {"Left_Channel_Sound_Buffer_Data",      DataType_Int32_t,     I2S_BUFFER_SIZE     },
      {"FFT_Bin_Data",                        DataType_Int16_t,     FFT_LENGTH          }
    };
};

#endif
