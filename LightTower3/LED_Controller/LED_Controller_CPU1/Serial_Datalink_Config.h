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

class SPIDataLinkSlave: public NamedItem
                      , public SPI_Datalink_Slave
                      , public SPI_Receive_Notifier
                      , public QueueManager
{
  public:
    SPIDataLinkSlave( String Title
                    , uint8_t SCK
                    , uint8_t MISO
                    , uint8_t MOSI
                    , uint8_t SS
                    , uint8_t DMA_Channel )
                    : NamedItem(Title) 
                    , QueueManager(Title, GetDataItemConfigCount())
                    , SPI_Datalink_Slave(Title, SCK, MISO, MOSI, SS, DMA_Channel) {}
    virtual ~SPIDataLinkSlave(){}
    void SetupSPIDataLink()
    {
      ESP_LOGE("SPI_Datalink_Config", "%s: Setting Up", GetTitle().c_str());
      SetupQueueManager();
      SetSerialDataLinkDataItems(GetQueueManagerDataItems(), GetQueueManagerDataItemCount());
      RegisterForReceivedDataTransferNotification(this);
      ESP_LOGE("SPI_Datalink_Config", "%s: Setup Complete", GetTitle().c_str());
    }
     
    //SPI_Receive_Notifier Interface
    size_t ReceivedBytesTransferNotification(uint8_t *RXBuffer, size_t BytesReceived)
    {
      String ResultString;
      if(NULL != RXBuffer && BytesReceived > 0)
      {
        for (int i = 0; i < BytesReceived; i++) 
        { 
          ResultString += ((char*)RXBuffer)[i];
        } 
        ESP_LOGV("SPI_Datalink_Config", "Received: %s", ResultString.c_str());
        DeSerializeJsonToMatchingDataItem(ResultString.c_str()); 
      }
      return ResultString.length();
    }
  private:
    
    //QueueManager Interface
    static const size_t m_SPIDatalinkConfigCount = 10;
    DataItemConfig_t m_ItemConfig[m_SPIDatalinkConfigCount]
    {
      { "R_BANDS",                  DataType_Float,                  32,  Transciever_RX,   4 },
      { "L_BANDS",                  DataType_Float,                  32,  Transciever_RX,   4 },
      { "Processed_Frame",          DataType_ProcessedSoundFrame_t,  1,   Transciever_RX,   4 },
      { "R_MAXBAND",                DataType_MaxBandSoundData_t,     1,   Transciever_RX,   4 },
      { "L_MAXBAND",                DataType_MaxBandSoundData_t,     1,   Transciever_RX,   4 },
      { "R_MAJOR_FREQ",             DataType_Float,                  1,   Transciever_RX,   4 },
      { "L_MAJOR_FREQ",             DataType_Float,                  1,   Transciever_RX,   4 },
      { "Source Is Connected",      DataType_bool,                   1,   Transciever_TX,   1 },
    };
    DataItemConfig_t* GetDataItemConfig() { return m_ItemConfig; }
    size_t GetDataItemConfigCount() { return m_SPIDatalinkConfigCount; }
};

#endif
