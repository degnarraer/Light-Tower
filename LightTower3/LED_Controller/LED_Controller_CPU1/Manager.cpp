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
                , SerialDataLink &SerialDataLink
                , Bluetooth_Sink &BT_In
                , I2S_Device &Mic_In )
                : NamedItem(Title)
                , m_StatisticalEngine(StatisticalEngine)
                , m_SerialDataLink(SerialDataLink)
                , m_BT_In(BT_In)
                , m_Mic_In(Mic_In)
{
}
Manager::~Manager()
{
}

void Manager::Setup()
{
  //Set Bluetooth Power to Max
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT, ESP_PWR_LVL_P9);
  m_AudioStreamMaster.Setup();
  m_Mic_In.Setup();
  m_BT_In.Setup();
  m_Mic_In.ResgisterForDataBufferRXCallback(this);
  m_BT_In.ResgisterForRxCallback(this);
  SetInputType(InputType_Bluetooth);
  //SetInputType(InputType_Microphone);
}

void Manager::ProcessEventQueue()
{
  switch(m_InputType)
  {
    case InputType_Microphone:
      m_Mic_In.ProcessEventQueue();
    break;
    case InputType_Bluetooth:
    break;
    default:
    break;
  }
  MoveDataFromQueueToQueue( "Manager 1"
                          , m_SerialDataLink.GetQueueHandleRXForDataItem("R_BANDS")
                          , m_StatisticalEngine.GetQueueHandleRXForDataItem("R_BANDS")
                          , m_SerialDataLink.GetTotalByteCountForDataItem("R_BANDS")
                          , false
                          , false );

  MoveDataFromQueueToQueue( "Manager 2"
                          , m_SerialDataLink.GetQueueHandleRXForDataItem("R_PSD")
                          , m_StatisticalEngine.GetQueueHandleRXForDataItem("R_PSD")
                          , m_SerialDataLink.GetTotalByteCountForDataItem("R_PSD")
                          , false
                          , false );
                          
  MoveDataFromQueueToQueue( "Manager 3"
                          , m_SerialDataLink.GetQueueHandleRXForDataItem("R_MAXBAND")
                          , m_StatisticalEngine.GetQueueHandleRXForDataItem("R_MAXBAND")
                          , m_SerialDataLink.GetTotalByteCountForDataItem("R_MAXBAND")
                          , false
                          , false );
                          
  MoveDataFromQueueToQueue( "Manager 4"
                          , m_SerialDataLink.GetQueueHandleRXForDataItem("L_BANDS")
                          , m_StatisticalEngine.GetQueueHandleRXForDataItem("L_BANDS")
                          , m_SerialDataLink.GetTotalByteCountForDataItem("L_BANDS")
                          , false
                          , false );

  MoveDataFromQueueToQueue( "Manager 5"
                          , m_SerialDataLink.GetQueueHandleRXForDataItem("L_PSD")
                          , m_StatisticalEngine.GetQueueHandleRXForDataItem("L_PSD")
                          , m_SerialDataLink.GetTotalByteCountForDataItem("L_PSD")
                          , false
                          , false );
                          
  MoveDataFromQueueToQueue( "Manager 6"
                          , m_SerialDataLink.GetQueueHandleRXForDataItem("L_MAXBAND")
                          , m_StatisticalEngine.GetQueueHandleRXForDataItem("L_MAXBAND")
                          , m_SerialDataLink.GetTotalByteCountForDataItem("L_MAXBAND")
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
    break;
    case InputType_Bluetooth:
      m_Mic_In.StopDevice();
      m_BT_In.StartDevice();
    break;
    default:
      m_Mic_In.StopDevice();
      m_BT_In.StopDevice();
    break;
  }
}
//Bluetooth_Callback
void Manager::BTDataReceived(const uint8_t *data, uint32_t length)
{
    assert(0 == length % sizeof(int32_t));
    size_t FramesToTx = length / sizeof(int32_t) / 2;
    int32_t *I2C_RXBuffer = (int32_t*)data;
    for(int i = 0; i < FramesToTx; ++i)
    {
      Frame_t aFrame;
      aFrame.channel1 = I2C_RXBuffer[2*i] >> 16;
      aFrame.channel2 = I2C_RXBuffer[2*i + 1] >> 16;
      m_AudioStreamMaster.GetTxBufferPointer()[i] = aFrame;
    }
    m_AudioStreamMaster.TxAudioFrames(FramesToTx);
}
//I2S_Device_Callback
void Manager::DataBufferModifyRX(String DeviceTitle, uint8_t* DataBuffer, size_t ByteCount, size_t SampleCount)
{

}

void Manager::RightChannelDataBufferModifyRX(String DeviceTitle, uint8_t* DataBuffer, size_t ByteCount, size_t SampleCount)
{
}

void Manager::LeftChannelDataBufferModifyRX(String DeviceTitle, uint8_t* DataBuffer, size_t ByteCount, size_t SampleCount)
{
}
