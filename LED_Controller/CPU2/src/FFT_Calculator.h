#ifndef FFT_CALCULATOR_BASE_H
#define FFT_CALCULATOR_BASE_H

#include <DataTypes.h>
#include "Streaming.h"
#include "esp_system.h"
#include "esp_task_wdt.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "arduinoFFT.h"
#include "fix_fft.h"

class FFT_CalculatorBase
{
public:
    FFT_CalculatorBase(int32_t FFT_Size, int32_t SampleRate, BitLength_t BitLength)
        : m_FFT_Size(FFT_Size), m_FFT_SampleRate(SampleRate)
    {
        m_Semaphore = xSemaphoreCreateMutex();
        if (m_Semaphore == nullptr)
        {
            ESP_LOGE("FFT_CalculatorBase", "ERROR! Unable to create Semaphore.");
        }

        switch (BitLength)
        {
        case BitLength_32:
            m_BitLengthMaxValue = static_cast<int32_t>((1ULL << 31) - 1);
            break;
        case BitLength_16:
            m_BitLengthMaxValue = static_cast<int32_t>((1ULL << 15) - 1);
            break;
        case BitLength_8:
            m_BitLengthMaxValue = static_cast<int32_t>((1ULL << 7) - 1);
            break;
        default:
            m_BitLengthMaxValue = static_cast<int32_t>((1ULL << 31) - 1);
            break;
        }
    }

    virtual ~FFT_CalculatorBase()
    {
        if (m_Semaphore)
        {
            vSemaphoreDelete(m_Semaphore);
            m_Semaphore = nullptr;
        }
    }

    void ResetCalculator()
    {
        if (xSemaphoreTakeRecursive(m_Semaphore, pdMS_TO_TICKS(5)) == pdTRUE)
        {
            m_SolutionReady = false;
            m_CurrentIndex = 0;
            m_MaxFFTBinValue = 0.0f;
            m_MaxFFTBinIndex = 0;
            m_MajorPeak = 0.0f;
            xSemaphoreGiveRecursive(m_Semaphore);
        }
        else
        {
            ESP_LOGW("Semaphore Take Failure", "WARNING! Failed to take Semaphore");
        }
    }

    virtual bool PushValueAndCalculateNormalizedFFT(int32_t value, float Gain) = 0;
    virtual bool CalculateNormalizedFFT(int32_t* inputBuffer, size_t bufferSize, float Gain) = 0;
    virtual float* GetNormalizedFFTBuffer() = 0;

protected:
    uint32_t m_CurrentIndex = 0;
    uint32_t m_FFT_Size = 0;
    uint32_t m_FFT_SampleRate = 0;
    int32_t m_BitLengthMaxValue = 0;
    float m_MaxFFTBinValue = 0.0f;
    uint32_t m_MaxFFTBinIndex = 0;
    float m_MajorPeak = 0.0f;
    bool m_SolutionReady = false;
    SemaphoreHandle_t m_Semaphore = nullptr;
};

class FFT_Calculator_Arduino : public FFT_CalculatorBase
{
public:
    FFT_Calculator_Arduino(int32_t FFT_Size, int32_t SampleRate, BitLength_t BitLength)
        : FFT_CalculatorBase(FFT_Size, SampleRate, BitLength)
    {
        mp_RealBuffer = (float *)malloc(sizeof(float) * m_FFT_Size);
        mp_ImaginaryBuffer = (float *)malloc(sizeof(float) * m_FFT_Size);
        m_MyFFT = new ArduinoFFT<float>(mp_RealBuffer, mp_ImaginaryBuffer, m_FFT_Size, m_FFT_SampleRate);
    }

