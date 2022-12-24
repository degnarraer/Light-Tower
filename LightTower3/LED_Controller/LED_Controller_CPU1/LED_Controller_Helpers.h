
#ifndef LED_Controller_Helpers_H
#define LED_Controller_Helpers_H


BluetoothA2DPSink m_BTSink;
Bluetooth_Sink m_BT_In = Bluetooth_Sink( "Bluetooth"
                                       , m_BTSink
                                       , "LED Tower of Power"
                                       , I2S_NUM_1                          // I2S Interface
                                       , i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_TX)
                                       , I2S_SAMPLE_RATE
                                       , I2S_BITS_PER_SAMPLE_16BIT
                                       , I2S_CHANNEL_FMT_RIGHT_LEFT
                                       , i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB)
                                       , I2S_CHANNEL_STEREO
                                       , true                               // Use APLL                                    
                                       , I2S_BUFFER_COUNT                   // Buffer Count
                                       , I2S_CHANNEL_SAMPLE_COUNT           // Buffer Size
                                       , I2S2_SCLCK_PIN                     // Serial Clock Pin
                                       , I2S2_WD_PIN                        // Word Selection Pin
                                       , I2S2_SDIN_PIN                      // Serial Data In Pin
                                       , I2S2_SDOUT_PIN );                  // Serial Data Out Pin
                                    
//Callbacks for Bluetooth Sink
void data_received_callback() 
{
  m_BT_In.data_received_callback();
}

//Callbacks for Bluetooth Sink
void read_data_stream(const uint8_t *data, uint32_t length)
{
  m_BT_In.read_data_stream(data, length);
}

//Callbacks for Bluetooth Sink
// for esp_a2d_connection_state_t see https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/bluetooth/esp_a2dp.html#_CPPv426esp_a2d_connection_state_t
void connection_state_changed(esp_a2d_connection_state_t state, void *ptr)
{
  ESP_LOGD("Startup", "State: %s", m_BTSink.to_str(state));
}

//Callbacks for Bluetooth Sink
// for esp_a2d_audio_state_t see https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/bluetooth/esp_a2dp.html#_CPPv421esp_a2d_audio_state_t
void audio_state_changed(esp_a2d_audio_state_t state, void *ptr)
{
  ESP_LOGD("Startup", "State: %s", m_BTSink.to_str(state));
}


I2S_Device m_Mic_In = I2S_Device( "Microphone"
                                , I2S_NUM_0                          // I2S Interface
                                , i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX)
                                , I2S_SAMPLE_RATE
                                , I2S_BITS_PER_SAMPLE_32BIT
                                , I2S_CHANNEL_FMT_RIGHT_LEFT
                                , i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB)
                                , I2S_CHANNEL_STEREO
                                , true                               // Use APLL
                                , I2S_BUFFER_COUNT                   // Buffer Count
                                , I2S_CHANNEL_SAMPLE_COUNT           // Buffer Size
                                , I2S1_SCLCK_PIN                     // Serial Clock Pin
                                , I2S1_WD_PIN                        // Word Selection Pin
                                , I2S1_SDIN_PIN                      // Serial Data In Pin
                                , I2S1_SDOUT_PIN );                  // Serial Data Out Pin

I2S_Device m_I2S_Out = I2S_Device( "I2S Out"
                                 , I2S_NUM_1                        // I2S Interface
                                 , i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_TX)
                                 , I2S_SAMPLE_RATE
                                 , I2S_BITS_PER_SAMPLE_16BIT
                                 , I2S_CHANNEL_FMT_RIGHT_LEFT
                                 , i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB)
                                 , I2S_CHANNEL_STEREO
                                 , true                            // Use APLL
                                 , I2S_BUFFER_COUNT                // Buffer Count
                                 , I2S_CHANNEL_SAMPLE_COUNT        // Buffer Size
                                 , I2S2_SCLCK_PIN                  // Serial Clock Pin
                                 , I2S2_WD_PIN                     // Word Selection Pin
                                 , I2S2_SDIN_PIN                   // Serial Data In Pin
                                 , I2S2_SDOUT_PIN );               // Serial Data Out Pin
                                  
StatisticalEngine m_StatisticalEngine = StatisticalEngine();
StatisticalEngineModelInterface m_StatisticalEngineModelInterface = StatisticalEngineModelInterface(m_StatisticalEngine);
VisualizationPlayer m_VisualizationPlayer = VisualizationPlayer(m_StatisticalEngineModelInterface);

SPIDataLinkSlave m_SPIDataLinkSlave = SPIDataLinkSlave( );
                                                      
CalculateFPS m_CalculateFPS("Main Loop", 1000);
TaskScheduler m_Scheduler;

Manager m_Manager = Manager("Manager", m_StatisticalEngine
                           , m_SPIDataLinkSlave
                           , m_BT_In
                           , m_Mic_In
                           , m_I2S_Out);


void InitLocalVariables()
{
  m_BTSink.set_stream_reader(read_data_stream, true);
  m_BTSink.set_on_data_received(data_received_callback);  
  m_Manager.Setup();
  m_SPIDataLinkSlave.SetupSPIDataLink();
  m_Scheduler.AddTask(m_CalculateFPS);
  m_Scheduler.AddTask(m_StatisticalEngineModelInterface);
  m_Scheduler.AddTask(m_VisualizationPlayer);
}

void PrintStartupData()
{
  ESP_LOGV("LED_Controller1", "%s, ", __func__);
  ESP_LOGI("LED_Controller1", "Serial Datalink Configured");
  ESP_LOGI("LED_Controller1", "Xtal Clock Frequency: %i MHz", getXtalFrequencyMhz());
  ESP_LOGI("LED_Controller1", "CPU Clock Frequency: %i MHz", getCpuFrequencyMhz());
  ESP_LOGI("LED_Controller1", "Apb Clock Frequency: %i Hz", getApbFrequency());
}
void PrintFreeHeap()
{
  ESP_LOGE("LED_Controller_CPU1", "Total heap: %d", ESP.getHeapSize());
  ESP_LOGE("LED_Controller_CPU1", "Free heap: %d", ESP.getFreeHeap());
  ESP_LOGE("LED_Controller_CPU1", "Total PSRAM: %d", ESP.getPsramSize());
  ESP_LOGE("LED_Controller_CPU1", "Free PSRAM: %d", ESP.getFreePsram());
}

void InitSerialCommunication()
{
  //PC Serial Communication
  Serial.flush();
  Serial.begin(500000);
  Serial.flush();
}


#endif
