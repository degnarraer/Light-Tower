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
    for (uint8_t i = 0; i < N_MASTER_QUEUES; ++i) 
	{
		spi_rx_buf[i] = m_SPI_Master.allocDMABuffer(SPI_MAX_DATA_BYTES);
		spi_tx_buf[i] = m_SPI_Master.allocDMABuffer(SPI_MAX_DATA_BYTES);
		if(NULL == spi_rx_buf[i] || NULL == spi_tx_buf[i])
		{
			ESP_LOGE("SPI_Datalink", "Error Creating DMA Buffers!");
		}
	}
	m_SPI_Master.setDMAChannel(m_DMA_Channel);
	m_SPI_Master.setMaxTransferSize(SPI_MAX_DATA_BYTES);
	m_SPI_Master.setDataMode(SPI_MODE0);
	m_SPI_Master.setFrequency(CLOCK_SPEED);
	m_SPI_Master.setDutyCyclePos(DUTY_CYCLE_POS);
	m_SPI_Master.setQueueSize(N_MASTER_QUEUES);
	m_SPI_Master.begin(m_SPI_BUS, m_SCK, m_MISO, m_MOSI, m_SS);
}

void SPI_Datalink_Master::ProcessEventQueue()
{
	if(NULL != m_DataItems)
	{
		if(true == m_TransmitQueuedDataFlag)
		{
			TransmitQueuedData();
		}
		size_t MessageCount = 0;
		size_t MaxTransmits = 5;
		for(int i = 0; i < m_DataItemsCount; ++i)
		{
			if(NULL != m_DataItems[i].QueueHandle_TX)
			{
				MessageCount += uxQueueMessagesWaiting(m_DataItems[i].QueueHandle_TX);
			}
		}
		if(0 == MessageCount)
		{
			size_t CurrentIndex = m_Queued_Transactions % N_MASTER_QUEUES;
			memset(spi_rx_buf[CurrentIndex], 0, SPI_MAX_DATA_BYTES);
			memset(spi_tx_buf[CurrentIndex], 0, SPI_MAX_DATA_BYTES);
			delay(10); //WITHOUT THIS WE SEND GARBAGE DATA
			m_SPI_Master.queue(NULL, spi_rx_buf[CurrentIndex], SPI_MAX_DATA_BYTES);
			++m_Queued_Transactions;
			if(m_Queued_Transactions - m_Queued_Transactions_Reset_Point >= N_MASTER_QUEUES)
			{
				TransmitQueuedData();
			}
		}
		else
		{
			while(MessageCount > 0 && MaxTransmits > 0)
			{
				for(int i = 0; i < m_DataItemsCount; ++i)
				{
					if(NULL != m_DataItems[i].QueueHandle_TX)
					{
						if(uxQueueMessagesWaiting(m_DataItems[i].QueueHandle_TX) > 0)
						{
							if ( xQueueReceive(m_DataItems[i].QueueHandle_TX, m_DataItems[i].DataBuffer, portMAX_DELAY) == pdTRUE )
							{
								size_t CurrentIndex = m_Queued_Transactions % N_MASTER_QUEUES;
								memset(spi_rx_buf[CurrentIndex], 0, SPI_MAX_DATA_BYTES);
								memset(spi_tx_buf[CurrentIndex], 0, SPI_MAX_DATA_BYTES);
								size_t DataLength = EncodeDataToBuffer(m_DataItems[i].Name, m_DataItems[i].DataType, m_DataItems[i].DataBuffer, m_DataItems[i].Count, spi_tx_buf[CurrentIndex], SPI_MAX_DATA_BYTES);
								delay(10); //WITHOUT THIS WE SEND GARBAGE DATA
								if(true == m_SpewToConsole) Serial << "TX: " << String((char*)spi_tx_buf[CurrentIndex]).c_str() << "\n";
								ESP_LOGI("SPI_Datalink", "TX: %s", String((char*)spi_tx_buf[CurrentIndex]).c_str());
								if(0 == DataLength)
								{
									m_SPI_Master.queue(NULL, spi_rx_buf[CurrentIndex], SPI_MAX_DATA_BYTES);
								}
								else
								{
									m_SPI_Master.queue(spi_tx_buf[CurrentIndex], spi_rx_buf[CurrentIndex], SPI_MAX_DATA_BYTES);
								}
								++m_Queued_Transactions;
								--MessageCount;
							}
						}
					}
					if(m_Queued_Transactions - m_Queued_Transactions_Reset_Point >= N_MASTER_QUEUES)
					{
						TransmitQueuedData();
					}
				}
			}
			--MaxTransmits;
		}
	}
}

