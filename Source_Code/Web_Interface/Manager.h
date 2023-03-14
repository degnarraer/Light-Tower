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

    void Setup();
    void ProcessEventQueue();
    void MoveDataBetweenSerialAndWebPage();
    struct Signal
    {
      String Name;
      bool A_To_B;
      bool B_To_A;
    };

  private:
    SPIDataLinkSlave &m_SPIDataLinkSlave;
    SettingsWebServerManager &m_SettingsWebServerManager;
    
    static const uint8_t m_SignalCount = 12;
    Signal m_Signals[m_SignalCount] = { { "Sound State",              true, false }
                                      , { "Source SSID",              true, true }
                                      , { "Source Connection Status", true, false }
                                      , { "Source BT Reset",          true, true }
                                      , { "Source ReConnect",         true, true }
                                      , { "Sink SSID",                true, true }
                                      , { "Sink Enable",              true, true }
                                      , { "Sink Connection Status",   true, false }
                                      , { "Sink BT Reset",            true, true }
                                      , { "Sink ReConnect",           true, true }
                                      , { "Amplitude Gain",           true, true }
                                      , { "FFT Gain",                 true, true } };
};

#endif
