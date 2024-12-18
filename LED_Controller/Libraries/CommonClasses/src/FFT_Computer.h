#include <Arduino.h>
#include <math.h>
#include <vector>
#include "xtensa/core-macros.h"  // For ESP32 low-level operations
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "Helpers.h"

enum DataWidth_t
{
  DataWidth_32,
  DataWidth_16,
  DataWidth_8,
};

class FFT_Computer {
    typedef void FFT_Results_Callback(float *leftMagnitudes, float* sortedFrequenciesLeft, float* rightMagnitudes, float* sortedFrequenciesRight, size_t count, void* args);
private:
    FFT_Results_Callback* m_CallBack = nullptr;
    bool m_IsMultithreaded = false;
    void* mp_CallBackArgs = nullptr;
    int m_fftSize;                                  // FFT size (e.g., 8192)
    int m_magnitudeSize;    
    int m_hopSize;                                  // Hop size (number of samples between FFTs)
    float m_f_s;                                    // Sample Rate
    RingbufHandle_t m_ringBuffer;                   // DMA-friendly ring buffer handle
    std::vector<float> mp_real_right_channel;         // Real part of FFT input
    std::vector<float> mp_imag_right_channel;         // Imaginary part of FFT input
    std::vector<float> mp_magnitudes_right_channel;   // FFT magnitudes
    std::vector<float> mp_real_left_channel;          // Real part of FFT input
    std::vector<float> mp_imag_left_channel;          // Imaginary part of FFT input
    std::vector<float> mp_magnitudes_left_channel;    // FFT magnitudes
    int m_bufferIndex;                              // Current write index in the circular buffer
    int m_samplesSinceLastFFT;                      // Counter for samples pushed since the last FFT
    SemaphoreHandle_t m_mutex;                      // mutex for thread-safe access to the circular buffer
    UBaseType_t m_uxPriority;
    BaseType_t m_xCoreID;

    TaskHandle_t m_fftTaskHandle = nullptr;         // Task handle for the FFT computation thread
    bool m_normalizeMagnitudes = false;             // Whether to normalize the magnitudes to 0.0 to 1.0
    bool m_isInitialized = false;                   // Whether the setup function has been called

    // Data width (8, 16, or 32-bit)
    DataWidth_t m_dataWidth;

public:    
    // Constructor with optional normalizeMagnitudes and dataWidth
    FFT_Computer(int fftSize, int hopSize, float f_s, bool normalizeMagnitudes, DataWidth_t dataWidth)
        : m_fftSize(fftSize)
        , m_hopSize(hopSize)
        , m_f_s(f_s)
        , m_normalizeMagnitudes(normalizeMagnitudes)
        , m_dataWidth(dataWidth)
        , m_isInitialized(false)
        , m_IsMultithreaded(false)
        , m_bufferIndex(0)
        , m_samplesSinceLastFFT(0)
        , m_magnitudeSize(m_fftSize/2) {}
    FFT_Computer(int fftSize, int hopSize, float f_s, bool normalizeMagnitudes, DataWidth_t dataWidth, UBaseType_t uxPriority, BaseType_t xCoreID)
        : m_fftSize(fftSize)
        , m_hopSize(hopSize)
        , m_f_s(f_s)
        , m_normalizeMagnitudes(normalizeMagnitudes)
        , m_dataWidth(dataWidth)
        , m_uxPriority(uxPriority)
        , m_xCoreID(xCoreID)
        , m_isInitialized(false)
        , m_IsMultithreaded(true)
        , m_bufferIndex(0)
        , m_samplesSinceLastFFT(0)
        , m_magnitudeSize(m_fftSize/2) {}

    // Destructor
    ~FFT_Computer() {
        if (m_isInitialized)
        {
            if(m_mutex)
            {
                ESP_LOGD("~FFT_Computer", "m_mutex");
                free(m_mutex);
                m_mutex = nullptr;
            }
            if(m_ringBuffer)
            {
                ESP_LOGD("~FFT_Computer", "m_ringBuffer");
                free(m_ringBuffer);
                m_ringBuffer = nullptr;
            }
            if(m_fftTaskHandle)
            {
                ESP_LOGD("~FFT_Computer", "m_fftTaskHandle");
                free(m_fftTaskHandle);
                m_fftTaskHandle= nullptr;
            }
        }
    }

    void Setup(FFT_Results_Callback* callback, void* callBackArgs) {
        m_CallBack = callback;
        mp_CallBackArgs = callBackArgs;
        if (m_isInitialized) {
            return;
        }

        mp_real_right_channel.resize(m_fftSize, 0.0f);
        mp_imag_right_channel.resize(m_fftSize, 0.0f);
        mp_magnitudes_right_channel.resize(m_magnitudeSize, 0.0f);
        mp_real_left_channel.resize(m_fftSize, 0.0f);
        mp_imag_left_channel.resize(m_fftSize, 0.0f);
        mp_magnitudes_left_channel.resize(m_magnitudeSize, 0.0f);

        m_ringBuffer = xRingbufferCreate((m_fftSize * 2) * sizeof(Frame_t), RINGBUF_TYPE_BYTEBUF);
        if (!m_ringBuffer) {
            ESP_LOGE("Setup", "ERROR! Failed to create ring buffer.");
            while (1);
        }

        m_mutex = xSemaphoreCreateMutex();
        if(m_IsMultithreaded) xTaskCreatePinnedToCore(Static_PerformFFTTask, "FFTTask", 5000, this, m_uxPriority, &m_fftTaskHandle, m_xCoreID);
        m_isInitialized = true;
    }