    virtual ~FFT_Calculator_Arduino()
    {
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

    float* GetNormalizedFFTBuffer()
    {
        return mp_RealBuffer;
    }

    bool PushValueAndCalculateNormalizedFFT(int32_t value, float Gain) override
    {
        if (xSemaphoreTakeRecursive(m_Semaphore, pdMS_TO_TICKS(5)) == pdTRUE)
        {
            if (!mp_RealBuffer || !mp_ImaginaryBuffer || !m_MyFFT || m_BitLengthMaxValue == 0 || m_FFT_Size == 0)
            {
                ESP_LOGE("PushValueAndCalculateNormalizedFFT", "ERROR!: Initialization or parameter error.");
                xSemaphoreGiveRecursive(m_Semaphore);
                return false;
            }

            value = std::clamp(value, -m_BitLengthMaxValue, m_BitLengthMaxValue);

            mp_RealBuffer[m_CurrentIndex] = value;
            mp_ImaginaryBuffer[m_CurrentIndex] = 0.0f;
            m_CurrentIndex++;

            if (m_CurrentIndex >= m_FFT_Size)
            {
                m_CurrentIndex = 0;
                m_MyFFT->compute(FFTDirection::Forward);
                m_MyFFT->complexToMagnitude();
                for (int i = 0; i < m_FFT_Size; ++i)
                {
                    mp_RealBuffer[i] = (2.0f * mp_RealBuffer[i]) / (float)m_FFT_Size * Gain / m_BitLengthMaxValue;
                }
                xSemaphoreGiveRecursive(m_Semaphore);
                return true;
            }
        }
        else
        {
            ESP_LOGW("Semaphore Take Failure", "WARNING! Failed to take Semaphore");
        }
        return false;
    }

    bool CalculateNormalizedFFT(int32_t* inputBuffer, size_t bufferSize, float Gain) override
    {
        // ArduinoFFT-based calculation
        if (bufferSize != m_FFT_Size)
        {
            ESP_LOGE("CalculateNormalizedFFT", "Buffer size mismatch");
            return false;
        }

        for (size_t i = 0; i < bufferSize; ++i)
        {
            mp_RealBuffer[i] = std::clamp(inputBuffer[i], -m_BitLengthMaxValue, m_BitLengthMaxValue);
            mp_ImaginaryBuffer[i] = 0.0f;
        }

        m_MyFFT->compute(FFTDirection::Forward);
        m_MyFFT->complexToMagnitude();

        for (size_t i = 0; i < bufferSize; ++i)
        {
            mp_RealBuffer[i] = (2.0f * mp_RealBuffer[i]) / (float)bufferSize * Gain / m_BitLengthMaxValue;
        }

        return true;
    }

private:
    ArduinoFFT<float> *m_MyFFT = nullptr;
    float *mp_RealBuffer = nullptr;
    float *mp_ImaginaryBuffer = nullptr;
};

class FFT_Calculator_Fix : public FFT_CalculatorBase
{
public:
    FFT_Calculator_Fix(int32_t FFT_Size, int32_t SampleRate, BitLength_t BitLength)
        : FFT_CalculatorBase(FFT_Size, SampleRate, BitLength)
    {
        if (xSemaphoreTakeRecursive(m_Semaphore, pdMS_TO_TICKS(5)) == pdTRUE)
        {
            mp_BufferInt = (int32_t *)malloc(sizeof(int32_t) * m_FFT_Size);
            mp_NormalizedBuffer = (float *)malloc(sizeof(float) * m_FFT_Size);
            mp_RealBufferChar = (char *)malloc(sizeof(char) * m_FFT_Size);
            mp_ImaginaryBufferChar = (char *)malloc(sizeof(char) * m_FFT_Size);
            m_scaleFactor = 127.0/m_BitLengthMaxValue;
            xSemaphoreGiveRecursive(m_Semaphore);
        }
        else
        {
            ESP_LOGW("Semaphore Take Failure", "WARNING! Failed to take Semaphore");
        }
    }

    virtual ~FFT_Calculator_Fix()
    {
        if (xSemaphoreTakeRecursive(m_Semaphore, pdMS_TO_TICKS(5)) == pdTRUE)
        {
            if (mp_BufferInt)
            {
                free(mp_BufferInt);
                mp_BufferInt = nullptr;
            }
            if (mp_NormalizedBuffer)
            {
                free(mp_NormalizedBuffer);
                mp_NormalizedBuffer = nullptr;
            }
            if (mp_RealBufferChar)
            {
                free(mp_RealBufferChar);
                mp_RealBufferChar = nullptr;
            }
            if (mp_ImaginaryBufferChar)
            {
                free(mp_ImaginaryBufferChar);
                mp_ImaginaryBufferChar = nullptr;
            }
            xSemaphoreGiveRecursive(m_Semaphore);
        }
        else
        {
            ESP_LOGW("Semaphore Take Failure", "WARNING! Failed to take Semaphore");
        }
    }

