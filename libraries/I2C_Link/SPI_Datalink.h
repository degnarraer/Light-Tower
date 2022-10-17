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

#define AUDIO_BUFFER_LENGTH 4096
#define SPI_MAX_DATA_BYTES 1024
#define SPI_MAX_SERIALIZED_BYTES 1028
#define MAX_FRAMES_PER_PACKET 256
#define DUTY_CYCLE_POS 128
#define CLOCK_SPEED 4000000

class SPI_Slave_Notifier
{
	public:
		SPI_Slave_Notifier(){}
		virtual ~SPI_Slave_Notifier(){}
		virtual size_t SendBytesTransferNotification(uint8_t *TXBuffer, size_t MaxBytesToSend) = 0;
		virtual size_t ReceivedBytesTransferNotification(uint8_t *RXBuffer, size_t BytesReceived) = 0;
	private:
};

class SPI_Datalink: public DataSerializer
{
	public:
		SPI_Datalink( uint8_t SCK
					, uint8_t MISO
					, uint8_t MOSI
					, uint8_t SS )
					: m_SCK(SCK)
					, m_MISO(MISO)
					, m_MOSI(MOSI)
					, m_SS(SS)
					{
					}
		virtual ~SPI_Datalink(){}
	private:
	protected:
		uint8_t m_SCK;
		uint8_t m_MISO;
		uint8_t m_MOSI;
		uint8_t m_SS;
};

class SPI_Datalink_Master: public NamedItem
						 , public SPI_Datalink
{
	public:
		SPI_Datalink_Master( String Title
						   , uint8_t SCK
						   , uint8_t MISO
						   , uint8_t MOSI
						   , uint8_t SS )
						   : NamedItem(Title)
						   , SPI_Datalink(SCK, MISO, MOSI, SS){}
		virtual ~SPI_Datalink_Master(){}
		void Setup_SPI_Master()
		{
			spi_rx_buf = m_SPI_Master.allocDMABuffer(SPI_MAX_SERIALIZED_BYTES);
			spi_tx_buf = m_SPI_Master.allocDMABuffer(SPI_MAX_SERIALIZED_BYTES);
			memset(spi_rx_buf, 0, SPI_MAX_SERIALIZED_BYTES);
			memset(spi_tx_buf, 0, SPI_MAX_SERIALIZED_BYTES);
			m_SPI_Master.setDMAChannel(2);
			m_SPI_Master.setMaxTransferSize(SPI_MAX_SERIALIZED_BYTES);
			m_SPI_Master.setDataMode(SPI_MODE0);
			m_SPI_Master.setFrequency(CLOCK_SPEED);
			m_SPI_Master.setDutyCyclePos(DUTY_CYCLE_POS);
			Begin();
		}
		size_t TransferBytes(uint8_t *TXBuffer, uint8_t *RXBuffer, size_t Length);
		bool Begin() 
		{ 
			return m_SPI_Master.begin(HSPI, m_SCK, m_MISO, m_MOSI, m_SS);
		}
		bool End()
		{
			return m_SPI_Master.end();
		}
	protected:
		uint8_t* spi_tx_buf;
		uint8_t* spi_rx_buf;
	private:
		ESP32DMASPI::Master m_SPI_Master;
};

class SPI_Datalink_Slave: public NamedItem
						, public SPI_Datalink
{
	public:
		SPI_Datalink_Slave( String Title
						  , uint8_t SCK
						  , uint8_t MISO
						  , uint8_t MOSI
						  , uint8_t SS )
						  : NamedItem(Title)
						  , SPI_Datalink(SCK, MISO, MOSI, SS){}
		virtual ~SPI_Datalink_Slave(){}
		void Setup_SPI_Slave()
		{
			spi_rx_buf = m_SPI_Slave.allocDMABuffer(SPI_MAX_SERIALIZED_BYTES);
			spi_tx_buf = m_SPI_Slave.allocDMABuffer(SPI_MAX_SERIALIZED_BYTES);
			memset(spi_rx_buf, 0, SPI_MAX_SERIALIZED_BYTES);
			memset(spi_tx_buf, 0, SPI_MAX_SERIALIZED_BYTES);
			m_SPI_Slave.setDMAChannel(2);
			m_SPI_Slave.setMaxTransferSize(SPI_MAX_SERIALIZED_BYTES);
			m_SPI_Slave.setDataMode(SPI_MODE0);
			m_SPI_Slave.begin(HSPI, m_SCK, m_MISO, m_MOSI, m_SS);
			Serial << "SPI Slave Begin: " << m_SCK << " " << m_MISO << " " << m_MOSI << " " << m_SS << "\n";
			xTaskCreatePinnedToCore
			(
				static_task_wait_spi,
				"task_wait_spi",
				10000,
				this,
				configMAX_PRIORITIES-1,
				&task_handle_wait_spi,
				0
			);
			xTaskCreatePinnedToCore
			(
				static_task_process_buffer,
				"task_process_buffer",
				10000,
				this,
				configMAX_PRIORITIES-1,
				&task_handle_process_buffer,
				0
			);
			xTaskNotifyGive(task_handle_wait_spi);
				
		}
		size_t TransferBytes(uint8_t *RXBuffer, uint8_t *TXBuffer, size_t Length);
		void RegisterForDataTransferNotification(SPI_Slave_Notifier *Notifiee)
		{
			m_Notifiee = Notifiee;
		}
	protected:
		uint8_t* spi_tx_buf;
		uint8_t* spi_rx_buf;
	private:
		SPI_Slave_Notifier *m_Notifiee = NULL;
		ESP32DMASPI::Slave m_SPI_Slave;
		TaskHandle_t task_handle_process_buffer = 0;
		TaskHandle_t task_handle_wait_spi = 0;
		static void static_task_wait_spi(void* pvParameters);
		void task_wait_spi();
		static void static_task_process_buffer(void* pvParameters);
		void task_process_buffer();
};

