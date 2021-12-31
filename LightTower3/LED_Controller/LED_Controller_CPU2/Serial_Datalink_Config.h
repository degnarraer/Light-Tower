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
#include "Manager.h"


class Manager;
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
      Serial << GetTitle() << ": Setup\n";
      SetupQueueManager();
      SetSerialDataLinkDataItems(GetQueueManagerDataItems(), GetQueueManagerDataItemCount());
    }
    
    //QueueManager Interface
    DataItemConfig_t* GetDataItemConfig() { return m_ItemConfig; }
    size_t GetDataItemConfigCount() { return m_ConfigCount; }
  private:
    
    static const size_t m_ConfigCount = 4;
    DataItemConfig_t m_ItemConfig[m_ConfigCount]
    {
      { "R_FFT",     DataType_Int16_t,                32,   Transciever_RX },
      { "L_FFT",     DataType_Int16_t,                32,   Transciever_RX },
      { "R_PSD",     DataType_ProcessedSoundData_t,   1,    Transciever_RX },
      { "L_PSD",     DataType_ProcessedSoundData_t,   1,    Transciever_RX }
    };
};


#endif