    float* GetNormalizedFFTBuffer()
    {
        return mp_NormalizedBuffer;
    }
    
    bool PushValueAndCalculateNormalizedFFT(int32_t value, float Gain) override
    {
        if (xSemaphoreTakeRecursive(m_Semaphore, pdMS_TO_TICKS(5)) == pdTRUE)
        {
            if (!mp_BufferInt || !mp_RealBufferChar || !mp_ImaginaryBufferChar)
            {
                ESP_LOGE("PushValueAndCalculateNormalizedFFT", "ERROR!: Initialization error.");
                xSemaphoreGiveRecursive(m_Semaphore);
                return false;
            }

            value = std::clamp(value, -m_BitLengthMaxValue, m_BitLengthMaxValue);

            mp_BufferInt[m_CurrentIndex] = value;
            mp_ImaginaryBufferChar[m_CurrentIndex] = 0;
            m_CurrentIndex++;

            if (m_CurrentIndex >= m_FFT_Size)
            {
                m_CurrentIndex = 0;
                convertInt32ToChar(mp_BufferInt, mp_RealBufferChar, m_FFT_Size);
                fix_fft(mp_RealBufferChar, mp_ImaginaryBufferChar, m_FFT_Size, 0);
                for (int i = 0; i < m_FFT_Size; ++i)
                {
                    mp_NormalizedBuffer[i] = ((2.0f * mp_RealBufferChar[i]) / (float)m_FFT_Size) * Gain / 127.0;
                }
                xSemaphoreGiveRecursive(m_Semaphore);
                return true;
            }
        }
        else
        {
            ESP_LOGW("Semaphore Take Failure", "WARNING! Failed to take Semaphore");
        }
        return false;
    }
    
    bool CalculateNormalizedFFT(int32_t* inputBuffer, size_t bufferSize, float Gain) override
    {
        if (xSemaphoreTakeRecursive(m_Semaphore, pdMS_TO_TICKS(5)) == pdTRUE)
        {
            if (bufferSize != m_FFT_Size)
            {
                ESP_LOGE("CalculateNormalizedFFT", "Buffer size mismatch");
                xSemaphoreGiveRecursive(m_Semaphore);
                return false;
            }

            for (size_t i = 0; i < bufferSize; ++i)
            {
                mp_BufferInt[i] = std::clamp(inputBuffer[i], -m_BitLengthMaxValue, m_BitLengthMaxValue);
                mp_ImaginaryBufferChar[i] = 0;
            }

            convertInt32ToChar(inputBuffer, mp_RealBufferChar, m_FFT_Size);
            fix_fft(mp_RealBufferChar, mp_ImaginaryBufferChar, m_FFT_Size, 0);

            for (size_t i = 0; i < bufferSize; ++i)
            {
                mp_NormalizedBuffer[i] = ((2.0f * mp_RealBufferChar[i]) / (float)m_FFT_Size) * Gain / 127.0;
            }
            xSemaphoreGiveRecursive(m_Semaphore);
        }
        else
        {
            ESP_LOGW("Semaphore Take Failure", "WARNING! Failed to take Semaphore");
        }
        return true;
    }

    void convertInt32ToChar(int32_t* input, char* output, size_t length)
    {
        if (xSemaphoreTakeRecursive(m_Semaphore, pdMS_TO_TICKS(5)) == pdTRUE)
        {
            int32_t maxValue = 0;

            for (size_t i = 0; i < length; ++i)
            {
                if (abs(input[i]) > maxValue)
                {
                    maxValue = abs(input[i]);
                }
            }

            float scaleFactor = (maxValue > 127) ? (127.0 / maxValue) : 1.0;

            for (size_t i = 0; i < length; ++i)
            {
                output[i] = (char)(input[i] * scaleFactor);
            }
            xSemaphoreGiveRecursive(m_Semaphore);
        }
        else
        {
            ESP_LOGW("Semaphore Take Failure", "WARNING! Failed to take Semaphore");
        }
    }

private:
    int32_t *mp_BufferInt = nullptr;
    float *mp_NormalizedBuffer = nullptr;
    char *mp_RealBufferChar = nullptr;
    char *mp_ImaginaryBufferChar = nullptr;
    float m_scaleFactor;
};

#endif
