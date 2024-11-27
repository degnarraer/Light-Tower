#ifndef FFT_CALCULATOR_H
#define FFT_CALCULATOR_H

#include "arduinoFFT.h"
#include <DataTypes.h>
#include "Streaming.h"
#include "esp_system.h"
#include "esp_task_wdt.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

class FFT_Calculator
{
public:
    FFT_Calculator(int32_t FFT_Size, int32_t SampleRate, BitLength_t BitLength)
        : m_FFT_Size(FFT_Size), m_FFT_SampleRate(SampleRate)
    {
        mp_RealBuffer = (float *)malloc(sizeof(float) * m_FFT_Size);
        mp_ImaginaryBuffer = (float *)malloc(sizeof(float) * m_FFT_Size);
        m_MyFFT = new ArduinoFFT<float>(mp_RealBuffer, mp_ImaginaryBuffer, m_FFT_Size, m_FFT_SampleRate);
        m_Semaphore = xSemaphoreCreateMutex();
        if(m_Semaphore == nullptr)
        {
            ESP_LOGE("FFT_Calculator", "ERROR! Unable to create Semaphore.");
        }

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
    }

    virtual ~FFT_Calculator()
    {
        if (m_Semaphore)
        {
            vSemaphoreDelete(m_Semaphore);
            m_Semaphore = nullptr;
        }
        if (mp_RealBuffer)
        {
            free(mp_RealBuffer);
            mp_RealBuffer = nullptr;
        }
        if (mp_ImaginaryBuffer)
        {
            free(mp_ImaginaryBuffer);
            mp_ImaginaryBuffer = nullptr;
        }
        if (m_MyFFT)
        {
            delete m_MyFFT;
            m_MyFFT = nullptr;
        }
    }

    void ResetCalculator()
    {
        if (xSemaphoreTakeRecursive(m_Semaphore, pdMS_TO_TICKS(100)) == pdTRUE)
        {
            m_SolutionReady = false;
            m_CurrentIndex = 0;
            m_MaxFFTBinValue = 0;
            m_MaxFFTBinIndex = 0;
            xSemaphoreGiveRecursive(m_Semaphore);
        }
        else
        {
            ESP_LOGW("Semaphore Take Failure", "WARNING! Failed to take Semaphore");
        }
    }

    int32_t GetFreeSpace() { return m_FFT_Size - m_CurrentIndex; }

    float GetFFTBufferValue(int32_t index)
    {
        float value = 0.0f;
        if (xSemaphoreTakeRecursive(m_Semaphore, pdMS_TO_TICKS(100)) == pdTRUE)
        {
            assert(true == m_SolutionReady);
            assert(index < m_FFT_Size / 2);
            value = mp_RealBuffer[index];
            xSemaphoreGiveRecursive(m_Semaphore);
        }
        else
        {
            ESP_LOGW("Semaphore Take Failure", "WARNING! Failed to take Semaphore");
        }
        return value;
    }

    float GetFFTMaxValue() { return m_MaxFFTBinValue; }

    int32_t GetFFTMaxValueBin()
    {
        int32_t index = 0;
        if (xSemaphoreTakeRecursive(m_Semaphore, pdMS_TO_TICKS(100)) == pdTRUE)
        {
            assert(true == m_SolutionReady);
            index = m_MaxFFTBinIndex;
            xSemaphoreGiveRecursive(m_Semaphore);
        }
        else
        {
            ESP_LOGW("Semaphore Take Failure", "WARNING! Failed to take Semaphore");
        }
        return index;
    }

    float GetMajorPeak()
    {
        float peak = 0.0f;
        if (xSemaphoreTakeRecursive(m_Semaphore, pdMS_TO_TICKS(100)) == pdTRUE)
        {
            assert(true == m_SolutionReady);
            peak = m_MajorPeak;
            xSemaphoreGiveRecursive(m_Semaphore);
        }
        else
        {
            ESP_LOGW("Semaphore Take Failure", "WARNING! Failed to take Semaphore");
        }
        return peak;
    }

