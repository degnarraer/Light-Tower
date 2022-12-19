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
                     , QueueManager(Title, GetDataItemConfigCount())
                     , SPI_Datalink_Master(Title, SCK, MISO, MOSI, SS, DMA_Channel) {}
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
    SPIDataLinkToCPU1() : SPIDataLinkMaster("SPI Datalink 1"
                        , SPI1_PIN_SCK
                        , SPI1_PIN_MISO
                        , SPI1_PIN_MOSI
                        , SPI1_PIN_SS
                        , 1 ) {}
    virtual ~SPIDataLinkToCPU1(){}

  private:
    //QueueManager Interface
    static const size_t m_SPIDataLinkToCPU1ConfigCount = 8;
    DataItemConfig_t m_ItemConfig[m_SPIDataLinkToCPU1ConfigCount]
    {
      { "R_BANDS",          DataType_Float,                  NUMBER_OF_BANDS,    Transciever_TX,   4 },
      { "L_BANDS",          DataType_Float,                  NUMBER_OF_BANDS,    Transciever_TX,   4 },
      { "Processed_Frame",  DataType_ProcessedSoundFrame_t,  1,                  Transciever_TX,   4 },
      { "R_MAXBAND",        DataType_MaxBandSoundData_t,     1,                  Transciever_TX,   4 },
      { "L_MAXBAND",        DataType_MaxBandSoundData_t,     1,                  Transciever_TX,   4 },
      { "R_MAJOR_FREQ",     DataType_Float,                  1,                  Transciever_TX,   4 },
      { "L_MAJOR_FREQ",     DataType_Float,                  1,                  Transciever_TX,   4 },
      { "My SSID",          DataType_String,                 1,                  Transciever_TXRX, 1 },
    };
    
    //QueueManager Interface
    DataItemConfig_t* GetDataItemConfig() override { return m_ItemConfig; }
    size_t GetDataItemConfigCount() override { return m_SPIDataLinkToCPU1ConfigCount; }
};

class SPIDataLinkToCPU3: public SPIDataLinkMaster
{
  public:
    SPIDataLinkToCPU3() : SPIDataLinkMaster("SPI Datalink 1"
                        , SPI2_PIN_SCK
                        , SPI2_PIN_MISO
                        , SPI2_PIN_MOSI
                        , SPI2_PIN_SS
                        , 1 ) {}
    virtual ~SPIDataLinkToCPU3(){}

  private:
    //QueueManager Interface
    static const size_t m_SPIDataLinkToCPU3ConfigCount = 5;
    DataItemConfig_t m_ItemConfig[m_SPIDataLinkToCPU3ConfigCount]
    {
      { "My SSID",                  DataType_String,  1,    Transciever_TXRX,   1 },
      { "Speaker SSID",             DataType_String,  1,    Transciever_TXRX,   1 },
      { "Available Speakers",       DataType_String,  10,   Transciever_TXRX,   1 },
      { "Input Bluetooth Reset",    DataType_bool,    1,    Transciever_TXRX,   1 },
      { "Output Bluetooth Reset",   DataType_bool,    1,    Transciever_TXRX,   1 },
    };
    
    //QueueManager Interface
    DataItemConfig_t* GetDataItemConfig() override { return m_ItemConfig; }
    size_t GetDataItemConfigCount() override { return m_SPIDataLinkToCPU3ConfigCount; }
};

#endif