    void PushFrames(Frame_t *frames, size_t count)
    {
        if (!m_isInitialized)
        {
            ESP_LOGW("PushFrames", "WARNING! FFT class not initialized. Please call setup() first.");
            return;
        }

        size_t byteToWrite = count * sizeof(Frame_t);
        if (xRingbufferSend(m_ringBuffer, frames, byteToWrite, pdMS_TO_TICKS(0)) != pdTRUE)
        {
            static LogWithRateLimit PushFramesRateLimitedLog(1000);
            PushFramesRateLimitedLog.Log(ESP_LOG_WARN, "PushFrames", "WARNING! Ring buffer is full! Frames dropped.");
        }
        else
        {
            m_samplesSinceLastFFT += count;
            ESP_LOGD("PushFrames", "Count: %i.", count);

            if (m_samplesSinceLastFFT >= m_hopSize)
            {
                ESP_LOGD("PushFrames", "Samples: \"%i\" Hop Length: \"%i\" Buffer Index: \"%i\"", m_samplesSinceLastFFT, m_hopSize, m_bufferIndex);
                if(m_IsMultithreaded)
                {
                    xTaskNotifyGive(m_fftTaskHandle);
                }
                else
                {
                    PerformFFT();
                }
                m_samplesSinceLastFFT = 0;
            }
        }
    }

    float* GetMagnitudes() {
        return mp_magnitudes_right_channel.data();
    }

    void PrintBuffer(char* title, std::vector<float>& buffer) {
        Serial.printf("%s:", title);
        for (int i = 0; i < buffer.size(); i++) {
            if(i!=0) Serial.print("|");
            Serial.printf("%i:%f", i, buffer[i]);
        }
        Serial.print("\n");
    }

private:

    void PerformFFTTask() 
    {
        while (true)
        {
            // Wait for notification to start FFT processing
            ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
            PerformFFT();
        }
    }
    void PerformFFT() 
    {
        // Retrieve frames from the ring buffer
        size_t receivedByteCount;
        size_t requestedByteCount = std::min(m_hopSize*sizeof(Frame_t), m_fftSize*sizeof(Frame_t));
        ESP_LOGD("PerformFFT", "Requested %i bytes", requestedByteCount);
        Frame_t *frames = (Frame_t *)xRingbufferReceiveUpTo(m_ringBuffer, &receivedByteCount, pdMS_TO_TICKS(0), requestedByteCount);
        ESP_LOGD("PerformFFT", "Received %i bytes", receivedByteCount);
        if (!frames)
        {
            ESP_LOGW("PerformFFT", "WARNING! Ring buffer is empty or timed out.");
            return;
        }
        if(receivedByteCount % sizeof(Frame_t) != 0)
        {
            ESP_LOGW("PerformFFT", "WARNING! Improper received byte count.");
            vRingbufferReturnItem(m_ringBuffer, frames); // Return the item
            return;
        }
        size_t frameCount = receivedByteCount / sizeof(Frame_t);
        ESP_LOGD("PerformFFT", "Received %i Frames", frameCount);

        for (size_t i = 0; i < frameCount; ++i)
        {
            // Sliding window
            mp_real_left_channel.push_back(frames[i].channel1);
            mp_real_left_channel.erase(mp_real_left_channel.begin());
            mp_real_right_channel.push_back(frames[i].channel2);
            mp_real_right_channel.erase(mp_real_right_channel.begin());
        }
        mp_imag_right_channel.resize(m_fftSize, 0.0f);
        mp_imag_left_channel.resize(m_fftSize, 0.0f);


        // Update buffer index and process FFT if the buffer is full
        m_bufferIndex += frameCount;
        if (m_bufferIndex >= m_fftSize)
        {
            ESP_LOGW("PerformFFT", "Performing FFT with buffer size: %i", m_fftSize);
            ProcessFFT();
            m_bufferIndex -= m_hopSize;
        }
        vRingbufferReturnItem(m_ringBuffer, frames);
    }

    static void Static_PerformFFTTask(void* pvParameters) {
        FFT_Computer* fft = (FFT_Computer*)pvParameters;
        fft->PerformFFTTask();  
    }

