#include "DataItem.h"


#define CPU1_RX                   12
#define CPU1_TX                   13
#define CPU3_RX                   14
#define CPU3_TX                   15

DataSerializer m_DataSerializer;
SerialPortMessageManager m_CPU3SerialPortMessageManager = SerialPortMessageManager("CPU3", Serial2, m_DataSerializer);
DataItem <float, 1> m_FFTGain = DataItem<float, 1>("FFT Gain", 0, RxTxType_Tx_Periodic, 1000, m_CPU3SerialPortMessageManager);
    
void setup()
{
  Serial.begin(500000);
  Serial.flush();
  Serial1.begin(500000, SERIAL_8N1, CPU1_RX, CPU1_TX);
  Serial1.flush();
  Serial2.begin(500000, SERIAL_8N1, CPU3_RX, CPU3_TX);
  Serial2.flush();
  m_CPU3SerialPortMessageManager.SetupSerialPortMessageManager();

}

void loop()
{

}
