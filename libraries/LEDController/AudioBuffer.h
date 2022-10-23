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

#ifndef AudioBuffer_H
#define AudioBuffer_H

#include "Helpers.h"
#include "circle_buf.h"

#define AUDIO_BUFFER_FRAME_LENGTH 1024

class AudioBuffer: public NamedItem
{
	public:
		AudioBuffer( String Title )
				   : NamedItem(Title)
				   {}
		virtual ~AudioBuffer()
		{
			FreeMemory();
		}
		void Initialize()
		{	
			AllocateMemory();		
			if(0 != pthread_mutex_init(&m_Lock, NULL))
			{
			   ESP_LOGE("AudioBuffer", "Failed to Create Lock");
			}
		}
		void AllocateMemory()
		{
			size_t CircleBuffSize = sizeof(bfs::CircleBuf<Frame_t, AUDIO_BUFFER_FRAME_LENGTH>);
			void *CircularBuffer_Raw = (bfs::CircleBuf<Frame_t, AUDIO_BUFFER_FRAME_LENGTH>*)heap_caps_malloc(CircleBuffSize, MALLOC_CAP_SPIRAM);
			m_CircularAudioBuffer = new(CircularBuffer_Raw) bfs::CircleBuf<Frame_t, AUDIO_BUFFER_FRAME_LENGTH>;
		}
		void FreeMemory()
		{
			heap_caps_free(m_CircularAudioBuffer);
		}
		float GetNormalizedFillPercent() 
		{ 
			return (float)GetFrameCount() / (float)GetFrameCapacity(); 
		}
		size_t GetFrameCapacity();
		size_t GetFrameCount();
		size_t GetFreeFrameCount();
		size_t ReadAudioFrames(Frame_t *FrameBuffer, size_t FrameCount);
		size_t WriteAudioFrames( Frame_t *FrameBuffer, size_t FrameCount );
		bool WriteAudioFrame( Frame_t FrameBuffer );
		bool ClearAudioBuffer();
		bfs::optional<Frame_t> ReadAudioFrame();
	private:
		bfs::CircleBuf<Frame_t, AUDIO_BUFFER_FRAME_LENGTH> *m_CircularAudioBuffer;
		pthread_mutex_t m_Lock;
};

#endif