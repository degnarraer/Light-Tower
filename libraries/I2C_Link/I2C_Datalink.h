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

#define AUDIO_BUFFER_LENGTH 4096
#define I2C_MAX_BYTES 4096
#define SPI_MAX_DATA_BYTES 4096
#define MAX_FRAMES_PER_PACKET 1024

class SPI_Slave_Notifier
{
	public:
		SPI_Slave_Notifier(){}
		virtual ~SPI_Slave_Notifier(){}
		virtual size_t SendBytesTransferNotification(uint8_t *TXBuffer, size_t MaxBytesToSend) = 0;
		virtual size_t ReceivedBytesTransferNotification(uint8_t *RXBuffer, size_t BytesReceived) = 0;
	private:
};

class SPI_Datalink
{
	public:
		SPI_Datalink( uint8_t MISO
					, uint8_t MOSI
					, uint8_t SCK
					, uint8_t SS )
					: m_MOSI(MOSI)
					, m_MISO(MISO)
					, m_SCK(SCK)
					, m_SS(SS){}
		virtual ~SPI_Datalink(){}
		virtual size_t TransferBytes(uint8_t *RXBuffer, uint8_t *TXBuffer, size_t Length) = 0;
	private:
	protected:
		uint8_t m_MISO;
		uint8_t m_MOSI;
		uint8_t m_SCK;
		uint8_t m_SS;
		static const int m_spiClk = 1000000; // 1 MHz
		static const uint32_t BUFFER_SIZE = SPI_MAX_DATA_BYTES;
};

class SPI_Datalink_Master: public NamedItem
						 , public SPI_Datalink
{
	public:
		SPI_Datalink_Master( String Title
						   , uint8_t MISO
						   , uint8_t MOSI
						   , uint8_t SCK
						   , uint8_t SS )
						   : NamedItem(Title)
						   , SPI_Datalink(MISO, MOSI, SCK, SS){}
		virtual ~SPI_Datalink_Master(){}
		void Setup_SPI_Master()
		{
			m_SPI_Master.setDMAChannel(1);
			spi_tx_buf = m_SPI_Master.allocDMABuffer(BUFFER_SIZE);
			spi_rx_buf = m_SPI_Master.allocDMABuffer(BUFFER_SIZE);
			m_SPI_Master.setDataMode(SPI_MODE0);
			m_SPI_Master.setFrequency(m_spiClk);
			m_SPI_Master.setMaxTransferSize(BUFFER_SIZE);
			m_SPI_Master.begin(HSPI, m_SCK, m_MISO, m_MOSI, m_SS);
		}
		size_t TransferBytes(uint8_t *RXBuffer, uint8_t *TXBuffer, size_t Length);
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
						  , uint8_t MISO
						  , uint8_t MOSI
						  , uint8_t SCK
						  , uint8_t SS )
						  : NamedItem(Title)
						  , SPI_Datalink(MISO, MOSI, SCK, SS ){}
		virtual ~SPI_Datalink_Slave(){}
		void Setup_SPI_Slave()
		{
			m_SPI_Slave.setDMAChannel(1);
			m_SPI_Slave.setDataMode(SPI_MODE0);
			m_SPI_Slave.setMaxTransferSize(BUFFER_SIZE);
			spi_tx_buf = m_SPI_Slave.allocDMABuffer(BUFFER_SIZE);
			spi_rx_buf = m_SPI_Slave.allocDMABuffer(BUFFER_SIZE);
			m_SPI_Slave.begin(HSPI, m_SCK, m_MISO, m_MOSI, m_SS);
			xTaskCreatePinnedToCore
			(
				static_task_wait_spi,
				"task_wait_spi",
				4096,
				this,
				configMAX_PRIORITIES-1,
				&task_handle_wait_spi,
				0
			);
			xTaskCreatePinnedToCore
			(
				static_task_process_buffer,
				"task_process_buffer",
				4096,
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
			Serial << "Available Frames: " << AvailableFrameCount << "\n";
			size_t FramesBuffered = m_AudioBuffer.ReadAudioFrames((Frame_t *)TXBuffer, min(AvailableFrameCount, MaxFramesToSend));
			size_t ByteLength = FramesBuffered * sizeof(Frame_t);
			return ByteLength;
		}
		size_t ReceivedBytesTransferNotification(uint8_t *RXBuffer, size_t BytesReceived)
		{
			return sizeof 0;
		}
	private:
		AudioBuffer &m_AudioBuffer;
};















class I2C_Datalink
{
  public:
    I2C_Datalink( uint8_t SDA_Pin, uint8_t SCL_Pin )
                : m_SDA_PIN(SDA_Pin)
                , m_SCL_PIN(SCL_Pin){}
    virtual ~I2C_Datalink(){}
  protected:
    uint8_t m_SDA_PIN;
    uint8_t m_SCL_PIN;
    uint8_t m_I2C_Address;
    uint32_t m_Freq;
    uint8_t m_RequestAttempts;
    uint8_t m_RequestTimeout;
};

class I2C_Datalink_Master: public NamedItem
                         , public I2C_Datalink
{
  public:
    I2C_Datalink_Master( String Title, TwoWire &TwoWire, uint8_t SDA_Pin, uint8_t SCL_Pin )
                       : NamedItem(Title)
                       , I2C_Datalink(SDA_Pin, SCL_Pin)
                       , m_TwoWire(&TwoWire){}
    virtual ~I2C_Datalink_Master(){}

    //Master Functions
	size_t ReadDataFromSlave(uint8_t SlaveAddress, unsigned char *data, size_t count);
    void WriteDataToSlave(uint8_t SlaveAddress, unsigned char *data, size_t ByteCount);
  protected:
    void SetupMaster(uint32_t Freq, uint8_t RequestAttempts, uint8_t RequestTimeout);
  private:  
	TwoWire *m_TwoWire = NULL;
};

class I2C_Datalink_Slave: public NamedItem
                        , public I2C_Datalink
{
  public:
    I2C_Datalink_Slave( String Title, TwoWireSlave &TwoWireSlave, uint8_t SDA_Pin, uint8_t SCL_Pin )
                      : NamedItem(Title)
                      , I2C_Datalink(SDA_Pin, SCL_Pin)
                      , m_TwoWireSlave(&TwoWireSlave){}
    virtual ~I2C_Datalink_Slave(){}
  protected:
    void UpdateI2C();
	void SetupSlave( uint8_t My_Address, TwoWireSlaveNotifiee *TwoWireSlaveNotifiee );
    TwoWireSlave *m_TwoWireSlave = NULL;
};


#endif