/*
    Light Tower by Rob Shockency
    Copyright (C) 2021 Rob Shockency degnarraer@yahoo.com

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version of the License, or
    (at your option) any later version. 3
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
    static const size_t m_SPIDatalinkConfigCount = 7;
    DataItemConfig_t m_ItemConfig[m_SPIDatalinkConfigCount]
    {
      { "Source Is Connected",      DataType_bool_t,        1,    Transciever_RX,     20 },
      { "Sound State",              DataType_SoundState_t,  1,    Transciever_RX,     20 },
      { "Amplitude Gain",           DataType_Float_t,       1,    Transciever_TXRX,   20 },
      { "FFT Gain",                 DataType_Float_t,       1,    Transciever_TXRX,   20 },
      { "Found Speaker SSIDS",      DataType_String_t,      1,    Transciever_TXRX,   2  },
      { "Target Speaker SSID",      DataType_String_t,      1,    Transciever_TXRX,   2  },
      { "My SSID",                  DataType_String_t,      1,    Transciever_TXRX,   2  },
    };
    DataItemConfig_t* GetDataItemConfig() { return m_ItemConfig; }
    size_t GetDataItemConfigCount() { return m_SPIDatalinkConfigCount; }
};

#endif
