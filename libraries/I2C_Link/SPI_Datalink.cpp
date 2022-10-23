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

size_t SPI_Datalink_Master::TransferBytes(uint8_t * TXBuffer, uint8_t * RXBuffer, size_t Length)
{
	assert(Length <= SPI_MAX_DATA_BYTES);
	size_t ReceivedLength = m_SPI_Master.transfer(TXBuffer, RXBuffer, Length);
	return ReceivedLength;
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
		memset(spi_rx_buf, 0, SPI_MAX_DATA_BYTES);
		memset(spi_tx_buf, 0, SPI_MAX_DATA_BYTES);
		size_t ActualBufferSize = m_Notifiee->SendBytesTransferNotification(spi_tx_buf, SPI_MAX_DATA_BYTES);
		assert( 0 == ActualBufferSize % sizeof(Frame_t) );
		m_SPI_Slave.wait(spi_rx_buf, spi_tx_buf, SPI_MAX_DATA_BYTES );
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
		xTaskNotifyGive(task_handle_wait_spi);
	}
}

void AudioStreamMaster::Setup()
{
	ESP_LOGE("AudioStreamMaster", "Setup Audio Stream");
	Setup_SPI_Master();
}

size_t AudioStreamMaster::TxAudioFrames()
{
	Serial << "Tx\n";
	size_t FramesTXed = 0;
	size_t SPIFrameLength = SPI_FRAMES_LENGTH;
	if(m_AudioBuffer.GetFrameCount() >= SPIFrameLength)
	{
		while( m_AudioBuffer.GetFrameCount() >= SPIFrameLength )
		{
			Serial << "T1\n";
			memset(spi_rx_buf, 0, SPI_MAX_DATA_BYTES);
			memset(spi_tx_buf, 0, SPI_MAX_DATA_BYTES);
			size_t FramesRead = m_AudioBuffer.ReadAudioFrames((Frame_t*)spi_tx_buf, SPIFrameLength);
			size_t BytesRead = FramesRead*sizeof(Frame_t);
			assert(FramesRead == SPIFrameLength);
			TransferBytes(spi_tx_buf, spi_rx_buf, BytesRead);
			FramesTXed+=FramesRead;
		}
	}
	else
	{
		Serial << "T2\n";
		memset(spi_rx_buf, 0, SPI_MAX_DATA_BYTES);
		memset(spi_tx_buf, 0, SPI_MAX_DATA_BYTES);
		size_t FramesRead = m_AudioBuffer.ReadAudioFrames((Frame_t*)spi_tx_buf, m_AudioBuffer.GetFrameCount());
		size_t BytesRead = FramesRead*sizeof(Frame_t);
		assert(FramesRead <= SPIFrameLength);
		TransferBytes(spi_tx_buf, spi_rx_buf, BytesRead);
		FramesTXed+=FramesRead;
	}
	Serial << FramesTXed << "\n";
	return FramesTXed;
}

void AudioStreamSlave::Setup()
{
	ESP_LOGE("AudioStreamSlave", "Setup Audio Stream");
	Setup_SPI_Slave();
	RegisterForDataTransferNotification(this);
}

size_t AudioStreamSlave::GetFrameCount()
{
	return m_AudioBuffer.GetFrameCount();
}

size_t AudioStreamSlave::GetAudioFrames(Frame_t *FrameBuffer, size_t FrameCount)
{
	return m_AudioBuffer.ReadAudioFrames(FrameBuffer, FrameCount);
}

size_t AudioStreamSlave::SetAudioFrames(Frame_t *FrameBuffer, size_t FrameCount)
{
	return m_AudioBuffer.WriteAudioFrames(FrameBuffer, FrameCount);
}

bool AudioStreamSlave::SetAudioFrame(Frame_t Frame)
{
	return m_AudioBuffer.WriteAudioFrame(Frame);
}

size_t AudioStreamSlave::SendBytesTransferNotification(uint8_t *TXBuffer, size_t BytesToSend)
{
	return 0;
}

size_t AudioStreamSlave::ReceivedBytesTransferNotification(uint8_t *RXBuffer, size_t BytesReceived)
{
	size_t FramesToWrite = BytesReceived / sizeof(Frame_t);
	size_t Result = SetAudioFrames((Frame_t*)RXBuffer, BytesReceived / sizeof(Frame_t));
	return Result;
}