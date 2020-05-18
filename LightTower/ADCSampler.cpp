#include "ADCSampler.h"

ADCSampler::ADCSampler()
{
  dataReady = false;
  adcDMAIndex = 0;
  adcTransferIndex = 0;
  for (int i = 0; i < NUMBER_OF_BUFFERS; i++)
  {
    memset((void *)adcBuffer[i], 0, BUFFER_SIZE);
  }
}
void ADCSampler::SetSampleRateAndStart(unsigned int samplingRate)
{
  this->samplingRate = samplingRate;
  // Turning devices Timer on.
  pmc_enable_periph_clk(ID_TC1);


  // Configure timer
  TC_Configure(TC0, 1, TC_CMR_WAVE | TC_CMR_WAVSEL_UP_RC | TC_CMR_ACPA_CLEAR | TC_CMR_ACPC_SET | TC_CMR_ASWTRG_CLEAR | TC_CMR_TCCLKS_TIMER_CLOCK1);

  // It is good to have the timer 0 on PIN2, good for Debugging
  //int result = PIO_Configure( PIOB, PIO_PERIPH_B, PIO_PB25B_TIOA1, PIO_DEFAULT);

  // Configure ADC pin A7
  //  the below code is taken from adc_init(ADC, SystemCoreClock, ADC_FREQ_MAX, ADC_STARTUP_FAST);

  ADC->ADC_CR = ADC_CR_SWRST;         // Reset the controller.
  ADC->ADC_MR = 0;                    // Reset Mode Register.
  ADC->ADC_PTCR = (ADC_PTCR_RXTDIS | ADC_PTCR_TXTDIS); // Reset PDC transfer.

  ADC->ADC_MR |= ADC_MR_PRESCAL(3);   // ADC clock = MSCK/((PRESCAL+1)*2), 13 -> 750000 Sps
  ADC->ADC_MR |= ADC_MR_STARTUP_SUT0; // What is this by the way?
  ADC->ADC_MR |= ADC_MR_TRACKTIM(15);
  ADC->ADC_MR |= ADC_MR_TRANSFER(1);
  ADC->ADC_MR |= ADC_MR_TRGEN_EN;
  ADC->ADC_MR |= ADC_MR_TRGSEL_ADC_TRIG2; // selecting TIOA1 as trigger.
  ADC->ADC_CHER = ADC_CHANNELS;

  /* Interupts */
  ADC->ADC_IDR   = ~ADC_IDR_ENDRX;
  ADC->ADC_IER   =  ADC_IER_ENDRX;
  /* Waiting for ENDRX as end of the transfer is set
    when the current DMA transfer is done (RCR = 0), i.e. it doesn't include the
    next DMA transfer.
    If we trigger on RXBUFF This flag is set if there is no more DMA transfer in
    progress (RCR = RNCR = 0). Hence we may miss samples.
  */

  
  unsigned int cycles = 42000000 / samplingRate;

  /*  timing of ADC */
  TC_SetRC(TC0, 1, cycles);      // TIOA0 goes HIGH on RC.
  TC_SetRA(TC0, 1, cycles / 2);  // TIOA0 goes LOW  on RA.

  // We have to reinitalise just in case the Sampler is stopped and restarted...
  dataReady = false;
  adcDMAIndex = 0;
  adcTransferIndex = 0;
  for (int i = 0; i < NUMBER_OF_BUFFERS; i++)
  {
    memset((void *)adcBuffer[i], 0, BUFFER_SIZE);
  }

  ADC->ADC_RPR  = (unsigned long) adcBuffer[adcDMAIndex % NUMBER_OF_BUFFERS];  // DMA buffer
  ADC->ADC_RCR  = (unsigned int)  BUFFER_SIZE;  // ADC works in half-word mode.
  ADC->ADC_RNPR = (unsigned long) adcBuffer[(adcDMAIndex + 1) % NUMBER_OF_BUFFERS];  // next DMA buffer
  ADC->ADC_RNCR = (unsigned int)  BUFFER_SIZE;

  // Enable interrupts
  NVIC_EnableIRQ(ADC_IRQn);
  ADC->ADC_PTCR  =  ADC_PTCR_RXTEN;  // Enable receiving data.
  ADC->ADC_CR   |=  ADC_CR_START;    //start waiting for trigger.

  // Start timer
  TC0->TC_CHANNEL[1].TC_SR;
  TC0->TC_CHANNEL[1].TC_CCR = TC_CCR_CLKEN;
  TC_Start(TC0, 1);
}
void ADCSampler::End()
{

}

void ADCSampler::HandleInterrupt()
{
  unsigned long status = ADC->ADC_ISR;
  if (status & ADC_ISR_ENDRX)
  {
    StartNextBuffer();
    dataReady = true;
  }
}
void ADCSampler::StartNextBuffer()
{
  bufferOverflow = ((adcDMAIndex - adcTransferIndex) >= NUMBER_OF_BUFFERS ? true : false);
  adcDMAIndex = adcDMAIndex + 1;
  if(true == bufferOverflow) adcTransferIndex += 1;
  ADC->ADC_RNPR  = (unsigned long) adcBuffer[(adcDMAIndex + 1) % NUMBER_OF_BUFFERS];
  ADC->ADC_RNCR  = BUFFER_SIZE;
}
unsigned int ADCSampler::GetNumberOfReadings()
{
  return adcDMAIndex - adcTransferIndex;
}
bool ADCSampler::IsAvailable()
{
  return dataReady;
}
unsigned int ADCSampler::GetSamplingRate()
{
  return samplingRate;
}

uint16_t* ADCSampler::GetFilledBuffer(int *bufferLength)
{
  if(true == debugMode && debugLevel >= 3) Serial << "Read Buffer: " << adcDMAIndex << "|" << adcTransferIndex << "\n";
  *bufferLength = BUFFER_SIZE;
  return adcBuffer[(adcTransferIndex % NUMBER_OF_BUFFERS)];
}

void ADCSampler::SetReadCompleted()
{
  adcTransferIndex += 1;
  if(adcTransferIndex >= adcDMAIndex - 1)
  {
    //if(true == debugMode && debugLevel >= 0) Serial << "3\n";
    dataReady = false;
  }
  else
  {
    //if(true == debugMode && debugLevel >= 0) Serial << "1";
    dataReady = true;
  }
  if(true == debugMode && debugLevel >= 3) Serial << "Read Buffer Done\n";
}
