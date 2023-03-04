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

#include "Manager.h"

void Manager::Setup()
{
  m_SPIDataLinkSlave.SetSpewToConsole(false, false);
}
void Manager::ProcessEventQueue()
{
    MoveDataBetweenSerialAndWebPage();
}

void Manager::MoveDataBetweenSerialAndWebPage()
{
  struct Signal
  {
    String Name;
    bool A_To_B;
    bool B_To_A;
  };
  const uint8_t count = 11;
  Signal Signals[count] = { { "Sound State",              true, false }
                          , { "Amplitude Gain",           true, true }
                          , { "FFT Gain",                 true, true }
                          , { "Sink SSID",                true, true }
                          , { "Source SSID",              true, true }
                          , { "Source Connection Status", true, false }
                          , { "Sink Connection Status",   true, false }
                          , { "Sink ReConnect",           true, true }
                          , { "Sink BT Reset",            true, true }
                          , { "Source ReConnect",         true, true }
                          , { "Source BT Reset",          true, true } };
                                      
  for(int i = 0; i < count; ++i)
  {
    if(Signals[i].A_To_B)
    {
      MoveDataFromQueueToQueue( "Manager: Move Data from Datalink to Web Page: " + Signals[i].Name
                              , m_SPIDataLinkSlave.GetQueueHandleRXForDataItem(Signals[i].Name.c_str())
                              , m_SettingsWebServerManager.GetQueueHandleTXForDataItem(Signals[i].Name.c_str())
                              , m_SPIDataLinkSlave.GetTotalByteCountForDataItem(Signals[i].Name.c_str())
                              , 0
                              , false );
    }
    if(Signals[i].B_To_A)
    {
      MoveDataFromQueueToQueue( "Manager: Move Data from Web Page to Datalink: " + Signals[i].Name
                              , m_SettingsWebServerManager.GetQueueHandleRXForDataItem(Signals[i].Name.c_str())
                              , m_SPIDataLinkSlave.GetQueueHandleTXForDataItem(Signals[i].Name.c_str())
                              , m_SettingsWebServerManager.GetTotalByteCountForDataItem(Signals[i].Name.c_str())
                              , 0
                              , false );
    }
  }
}
