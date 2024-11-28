#ifndef FFT_CALCULATOR_H
#define FFT_CALCULATOR_H

#include <Arduino.h>
#include <DataTypes.h>
#include <cmath>
#include <cstring>
#include <cassert>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

// Include FFT libraries
#include <arduinoFFT.h>
#include <kiss_fft.h>
#include <dsp/dsp.h>

// FFT Type Enumeration
enum class FFT_Type
{
    ArduinoFFT,
    KissFFT,
    ESPDSP
};

class FFT_Calculator
{
public:
    FFT_Calculator(int32_t FFT_Size, int32_t SampleRate, BitLength_t BitLength, FFT_Type fftType)
        : m_FFT_Size(FFT_Size), m_FFT_SampleRate(SampleRate), m_FFT_Type(fftType)
    {
        // Allocate buffers
        mp_RealBuffer = (float *)malloc(sizeof(float) * m_FFT_Size);
        mp_ImaginaryBuffer = (float *)malloc(sizeof(float) * m_FFT_Size);

        // Initialize semaphore
        m_Semaphore = xSemaphoreCreateMutex();
        if (!m_Semaphore)
        {
            ESP_LOGE("FFT_Calculator", "ERROR! Unable to create Semaphore.");
        }

        // Configure bit length
        switch (BitLength)
        {
        case BitLength_32:
            m_BitLengthMaxValue = 1 << 32;
            break;
        case BitLength_16:
            m_BitLengthMaxValue = 1 << 16;
            break;
        case BitLength_8:
            m_BitLengthMaxValue = 1 << 8;
            break;
        default:
            m_BitLengthMaxValue = 1 << 32;
            break;
        }

        // Initialize based on FFT type
        switch (m_FFT_Type)
        {
        case FFT_Type::ArduinoFFT:
            m_ArduinoFFT = new ArduinoFFT<float>(mp_RealBuffer, mp_ImaginaryBuffer, m_FFT_Size, m_FFT_SampleRate);
            break;
        case FFT_Type::KissFFT:
            m_KissCfg = kiss_fft_alloc(m_FFT_Size, 0, nullptr, nullptr);
            break;
        case FFT_Type::ESPDSP:
            // No additional initialization needed for ESP-DSP
            break;
        default:
            ESP_LOGE("FFT_Calculator", "ERROR! Unknown FFT type.");
            break;
        }
    }

    virtual ~FFT_Calculator()
    {
        if (m_Semaphore)
        {
            vSemaphoreDelete(m_Semaphore);
        }
        if (mp_RealBuffer)
        {
            free(mp_RealBuffer);
        }
        if (mp_ImaginaryBuffer)
        {
            free(mp_ImaginaryBuffer);
        }
        if (m_ArduinoFFT)
        {
            delete m_ArduinoFFT;
        }
        if (m_KissCfg)
        {
            free(m_KissCfg);
        }
    }

    bool CalculateFFT(float *inputBuffer, size_t inputSize, float gain)
    {
        if (inputSize != m_FFT_Size)
        {
            ESP_LOGE("FFT_Calculator", "ERROR! Input size does not match FFT size.");
            return false;
        }

        if (xSemaphoreTakeRecursive(m_Semaphore, pdMS_TO_TICKS(5)) == pdTRUE)
        {
            // Populate real and imaginary buffers
            for (size_t i = 0; i < m_FFT_Size; ++i)
            {
                mp_RealBuffer[i] = inputBuffer[i] * gain;
                mp_ImaginaryBuffer[i] = 0.0f;
            }

            switch (m_FFT_Type)
            {
            case FFT_Type::ArduinoFFT:
                m_ArduinoFFT->compute(FFTDirection::Forward);
                m_ArduinoFFT->complexToMagnitude();
                m_MajorPeak = m_ArduinoFFT->majorPeak();
                break;

            case FFT_Type::KissFFT:
            {
                kiss_fft_cpx *kissInput = new kiss_fft_cpx[m_FFT_Size];
                kiss_fft_cpx *kissOutput = new kiss_fft_cpx[m_FFT_Size];

                for (size_t i = 0; i < m_FFT_Size; ++i)
                {
                    kissInput[i].r = mp_RealBuffer[i];
                    kissInput[i].i = mp_ImaginaryBuffer[i];
                }

                kiss_fft(m_KissCfg, kissInput, kissOutput);

                for (size_t i = 0; i < m_FFT_Size; ++i)
                {
                    mp_RealBuffer[i] = sqrtf(kissOutput[i].r * kissOutput[i].r + kissOutput[i].i * kissOutput[i].i);
                }

                delete[] kissInput;
                delete[] kissOutput;
                break;
            }

            case FFT_Type::ESPDSP:
                dsps_fft2r_init_fc32(NULL, m_FFT_Size);
                dsps_fft2r_fc32(mp_RealBuffer, m_FFT_Size);
                dsps_bit_rev_fc32(mp_RealBuffer, m_FFT_Size);
                dsps_cplx2reC_fc32(mp_RealBuffer, m_FFT_Size);
                for (size_t i = 0; i < m_FFT_Size; ++i)
                {
                    mp_RealBuffer[i] = sqrtf(mp_RealBuffer[2 * i] * mp_RealBuffer[2 * i] + mp_RealBuffer[2 * i + 1] * mp_RealBuffer[2 * i + 1]);
                }
                break;

            default:
                ESP_LOGE("FFT_Calculator", "ERROR! Unsupported FFT type.");
                break;
            }

            xSemaphoreGiveRecursive(m_Semaphore);
            return true;
        }
        else
        {
            ESP_LOGW("FFT_Calculator", "WARNING! Failed to take semaphore.");
            return false;
        }
    }

    float GetFFTBufferValue(int32_t index)
    {
        float value = 0.0f;
        if (xSemaphoreTakeRecursive(m_Semaphore, pdMS_TO_TICKS(5)) == pdTRUE)
        {
            assert(index < m_FFT_Size / 2);
            value = mp_RealBuffer[index];
            xSemaphoreGiveRecursive(m_Semaphore);
        }
        else
        {
            ESP_LOGW("FFT_Calculator", "WARNING! Failed to take semaphore.");
        }
        return value;
    }

    float GetMajorPeak()
    {
        return m_MajorPeak;
    }

private:
    FFT_Type m_FFT_Type;
    uint32_t m_FFT_Size;
    uint32_t m_FFT_SampleRate;
    float *mp_RealBuffer;
    float *mp_ImaginaryBuffer;
    int32_t m_BitLengthMaxValue;
    float m_MajorPeak = 0.0f;

    // ArduinoFFT
    ArduinoFFT<float> *m_ArduinoFFT = nullptr;

    // kissFFT
    kiss_fft_cfg m_KissCfg = nullptr;

    // Semaphore for thread safety
    SemaphoreHandle_t m_Semaphore = nullptr;
};

#endif
