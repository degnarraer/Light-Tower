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

Manager::Manager( String Title
                , StatisticalEngine &StatisticalEngine
                , SPIDataLinkSlave &SPIDataLinkSlave
                , Bluetooth_Sink &BT_In
                , I2S_Device &Mic_In
                , I2S_Device &I2S_Out )
                : NamedItem(Title)
                , m_StatisticalEngine(StatisticalEngine)
                , m_SPIDataLinkSlave(SPIDataLinkSlave)
                , m_BT_In(BT_In)
                , m_Mic_In(Mic_In) 
                , m_I2S_Out(I2S_Out)
{
}
Manager::~Manager()
{
}

void Manager::Setup()
{
  //Set Bluetooth Power to Max
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT, ESP_PWR_LVL_P9);
  m_Mic_In.SetCallback(this);
  m_BT_In.ResgisterForRxCallback(this);
  //SetInputType(InputType_Bluetooth);
  //SetInputType(InputType_Microphone);
}

void Manager::ProcessEventQueue()
{
  switch(m_InputType)
  {
    case InputType_Microphone:
      m_Mic_In.ProcessEventQueue();
      m_I2S_Out.ProcessEventQueue();
    break;
    case InputType_Bluetooth:
      m_I2S_Out.ProcessEventQueue();
    break;
    default:
    break;
  }
  MoveDataFromQueueToQueue( "Manager 1"
                          , m_SPIDataLinkSlave.GetQueueHandleRXForDataItem("R_BANDS")
                          , m_StatisticalEngine.GetQueueHandleRXForDataItem("R_BANDS")
                          , m_SPIDataLinkSlave.GetTotalByteCountForDataItem("R_BANDS")
                          , false
                          , false );

  MoveDataFromQueueToQueue( "Manager 2"
                          , m_SPIDataLinkSlave.GetQueueHandleRXForDataItem("R_PSD")
                          , m_StatisticalEngine.GetQueueHandleRXForDataItem("R_PSD")
                          , m_SPIDataLinkSlave.GetTotalByteCountForDataItem("R_PSD")
                          , false
                          , false );
                          
  MoveDataFromQueueToQueue( "Manager 3"
                          , m_SPIDataLinkSlave.GetQueueHandleRXForDataItem("R_MAXBAND")
                          , m_StatisticalEngine.GetQueueHandleRXForDataItem("R_MAXBAND")
                          , m_SPIDataLinkSlave.GetTotalByteCountForDataItem("R_MAXBAND")
                          , false
                          , false );
                          
  MoveDataFromQueueToQueue( "Manager 4"
                          , m_SPIDataLinkSlave.GetQueueHandleRXForDataItem("L_BANDS")
                          , m_StatisticalEngine.GetQueueHandleRXForDataItem("L_BANDS")
                          , m_SPIDataLinkSlave.GetTotalByteCountForDataItem("L_BANDS")
                          , false
                          , false );

  MoveDataFromQueueToQueue( "Manager 5"
                          , m_SPIDataLinkSlave.GetQueueHandleRXForDataItem("L_PSD")
                          , m_StatisticalEngine.GetQueueHandleRXForDataItem("L_PSD")
                          , m_SPIDataLinkSlave.GetTotalByteCountForDataItem("L_PSD")
                          , false
                          , false );
                          
  MoveDataFromQueueToQueue( "Manager 6"
                          , m_SPIDataLinkSlave.GetQueueHandleRXForDataItem("L_MAXBAND")
                          , m_StatisticalEngine.GetQueueHandleRXForDataItem("L_MAXBAND")
                          , m_SPIDataLinkSlave.GetTotalByteCountForDataItem("L_MAXBAND")
                          , false
                          , false );
}

void Manager::SetInputType(InputType_t Type)
{
  m_InputType = Type;
  switch(m_InputType)
  {
    case InputType_Microphone:
      m_BT_In.StopDevice();
      m_Mic_In.StartDevice();
      m_I2S_Out.StartDevice();
    break;
    case InputType_Bluetooth:
      m_BT_In.StartDevice();
      m_Mic_In.StopDevice();
      m_I2S_Out.StopDevice();
    break;
    default:
      m_BT_In.StopDevice();
      m_Mic_In.StopDevice();
      m_I2S_Out.StopDevice();
    break;
  }
}
//Bluetooth_Callback
void Manager::BTDataReceived(uint8_t *data, uint32_t length)
{
}
//I2S_Device_Callback
void Manager::I2SDataReceived(String DeviceTitle, uint8_t *data, uint32_t length)
{
  m_I2S_Out.WriteSoundBufferData((uint8_t *)data, length);
}
