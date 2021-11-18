
#ifndef DataManager_H
#define DataManager_H

#include "EventSystem.h"

class DataManager: public EventSystemCaller 
{
  public:
    DataManager()
    {
      ResisterNotificationContext(MicrophoneDataReady);
      ResisterNotificationContext(MicrophoneRightDataReady);
      ResisterNotificationContext(MicrophoneLeftDataReady);
    }
    virtual ~DataManager()
    {
      delete m_SoundBufferData;
      delete m_LeftChannel_SoundBufferData;
      delete m_RightChannel_SoundBufferData;
    }

    //Microphone Data
    const String MicrophoneDataReady = "MicDataReady";
    const String MicrophoneRightDataReady = "MicRightChannelDataReady";
    const String MicrophoneLeftDataReady = "MicLeftChannelDataReady";
    void SetSoundBufferMemorySize(size_t BitsPerSample, size_t Samples, size_t Channels)
    {
      m_Samples = Samples;
      m_Channels = Channels;
      m_BitsPerSample = BitsPerSample;
      m_SoundBufferData = (int32_t*)malloc((m_BitsPerSample/8) * m_Samples * m_Channels);
      m_LeftChannel_SoundBufferData = (int32_t*)malloc((m_BitsPerSample/8) * m_Samples);
      m_RightChannel_SoundBufferData = (int32_t*)malloc((m_BitsPerSample/8) * m_Samples);
    }
    size_t GetSamples(){return m_Samples;}
    size_t GetChannels(){return m_Channels;}
    size_t GetBitsPerSample(){return m_BitsPerSample;}
    int32_t GetSoundBufferData(int index){return m_SoundBufferData[index];}
    void SetSoundBufferData(int32_t *SoundBufferData)
    {
      memcpy(m_SoundBufferData, SoundBufferData, sizeof(int32_t) * m_Samples * m_Channels);
      SendNotificationToCallees(MicrophoneDataReady);
    }
    int32_t GetRightChannelSoundBufferData(int index){return m_RightChannel_SoundBufferData[index];}
    void SetRightChannelSoundBufferData(int32_t *SoundBufferData)
    {
      memcpy(m_RightChannel_SoundBufferData, SoundBufferData, sizeof(int32_t) * m_Samples);
      SendNotificationToCallees(MicrophoneRightDataReady);
    }
    int32_t GetLeftChannelSoundBufferData(int index){return m_LeftChannel_SoundBufferData[index];}
    void SetLeftChannelSoundBufferData(int32_t *SoundBufferData)
    {
      memcpy(m_LeftChannel_SoundBufferData, SoundBufferData, sizeof(int32_t) * m_Samples);
      SendNotificationToCallees(MicrophoneLeftDataReady);
    }
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
