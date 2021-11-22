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

#include "i2s_EventHandler.h"

I2S_EventHandler::I2S_EventHandler(String Title, DataManager &DataManager): m_DataManager(DataManager)
                                                                          , Task(Title, m_DataManager){}
I2S_EventHandler::~I2S_EventHandler()
{
  m_Mic->DeRegisterForEventNotification(this, MicrophoneNotificationRX);
  delete m_Mic;
}

void I2S_EventHandler::Setup()
{
  if(true == DEBUG_EVENT_HANDLER)Serial << "Setup i2s Event Handler: " << m_Title << "\n";
  m_Mic = new I2S_Device( "Microphone"
                        , m_DataManager
                        , I2S_NUM_0
                        , i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX)
                        , 44100
                        , I2S_BITS_PER_SAMPLE_32BIT
                        , I2S_CHANNEL_FMT_RIGHT_LEFT
                        , i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB)
                        , I2S_CHANNEL_STEREO
                        , 10
                        , 128
                        , 12
                        , 13
                        , 14
                        , I2S_PIN_NO_CHANGE
                        , MicrophoneNotificationRX
                        , "" );
    m_Speaker = new I2S_Device( "Speaker"
                              , m_DataManager
                              , I2S_NUM_1
                              , i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_TX)
                              , 44100
                              , I2S_BITS_PER_SAMPLE_32BIT
                              , I2S_CHANNEL_FMT_RIGHT_LEFT
                              , i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB)
                              , I2S_CHANNEL_STEREO
                              , 10
                              , 128
                              , 26
                              , 25
                              , I2S_PIN_NO_CHANGE
                              , 33
                              , ""
                              , SpeakerNotificationTX );
  AddTask(*m_Mic);
  AddTask(*m_Speaker);
  m_Mic->RegisterForEventNotification(this, MicrophoneNotificationRX);
  m_Speaker->RegisterForEventNotification(this, SpeakerNotificationTX);
  m_Mic->StartDevice();
  m_Speaker->StartDevice();
}

bool I2S_EventHandler::CanRunMyTask()
{ 
  return false;
}

void I2S_EventHandler::RunMyTask()
{
  
}

void I2S_EventHandler::EventSystemNotification(String context)
{
  if(MicrophoneNotificationRX == context)
  {
    m_DataManager.SetSoundBufferData(m_Mic->GetSoundBufferData());
    m_DataManager.SetRightChannelSoundBufferData(m_Mic->GetRightSoundBufferData());
    m_DataManager.SetLeftChannelSoundBufferData(m_Mic->GetLeftSoundBufferData());
    m_Speaker->SetSoundBufferData(m_Mic->GetSoundBufferData());
  }
}
