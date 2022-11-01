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
#include "SPI_Datalink.h"

class Manager;


class SPIDataLinkSlave: public NamedItem
                      , public SPI_Datalink_Slave
                      , public SPI_Slave_Notifier
                      , public QueueManager
{
  public:
    SPIDataLinkSlave( String Title
                    , uint8_t SCK
                    , uint8_t MISO
                    , uint8_t MOSI
                    , uint8_t SS )
                    : NamedItem(Title)
                    , SPI_Datalink_Slave(Title, SCK, MISO, MOSI, SS)
                    , QueueManager(Title, m_ConfigCount) {}
    virtual ~SPIDataLinkSlave(){}
    void SetupSPIDataLinkSlave()
    {
      Setup_SPI_Slave();
      SetupQueueManager();
      RegisterForDataTransferNotification(this);
    }
    
    //SPI_Slave_Notifier Interface
    size_t SendBytesTransferNotification(uint8_t *TXBuffer, size_t BytesToSend){}
    size_t ReceivedBytesTransferNotification(uint8_t *RXBuffer, size_t BytesReceived)
    {
      String Result = String((char*)RXBuffer);
      DeSerializeJsonToMatchingDataItem(Result);
    }
    
    //QueueManager Interface
    DataItemConfig_t* GetDataItemConfig() { return m_ItemConfig; }
    size_t GetDataItemConfigCount() { return m_ConfigCount; }
  private:
    
    static const size_t m_ConfigCount = 8;
    DataItemConfig_t m_ItemConfig[m_ConfigCount]
    {
      { "R_BANDS",      DataType_Float,                  32,  Transciever_RX,   1 },
      { "L_BANDS",      DataType_Float,                  32,  Transciever_RX,   1 },
      { "R_PSD",        DataType_ProcessedSoundData_t,   1,   Transciever_RX,   1 },
      { "L_PSD",        DataType_ProcessedSoundData_t,   1,   Transciever_RX,   1 },
      { "R_MAXBAND",    DataType_MaxBandSoundData_t,     1,   Transciever_RX,   1 },
      { "L_MAXBAND",    DataType_MaxBandSoundData_t,     1,   Transciever_RX,   1 },
      { "R_MAJOR_FREQ", DataType_Float,                  1,   Transciever_RX,   1 },
      { "L_MAJOR_FREQ", DataType_Float,                  1,   Transciever_RX,   1 },
    };
};


class SerialDataLink: public NamedItem
                    , public SerialDataLinkCore
                    , public QueueManager
{
  public:
    SerialDataLink(String Title, HardwareSerial &hSerial): NamedItem(Title)
                                                         , SerialDataLinkCore(Title, hSerial)
                                                         , QueueManager(Title, m_ConfigCount) {}
    virtual ~SerialDataLink(){}
    void SetupSerialDataLink()
    {
      SetupQueueManager();
      SetSerialDataLinkDataItems(GetQueueManagerDataItems(), GetQueueManagerDataItemCount());
    }
    
    //QueueManager Interface
    DataItemConfig_t* GetDataItemConfig() { return m_ItemConfig; }
    size_t GetDataItemConfigCount() { return m_ConfigCount; }
    
  private:
    static const size_t m_ConfigCount = 8;
    DataItemConfig_t m_ItemConfig[m_ConfigCount]
    {
      { "R_BANDS",      DataType_Float,                  32,  Transciever_RX,   1 },
      { "L_BANDS",      DataType_Float,                  32,  Transciever_RX,   1 },
      { "R_PSD",        DataType_ProcessedSoundData_t,   1,   Transciever_RX,   1 },
      { "L_PSD",        DataType_ProcessedSoundData_t,   1,   Transciever_RX,   1 },
      { "R_MAXBAND",    DataType_MaxBandSoundData_t,     1,   Transciever_RX,   1 },
      { "L_MAXBAND",    DataType_MaxBandSoundData_t,     1,   Transciever_RX,   1 },
      { "R_MAJOR_FREQ", DataType_Float,                  1,   Transciever_RX,   1 },
      { "L_MAJOR_FREQ", DataType_Float,                  1,   Transciever_RX,   1 },
    };

};


#endif
