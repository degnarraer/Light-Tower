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
  for(int i = 0; i < m_SignalCount; ++i)
  {
    if(m_Signals[i].A_To_B)
    {
      String Message = "Manager: Move Data from Datalink to Web Page: " + m_Signals[i].Name;
      MoveDataFromQueueToQueue( Message
                              , m_SPIDataLinkSlave.GetQueueHandleRXForDataItem(m_Signals[i].Name)
                              , m_SPIDataLinkSlave.GetTotalByteCountForDataItem(m_Signals[i].Name)
                              , m_SettingsWebServerManager.GetQueueHandleTXForDataItem(m_Signals[i].Name)
                              , m_SettingsWebServerManager.GetTotalByteCountForDataItem(m_Signals[i].Name)
                              , 0
                              , false );
    }
    if(m_Signals[i].B_To_A)
    {
      String Message = "Manager: Move Data from Web Page to Datalink: " + m_Signals[i].Name;
      MoveDataFromQueueToQueue( Message
                              , m_SettingsWebServerManager.GetQueueHandleRXForDataItem(m_Signals[i].Name)
                              , m_SettingsWebServerManager.GetTotalByteCountForDataItem(m_Signals[i].Name)
                              , m_SPIDataLinkSlave.GetQueueHandleTXForDataItem(m_Signals[i].Name)
                              , m_SPIDataLinkSlave.GetTotalByteCountForDataItem(m_Signals[i].Name)
                              , 0
                              , false );
    }
  }
}
