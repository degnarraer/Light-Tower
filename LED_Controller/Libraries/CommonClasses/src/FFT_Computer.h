#include <Arduino.h>
#include <math.h>
#include <vector>
#include <memory>
#include "xtensa/core-macros.h"  // For ESP32 low-level operations
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "Helpers.h"
#include "CircularVector.h"
#include "PSRamAllocator.h"
#include "Tunes.h"

#define SEMAPHORE_WAIT_MS 10

struct FFT_Bin_Data
{
    float Frequency = 0.0f;
    float Magnitude = 0.0f;
    float NormalizedMagnitude = 0.0f;
};
typedef FFT_Bin_Data FFT_Bin_Data_t;

struct PsMallocDeleter {
    void operator()(void* ptr) const {
        free(ptr);
    }
};

struct FFT_Bin_Data_Set
{
    std::unique_ptr<FFT_Bin_Data_t[], PsMallocDeleter> Left_Channel = nullptr;
    std::unique_ptr<FFT_Bin_Data_t[], PsMallocDeleter> Right_Channel = nullptr;
    size_t MaxRightBin = 0;
    size_t MaxLeftBin = 0;
    size_t Count = 0;
    FFT_Bin_Data_Set(){}
    FFT_Bin_Data_Set( std::unique_ptr<FFT_Bin_Data_t[], PsMallocDeleter> Left_Channel_in
                    , std::unique_ptr<FFT_Bin_Data_t[], PsMallocDeleter> Right_Channel_in
                    , size_t MaxRightBin_in
                    , size_t MaxLeftBin_in
                    , size_t Count_in )
                    : Left_Channel(std::move(Left_Channel_in))
                    , Right_Channel(std::move(Right_Channel_in))
                    , MaxRightBin(MaxRightBin_in)
                    , MaxLeftBin(MaxLeftBin_in)
                    , Count(Count_in){}
    virtual ~FFT_Bin_Data_Set()
    {
        ESP_LOGD("~FFT_Bin_Data_Set", "Deleting Data Set");
    }
};
typedef FFT_Bin_Data_Set FFT_Bin_Data_Set_t;

enum DataWidth_t
{
  DataWidth_32,
  DataWidth_16,
  DataWidth_8,
};

class FFT_Computer {
    typedef void FFT_Results_Callback(std::unique_ptr<FFT_Bin_Data_Set_t> &sp_FFT_Bin_Data, void* args);
private:
    alignas(4) FFT_Results_Callback* mp_CallBack = nullptr;
    bool m_IsMultithreaded = false;
    void* mp_CallBackArgs = nullptr;
    size_t m_fftSize;                       // FFT size (e.g., 8192)
    size_t m_magnitudeSize; 
    size_t m_hopSize;                       // Hop size (number of samples between FFTs)
    float m_f_s;                            // Sample Rate
    float m_Gain = 50.0;
    ShocksRingBuffer* mp_ringBuffer;        // Ring Buffer
    std::vector<Frame_t> m_frames;          // frame Buffer
    std::unique_ptr<float[], PsMallocDeleter> sp_real_right_channel;           // Real part of FFT input
    std::unique_ptr<float[], PsMallocDeleter> sp_imag_right_channel;           // Imaginary part of FFT input
    std::unique_ptr<float[], PsMallocDeleter> sp_real_left_channel;            // Real part of FFT input
    std::unique_ptr<float[], PsMallocDeleter> sp_imag_left_channel;            // Imaginary part of FFT input
    std::unique_ptr<float[], PsMallocDeleter> sp_magnitudes_right_channel;     // FFT magnitudes
    std::unique_ptr<float[], PsMallocDeleter> sp_magnitudes_left_channel;      // FFT magnitudes
    UBaseType_t m_uxPriority;
    BaseType_t m_xCoreID;

    
    unsigned long m_totalFrames;
    unsigned long m_totalProcessedFrames;
    unsigned long m_framesSinceLastFFT;
    unsigned long m_NotificationCount;

