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

class SPIDataLinkMaster: public SPI_Datalink_Master
                       , public QueueManager
{
  public:
    SPIDataLinkMaster( String Title
                     , uint8_t SPI_BUS
                     , uint8_t SCK
                     , uint8_t MISO
                     , uint8_t MOSI
                     , uint8_t SS
                     , uint8_t DMA_Channel)
                     : QueueManager(Title, GetDataItemConfigCount())
                     , SPI_Datalink_Master(Title, SPI_BUS, SCK, MISO, MOSI, SS, DMA_Channel) {}
    virtual ~SPIDataLinkMaster(){}
    void SetupSPIDataLink()
    {
      ESP_LOGE("SPI_Datalink_Config", "%s: Setting Up", GetTitle().c_str());
      SetupQueueManager();
      SetSerialDataLinkDataItems(GetQueueManagerDataItems(), GetQueueManagerDataItemCount());
      ESP_LOGE("SPI_Datalink_Config", "%s: Setup Complete", GetTitle().c_str());
    }
    void TransmitQueuedData()
    {
      TriggerEarlyDataTransmit();
    }
    
  private:
    //QueueManager Interface
    virtual size_t GetDataItemConfigCount(){ return 0; }
    virtual DataItemConfig_t* GetDataItemConfig(){ return NULL; }
};

class SPIDataLinkToCPU1: public SPIDataLinkMaster
{
  public:
    SPIDataLinkToCPU1() : SPIDataLinkMaster("SPI Datalink to CPU 1"
                        , HSPI
                        , HSPI_PIN_SCK
                        , HSPI_PIN_MISO
                        , HSPI_PIN_MOSI
                        , HSPI_PIN_SS
                        , 1 ) {}
    virtual ~SPIDataLinkToCPU1(){}

  private:
    //QueueManager Interface
    static const size_t m_SPIDataLinkToCPU1ConfigCount = 9;
    DataItemConfig_t m_ItemConfig[m_SPIDataLinkToCPU1ConfigCount]
    {
      { "R_BANDS",              DataType_Float_t,                NUMBER_OF_BANDS,    Transciever_TX,   4 },
      { "L_BANDS",              DataType_Float_t,                NUMBER_OF_BANDS,    Transciever_TX,   4 },
      { "Processed_Frame",      DataType_ProcessedSoundFrame_t,  1,                  Transciever_TX,   4 },
      { "R_MAXBAND",            DataType_MaxBandSoundData_t,     1,                  Transciever_TX,   4 },
      { "L_MAXBAND",            DataType_MaxBandSoundData_t,     1,                  Transciever_TX,   4 },
      { "R_MAJOR_FREQ",         DataType_Float_t,                1,                  Transciever_TX,   4 },
      { "L_MAJOR_FREQ",         DataType_Float_t,                1,                  Transciever_TX,   4 },
      { "Source Is Connected",  DataType_bool_t,                 1,                  Transciever_RX,   4 },
      { "Sound State",          DataType_SoundState_t,           1,                  Transciever_RX,   4 },
    };
    
    //QueueManager Interface
    DataItemConfig_t* GetDataItemConfig() override { return m_ItemConfig; }
    size_t GetDataItemConfigCount() override { return m_SPIDataLinkToCPU1ConfigCount; }
};

class SPIDataLinkToCPU3: public SPIDataLinkMaster
{
  public:
    SPIDataLinkToCPU3() : SPIDataLinkMaster("SPI Datalink to CPU 3"
                        , VSPI
                        , VSPI_PIN_SCK
                        , VSPI_PIN_MISO
                        , VSPI_PIN_MOSI
                        , VSPI_PIN_SS
                        , 2 ) {}
    virtual ~SPIDataLinkToCPU3(){}

  private:
    //QueueManager Interface
    static const size_t m_SPIDataLinkToCPU3ConfigCount = 2;
    DataItemConfig_t m_ItemConfig[m_SPIDataLinkToCPU3ConfigCount]
    {
      { "Source Is Connected",  DataType_bool_t,        1,    Transciever_TX, 4 },
      { "Sound State",          DataType_SoundState_t,  1,    Transciever_TX, 4 },
    };
    
    //QueueManager Interface
    DataItemConfig_t* GetDataItemConfig() override { return m_ItemConfig; }
    size_t GetDataItemConfigCount() override { return m_SPIDataLinkToCPU3ConfigCount; }
};

#endif