    void ProcessFFT()
    {
        ComputeFFT(mp_real_right_channel, mp_imag_right_channel);
        ComputeFFT(mp_real_left_channel, mp_imag_left_channel);
        float maxMagnitude = GetMaxMagnitude();
        Serial.println(maxMagnitude);
        for (int i = 0; i < m_magnitudeSize; i++)
        {
            mp_magnitudes_right_channel[i] = sqrtf(mp_real_right_channel[i] * mp_real_right_channel[i] + mp_imag_right_channel[i] * mp_imag_right_channel[i]);
            mp_magnitudes_left_channel[i] = sqrtf(mp_real_left_channel[i] * mp_real_left_channel[i] + mp_imag_left_channel[i] * mp_imag_left_channel[i]);
            if (m_normalizeMagnitudes && maxMagnitude != 0.0f)
            {
                mp_magnitudes_right_channel[i] /= maxMagnitude;
                mp_magnitudes_left_channel[i] /= maxMagnitude;
            }
        }
        if (m_CallBack)
        {
            m_CallBack( mp_magnitudes_left_channel.data()
                      , GetSortedFrequencies(mp_magnitudes_left_channel).data()
                      , mp_magnitudes_right_channel.data()
                      , GetSortedFrequencies(mp_magnitudes_right_channel).data()
                      , m_magnitudeSize
                      , mp_CallBackArgs);
        }    
    }

    void ComputeFFT(std::vector<float>& real, std::vector<float>& imag)
    {
        int n = m_fftSize;
        int logN = log2(n);

        // Step 1: Bit-reversal reordering
        for (int i = 0; i < n; i++)
        {
            int j = ReverseBits(i, logN);
            if (i < j)
            {
                Swap(real[i], real[j]);
                Swap(imag[i], imag[j]);
            }
        }

        // Step 2: Precompute sine and cosine lookup tables
        static float* sineTable = nullptr;
        static float* cosineTable = nullptr;
        static int tableSize = 0;

        if (tableSize != n / 2) 
        {
            ESP_LOGW("ComputeFFT", "Calculating Sin Cos Lookups Tables.");
            // Free old tables if they exist
            delete[] sineTable;
            delete[] cosineTable;

            // Allocate new tables
            tableSize = n / 2;
            sineTable = new float[tableSize];
            cosineTable = new float[tableSize];

            // Fill the tables
            for (int i = 0; i < tableSize; i++)
            {
                float angle = -2.0f * M_PI * i / n;
                sineTable[i] = sinf(angle);
                cosineTable[i] = cosf(angle);
            }
        }

        // Step 3: FFT computation using lookup tables
        for (int s = 1; s <= logN; s++)
        {
            int m = 1 << s;
            int halfM = m / 2;
            int step = n / m; // Index step in the sine/cosine tables

            for (int k = 0; k < n; k += m)
            {
                for (int j = 0; j < halfM; j++)
                {
                    float cosA = cosineTable[j * step];
                    float sinA = sineTable[j * step];

                    float real_kjm = real[k + j + halfM];
                    float imag_kjm = imag[k + j + halfM];

                    float tReal = cosA * real_kjm - sinA * imag_kjm;
                    float tImag = sinA * real_kjm + cosA * imag_kjm;

                    real[k + j + halfM] = real[k + j] - tReal;
                    imag[k + j + halfM] = imag[k + j] - tImag;

                    real[k + j] += tReal;
                    imag[k + j] += tImag;
                }
            }
        }
    }

    int ReverseBits(int num, int bits)
    {
        int reversed = 0;
        for (int i = 0; i < bits; i++)
        {
            if (num & (1 << i))
            {
                reversed |= 1 << (bits - 1 - i);
            }
        }
        return reversed;
    }

    void Swap(float& a, float& b)
    {
        float temp = a;
        a = b;
        b = temp;
    }

    float GetMaxMagnitude()
    {
        switch (m_dataWidth)
        {
            case DataWidth_8:
                return 255.0f;
            break;
            case DataWidth_16:
                return 65535.0f;
            break;
            case DataWidth_32:
                return 4294967295.0f;
            break;
            default:
                ESP_LOGW("GetMaxMagnitude", "WARNING! Undefined Bit Width!");
                return 1.0;
            break;
        }
    }

    float binToFrequency(int binIndex)
    {
        if (binIndex < 0 || binIndex >= m_fftSize / 2)
        {
            return -1.0f;
        }
        
        return m_f_s * binIndex / m_fftSize;
    }

    std::vector<float> GetSortedFrequencies(std::vector<float>& inputArray) {
        std::vector<int> indexes(inputArray.size());
        
        // Initialize the indexes array with 0, 1, 2, ..., N-1
        for (int i = 0; i < inputArray.size(); ++i) {
            indexes[i] = i;
        }

        // Sort indexes based on the values in inputArray
        std::sort(indexes.begin(), indexes.end(), [&inputArray](int a, int b) {
            return inputArray[a] > inputArray[b]; // Descending order of magnitudes
        });

        // Map sorted indices to their corresponding frequencies
        std::vector<float> sortedFrequencies(indexes.size());
        for (int i = 0; i < indexes.size(); ++i) {
            sortedFrequencies[i] = binToFrequency(indexes[i]);
        }

        return sortedFrequencies;
    }

};