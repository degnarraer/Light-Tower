/*
  Streaming of sound data with Bluetooth to other Bluetooth device.
  We generate 2 tones which will be sent to the 2 channels.
  
  Copyright (C) 2020 Phil Schatzmann
  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <BluetoothA2DPSource.h>
#include <math.h>

#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include "esp_log.h"
static const char *TAG = "EXAMPLE";

#define c3_frequency  130.81

BluetoothA2DPSource a2dp_source;

// The supported audio codec in ESP32 A2DP is SBC. SBC audio stream is encoded
// from PCM data normally formatted as 44.1kHz sampling rate, two-channel 16-bit sample data
int32_t get_data_channels(Frame *frame, int32_t channel_len) {
    static double m_time = 0.0;
    double m_amplitude = 10000.0;  // -32,768 to 32,767
    double m_deltaTime = 1.0 / 44100.0;
    double m_phase = 0.0;
    double double_Pi = PI * 2.0;
    // fill the channel data
    for (int sample = 0; sample < channel_len; ++sample) {
        double angle = double_Pi * c3_frequency * m_time + m_phase;
        frame[sample].channel1 = m_amplitude * sin(angle);
        frame[sample].channel2 = frame[sample].channel1;
        m_time += m_deltaTime;
    }

    return channel_len;
}

unsigned long int currentTime = 0;
void setup() {
  //PC Serial Communication
  Serial.begin(500000); 
  ESP_LOGD(TAG, "%s, ", __func__);
  a2dp_source.set_ssp_enabled(true);
  a2dp_source.set_local_name("LED Tower of Power");
  a2dp_source.set_nvs_init(true);
  a2dp_source.set_reset_ble(true);
  a2dp_source.set_pin_code("1234");
  a2dp_source.start("Soundbar", get_data_channels);
  currentTime = millis();
}

void loop() {
  if(millis() - currentTime > 1000)
  {
    currentTime = millis();
  }
}
