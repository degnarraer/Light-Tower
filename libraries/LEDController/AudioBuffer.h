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


#include "Datatypes.h"
#include "CircularBuffer.h"

#define AUDIO_BUFFER_FRAME_LENGTH 1024

class AudioBuffer
{
  public:
    AudioBuffer(){}
    virtual ~AudioBuffer()
    {
      FreeMemory();
    }
    void Initialize();
    void AllocateMemory();
    void FreeMemory();
	
	bool Push(Frame_t Frame);
	bool Unshift(Frame_t Frame);
	Frame_t Pop();
	Frame_t Shift();
	
    bool IsEmpty();
	bool IsFull();
	size_t Size();
	size_t Available();
	size_t Capacity();
	void Clear();
  private:
    CircularBuffer<Frame_t, AUDIO_BUFFER_FRAME_LENGTH> *m_CircularAudioBuffer;
    pthread_mutex_t m_Lock;
};

#endif
