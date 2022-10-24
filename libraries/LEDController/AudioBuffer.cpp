
#include "AudioBuffer.h"

void AudioBuffer::Initialize()
{ 
  AllocateMemory();   
  if(0 != pthread_mutex_init(&m_Lock, NULL))
  {
     ESP_LOGE("TestClass", "Failed to Create Lock");
  }
}

void AudioBuffer::AllocateMemory()
{
 size_t CircleBuffSize = sizeof(CircularBuffer<Frame_t, AUDIO_BUFFER_FRAME_LENGTH>);
  void *CircularBuffer_Raw = (CircularBuffer<Frame_t, AUDIO_BUFFER_FRAME_LENGTH>*)heap_caps_malloc(CircleBuffSize, MALLOC_CAP_SPIRAM);
  m_CircularAudioBuffer = new(CircularBuffer_Raw) CircularBuffer<Frame_t, AUDIO_BUFFER_FRAME_LENGTH>;
}

void AudioBuffer::FreeMemory()
{
 heap_caps_free(m_CircularAudioBuffer);
}

bool AudioBuffer::Push(Frame_t Frame)
{
	bool Result = false;
	if(0 == pthread_mutex_lock(&m_Lock))
	{
		Result = m_CircularAudioBuffer->push(Frame);
		pthread_mutex_unlock(&m_Lock);
	}
	return Result;
}

Frame_t AudioBuffer::Pop()
{
	Frame_t Result;
	if(0 == pthread_mutex_lock(&m_Lock))
	{
		Result = m_CircularAudioBuffer->pop();
		pthread_mutex_unlock(&m_Lock);
	}
	return Result;
}

bool AudioBuffer::Unshift(Frame_t Frame)
{
	bool Result = false;
	if(0 == pthread_mutex_lock(&m_Lock))
	{
		Result = m_CircularAudioBuffer->unshift(Frame);
		pthread_mutex_unlock(&m_Lock);
	}
	return Result;
}

Frame_t AudioBuffer::Shift()
{
	Frame_t Result;
	if(0 == pthread_mutex_lock(&m_Lock))
	{
		Result = m_CircularAudioBuffer->shift();
		pthread_mutex_unlock(&m_Lock);
	}
	return Result;
}

bool AudioBuffer::IsEmpty()
{
	bool Result = false;
	if(0 == pthread_mutex_lock(&m_Lock))
	{
		Result = m_CircularAudioBuffer->isEmpty();
		pthread_mutex_unlock(&m_Lock);
	}
	return Result;
}

bool AudioBuffer::IsFull()
{
	bool Result = false;
	if(0 == pthread_mutex_lock(&m_Lock))
	{
		Result = m_CircularAudioBuffer->isFull();
		pthread_mutex_unlock(&m_Lock);
	}
	return Result;
}

size_t AudioBuffer::Size()
{
	size_t Result = 0;
	if(0 == pthread_mutex_lock(&m_Lock))
	{
		Result = m_CircularAudioBuffer->size();
		pthread_mutex_unlock(&m_Lock);
	}
	return Result;
}

size_t AudioBuffer::Available()
{
	size_t Result = 0;
	if(0 == pthread_mutex_lock(&m_Lock))
	{
		Result = m_CircularAudioBuffer->available();
		pthread_mutex_unlock(&m_Lock);
	}
	return Result;
}

void AudioBuffer::Clear()
{
	if(0 == pthread_mutex_lock(&m_Lock))
	{
		m_CircularAudioBuffer->clear();
		pthread_mutex_unlock(&m_Lock);
	}
}