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
		memset(spi_rx_buf[i], 0, SPI_MAX_DATA_BYTES);
		memset(spi_tx_buf[i], 0, SPI_MAX_DATA_BYTES);
		if(NULL == spi_rx_buf[i] || NULL == spi_tx_buf[i])
		{
			ESP_LOGE("SPI_Datalink", "Error Creating DMA Buffers!");
		}
	}
	m_SPI_Master.setDMAChannel(m_DMA_Channel);
	m_SPI_Master.setMaxTransferSize(SPI_MAX_DATA_BYTES);
	m_SPI_Master.setDataMode(SPI_MODE);
	m_SPI_Master.setFrequency(CLOCK_SPEED);
	m_SPI_Master.setDutyCyclePos(DUTY_CYCLE_POS);
	m_SPI_Master.setQueueSize(N_MASTER_QUEUES);
	m_SPI_Master.begin(m_SPI_BUS, m_SCK, m_MISO, m_MOSI, m_SS);
	ESP_LOGE("SPI_Datalink", "SPI Master Configured");
}

void SPI_Datalink_Master::ProcessEventQueue()
{
	if(NULL != m_DataItems)
	{
		size_t MaxMessageCount = 0;
		size_t TotalMessageCount = 0;
		for(int i = 0; i < m_DataItemsCount; ++i)
		{
			if(NULL != m_DataItems[i].QueueHandle_TX)
			{
				size_t MessageCount = uxQueueMessagesWaiting(m_DataItems[i].QueueHandle_TX);
				TotalMessageCount += MessageCount;
				if(MessageCount > MaxMessageCount)
				{
					MaxMessageCount = MessageCount;
				}
			}
		}
		if(0 == TotalMessageCount)
		{
			uint32_t CurrentIndex = m_Queued_Transactions  % N_MASTER_QUEUES;
			memset(spi_rx_buf[CurrentIndex], 0, SPI_MAX_DATA_BYTES);
			memset(spi_tx_buf[CurrentIndex], 0, SPI_MAX_DATA_BYTES);
			if(true == m_SPI_Master.queue(spi_tx_buf[CurrentIndex], spi_rx_buf[CurrentIndex], SPI_MAX_DATA_BYTES))
			{
				++m_Queued_Transactions;
				delay(2); // NEED THIS FOR SOME REASON ELSE DATA GETS CORRUPTED
				TransmitQueuedData();
			}
		}
		else
		{
			for(int i = 0; i < MaxMessageCount; ++i)
			{
				for(int j = 0; j < m_DataItemsCount; ++j)
				{
					if(m_Queued_Transactions - m_Queued_Transactions_Reset_Point >= N_MASTER_QUEUES)
					{
						TransmitQueuedData();
					}
					if(NULL != m_DataItems[j].QueueHandle_TX)
					{
						if(uxQueueMessagesWaiting(m_DataItems[j].QueueHandle_TX) > 0)
						{
							uint8_t DataBuffer[GetSizeOfDataType(m_DataItems[j].DataType) * m_DataItems[j].Count];
							if ( xQueuePeek(m_DataItems[j].QueueHandle_TX, DataBuffer, 0) == pdTRUE )
							{
								uint32_t CurrentIndex = m_Queued_Transactions  % N_MASTER_QUEUES;
								memset(spi_rx_buf[CurrentIndex], 0, SPI_MAX_DATA_BYTES);
								memset(spi_tx_buf[CurrentIndex], 0, SPI_MAX_DATA_BYTES);
								size_t DataLength = 0;
								DataLength = EncodeDataToBuffer(m_DataItems[j].Name, m_DataItems[j].DataType, DataBuffer, m_DataItems[j].Count, (char*)spi_tx_buf[CurrentIndex], SPI_MAX_DATA_BYTES);
								if(true == m_SpewTXToConsole && 0 < DataLength)
								{
									ESP_LOGE("SPI_Datalink", "TX: %s", String((char*)spi_tx_buf[CurrentIndex]).c_str());
								}
								if(true == m_SPI_Master.queue(spi_tx_buf[CurrentIndex], spi_rx_buf[CurrentIndex], SPI_MAX_DATA_BYTES))
								{
									++m_Queued_Transactions;
									delay(2); // NEED THIS FOR SOME REASON ELSE DATA GETS CORRUPTED
									xQueueReceive(m_DataItems[j].QueueHandle_TX, DataBuffer, 0);
								}
							}
							else
							{
								break;
							}
						}
					}
				}
			}
			TransmitQueuedData();
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

size_t SPI_Datalink_Master::EncodeDataToBuffer(String DataItemName, DataType_t DataType, void* Object, size_t Count, char *Buffer, size_t MaxBytesToEncode)
{
	String DataToSend = SerializeDataToJson(DataItemName, DataType, Object, Count).c_str();
	size_t DataToSendLength = DataToSend.length();
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
	uint32_t QueueCount = m_Queued_Transactions - m_Queued_Transactions_Reset_Point;
	for(int i = 0; i < QueueCount; ++i)
	{ 
		uint32_t CurrentIndex = m_DeQueued_Transactions % N_MASTER_QUEUES;
		String ResultString( String((char*) (spi_rx_buf[CurrentIndex])) );
		++m_DeQueued_Transactions;
		if(strlen(ResultString.c_str()) > 0)
		{
			if(true == m_SpewRXToConsole)
			{
				ESP_LOGE("SPI_Datalink_Config", "RX: %s", ResultString.c_str());
			}
			DeSerializeJsonToMatchingDataItem(ResultString.c_str(), m_SpewRXToConsole);
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
		memset(spi_rx_buf[i], 0, SPI_MAX_DATA_BYTES);
		memset(spi_tx_buf[i], 0, SPI_MAX_DATA_BYTES);
		if(NULL == spi_rx_buf[i] || NULL == spi_tx_buf[i])
		{
			ESP_LOGE("SPI_Datalink", "Error Creating DMA Buffers!");
		}
	}
	m_SPI_Slave.setDMAChannel(m_DMA_Channel);
	m_SPI_Slave.setMaxTransferSize(SPI_MAX_DATA_BYTES);
	m_SPI_Slave.setQueueSize(N_SLAVE_QUEUES);
	m_SPI_Slave.setDataMode(SPI_MODE);
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
	size_t SPIMessageCount = m_SPI_Slave.available();
	for(int i = 0; i < SPIMessageCount; ++i)
	{
		uint32_t CurrentIndex = m_DeQueued_Transactions % N_SLAVE_QUEUES;
		String ResultString = String((char*)(spi_rx_buf[CurrentIndex]));
		if(0 < m_SPI_Slave.size())
		{
			m_SPI_Slave.pop();
			++m_DeQueued_Transactions;
			if(ResultString.length() > 0)
			{
				if(true == m_SpewRXToConsole)
				{
					ESP_LOGE("SPI_Datalink", "RX: %s", ResultString.c_str());
				}
				DeSerializeJsonToMatchingDataItem(ResultString.c_str(), m_SpewRXToConsole); 
			}
		}
	}
}

void SPI_Datalink_Slave::QueueUpNewTransactions()
{
	size_t ItemsToQueue = N_SLAVE_QUEUES - (m_SPI_Slave.remained() + m_SPI_Slave.available());
	for( int i = 0; i < ItemsToQueue; ++i )
	{
		uint32_t CurrentIndex = m_Queued_Transactions % N_SLAVE_QUEUES;
		memset(spi_rx_buf[CurrentIndex], 0, SPI_MAX_DATA_BYTES);
		memset(spi_tx_buf[CurrentIndex], 0, SPI_MAX_DATA_BYTES);
		size_t ResultSize = GetNextTXStringFromDataItems(spi_tx_buf[CurrentIndex], SPI_MAX_DATA_BYTES);
		String ResultString = (char*)(spi_tx_buf[CurrentIndex]);
		if(true == m_SPI_Slave.queue(spi_rx_buf[CurrentIndex], spi_tx_buf[CurrentIndex], SPI_MAX_DATA_BYTES))
		{
			++m_Queued_Transactions;
			if(true == m_SpewTXToConsole)
			{
				if(0 < ResultSize)
				{
					ESP_LOGE("SPI_Datalink_Config", "TX: %s", ResultString.c_str());
				}
			}
		}
		else
		{
			break;
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
			if(NULL != m_DataItems[m_CurrentDataItemToTX].QueueHandle_TX)
			{
				if(uxQueueMessagesWaiting(m_DataItems[m_CurrentDataItemToTX].QueueHandle_TX) > 0)
				{
					String Name = m_DataItems[m_CurrentDataItemToTX].Name;
					byte Buffer[GetSizeOfDataType(m_DataItems[m_CurrentDataItemToTX].DataType) * m_DataItems[m_CurrentDataItemToTX].Count];
					if ( xQueueReceive(m_DataItems[m_CurrentDataItemToTX].QueueHandle_TX, Buffer, 0) == pdTRUE )
					{
						ResultingSize = EncodeDataToBuffer(m_DataItems[m_CurrentDataItemToTX].Name, m_DataItems[m_CurrentDataItemToTX].DataType, Buffer, m_DataItems[m_CurrentDataItemToTX].Count, (char*)TXBuffer, BytesToSend);
						break;
					}
					else
					{
						break;
					}
				}
			}
			m_CurrentDataItemToTX = (m_CurrentDataItemToTX + 1) % m_DataItemsCount;;
		}
	}
	return ResultingSize;
}

size_t SPI_Datalink_Slave::EncodeDataToBuffer(String DataItemName, DataType_t DataType, void* Object, size_t Count, char *Buffer, size_t MaxBytesToEncode)
{
	String DataToSend = SerializeDataToJson(DataItemName, DataType, Object, Count);
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
	ESP_LOGV("SPI_Datalink", "TX: %s", DataToSend.c_str());
	return DataToSendLength;
}