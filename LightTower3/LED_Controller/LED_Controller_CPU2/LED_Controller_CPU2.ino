#include "Manager.h"
#include "Tunes.h"
#include "esp_log.h"

TaskHandle_t ManagerTask;
TaskHandle_t ProcessSoundPowerTask;
TaskHandle_t ProcessFFTTask;
TaskHandle_t SPIDataLinkMasterTXTask;

I2S_Device m_I2S_In = I2S_Device( "I2S_In"
                                 , I2S_NUM_1
                                 , i2s_mode_t(I2S_MODE_SLAVE | I2S_MODE_RX)
                                 , I2S_SAMPLE_RATE
                                 , I2S_BITS_PER_SAMPLE_16BIT
                                 , I2S_CHANNEL_FMT_RIGHT_LEFT
                                 , i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB)
                                 , I2S_CHANNEL_STEREO
                                 , true                               // Use APLL
                                 , I2S_BUFFER_COUNT                   // Buffer Count
                                 , I2S_SAMPLE_COUNT                   // Buffer Size
                                 , I2S1_SCLCK_PIN                     // Serial Clock Pin
                                 , I2S1_WD_PIN                        // Word Selection Pin
                                 , I2S1_SDIN_PIN                      // Serial Data In Pin
                                 , I2S1_SDOUT_PIN );                  // Serial Data Out Pin 

BluetoothA2DPSource a2dp_source;
Bluetooth_Source m_BT_Out = Bluetooth_Source( "Bluetooth Source"
                                            , a2dp_source
                                            , "AL HydraMini" );

SPIDataLinkMaster m_SPIDataLinkMaster = SPIDataLinkMaster( "SPI Datalink", 15, 17, 18, 19, 0);
                                             
Sound_Processor m_SoundProcessor = Sound_Processor( "Sound Processor"
                                                  , m_SPIDataLinkMaster );                                            
Manager m_Manager = Manager("Manager"
                           , m_SoundProcessor
                           , m_SPIDataLinkMaster
                           , m_BT_Out
                           , m_I2S_In);


int32_t SetBTTxData(uint8_t *Data, int32_t channel_len)
{
  return m_Manager.SetBTTxData(Data, channel_len);
}

void setup() 
{  
  //PC Serial Communication
  Serial.flush();
  Serial.begin(500000); // 9600 bps, 8 bits no parity 1 stop bit
  Serial.flush();
  
  ESP_LOGV("LED_Controller2", "%s, ", __func__);
  ESP_LOGE("LED_Controller2", "Serial Datalink Configured");
  ESP_LOGE("LED_Controller2", "Xtal Clock Frequency: %i MHz", getXtalFrequencyMhz());
  ESP_LOGE("LED_Controller2", "CPU Clock Frequency: %i MHz", getCpuFrequencyMhz());
  ESP_LOGE("LED_Controller2", "Apb Clock Frequency: %i Hz", getApbFrequency());

  m_I2S_In.Setup();
  m_BT_Out.SetCallback(SetBTTxData);
  m_BT_Out.Setup();
  m_SoundProcessor.SetupSoundProcessor();
  m_SPIDataLinkMaster.SetupSPIDataLink();
  m_Manager.Setup();
  
  xTaskCreatePinnedToCore( ManagerTaskLoop, "ManagerTask", 20000, NULL, configMAX_PRIORITIES-1, &ManagerTask, 1); 
  xTaskCreatePinnedToCore( SPIDataLinkMasterTXTaskLoop, "SPIDataLinkMasterTXTask", 4000, NULL, configMAX_PRIORITIES - 2, &SPIDataLinkMasterTXTask, 1);
  xTaskCreatePinnedToCore( ProcessSoundPowerTaskLoop, "ProcessSoundPowerTask", 10000, NULL, configMAX_PRIORITIES - 1, &ProcessSoundPowerTask, 0);
  xTaskCreatePinnedToCore( ProcessFFTTaskLoop, "ProcessFFTTask", 4000, NULL, configMAX_PRIORITIES - 1, &ProcessFFTTask, 0);
  
  ESP_LOGE("LED_Controller_CPU2", "Total heap: %d", ESP.getHeapSize());
  ESP_LOGE("LED_Controller_CPU2", "Free heap: %d", ESP.getFreeHeap());
  ESP_LOGE("LED_Controller_CPU2", "Total PSRAM: %d", ESP.getPsramSize());
  ESP_LOGE("LED_Controller_CPU2", "Free PSRAM: %d", ESP.getFreePsram());
}

void loop()
{
}

void ProcessSoundPowerTaskLoop(void * parameter)
{
  while(true)
  {
    m_SoundProcessor.ProcessSoundPower();
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

void ProcessFFTTaskLoop(void * parameter)
{
  while(true)
  {
    m_SoundProcessor.ProcessFFT();
    vTaskDelay(50 / portTICK_PERIOD_MS);
  }
}

void ManagerTaskLoop(void * parameter)
{
  while(true)
  {
    m_Manager.ProcessEventQueue();
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
}

void SPIDataLinkMasterTXTaskLoop(void * parameter)
{
  while(true)
  {
    m_SPIDataLinkMaster.ProcessTXEventQueue();
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
}