    TaskHandle_t m_fft_Calculator_TaskHandle = nullptr;     // Task handle for the FFT Data Getter
    bool m_isInitialized = false;                           // Whether the setup function has been called
    DataWidth_t m_dataWidth;

public:    
    FFT_Computer(int fftSize, int hopSize, float f_s, DataWidth_t dataWidth)
        : m_fftSize(fftSize)
        , m_hopSize(hopSize)
        , m_f_s(f_s)
        , m_dataWidth(dataWidth)
        , m_isInitialized(false)
        , m_IsMultithreaded(false)
        , m_magnitudeSize(m_fftSize/2)
        , m_totalFrames(0)
        , m_totalProcessedFrames(0)
        , m_framesSinceLastFFT(0) {}
    FFT_Computer(int fftSize, int hopSize, float f_s, DataWidth_t dataWidth, UBaseType_t uxPriority, BaseType_t xCoreID)
        : m_fftSize(fftSize)
        , m_hopSize(hopSize)
        , m_f_s(f_s)
        , m_dataWidth(dataWidth)
        , m_uxPriority(uxPriority)
        , m_xCoreID(xCoreID)
        , m_isInitialized(false)
        , m_IsMultithreaded(true)
        , m_magnitudeSize(m_fftSize/2)
        , m_totalFrames(0)
        , m_totalProcessedFrames(0)
        , m_framesSinceLastFFT(0)  {}

    // Destructor
    ~FFT_Computer() {
        if (m_isInitialized)
        {
            if(mp_ringBuffer)
            {
                ESP_LOGD("~FFT_Computer", "mp_ringBuffer");
                delete mp_ringBuffer;
                mp_ringBuffer = nullptr;
            }
            if(m_fft_Calculator_TaskHandle)
            {
                ESP_LOGD("~FFT_Computer", "m_fft_Calculator_TaskHandle");
                vTaskDelete(m_fft_Calculator_TaskHandle);
                m_fft_Calculator_TaskHandle= nullptr;
            }
        }
    }

    void Setup(FFT_Results_Callback* callback, void* callBackArgs)
    {
        if (m_isInitialized) return;
        if ((uintptr_t)callback % 4 != 0)
        {
            ESP_LOGE("Setup", "Callback is misaligned: %p", callback);
            return;
        }
        else
        {
            mp_CallBack = callback;
        }
        mp_CallBackArgs = callBackArgs;
        mp_ringBuffer = new ShocksRingBuffer(m_fftSize);
        sp_real_right_channel = std::unique_ptr<float[], PsMallocDeleter>(static_cast<float*>(ps_malloc(m_fftSize * sizeof(float))));
        sp_imag_right_channel = std::unique_ptr<float[], PsMallocDeleter>(static_cast<float*>(ps_malloc(m_fftSize * sizeof(float))));
        sp_real_left_channel = std::unique_ptr<float[], PsMallocDeleter>(static_cast<float*>(ps_malloc(m_fftSize * sizeof(float))));
        sp_imag_left_channel = std::unique_ptr<float[], PsMallocDeleter>(static_cast<float*>(ps_malloc(m_fftSize * sizeof(float))));
        sp_magnitudes_right_channel = std::unique_ptr<float[], PsMallocDeleter>(static_cast<float*>(ps_malloc(m_magnitudeSize * sizeof(float))));
        sp_magnitudes_left_channel = std::unique_ptr<float[], PsMallocDeleter>(static_cast<float*>(ps_malloc(m_magnitudeSize * sizeof(float))));

        if(m_IsMultithreaded)
        {            
            xTaskCreate(Static_Process_FFT_Task, "FFT Task", 5000, this, m_uxPriority, &m_fft_Calculator_TaskHandle);
            if(m_fft_Calculator_TaskHandle) ESP_LOGD("Setup", "FFT Calculator Task Started.");
            else ESP_LOGE("Setup", "ERROR! Error creating FFT Calculator Task.");            
        }
        m_isInitialized = true;
    }

    void SetGain(float gain)
    {
        m_Gain = gain;
    }

