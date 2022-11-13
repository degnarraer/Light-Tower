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

#ifndef I2C_Datalink_H
#define I2C_Datalink_H
#include <Wire.h>
#include <WireSlaveRequest.h>
#include <WireSlave.h>
#include <Helpers.h>
#include <Serial_Datalink_Core.h>
#include "circle_buf.h"
#include <SPI.h>
#include <ESP32DMASPIMaster.h>
#include <ESP32DMASPISlave.h>
#include "DataSerializer.h"

#define SPI_MAX_DATA_BYTES 500
#define N_SLAVE_QUEUES 20
#define N_MASTER_QUEUES 20
#define DUTY_CYCLE_POS 128
#define CLOCK_SPEED 4000000

class SPI_Slave_Notifier
{
	public:
		SPI_Slave_Notifier(){}
		virtual ~SPI_Slave_Notifier(){}
		virtual size_t SendBytesTransferNotification(uint8_t *TXBuffer, size_t BytesToSend) = 0;
		virtual size_t ReceivedBytesTransferNotification(uint8_t *RXBuffer, size_t BytesReceived) = 0;
	private:
};

class SPI_Datalink: public DataSerializer
{
	public:
		SPI_Datalink( uint8_t SCK
					, uint8_t MISO
					, uint8_t MOSI
					, uint8_t SS
					, uint8_t DMA_Channel )
					: m_SCK(SCK)
					, m_MISO(MISO)
					, m_MOSI(MOSI)
					, m_SS(SS)
					, m_DMA_Channel(DMA_Channel)
					{
					}
		virtual ~SPI_Datalink(){}
		void SetSerialDataLinkDataItems(DataItem_t& DataItems, size_t Count) 
		{ 
			m_DataItems = &DataItems;
			m_DataItemsCount = Count;
			SetDataSerializerDataItems(DataItems, Count);
		}
	protected:
		uint8_t m_SCK;
		uint8_t m_MISO;
		uint8_t m_MOSI;
		uint8_t m_SS;
		uint8_t m_DMA_Channel = 0;
		DataItem_t* m_DataItems;
		size_t m_DataItemsCount = 0;
		size_t m_Queued_Transactions = 0;
	private:	
};

class SPI_Datalink_Master: public SPI_Datalink
{
	public:
		SPI_Datalink_Master( String Title
						   , uint8_t SCK
						   , uint8_t MISO
						   , uint8_t MOSI
						   , uint8_t SS
						   , uint8_t DMA_Channel )
						   : SPI_Datalink(SCK, MISO, MOSI, SS, DMA_Channel)
						   , m_Title(Title)
						   {
							  Setup_SPI_Master(); 
						   }
		virtual ~SPI_Datalink_Master(){}
		void ProcessDataTXEventQueue();
	protected:
		void Setup_SPI_Master();
		void QueueSingleTransaction(uint8_t *tx_Buff, uint8_t *rx_Buff, size_t Length);
		bool Begin();
		bool End();
	private:
		uint8_t *spi_tx_buf[N_MASTER_QUEUES];
		uint8_t *spi_rx_buf[N_MASTER_QUEUES];
		String m_Title = "";
		ESP32DMASPI::Master m_SPI_Master;
		void EncodeAndTransmitData(String Name, DataType_t DataType, void* Object, size_t Count);
		void ProcessTXData(DataItem_t DataItem);
};

class SPI_Datalink_Slave: public SPI_Datalink
{
	public:
		SPI_Datalink_Slave( String Title
						  , uint8_t SCK
						  , uint8_t MISO
						  , uint8_t MOSI
						  , uint8_t SS
						  , uint8_t DMA_Channel )
						  : SPI_Datalink(SCK, MISO, MOSI, SS, DMA_Channel)
						  , m_Title(Title)
						  {
							 Setup_SPI_Slave(); 
						  }
		virtual ~SPI_Datalink_Slave(){}
		void ProcessDataRXEventQueue();
		void RegisterForDataTransferNotification(SPI_Slave_Notifier *Notifiee);
	protected:
		void Setup_SPI_Slave();
	private:
		uint8_t* spi_tx_buf[N_SLAVE_QUEUES];
		uint8_t* spi_rx_buf[N_SLAVE_QUEUES];
		String m_Title = "";
		SPI_Slave_Notifier *m_Notifiee = NULL;
		ESP32DMASPI::Slave m_SPI_Slave;
};

#endif