bool SPI_Datalink_Master::Begin() 
{ 
	return m_SPI_Master.begin(m_SPI_BUS, m_SCK, m_MISO, m_MOSI, m_SS);
}
bool SPI_Datalink_Master::End()
{
	return m_SPI_Master.end();
}

size_t SPI_Datalink_Master::EncodeDataToBuffer(String DataTypeName, DataType_t DataType, void* Object, size_t Count, uint8_t *Buffer, size_t MaxBytesToEncode)
{
	String DataToSend = SerializeDataToJson(DataTypeName, DataType, Object, Count);
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
	assert(DataToSendLength <= MaxBytesToEncode);
	memcpy(Buffer, DataToSend.c_str(), DataToSendLength);
	return DataToSendLength;
}

void SPI_Datalink_Master::TransmitQueuedData()
{ 
	m_SPI_Master.yield();
	m_TransmitQueuedDataFlag = false;
	size_t QueueCount = m_Queued_Transactions - m_Queued_Transactions_Reset_Point;
	for(int i = 0; i < QueueCount; ++i)
	{
		size_t CurrentDeQueueIndex = m_DeQueued_Transactions % N_MASTER_QUEUES;
		String ResultString( (char*) (spi_rx_buf[CurrentDeQueueIndex]) );
		++m_DeQueued_Transactions;
		if(strlen(ResultString.c_str()) > 0)
		{
			if(true == m_SpewToConsole) Serial << "RX: " << ResultString.c_str() << "\n";
			ESP_LOGV("SPI_Datalink_Config", "RX: %s", ResultString.c_str());
			DeSerializeJsonToMatchingDataItem(ResultString.c_str(), true);
		}
	}	
	m_Queued_Transactions_Reset_Point = m_Queued_Transactions;
}

void SPI_Datalink_Slave::Setup_SPI_Slave()
{
	ESP_LOGE("SPI_Datalink", "Configuring SPI Slave");
    for (uint8_t i = 0; i < N_SLAVE_QUEUES; ++i) 
	{
		spi_rx_buf[i] = m_SPI_Slave.allocDMABuffer(SPI_MAX_DATA_BYTES);
		spi_tx_buf[i] = m_SPI_Slave.allocDMABuffer(SPI_MAX_DATA_BYTES);
		if(NULL == spi_rx_buf[i] || NULL == spi_tx_buf[i])
		{
			ESP_LOGE("SPI_Datalink", "Error Creating DMA Buffers!");
		}
	}
	m_SPI_Slave.setDMAChannel(m_DMA_Channel);
	m_SPI_Slave.setMaxTransferSize(SPI_MAX_DATA_BYTES);
	m_SPI_Slave.setQueueSize(N_SLAVE_QUEUES);
	m_SPI_Slave.setDataMode(SPI_MODE0);

	m_SPI_Slave.begin(m_SPI_BUS, m_SCK, m_MISO, m_MOSI, m_SS);
	ESP_LOGE("SPI_Datalink", "SPI Slave Configured");
}

bool SPI_Datalink_Slave::Begin() 
{ 
	return m_SPI_Slave.begin(m_SPI_BUS, m_SCK, m_MISO, m_MOSI, m_SS);
}

bool SPI_Datalink_Slave::End()
{
	return m_SPI_Slave.end();
}

void SPI_Datalink_Slave::ProcessEventQueue()
{
	ProcessCompletedTransactions();
	QueueUpNewTransactions();
}

