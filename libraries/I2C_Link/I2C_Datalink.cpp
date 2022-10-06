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
#include "I2C_Datalink.h"
#include "pthread.h"

void I2C_Datalink_Master::SetupMaster( uint32_t Freq, uint8_t RequestAttempts, uint8_t RequestTimeout )
{
  m_Freq = Freq;
  m_RequestAttempts = RequestAttempts;
  m_RequestTimeout = RequestTimeout;
  if(true == m_TwoWire->begin(m_SDA_PIN, m_SCL_PIN))
  {
    ESP_LOGI("I2C_Datalink", "I2C Master Device Named \"%s\" joined I2C bus", GetTitle().c_str());    
  }
  else
  {
    ESP_LOGE("I2C_Datalink", "I2C Master Device Named \"%s\" Setup Failed", GetTitle().c_str());
  }
}

size_t I2C_Datalink_Master::ReadDataFromSlave(uint8_t SlaveAddress, unsigned char *data, size_t ByteCountToRead)
{
	WireSlaveRequest slaveReq(*m_TwoWire, SlaveAddress, I2C_MAX_BYTES);
	slaveReq.setRetryDelay(m_RequestTimeout);
	slaveReq.setAttempts(m_RequestAttempts);
	size_t ByteCountRead = 0;
	if (true == slaveReq.request()) 
	{
		while( 0 < slaveReq.available() && ByteCountRead < ByteCountToRead ) 
		{
			data[ByteCountRead] = slaveReq.read();
			++ByteCountRead;
		}
	}
	else 
	{
		ESP_LOGI("I2C_Datalink", "I2C Master Device Named \"%s\" Read Data Request Error: %s", GetTitle().c_str(), slaveReq.lastStatusToString().c_str());
	}
	return ByteCountRead;
}

void I2C_Datalink_Master::WriteDataToSlave(uint8_t SlaveAddress, unsigned char *data, size_t ByteCountToWrite)
{
  WirePacker packer;
  for(int i = 0; i < ByteCountToWrite; ++i)
  {
	packer.write(data[i]);
  }
  packer.end();
  m_TwoWire->beginTransmission(SlaveAddress);
  while (packer.available())
  {
    m_TwoWire->write(packer.read());
  }
  m_TwoWire->endTransmission(true);
}

void I2C_Datalink_Slave::SetupSlave( uint8_t My_Address, TwoWireSlaveNotifiee *TwoWireSlaveNotifiee )
{
  m_I2C_Address = My_Address;
  m_TwoWireSlave->RegisterForNotification(TwoWireSlaveNotifiee);
  ESP_LOGI("I2C_Datalink", "I2C Slave Device Named \"%s\" Registered for Notifications", GetTitle().c_str());
  if(true == m_TwoWireSlave->begin(m_SDA_PIN, m_SCL_PIN, m_I2C_Address))
  {
    ESP_LOGI("I2C_Datalink", "I2C Slave Device Named \"%s\" joined I2C bus with addr #%d", GetTitle().c_str(), m_I2C_Address);
  }
  else
  {
    ESP_LOGE("I2C_Datalink", "I2C Slave Device Named \"%s\" Setup Failed!", GetTitle().c_str());
  }
}

void I2C_Datalink_Slave::UpdateI2C()
{
	if(NULL != m_TwoWireSlave)
	{
		m_TwoWireSlave->update();
	}
}

size_t AudioBuffer::GetFrameCapacity()
{
	size_t Capacity = 0;
	if(0 == pthread_mutex_lock(&m_Lock))
	{
		Capacity = m_CircularAudioBuffer.capacity();
		pthread_mutex_unlock(&m_Lock);
	}
	return Capacity;
}

bool AudioBuffer::ClearAudioBuffer()
{
	bool Success = false;
	if(0 == pthread_mutex_lock(&m_Lock))
	{
		m_CircularAudioBuffer.Clear();
		Success = true;
		pthread_mutex_unlock(&m_Lock);
	}
	return Success;
}

size_t AudioBuffer::GetFrameCount()
{
	size_t size = 0;
	if(0 == pthread_mutex_lock(&m_Lock))
	{
		size = m_CircularAudioBuffer.size();
		pthread_mutex_unlock(&m_Lock);
	}
	return size;
}

size_t AudioBuffer::GetAvailableCount()
{
	return GetFrameCapacity() - GetFrameCount();
}

