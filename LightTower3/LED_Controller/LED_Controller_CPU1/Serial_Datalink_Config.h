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

class SPIDataLinkSlave: public SPI_Datalink_Slave
                      , public QueueManager
{
  public:
    SPIDataLinkSlave()
                    : QueueManager( "SPI Slave", GetDataItemConfigCount() )
                    , SPI_Datalink_Slave( "SPI Slave"
                                        , HSPI
                                        , HSPI_PIN_SCK
                                        , HSPI_PIN_MISO
                                        , HSPI_PIN_MOSI
                                        , HSPI_PIN_SS
                                        , 1 ) {}
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
    static const size_t m_SPIDatalinkConfigCount = 9;
    DataItemConfig_t m_ItemConfig[m_SPIDatalinkConfigCount]
    {
      { "R_BANDS",                  DataType_Float_t,                32,  Transciever_RX,   4 },
      { "L_BANDS",                  DataType_Float_t,                32,  Transciever_RX,   4 },
      { "Processed_Frame",          DataType_ProcessedSoundFrame_t,  1,   Transciever_RX,   4 },
      { "R_MAXBAND",                DataType_MaxBandSoundData_t,     1,   Transciever_RX,   4 },
      { "L_MAXBAND",                DataType_MaxBandSoundData_t,     1,   Transciever_RX,   4 },
      { "R_MAJOR_FREQ",             DataType_Float_t,                1,   Transciever_RX,   4 },
      { "L_MAJOR_FREQ",             DataType_Float_t,                1,   Transciever_RX,   4 },
      { "Source Is Connected",      DataType_bool_t,                 1,   Transciever_TX,   20 },
      { "Sound State",              DataType_SoundState_t,           1,   Transciever_TX,   20 },
    };
    DataItemConfig_t* GetDataItemConfig() { return m_ItemConfig; }
    size_t GetDataItemConfigCount() { return m_SPIDatalinkConfigCount; }
};

#endif
