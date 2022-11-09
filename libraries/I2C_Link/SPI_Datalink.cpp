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

void SPI_Datalink_Master::Setup_SPI_Master()
{
	spi_rx_buf = m_SPI_Master.allocDMABuffer(SPI_MAX_DATA_BYTES);
	spi_tx_buf = m_SPI_Master.allocDMABuffer(SPI_MAX_DATA_BYTES);
	if(NULL == spi_rx_buf || NULL == spi_tx_buf)
	{
		ESP_LOGE("SPI_Datalink", "Error Creating DMA Buffers!");
	}
	m_SPI_Master.setDMAChannel(m_DMA_Channel);
	m_SPI_Master.setMaxTransferSize(SPI_MAX_DATA_BYTES);
	m_SPI_Master.setDataMode(SPI_MODE0);
	m_SPI_Master.setFrequency(CLOCK_SPEED);
	m_SPI_Master.setDutyCyclePos(DUTY_CYCLE_POS);
	memset(spi_rx_buf, 0, SPI_MAX_DATA_BYTES);
	memset(spi_tx_buf, 0, SPI_MAX_DATA_BYTES);
	Begin();
}
void SPI_Datalink_Master::ProcessDataTXEventQueue()
{
	if(NULL != m_DataItems)
	{
		for(int i = 0; i < m_DataItemsCount; ++i)
		{
			if(NULL != m_DataItems[i].QueueHandle_TX)
			{
				if(uxQueueMessagesWaiting(m_DataItems[i].QueueHandle_TX) > 0)
				{
					ProcessTXData(m_DataItems[i]);
				}
			}
		}
	}
}

size_t SPI_Datalink_Master::TransferBytes(size_t Length)
{
	assert(Length <= SPI_MAX_DATA_BYTES);
	size_t ReceivedLength = m_SPI_Master.transfer(spi_tx_buf, spi_rx_buf, Length);
	return ReceivedLength;
}

bool SPI_Datalink_Master::Begin() 
{ 
	return m_SPI_Master.begin(HSPI, m_SCK, m_MISO, m_MOSI, m_SS);
}
bool SPI_Datalink_Master::End()
{
	return m_SPI_Master.end();
}

void SPI_Datalink_Master::EncodeAndTransmitData(String Name, DataType_t DataType, void* Object, size_t Count)
{
	String DataToSend = SerializeDataToJson(Name, DataType, Object, Count);
	size_t DataToSendLength = strlen(DataToSend.c_str());
	size_t PadCount = 0;
	if(0 != DataToSendLength % 4)
	{
		PadCount = 4 - DataToSendLength % 4;
	}
	for(int i = 0; i < PadCount; ++i)
	{
		DataToSend += "\0";
	}
	DataToSendLength += PadCount;
	ESP_LOGE("SPI_Datalink", "TX: %s", DataToSend.c_str());
	assert(DataToSendLength < SPI_MAX_DATA_BYTES);
	memcpy(spi_tx_buf, DataToSend.c_str(), DataToSendLength);
	TransferBytes(DataToSendLength);
}

void SPI_Datalink_Master::ProcessTXData(DataItem_t DataItem)
{
	if(NULL != DataItem.QueueHandle_TX)
	{
		size_t MessageCount = uxQueueMessagesWaiting(DataItem.QueueHandle_TX);
		if(MessageCount > 0)
		{
			if ( xQueueReceive(DataItem.QueueHandle_TX, DataItem.DataBuffer, 0) == pdTRUE )
			{
				EncodeAndTransmitData(DataItem.Name, DataItem.DataType, DataItem.DataBuffer, DataItem.Count);
			}
		}
	}
}
void SPI_Datalink_Slave::Setup_SPI_Slave()
{
	ESP_LOGE("SPI_Datalink", "Configuring SPI Slave");
	spi_rx_buf = m_SPI_Slave.allocDMABuffer(SPI_MAX_DATA_BYTES);
	spi_tx_buf = m_SPI_Slave.allocDMABuffer(SPI_MAX_DATA_BYTES);
	if(NULL == spi_rx_buf || NULL == spi_tx_buf)
	{
		ESP_LOGE("SPI_Datalink", "Error Creating DMA Buffers!");
	}
	m_SPI_Slave.setDMAChannel(m_DMA_Channel);
	m_SPI_Slave.setMaxTransferSize(SPI_MAX_DATA_BYTES);
	m_SPI_Slave.setDataMode(SPI_MODE0);
	xTaskCreatePinnedToCore
	(
		static_task_wait_spi,
		"task_wait_spi",
		10000,
		this,
		configMAX_PRIORITIES-1,
		&task_handle_wait_spi,
		m_Core
	);
	xTaskCreatePinnedToCore
	(
		static_task_process_buffer,
		"task_process_buffer",
		10000,
		this,
		configMAX_PRIORITIES-1,
		&task_handle_process_buffer,
		m_Core
	);
	m_SPI_Slave.begin(HSPI, m_SCK, m_MISO, m_MOSI, m_SS);
	ESP_LOGE("SPI_Datalink", "SPI Slave Configured");
	xTaskNotifyGive(task_handle_wait_spi);
}
void SPI_Datalink_Slave::RegisterForDataTransferNotification(SPI_Slave_Notifier *Notifiee)
{
	m_Notifiee = Notifiee;
}
void SPI_Datalink_Slave::static_task_wait_spi(void* pvParameters)
{
	SPI_Datalink_Slave* slave = (SPI_Datalink_Slave*)pvParameters;
	slave->task_wait_spi();
}
void SPI_Datalink_Slave::task_wait_spi()
{
	while(true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		memset(spi_rx_buf, 0, SPI_MAX_DATA_BYTES);
		memset(spi_tx_buf, 0, SPI_MAX_DATA_BYTES);
		if(NULL != m_Notifiee)
		{
			size_t ActualBufferSize = m_Notifiee->SendBytesTransferNotification(spi_tx_buf, SPI_MAX_DATA_BYTES);
			assert( 0 == ActualBufferSize % sizeof(Frame_t) );
		}
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
	while(true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		if(NULL != m_Notifiee)
		{
			m_Notifiee->ReceivedBytesTransferNotification(spi_rx_buf, m_SPI_Slave.size());
		}
		m_SPI_Slave.pop();
		xTaskNotifyGive(task_handle_wait_spi);
	}
}

void SPI_Datalink_Slave::ProcessDataRXEventQueue()
{
	
}