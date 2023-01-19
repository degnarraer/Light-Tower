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
                , Sound_Processor &SoundProcessor
                , SPIDataLinkToCPU1 &SPIDataLinkToCPU1
                , SPIDataLinkToCPU3 &SPIDataLinkToCPU3
                , Bluetooth_Source &BT_Out
                , I2S_Device &I2S_In
                , ContinuousAudioBuffer<AUDIO_BUFFER_SIZE> &AudioBuffer )
                : NamedItem(Title)
                , m_SoundProcessor(SoundProcessor)
                , m_SPIDataLinkToCPU1(SPIDataLinkToCPU1)
                , m_SPIDataLinkToCPU3(SPIDataLinkToCPU3)
                , m_BT_Out(BT_Out)
                , m_I2S_In(I2S_In)
                , m_AudioBuffer(AudioBuffer)
{
}
Manager::~Manager()
{
}

void Manager::Setup()
{
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT, ESP_PWR_LVL_P9); //Set Bluetooth Power to Max
  m_SoundProcessor.SetupSoundProcessor();
  m_AudioBuffer.Initialize();
  m_I2S_In.StartDevice();
  m_BT_Out.StartDevice();
}

void Manager::ProcessEventQueue()
{
  m_I2S_In.ProcessEventQueue();
  MoveDataFromCPU3ToUs();
  MoveDataBetweenCPU1AndCPU3();
}

void Manager::MoveDataBetweenCPU1AndCPU3()
{
  const uint8_t count = 2;
  String Signals[count] = { "Source Is Connected"
                          , "Sound State" };
  
                      
  for(int i = 0; i < count; ++i)
  {
    MoveDataFromQueueToQueue( "Manager1: " + Signals[i]
                            , m_SPIDataLinkToCPU1.GetQueueHandleRXForDataItem(Signals[i].c_str())
                            , m_SPIDataLinkToCPU3.GetQueueHandleTXForDataItem(Signals[i].c_str())
                            , m_SPIDataLinkToCPU1.GetTotalByteCountForDataItem(Signals[i].c_str())
                            , false
                            , false );
  }
  m_SPIDataLinkToCPU3.TriggerEarlyDataTransmit();
}


void Manager::MoveDataFromCPU3ToUs()
{
  //Set Amplitude Gain from Amplitude Gain RX QUEUE
  float Amplitude_Gain = 1.0;
  if(true == m_SPIDataLinkToCPU3.GetValueFromRXQueue(&Amplitude_Gain, "Amplitude Gain", sizeof(Amplitude_Gain), true, false))
  {
    m_SoundProcessor.SetGain(Amplitude_Gain);
  }
  
  //Set FFT Gain from FFT Gain RX QUEUE
  float FFT_Gain = 1.0;
  if(true == m_SPIDataLinkToCPU3.GetValueFromRXQueue(&FFT_Gain, "FFT Gain", sizeof(FFT_Gain), true, false))
  {
    m_SoundProcessor.SetFFTGain(FFT_Gain);
  }
}

//I2S_Device_Callback
void Manager::I2SDataReceived(String DeviceTitle, uint8_t *Data, uint32_t channel_len)
{
}

//Bluetooth Source Callback
int32_t Manager::SetBTTxData(uint8_t *Data, int32_t channel_len)
{
  size_t ByteReceived = m_I2S_In.ReadSoundBufferData(Data, channel_len);
  assert(0 == ByteReceived % sizeof(uint32_t)); 
  size_t FrameCount = ByteReceived / sizeof(uint32_t);
  m_AudioBuffer.Push((Frame_t*)Data, FrameCount);
  return ByteReceived;
}
