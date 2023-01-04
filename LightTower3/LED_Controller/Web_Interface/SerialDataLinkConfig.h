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

#ifndef SERIAL_DATALINK_CONFIG_H
#define SERIAL_DATALINK_CONFIG_H

#include "SPI_Datalink.h"
#include "Tunes.h"

class SPIDataLinkSlave: public SPI_Datalink_Slave
                      , public QueueManager
{
  public:
    SPIDataLinkSlave( )
                    : QueueManager("SPI Dtatlink Slave", GetDataItemConfigCount())
                    , SPI_Datalink_Slave("SPI Dtatlink Slave"
                    , HSPI
                    , HSPI_PIN_SCK
                    , HSPI_PIN_MISO
                    , HSPI_PIN_MOSI
                    , HSPI_PIN_SS
                    , 2 ) {}
    virtual ~SPIDataLinkSlave(){}
    void SetupSPIDataLink()
    {
      ESP_LOGE("SPI_Datalink_Config", "%s: Setting Up", GetTitle().c_str());
      SetupQueueManager();
      SetSerialDataLinkDataItems(GetQueueManagerDataItems(), GetQueueManagerDataItemCount());
      ESP_LOGE("SPI_Datalink_Config", "%s: Setup Complete", GetTitle().c_str());
    }
  private:
    
    //QueueManager Interface
    static const size_t m_SPIDatalinkConfigCount = 2;
    DataItemConfig_t m_ItemConfig[m_SPIDatalinkConfigCount]
    {
      { "Source Is Connected",      DataType_bool,          1,    Transciever_RX,   4 },
      { "Sound State",              DataType_SoundState_t,  1,    Transciever_RX,   4 },
    };
    DataItemConfig_t* GetDataItemConfig() { return m_ItemConfig; }
    size_t GetDataItemConfigCount() { return m_SPIDatalinkConfigCount; }
};

#endif
