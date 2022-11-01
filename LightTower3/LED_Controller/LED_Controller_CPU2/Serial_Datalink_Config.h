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
#include <Serial_Datalink_Core.h>
#include "SPI_Datalink.h"

class SPIDataLinkMaster: public NamedItem
                       , public SPI_Datalink_Master
                       , public QueueManager
{
  public:
    SPIDataLinkMaster( String Title
                     , uint8_t SCK
                     , uint8_t MISO
                     , uint8_t MOSI
                     , uint8_t SS
                     , uint8_t DMA_Channel)
                     : NamedItem(Title) 
                     , QueueManager(Title, m_SPIDatalinkConfigCount)
                     , SPI_Datalink_Master(Title, SCK, MISO, MOSI, SS, DMA_Channel) {}
    virtual ~SPIDataLinkMaster(){}
    void SetupSPIDataLink()
    {
      ESP_LOGE("SPI_Datalink_Config", "%s: Setting Up", GetTitle().c_str());
      SetupQueueManager();
      SetSerialDataLinkDataItems(GetQueueManagerDataItems(), GetQueueManagerDataItemCount());
      Setup_SPI_Master();
      ESP_LOGE("SPI_Datalink_Config", "%s: Setup Complete", GetTitle().c_str());
    }
  private:
    
    //QueueManager Interface
    static const size_t m_SPIDatalinkConfigCount = 8;
    DataItemConfig_t m_ItemConfig[m_SPIDatalinkConfigCount]
    {
      { "R_BANDS",      DataType_Float,                  NUMBER_OF_BANDS,    Transciever_TX,   1 },
      { "L_BANDS",      DataType_Float,                  NUMBER_OF_BANDS,    Transciever_TX,   1 },
      { "R_PSD",        DataType_ProcessedSoundData_t,   1,                  Transciever_TX,   1 },
      { "L_PSD",        DataType_ProcessedSoundData_t,   1,                  Transciever_TX,   1 },
      { "R_MAXBAND",    DataType_MaxBandSoundData_t,     1,                  Transciever_TX,   1 },
      { "L_MAXBAND",    DataType_MaxBandSoundData_t,     1,                  Transciever_TX,   1 },
      { "R_MAJOR_FREQ", DataType_Float,                  1,                  Transciever_TX,   1 },
      { "L_MAJOR_FREQ", DataType_Float,                  1,                  Transciever_TX,   1 },
    };
    DataItemConfig_t* GetDataItemConfig() { return m_ItemConfig; }
    size_t GetDataItemConfigCount() { return m_SPIDatalinkConfigCount; }
};

#endif
