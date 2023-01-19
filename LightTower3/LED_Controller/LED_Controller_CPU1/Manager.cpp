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
  m_BT_In.Setup();
  m_Mic_In.Setup();
  m_I2S_Out.Setup();
  m_Mic_In.SetCallback(this);
  SetInputType(InputType_Bluetooth);
  //SetInputType(InputType_Microphone);
}

void Manager::UpdateSerialData()
{
  switch(m_InputType)
  {
    case InputType_Microphone:
      ProcessSoundStateStatus(true);
    break;
    case InputType_Bluetooth:
    {  
      ProcessBluetoothConnectionStatus(true);
      ProcessSoundStateStatus(true);
    }
    break;
    default:
    break;
  }
}

void Manager::ProcessEventQueue()
{
  switch(m_InputType)
  {
    case InputType_Microphone:
      m_Mic_In.ProcessEventQueue();
      m_I2S_Out.ProcessEventQueue();
      ProcessSoundStateStatus(false);
    break;
    case InputType_Bluetooth:
    {  
      m_I2S_Out.ProcessEventQueue();
      ProcessBluetoothConnectionStatus(false);
      ProcessSoundStateStatus(false);
    }
    break;
    default:
    break;
  }

  MoveDataFromQueueToQueue( "Manager 1"
                          , m_SPIDataLinkSlave.GetQueueHandleRXForDataItem("Processed_Frame")
                          , m_StatisticalEngine.GetQueueHandleRXForDataItem("Processed_Frame")
                          , m_SPIDataLinkSlave.GetTotalByteCountForDataItem("Processed_Frame")
                          , false
                          , false );
                          
  MoveDataFromQueueToQueue( "Manager 2"
                          , m_SPIDataLinkSlave.GetQueueHandleRXForDataItem("R_BANDS")
                          , m_StatisticalEngine.GetQueueHandleRXForDataItem("R_BANDS")
                          , m_SPIDataLinkSlave.GetTotalByteCountForDataItem("R_BANDS")
                          , false
                          , false );
                          
  MoveDataFromQueueToQueue( "Manager 3"
                          , m_SPIDataLinkSlave.GetQueueHandleRXForDataItem("L_BANDS")
                          , m_StatisticalEngine.GetQueueHandleRXForDataItem("L_BANDS")
                          , m_SPIDataLinkSlave.GetTotalByteCountForDataItem("L_BANDS")
                          , false
                          , false );
                          
  MoveDataFromQueueToQueue( "Manager 4"
                          , m_SPIDataLinkSlave.GetQueueHandleRXForDataItem("R_MAXBAND")
                          , m_StatisticalEngine.GetQueueHandleRXForDataItem("R_MAXBAND")
                          , m_SPIDataLinkSlave.GetTotalByteCountForDataItem("R_MAXBAND")
                          , false
                          , false );
                          
  MoveDataFromQueueToQueue( "Manager 5"
                          , m_SPIDataLinkSlave.GetQueueHandleRXForDataItem("L_MAXBAND")
                          , m_StatisticalEngine.GetQueueHandleRXForDataItem("L_MAXBAND")
                          , m_SPIDataLinkSlave.GetTotalByteCountForDataItem("L_MAXBAND")
                          , false
                          , false );
                          
  MoveDataFromQueueToQueue( "Manager 6"
                          , m_SPIDataLinkSlave.GetQueueHandleRXForDataItem("R_MAJOR_FREQ")
                          , m_StatisticalEngine.GetQueueHandleRXForDataItem("R_MAJOR_FREQ")
                          , m_SPIDataLinkSlave.GetTotalByteCountForDataItem("R_MAJOR_FREQ")
                          , false
                          , false );
                          
  MoveDataFromQueueToQueue( "Manager 7"
                          , m_SPIDataLinkSlave.GetQueueHandleRXForDataItem("L_MAJOR_FREQ")
                          , m_StatisticalEngine.GetQueueHandleRXForDataItem("L_MAJOR_FREQ")
                          , m_SPIDataLinkSlave.GetTotalByteCountForDataItem("L_MAJOR_FREQ")
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
  switch(m_InputType)
  {
    case InputType_Microphone:
    {
      uint16_t Buffer[length];
      for(int i = 0; i < length / sizeof(uint32_t); ++i)
      {
        uint32_t Value32 = ((uint32_t*)data)[i];
        uint16_t Value16 = Value32 >> 16;
        Buffer[i] = Value16;
      }
      m_I2S_Out.WriteSoundBufferData((uint8_t *)Buffer, length); 
    }
    break;
    case InputType_Bluetooth:
    {
      m_I2S_Out.WriteSoundBufferData((uint8_t *)data, length);
    }
    break;
    default:
    break;
  }
}


void Manager::ProcessBluetoothConnectionStatus(bool ForceUpdate)
{
  bool SendUpdate = false;
  bool IsConnected = m_BT_In.IsConnected();
  if(m_BluetoothIsConnected != IsConnected)
  {
    m_BluetoothIsConnected = IsConnected;
    SendUpdate = true;
    if(true == m_BluetoothIsConnected)
    {
      ESP_LOGI("Manager", "Bluetooth Source Connected!");
    }
    else
    {
      ESP_LOGI("Manager", "Bluetooth Source Disconnected!");
    }
  }
  if(true == ForceUpdate || true == SendUpdate)
  {
    static bool SourceIsConnectedValuePushError = false;
    PushValueToQueue( &m_BluetoothIsConnected
                    , m_SPIDataLinkSlave.GetQueueHandleTXForDataItem("Source Is Connected")
                    , false
                    , "Source Is Connected"
                    , SourceIsConnectedValuePushError );
  }
}


void Manager::ProcessSoundStateStatus(bool ForceUpdate)
{
  bool SendUpdate = false;
  SoundState_t SoundState = m_StatisticalEngine.GetSoundState();
  if(m_SoundState != SoundState)
  {
    m_SoundState = SoundState;
    SendUpdate = true;
    switch(m_SoundState)
    {
      case LastingSilenceDetected:
        ESP_LOGD("Manager", "Lasting Silence Detected");
      break;
      case SilenceDetected:
        ESP_LOGD("Manager", "Silence Detected");
      break;
      case SoundDetected:
        ESP_LOGD("Manager", "Sound Detected");
      break;
      default:
      break;
    }
  }
  if(true == ForceUpdate || true == SendUpdate)
  {
    static bool SoundStateValuePushError = false;
    PushValueToQueue( &m_SoundState
                    , m_SPIDataLinkSlave.GetQueueHandleTXForDataItem("Sound State")
                    , false
                    , "Sound State"
                    , SoundStateValuePushError );
  }
}
