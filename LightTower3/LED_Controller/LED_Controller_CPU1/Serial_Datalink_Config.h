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

#ifndef SerialDataLinkConfig_H
#define SerialDataLinkConfig_H

#include <Serial_Datalink_Core.h>
#include <DataTypes.h>


class Manager;
class SerialDataLink: public NamedItem
                    , public SerialDataLinkCore
{
  public:
    SerialDataLink(String Title): NamedItem(Title)
                                , SerialDataLinkCore(Title) {}
    virtual ~SerialDataLink(){}
    DataItemConfig_t* GetConfig() { return ItemConfig; }
    size_t GetConfigCount() { return m_ConfigCount; }
  private:
    
    static const size_t m_ConfigCount = 10;
    DataItemConfig_t ItemConfig[m_ConfigCount]
    {
      { "FFT_LBand_Data",   DataType_Int16_t,   32,   Transciever_TX },
      { "FFT_RBand_Data",   DataType_Int16_t,   32,   Transciever_TX },
      { "R_Power",          DataType_Float,     1,    Transciever_TX },
      { "L_Power",          DataType_Float,     1,    Transciever_TX },
      { "R_DB",             DataType_Float,     1,    Transciever_TX },
      { "L_DB",             DataType_Float,     1,    Transciever_TX },
      { "R_Min",            DataType_Int16_t,   1,    Transciever_TX },
      { "L_Min",            DataType_Int16_t,   1,    Transciever_TX },
      { "L_Max",            DataType_Int16_t,   1,    Transciever_TX },
      { "L_Max",            DataType_Int16_t,   1,    Transciever_TX },
    };

};


#endif
