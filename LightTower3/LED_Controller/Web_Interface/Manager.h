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
#ifndef MANAGER_H
#define MANAGER_H


#include <DataTypes.h>
#include <Helpers.h>
#include "SerialDatalinkConfig.h"
#include "WebServer.h"


class Manager: public NamedItem
             , public CommonUtils
             , public QueueController
{
  public:
    Manager( String Title
           , SPIDataLinkSlave &SPIDataLinkSlave
           , SettingsWebServerManager &SettingsWebServerManager)
           : NamedItem(Title)
           , m_SPIDataLinkSlave(SPIDataLinkSlave)
           , m_SettingsWebServerManager(SettingsWebServerManager)
           {
            
           }
    virtual ~Manager(){}

    void Setup(){}
    void ProcessEventQueue()
    {
        //Sound State Data Movement
        MoveDataFromQueueToQueue( "Manager: Sound State From Datalink To Web Page"
                                , m_SPIDataLinkSlave.GetQueueHandleRXForDataItem("Sound State")
                                , m_SettingsWebServerManager.GetQueueHandleTXForDataItem("Sound State")
                                , m_SPIDataLinkSlave.GetTotalByteCountForDataItem("Sound State")
                                , false
                                , false );
        
        //Amplitude Gain Data Movement                      
        MoveDataFromQueueToQueue( "Manager: Amplitude Gain From Datalink To Web Page"
                                , m_SPIDataLinkSlave.GetQueueHandleRXForDataItem("Amplitude Gain")
                                , m_SettingsWebServerManager.GetQueueHandleTXForDataItem("Amplitude Gain")
                                , m_SPIDataLinkSlave.GetTotalByteCountForDataItem("Amplitude Gain")
                                , false
                                , false );
        MoveDataFromQueueToQueue( "Manager: Amplitude Gain from Web Page To Datalink"
                                , m_SettingsWebServerManager.GetQueueHandleRXForDataItem("Amplitude Gain")
                                , m_SPIDataLinkSlave.GetQueueHandleTXForDataItem("Amplitude Gain")
                                , m_SettingsWebServerManager.GetTotalByteCountForDataItem("Amplitude Gain")
                                , false
                                , true );
        
        //FFT Gain Data Movement   
        MoveDataFromQueueToQueue( "Manager: FFT Gain From Datalink To Web Page"
                                , m_SPIDataLinkSlave.GetQueueHandleRXForDataItem("FFT Gain")
                                , m_SettingsWebServerManager.GetQueueHandleTXForDataItem("FFT Gain")
                                , m_SPIDataLinkSlave.GetTotalByteCountForDataItem("FFT Gain")
                                , false
                                , false );
        MoveDataFromQueueToQueue( "Manager: FFT Gain from Web Page To Datalink"
                                , m_SettingsWebServerManager.GetQueueHandleRXForDataItem("FFT Gain")
                                , m_SPIDataLinkSlave.GetQueueHandleTXForDataItem("FFT Gain")
                                , m_SettingsWebServerManager.GetTotalByteCountForDataItem("FFT Gain")
                                , false
                                , false );
    }

  private:
    SPIDataLinkSlave &m_SPIDataLinkSlave;
    SettingsWebServerManager &m_SettingsWebServerManager;
};

#endif
