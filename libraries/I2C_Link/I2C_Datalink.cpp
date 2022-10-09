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


size_t SPI_Datalink_Master::TransferBytes(uint8_t * RXBuffer, uint8_t * TXBuffer, size_t Length)
{
	ESP_LOGE("SPI_Datalink_Master", "Transfer Data");
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
		ESP_LOGE("SPI_Datalink_Slave", "Waiting");
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		size_t BufferSize = 0;
		m_Notifiee->SetTransferBytesNotification(spi_rx_buf, spi_tx_buf, BufferSize);
		//Insert Read Byte Count into Data Frame
		Serial << "Buffer Size: " << BufferSize << "\n";
		ESP_LOGE("SPI_Datalink_Slave", "Wait for transfer %i Bytes.", BufferSize);
		m_SPI_Slave.wait(spi_rx_buf, spi_tx_buf, BufferSize);
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
		ESP_LOGE("SPI_Datalink_Slave", "Processing");
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		ESP_LOGE("SPI_Datalink_Slave", "Process Buffer Queue Size: %i  Data Size: %i", m_SPI_Slave.available(), m_SPI_Slave.size());
		m_Notifiee->GetTransferBytesNotification(spi_rx_buf, spi_tx_buf, m_SPI_Slave.size());
		m_SPI_Slave.pop();
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
	size_t TotalFramesToFill = m_AudioBuffer.GetFreeFrameCount();
	size_t TotalBytesToFill = TotalFramesToFill*sizeof(Frame_t);
	size_t MaxBytes = SPI_MAX_DATA_BYTES;
	Serial << "Buffer More Audio Frames: " << TotalFramesToFill << "\n";
	uint8_t Buffer[SPI_MAX_DATA_BYTES];
	size_t BytesRead = TransferBytes(Buffer, NULL, min(TotalBytesToFill, MaxBytes));
	Serial << "Bytes Read: " << BytesRead << "\n";
	if(BytesRead > 0)
	{
		size_t FramesRead = BytesRead / sizeof(Frame_t);
		size_t TotalFramesFilled = m_AudioBuffer.WriteAudioFrames((Frame_t*)Buffer, FramesRead);
		ESP_LOGE("AudioBuffer", "Filled %i Frames", TotalFramesFilled);
		return TotalFramesFilled;
	}
	else
	{
		return 0;
	}
}

size_t AudioStreamRequester::GetFrameCount()
{
	return m_AudioBuffer.GetFrameCount();
}

size_t AudioStreamRequester::GetAudioFrames(Frame_t *FrameBuffer, size_t FrameCount)
{
	return m_AudioBuffer.ReadAudioFrames(FrameBuffer, FrameCount);
}



























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
		Serial << "Request\n";
		while( 0 < slaveReq.available() && ByteCountRead < ByteCountToRead ) 
		{
			data[ByteCountRead] = slaveReq.read();
			++ByteCountRead;
		}
	}
	else 
	{
		ESP_LOGE("I2C_Datalink", "I2C Master Device Named \"%s\" Read Data Request Error: %s", GetTitle().c_str(), slaveReq.lastStatusToString().c_str());
	}
	
	ESP_LOGE("I2C_Datalink", "Bytes Requested: %i\tBytes Read: %i", ByteCountToRead, ByteCountRead);
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