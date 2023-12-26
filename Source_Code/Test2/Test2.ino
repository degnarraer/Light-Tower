#include "DataItem.h"


#define CPU1_RX                   12
#define CPU1_TX                   13
#define CPU2_RX                   14
#define CPU2_TX                   15

DataSerializer m_DataSerializer;
SerialPortMessageManager m_CPU1SerialPortMessageManager = SerialPortMessageManager("CPU1", Serial1, m_DataSerializer);
SerialPortMessageManager m_CPU2SerialPortMessageManager = SerialPortMessageManager("CPU2", Serial2, m_DataSerializer);

DataItem <float, 1> m_AmplitudeGain = DataItem<float, 1>( "Amplitude Gain"
                                                        , 0
                                                        , RxTxType_Rx
                                                        , 1000
                                                        , 500
                                                        , m_CPU2SerialPortMessageManager);
                                                        
DataItem <float, 1> m_FFTGain = DataItem<float, 1>( "FFT Gain"
                                                  , 0
                                                  , RxTxType_Rx
                                                  , 1000
                                                  , 500
                                                  , m_CPU2SerialPortMessageManager);
                                                  
DataItem<ConnectionStatus_t, 1> m_ConnectionStatus = DataItem<ConnectionStatus_t, 1>( "Connection Status"
                                                                                    , Disconnected
                                                                                    , RxTxType_Rx
                                                                                    , 0
                                                                                    , 1000
                                                                                    , m_CPU2SerialPortMessageManager);

                                              
    
void setup()
{
  Serial.begin(500000);
  Serial.flush();
  Serial1.begin(500000, SERIAL_8N1, CPU1_RX, CPU1_TX);
  Serial1.flush();
  Serial2.begin(500000, SERIAL_8N1, CPU2_RX, CPU2_TX);
  Serial2.flush();
  m_CPU1SerialPortMessageManager.SetupSerialPortMessageManager();
  m_CPU2SerialPortMessageManager.SetupSerialPortMessageManager();

}

void loop()
{

}