size_t AudioBuffer::WriteAudioFrames( Frame_t *FrameBuffer, size_t FrameCount )
{
	size_t FramesWritten = 0;
	if(0 == pthread_mutex_lock(&m_Lock))
	{
		FramesWritten = m_CircularAudioBuffer.Write(FrameBuffer, FrameCount);
		pthread_mutex_unlock(&m_Lock);
	}
	return FramesWritten;
}

bool AudioBuffer::WriteAudioFrame( Frame_t &FrameBuffer )
{
	bool Success;
	if(0 == pthread_mutex_lock(&m_Lock))
	{
		Success = m_CircularAudioBuffer.Write(FrameBuffer);
		pthread_mutex_unlock(&m_Lock);
	}
	return Success;
}

size_t AudioBuffer::ReadAudioFrames(Frame_t *FrameBuffer, size_t FrameCount)
{
	size_t FramesRead = 0;
	if(0 == pthread_mutex_lock(&m_Lock))
	{
		FramesRead = m_CircularAudioBuffer.Read(FrameBuffer, FrameCount);
		pthread_mutex_unlock(&m_Lock);
	}
	return FramesRead;
}

bfs::optional<Frame_t> AudioBuffer::ReadAudioFrame()
{
	bfs::optional<Frame_t> FrameRead;
	if(0 == pthread_mutex_lock(&m_Lock))
	{
		FrameRead = m_CircularAudioBuffer.Read();
		pthread_mutex_unlock(&m_Lock);
	}
	return FrameRead;
}

void AudioStreamRequester::SetupAudioStreamRequester()
{
	SetupMaster(m_Freq, m_RequestAttempts, m_RequestTimeout);
}

size_t AudioStreamRequester::BufferMoreAudio(uint8_t SourceAddress)
{
	size_t FailCount = 0;
	size_t TotalBytesRead = 0;
	size_t TotalFramesRead = 0;
	size_t TotalFramesToRead = m_AudioBuffer.GetAvailableCount();
	size_t TotalBytesToRead = TotalFramesToRead * sizeof(Frame_t);
	
	if(0 < TotalFramesToRead)
	{
		while(0 < TotalFramesToRead - TotalFramesRead && FRAMES_PER_PACKET <= m_AudioBuffer.GetAvailableCount())
		{
			Frame_t ReceivedFrames[FRAMES_PER_PACKET];
			size_t BytesRead = ReadDataFromSlave(SourceAddress, &((unsigned char &)ReceivedFrames), sizeof(Frame_t));
			if(0 != BytesRead)
			{
				assert(0 == BytesRead % sizeof(Frame_t));
				TotalBytesRead += BytesRead;
				TotalFramesRead += BytesRead / sizeof(Frame_t);
				if(false == m_AudioBuffer.WriteAudioFrames(ReceivedFrames, TotalFramesRead))
				{
					ESP_LOGE("AudioBuffer", "Buffer Overrun");	
				}
			}
			else
			{
				++FailCount;
				if(FailCount >= 10) 
				{
					ESP_LOGE("AudioBuffer", "Buffer Update Fail");	
					break;
				}
			}
		}
	}
	ESP_LOGE("AudioBuffer", "Buffer Filled %i Frames with %i Frames", TotalFramesToRead, TotalFramesRead);
	assert(TotalFramesRead == TotalBytesRead / sizeof(Frame_t));
	return TotalFramesRead;
}

size_t AudioStreamRequester::GetFrameCount()
{
	return m_AudioBuffer.GetFrameCount();
}

size_t AudioStreamRequester::GetAudioFrames(Frame_t *FrameBuffer, size_t FrameCount)
{
	return m_AudioBuffer.ReadAudioFrames(FrameBuffer, FrameCount);
}


void AudioStreamSender::SetupAudioStreamSender()
{
	SetupSlave(m_I2C_Address, this);
}
void AudioStreamSender::UpdateStreamSender()
{
	UpdateI2C();
}
//TwoWireSlaveNotifiee Callbacks
void AudioStreamSender::ReceiveEvent(int howMany)
{
	ESP_LOGW("AudioBuffer", "ReceiveEvent");
}

void AudioStreamSender::RequestEvent()
{
	if(0 < m_AudioBuffer.GetFrameCount())
	{
		for(int i=0; i<FRAMES_PER_PACKET; ++i)
		{
			bfs::optional<Frame_t> ReadFrame = m_AudioBuffer.ReadAudioFrame();
			if(ReadFrame)
			{
				for(int i = 0; i < sizeof(Frame_t); ++i)
				{
					unsigned char *data = (unsigned char*)(&ReadFrame);
					m_TwoWireSlave->write(data[i]);
				}
			}
		}
	}
}