    void PushFrames(Frame_t *p_frames, size_t count)
    {
        assert(m_isInitialized);
        assert(p_frames);
        static LogWithRateLimit_Average<size_t> Push_Frames_RLL(1000, ESP_LOG_DEBUG);
        static LogWithRateLimit Push_Frames_Dropped_RLL(1000, ESP_LOG_WARN);
        if(count > 0)
        {
            size_t framesPushed = mp_ringBuffer->push(p_frames, count, SEMAPHORE_SHORT_BLOCK);
            if(framesPushed == count)
            {
                m_totalFrames += framesPushed;
                if(m_totalFrames - m_framesSinceLastFFT >= m_hopSize)
                {
                    m_framesSinceLastFFT = m_totalFrames;
                    xTaskNotifyGive(m_fft_Calculator_TaskHandle);
                }
            }
        }
    }

    float* GetMagnitudes() 
    {
        return sp_magnitudes_right_channel.get();
    }

    void PrintBuffer(char* title, std::vector<float>& buffer) 
    {
        Serial.printf("%s:", title);
        for (int i = 0; i < buffer.size(); i++)
        {
            if(i!=0) Serial.print("|");
            Serial.printf("%i:%f", i, buffer[i]);
        }
        Serial.print("\n");
    }

private:

    static void Static_Process_FFT_Task(void* pvParameters)
    {
        FFT_Computer* fft = (FFT_Computer*)pvParameters;
        fft->Process_FFT_Task();  
    }

    void Process_FFT_Task() 
    {
        static LogWithRateLimit PerformFFTTask_RLL(1000, ESP_LOG_DEBUG);
        while(true)
        {
            PerformFFTTask_RLL.Log(ESP_LOG_DEBUG, "Process_FFT_Task", "Process FFT Task Called");
            ProcessFFT();
        }
    }

