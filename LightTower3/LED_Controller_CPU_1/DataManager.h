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

#ifndef DataManager_H
#define DataManager_H
#define DEBUG_DATA_MANAGER false

#include "EventSystem.h"

class DataManager: public EventSystemCaller 
{
  public:
    DataManager();
    virtual ~DataManager();

    //Microphone Data
    const String MicrophoneDataReady = "MicDataReady";
    const String MicrophoneRightDataReady = "MicRightChannelDataReady";
    const String MicrophoneLeftDataReady = "MicLeftChannelDataReady";
    void SetMicrophoneSoundBufferMemorySize(size_t BitsPerSample, size_t Samples, size_t Channels);
    size_t GetSampleCount();
    size_t GetChannels();
    size_t GetBitsPerSample();
    int32_t GetSoundBufferData(int index);
    void SetSoundBufferData(int32_t *SoundBufferData);
    int32_t GetRightChannelSoundBufferData(int index);
    void SetRightChannelSoundBufferData(int32_t *SoundBufferData);
    int32_t GetLeftChannelSoundBufferData(int index);
    void SetLeftChannelSoundBufferData(int32_t *SoundBufferData);
  private:

    //Microphone Data
    size_t m_Samples;
    size_t m_Channels;
    size_t m_BitsPerSample;
    int32_t *m_SoundBufferData;
    int32_t *m_LeftChannel_SoundBufferData;
    int32_t *m_RightChannel_SoundBufferData;
};


#endif