void SPI_Datalink_Slave::ProcessCompletedTransactions()
{
	while(m_SPI_Slave.available())
	{
		size_t CurrentDeQueueIndex = m_DeQueued_Transactions % N_SLAVE_QUEUES;
		String ResultString = String( (char*)(spi_rx_buf[CurrentDeQueueIndex]) );
		if(ResultString.length() > 0)
		{
			ESP_LOGV("SPI_Datalink", "Received: %s", ResultString.c_str());
			DeSerializeJsonToMatchingDataItem(ResultString.c_str(), false); 
		}
		m_SPI_Slave.pop();
		++m_DeQueued_Transactions;
	}
}

void SPI_Datalink_Slave::QueueUpNewTransactions()
{
	while( (m_SPI_Slave.remained() + m_SPI_Slave.available() <= N_SLAVE_QUEUES - 1) )
	{
		size_t CurrentQueueIndex = m_Queued_Transactions % N_SLAVE_QUEUES;
		memset(spi_rx_buf[CurrentQueueIndex], 0, SPI_MAX_DATA_BYTES);
		memset(spi_tx_buf[CurrentQueueIndex], 0, SPI_MAX_DATA_BYTES);
		delay(10); //WITHOUT THIS WE SEND GARBAGE DATA
		size_t SendBytesSize = GetNextTXStringFromDataItems(spi_tx_buf[CurrentQueueIndex], SPI_MAX_DATA_BYTES);
		if(0 == SendBytesSize)
		{
			if( (m_SPI_Slave.remained() + m_SPI_Slave.available() <= N_SLAVE_QUEUES - 1) && (true == m_SPI_Slave.queue(spi_rx_buf[CurrentQueueIndex], SPI_MAX_DATA_BYTES)))
			{
				++m_Queued_Transactions;
			}
			else
			{
				break;
			}
		}
		else
		{
			if( ((m_SPI_Slave.remained() + m_SPI_Slave.available() <= N_SLAVE_QUEUES - 1)) && (true == m_SPI_Slave.queue(spi_rx_buf[CurrentQueueIndex], spi_tx_buf[CurrentQueueIndex], SPI_MAX_DATA_BYTES)))
			{
				++m_Queued_Transactions;
			}
			else
			{
				break;
			}
		}
	}
}
size_t SPI_Datalink_Slave::GetNextTXStringFromDataItems(uint8_t *TXBuffer, size_t BytesToSend)
{
	size_t ResultingSize = 0;
	if(NULL != m_DataItems)
	{
		for(int i = 0; i < m_DataItemsCount; ++i)
		{
			size_t index = m_CurrentDataItemToTX % m_DataItemsCount;
			++m_CurrentDataItemToTX;
			if(NULL != m_DataItems[index].QueueHandle_TX)
			{
				if(uxQueueMessagesWaiting(m_DataItems[index].QueueHandle_TX) > 0)
				{
					if ( xQueueReceive(m_DataItems[index].QueueHandle_TX, m_DataItems[index].DataBuffer, portMAX_DELAY) == pdTRUE )
					{
						ResultingSize = EncodeDataToBuffer(m_DataItems[index].Name, m_DataItems[index].DataType, m_DataItems[index].DataBuffer, m_DataItems[index].Count, TXBuffer, BytesToSend);
						break;
					}
				}
			}
		}
	}
	return ResultingSize;
}

size_t SPI_Datalink_Slave::EncodeDataToBuffer(String DataTypeName, DataType_t DataType, void* Object, size_t Count, uint8_t *Buffer, size_t MaxBytesToEncode)
{
	String DataToSend = SerializeDataToJson(DataTypeName, DataType, Object, Count);
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
	assert(DataToSendLength <= MaxBytesToEncode);
	memcpy(Buffer, DataToSend.c_str(), DataToSendLength);
	ESP_LOGI("SPI_Datalink", "TX: %s", DataToSend.c_str());
	return DataToSendLength;
}