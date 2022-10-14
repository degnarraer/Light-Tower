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
#include "SPI_Datalink.h"
#include "pthread.h"


size_t SPI_Datalink_Master::TransferBytes(uint8_t * RXBuffer, uint8_t * TXBuffer, size_t Length)
{
	assert(Length <= SPI_MAX_DATA_BYTES);
	size_t ReceivedLength = m_SPI_Master.transfer(RXBuffer, TXBuffer, Length);
	return ReceivedLength;
}

size_t SPI_Datalink_Slave::TransferBytes(uint8_t *RXBuffer, uint8_t *TXBuffer, size_t Length)
{
	return 0;
}
void SPI_Datalink_Slave::static_task_wait_spi(void* pvParameters)
{
	SPI_Datalink_Slave* slave = (SPI_Datalink_Slave*)pvParameters;
	slave->task_wait_spi();
}
void SPI_Datalink_Slave::task_wait_spi()
{
	while (1)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		memset(spi_tx_buf, 0, SPI_MAX_DATA_BYTES);
		size_t MaxBufferSize = SPI_MAX_DATA_BYTES;
		uint8_t Buffer[MaxBufferSize];
		size_t ActualBufferSize = m_Notifiee->SendBytesTransferNotification(Buffer, MaxBufferSize);
		assert( 0 == ActualBufferSize % sizeof(Frame_t) );
		size_t ActualFrameCount = ActualBufferSize / sizeof(Frame_t);
		String DataToSend = Serialize("AudioData", DataType_Frame_t, Buffer, ActualFrameCount);
		assert(strlen(DataToSend.c_str()) <= MaxBufferSize);
		memcpy(spi_tx_buf, (char *)DataToSend.c_str(), strlen(DataToSend.c_str()));
		m_SPI_Slave.wait(spi_rx_buf, spi_tx_buf, MaxBufferSize);
		xTaskNotifyGive(task_handle_process_buffer);		
	}
}
void SPI_Datalink_Slave::static_task_process_buffer(void* pvParameters)
{
	SPI_Datalink_Slave* slave = (SPI_Datalink_Slave*)pvParameters;
	slave->task_process_buffer();
}
void SPI_Datalink_Slave::task_process_buffer()
{
	while (1)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		m_Notifiee->ReceivedBytesTransferNotification(spi_rx_buf, m_SPI_Slave.size());
		m_SPI_Slave.pop();
		memset(spi_rx_buf, 0, SPI_MAX_DATA_BYTES);
		xTaskNotifyGive(task_handle_wait_spi);
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

size_t AudioBuffer::GetFreeFrameCount()
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
size_t AudioStreamRequester::BufferMoreAudio()
{
	memset(spi_tx_buf, 0, SPI_MAX_DATA_BYTES);
	size_t TotalFramesToFill = m_AudioBuffer.GetFreeFrameCount();
	if(0 == TotalFramesToFill)
	{
		m_AudioBuffer.ClearAudioBuffer();
		TotalFramesToFill = m_AudioBuffer.GetFreeFrameCount();
	}
	size_t TotalBytesToFill = TotalFramesToFill*sizeof(Frame_t);
	size_t MaxBytes = SPI_MAX_DATA_BYTES;
	size_t BytesRead = TransferBytes(spi_tx_buf, spi_rx_buf, MaxBytes);
	String Packet;
	for(int i = 0; i < MaxBytes; ++i)
	{
		if(0 == (char *)spi_rx_buf[i]) break;
		Packet = Packet + ((char *)spi_rx_buf)[i];
	}
	uint8_t DataBuffer[MaxBytes];
	size_t DataSize = DeSerialize(Packet, "AudioData", DataBuffer, MaxBytes);
	size_t FramesRead = DataSize / sizeof(Frame_t);
	size_t TotalFramesFilled = m_AudioBuffer.WriteAudioFrames((Frame_t*)DataBuffer, FramesRead);
	memset(spi_rx_buf, 0, SPI_MAX_DATA_BYTES);
	return TotalFramesFilled;
}

size_t AudioStreamRequester::GetFrameCount()
{
	return m_AudioBuffer.GetFrameCount();
}

size_t AudioStreamRequester::GetAudioFrames(Frame_t *FrameBuffer, size_t FrameCount)
{
	return m_AudioBuffer.ReadAudioFrames(FrameBuffer, FrameCount);
}