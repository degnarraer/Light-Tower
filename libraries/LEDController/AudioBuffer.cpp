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

#include "AudioBuffer.h"

size_t AudioBuffer::GetFrameCapacity()
{
	size_t Capacity = 0;
	if(0 == pthread_mutex_lock(&m_Lock))
	{
		Capacity = m_CircularAudioBuffer->capacity();
		pthread_mutex_unlock(&m_Lock);
	}
	return Capacity;
}

bool AudioBuffer::ClearAudioBuffer()
{
	bool Success = false;
	if(0 == pthread_mutex_lock(&m_Lock))
	{
		m_CircularAudioBuffer->Clear();
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
		size = m_CircularAudioBuffer->size();
		pthread_mutex_unlock(&m_Lock);
	}
	return size;
}

size_t AudioBuffer::GetFreeFrameCount()
{
	return GetFrameCapacity() - GetFrameCount();
}

size_t AudioBuffer::WriteAudioFrames( Frame_t *FrameBuffer, size_t FrameCount )
{
	size_t FramesWritten = 0;
	if(0 == pthread_mutex_lock(&m_Lock))
	{
		FramesWritten = m_CircularAudioBuffer->Write(FrameBuffer, FrameCount);
		pthread_mutex_unlock(&m_Lock);
	}
	return FramesWritten;
}

bool AudioBuffer::WriteAudioFrame( Frame_t Frame )
{
	bool Success = false;
	if(0 == pthread_mutex_lock(&m_Lock))
	{
		Success = m_CircularAudioBuffer->Write(Frame);
		pthread_mutex_unlock(&m_Lock);
	}
	return Success;
}

size_t AudioBuffer::ReadAudioFrames(Frame_t *FrameBuffer, size_t FrameCount)
{
	size_t FramesRead = 0;
	if(0 == pthread_mutex_lock(&m_Lock))
	{
		FramesRead = m_CircularAudioBuffer->Read(FrameBuffer, FrameCount);
		pthread_mutex_unlock(&m_Lock);
	}
	return FramesRead;
}

bfs::optional<Frame_t> AudioBuffer::ReadAudioFrame()
{
	bfs::optional<Frame_t> FrameRead;
	if(0 == pthread_mutex_lock(&m_Lock))
	{
		FrameRead = m_CircularAudioBuffer->Read();
		pthread_mutex_unlock(&m_Lock);
	}
	return FrameRead;
}