    float *GetMajorPeakPointer()
    {
        float *peak = nullptr;
        if (xSemaphoreTakeRecursive(m_Semaphore, pdMS_TO_TICKS(100)) == pdTRUE)
        {
            assert(true == m_SolutionReady);
            peak = &m_MajorPeak;
            xSemaphoreGiveRecursive(m_Semaphore);
        }
        else
        {
            ESP_LOGW("Semaphore Take Failure", "WARNING! Failed to take Semaphore");
        }
        return peak;
    }

    size_t GetRequiredValueCount()
    {
        size_t requiredCount = 0;
        if (xSemaphoreTakeRecursive(m_Semaphore, pdMS_TO_TICKS(100)) == pdTRUE)
        {
            requiredCount = m_FFT_Size - m_CurrentIndex;
            xSemaphoreGiveRecursive(m_Semaphore);
        }
        else
        {
            ESP_LOGW("Semaphore Take Failure", "WARNING! Failed to take Semaphore");
        }
        return requiredCount;
    }

    bool PushValuesAndCalculateNormalizedFFT(int32_t *value, size_t Count, float Gain)
    {
        bool result = false;
        for (int i = 0; i < Count; ++i)
        {
            result = PushValueAndCalculateNormalizedFFT(value[i], Gain);
        }
        return result;
    }

    bool PushValueAndCalculateNormalizedFFT(int32_t value, float Gain)
    {
        if (!mp_RealBuffer || !mp_ImaginaryBuffer || !m_MyFFT || m_BitLengthMaxValue == 0 || m_FFT_Size == 0)
        {
            ESP_LOGE("PushValueAndCalculateNormalizedFFT", "ERROR!: Initialization or parameter error.");
            ResetCalculator();
            return m_SolutionReady;
        }

        if (xSemaphoreTakeRecursive(m_Semaphore, pdMS_TO_TICKS(100)) == pdTRUE)
        {
            value = std::clamp(value, -m_BitLengthMaxValue, m_BitLengthMaxValue);

            if (m_CurrentIndex < m_FFT_Size)
            {
                mp_RealBuffer[m_CurrentIndex] = value;
                mp_ImaginaryBuffer[m_CurrentIndex] = 0.0f;
                ++m_CurrentIndex;
            }

            if (m_CurrentIndex >= m_FFT_Size)
            {
                m_CurrentIndex = 0;
                m_MyFFT->compute(FFTDirection::Forward);
                vTaskDelay(1);
                m_MyFFT->complexToMagnitude();
                m_MajorPeak = m_MyFFT->majorPeak();

                for (int i = 0; i < m_FFT_Size; ++i)
                {
                    mp_RealBuffer[i] = ((2.0f * mp_RealBuffer[i]) / (float)m_FFT_Size) * Gain / m_BitLengthMaxValue;
                    if (i < m_FFT_Size / 2 && mp_RealBuffer[i] > m_MaxFFTBinValue)
                    {
                        m_MaxFFTBinValue = mp_RealBuffer[i];
                        m_MaxFFTBinIndex = i;
                    }
                }
                m_SolutionReady = true;
            }
            xSemaphoreGiveRecursive(m_Semaphore);
        }
        else
        {
            ESP_LOGW("Semaphore Take Failure", "WARNING! Failed to take Semaphore");
        }
        return m_SolutionReady;
    }

private:
    uint32_t m_CurrentIndex = 0;
    uint32_t m_FFT_Size = 0;
    uint32_t m_FFT_SampleRate = 0;
    float *mp_RealBuffer = nullptr;
    float *mp_ImaginaryBuffer = nullptr;
    float m_MaxFFTBinValue = 0.0f;
    uint32_t m_MaxFFTBinIndex = 0;
    float m_MajorPeak = 0.0f;
    bool m_SolutionReady = false;
    int32_t m_BitLengthMaxValue = 0;
    ArduinoFFT<float> *m_MyFFT = nullptr;
    SemaphoreHandle_t m_Semaphore = nullptr; // FreeRTOS semaphore
};

#endif
