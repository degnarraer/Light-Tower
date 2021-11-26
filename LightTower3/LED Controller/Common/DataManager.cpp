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

#include "DataManager.h"

DataManager::DataManager()
{
  ResisterNotificationContext(MicrophoneDataReady);
  ResisterNotificationContext(MicrophoneRightDataReady);
  ResisterNotificationContext(MicrophoneLeftDataReady);
  ResisterNotificationContext(FFTDataReady);
}

DataManager::~DataManager()
{
  delete m_SoundBufferData;
  delete m_LeftChannel_SoundBufferData;
  delete m_RightChannel_SoundBufferData;
}

void DataManager::SetMicrophoneSoundBufferMemorySize(size_t BitsPerSample, size_t Samples, size_t Channels)
{
  m_Samples = Samples;
  m_Channels = Channels;
  m_BitsPerSample = BitsPerSample;
  m_SoundBufferData = (int32_t*)malloc((m_BitsPerSample/8) * m_Samples * m_Channels);
  m_LeftChannel_SoundBufferData = (int32_t*)malloc((m_BitsPerSample/8) * m_Samples);
  m_RightChannel_SoundBufferData = (int32_t*)malloc((m_BitsPerSample/8) * m_Samples);
}

size_t DataManager::GetSampleCount()
{
  return m_Samples;
}

size_t DataManager::GetChannels()
{
  return m_Channels;
}

size_t DataManager::GetBitsPerSample()
{
  return m_BitsPerSample;
}

int32_t DataManager::GetSoundBufferData(int index)
{
  return m_SoundBufferData[index];
}

void DataManager::SetSoundBufferData(int32_t *SoundBufferData)
{
  memcpy(m_SoundBufferData, SoundBufferData, sizeof(int32_t) * m_Samples * m_Channels);
  if(true == DEBUG_DATA_MANAGER) Serial << "Data Ready\n";
  SendNotificationToCallees(MicrophoneDataReady);
}

int32_t DataManager::GetRightChannelSoundBufferData(int index)
{
  return m_RightChannel_SoundBufferData[index];
}

void DataManager::SetRightChannelSoundBufferData(int32_t *SoundBufferData)
{
  memcpy(m_RightChannel_SoundBufferData, SoundBufferData, sizeof(int32_t) * m_Samples);
  if(true == DEBUG_DATA_MANAGER) Serial << "Right Data Ready\n";
  SendNotificationToCallees(MicrophoneRightDataReady);
}

int32_t DataManager::GetLeftChannelSoundBufferData(int index)
{
  return m_LeftChannel_SoundBufferData[index];
}

void DataManager::SetLeftChannelSoundBufferData(int32_t *SoundBufferData)
{
  memcpy(m_LeftChannel_SoundBufferData, SoundBufferData, sizeof(int32_t) * m_Samples);
  if(true == DEBUG_DATA_MANAGER) Serial << "Left Data Ready\n";
  SendNotificationToCallees(MicrophoneLeftDataReady);
}