class AudioBuffer: public NamedItem
{
	public:
		AudioBuffer( String Title)
				   : NamedItem(Title)
				   {}
		virtual ~AudioBuffer(){}
		void Initialize()
		{			
			if(0 != pthread_mutex_init(&m_Lock, NULL))
			{
			   ESP_LOGE("AudioBuffer", "Failed to Create Lock");
			}
			ClearAudioBuffer();
		}
		size_t GetFrameCapacity();
		bool ClearAudioBuffer();
		size_t GetFrameCount();
		size_t GetFreeFrameCount();
		size_t WriteAudioFrames( Frame_t *FrameBuffer, size_t FrameCount );
		bool WriteAudioFrame( Frame_t &FrameBuffer );
		size_t ReadAudioFrames(Frame_t *FrameBuffer, size_t FrameCount);
		bfs::optional<Frame_t> ReadAudioFrame();
	private:
		bfs::CircleBuf<Frame_t, AUDIO_BUFFER_LENGTH> m_CircularAudioBuffer;
		pthread_mutex_t m_Lock;
};

class AudioStreamRequester: public NamedItem
						  , public SPI_Datalink_Master
{
	public:
		AudioStreamRequester( String Title
							, AudioBuffer &AudioBuffer
							, uint32_t MISO
							, uint8_t MOSI
							, uint8_t SCK
							, uint8_t SS )
							: NamedItem(Title)
							, m_AudioBuffer(AudioBuffer)
							, SPI_Datalink_Master(Title + "SPI", MISO, MOSI, SCK, SS){}
		virtual ~AudioStreamRequester(){}
		size_t DataBufferBytesAvailable(){}
		size_t BufferMoreAudio();
		size_t GetFrameCount();
		size_t GetAudioFrames(Frame_t *FrameBuffer, size_t FrameCount);
		void Setup()
		{
			ESP_LOGE("AudioStreamRequester", "Setup");
			Setup_SPI_Master();
		}
	private:
		AudioBuffer &m_AudioBuffer;
};

class AudioStreamSender: public NamedItem
					   , public SPI_Datalink_Slave
					   , public SPI_Slave_Notifier
{
	public:
		AudioStreamSender( String Title
						 , AudioBuffer &AudioBuffer
						 , uint32_t MISO
						 , uint8_t MOSI
						 , uint8_t SCK
						 , uint8_t SS )
						 : NamedItem(Title)
						 , m_AudioBuffer(AudioBuffer)
						 , SPI_Datalink_Slave(Title + "SPI", MISO, MOSI, SCK, SS){}
		virtual ~AudioStreamSender(){}
		void Setup()
		{
			ESP_LOGE("AudioStreamRequester", "Setup");
			Setup_SPI_Slave();
			RegisterForDataTransferNotification(this);
		}
		
		size_t SendBytesTransferNotification(uint8_t *TXBuffer, size_t MaxBytesToSend)
		{	
			size_t AvailableFrameCount = m_AudioBuffer.GetFrameCount();
			size_t MaxFramesToSend = MaxBytesToSend / sizeof(Frame_t);
			size_t MaxFramesPerPacket = MAX_FRAMES_PER_PACKET;
			size_t MinFramesToSend = min( min( AvailableFrameCount, MaxFramesToSend ), MaxFramesPerPacket);
			size_t FramesBuffered = m_AudioBuffer.ReadAudioFrames( (Frame_t *)TXBuffer, MinFramesToSend);
			size_t ByteLength = FramesBuffered * sizeof(Frame_t);
			return ByteLength;
		}
		size_t ReceivedBytesTransferNotification(uint8_t *RXBuffer, size_t BytesReceived)
		{
			return 0;
		}
	private:
		AudioBuffer &m_AudioBuffer;
};

#endif