    void ProcessFFT()
    {
        static LogWithRateLimit ProcessFFT_FFT_Started_RLL(1000, ESP_LOG_DEBUG);
        static LogWithRateLimit ProcessFFT_FFT_DeQueue_Fail_RLL(1000, ESP_LOG_WARN);
        static LogWithRateLimit ProcessFFT_FFT_Complete_RLL(1000, ESP_LOG_DEBUG);
        static LogWithRateLimit ProcessFFT_Calling_Callbacks_RLL(1000, ESP_LOG_DEBUG);
        static LogWithRateLimit ProcessFFT_Null_Pointers_RLL(1000, ESP_LOG_ERROR);
        
        if(!mp_CallBack || !sp_real_right_channel || !sp_real_left_channel || !sp_imag_right_channel || !sp_imag_left_channel )
        {
            ProcessFFT_Null_Pointers_RLL.Log(ESP_LOG_ERROR, "ProcessFFT", "ERROR! Null Pointers");
            vTaskDelay(SEMAPHORE_LONG_BLOCK);
            return;
        }

        uint32_t notificationValue;
        if (xTaskNotifyWait(0x00, 0xFFFFFFFF, &notificationValue, SEMAPHORE_BLOCK) == pdTRUE)
        {
            std::unique_ptr<Frame_t[], PsMallocDeleter> sp_frames = std::unique_ptr<Frame_t[], PsMallocDeleter>((Frame_t*)ps_malloc(sizeof(Frame_t) * m_fftSize));
            if (!sp_frames)
            {
                ESP_LOGE("ProcessFFT", "ERROR! Failed to allocate memory for frames.");
                vTaskDelay(SEMAPHORE_LONG_BLOCK);
                return;
            }
            size_t receivedFrames = mp_ringBuffer->get(sp_frames.get(), m_fftSize, SEMAPHORE_BLOCK);
            if(receivedFrames != m_fftSize)
            {
                ESP_LOGE("ProcessFFT", "ERROR! Failed to receive expected frame count.");
                vTaskDelay(SEMAPHORE_LONG_BLOCK);
                return;
            }

            ProcessFFT_FFT_Started_RLL.Log(ESP_LOG_DEBUG, "ProcessFFT", "Process FFT Started");
            {
                for (int i = 0; i < m_fftSize; i++)
                {
                    sp_real_left_channel[i] = sp_frames[i].channel1;
                    sp_imag_left_channel[i] = 0.0f;
                    sp_real_right_channel[i] = sp_frames[i].channel2;
                    sp_imag_right_channel[i] = 0.0f;
                }
            }
            
            unsigned long startTime = millis();
            ComputeFFT(sp_real_left_channel.get(), sp_imag_left_channel.get(), m_fftSize);
            ComputeFFT(sp_real_right_channel.get(), sp_imag_right_channel.get(), m_fftSize);
            unsigned long stopTime = millis();
            ProcessFFT_FFT_Complete_RLL.Log(ESP_LOG_DEBUG, "ProcessFFT", "FFT Calculation Completed");

            float maxMagnitude = GetMaxMagnitude();
            std::unique_ptr<FFT_Bin_Data_t[], PsMallocDeleter> sp_freqMags_left = std::unique_ptr<FFT_Bin_Data_t[], PsMallocDeleter>(static_cast<FFT_Bin_Data_t*>(ps_malloc(m_magnitudeSize * sizeof(FFT_Bin_Data_t))));
            std::unique_ptr<FFT_Bin_Data_t[], PsMallocDeleter> sp_freqMags_right = std::unique_ptr<FFT_Bin_Data_t[], PsMallocDeleter>(static_cast<FFT_Bin_Data_t*>(ps_malloc(m_magnitudeSize * sizeof(FFT_Bin_Data_t))));
            if (!sp_freqMags_left || !sp_freqMags_right)
            {
                ESP_LOGE("ProcessFFT", "ERROR! Failed to allocate memory for FFT Bin Data.");
                vTaskDelay(SEMAPHORE_LONG_BLOCK);
                return;
            }
            size_t maxBin_Left = 0;
            size_t maxBin_Right = 0;
            
            for (int i = 0; i < m_magnitudeSize; ++i)
            {
                sp_magnitudes_right_channel[i] = m_Gain/1000.0 * sqrtf(sp_real_right_channel[i] * sp_real_right_channel[i] +sp_imag_right_channel[i] * sp_imag_right_channel[i]);
                sp_magnitudes_left_channel[i] = m_Gain/1000.0 * sqrtf(sp_real_left_channel[i] * sp_real_left_channel[i] + sp_imag_left_channel[i] * sp_imag_left_channel[i]);
                if(sp_magnitudes_left_channel[i] > sp_freqMags_left[maxBin_Left].Magnitude)
                {
                    maxBin_Left = i;
                }
                if(sp_magnitudes_right_channel[i] > sp_freqMags_right[maxBin_Right].Magnitude)
                {
                    maxBin_Right = i;
                }

                sp_freqMags_right[i].Frequency = binToFrequency(i);
                sp_freqMags_right[i].Magnitude = sp_magnitudes_right_channel[i];
                sp_freqMags_right[i].NormalizedMagnitude = sp_magnitudes_right_channel[i] / maxMagnitude;

                sp_freqMags_left[i].Frequency = binToFrequency(i);
                sp_freqMags_left[i].Magnitude = sp_magnitudes_left_channel[i];
                sp_freqMags_left[i].NormalizedMagnitude = sp_magnitudes_left_channel[i] / maxMagnitude;
                if(i % 100 == 0) vTaskDelay(SEMAPHORE_SHORT_BLOCK);
            }
            
            ProcessFFT_Calling_Callbacks_RLL.Log(ESP_LOG_DEBUG, "ProcessFFT", "Calling Callback");
            std::unique_ptr<FFT_Bin_Data_Set_t> sp_FFT_Bin_Data_Set = std::make_unique<FFT_Bin_Data_Set_t>(std::move(sp_freqMags_left), std::move(sp_freqMags_right), maxBin_Left, maxBin_Right, m_magnitudeSize);
            mp_CallBack(sp_FFT_Bin_Data_Set, mp_CallBackArgs);
        }
    }

    void ComputeFFT(float* real, float* imag, int n)
    {
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
            ESP_LOGD("ComputeFFT", "Calculating Sin Cos Lookups Tables.");
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
        for(int s = 1; s <= logN; s++)
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

};