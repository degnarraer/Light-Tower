#include <Arduino.h>
#include <math.h>
#include <vector>
#include <memory>
#include "xtensa/core-macros.h"  // For ESP32 low-level operations
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "Helpers.h"
#include "CircularVector.h"
#include "PSRamAllocator.h"

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
    typedef void FFT_Results_Callback(std::unique_ptr<FFT_Bin_Data_Set_t> sp_FFT_Bin_Data, void* args);
private:
    alignas(4) FFT_Results_Callback* mp_CallBack = nullptr;
    bool m_IsMultithreaded = false;
    void* mp_CallBackArgs = nullptr;
    size_t m_fftSize;                       // FFT size (e.g., 8192)
    size_t m_magnitudeSize; 
    size_t m_hopSize;                       // Hop size (number of samples between FFTs)
    float m_f_s;                            // Sample Rate
    ShocksRingBuffer* mp_ringBuffer;        // Ring Buffer
    std::vector<Frame_t> m_frames;          // frame Buffer
    std::unique_ptr<float[], PsMallocDeleter> sp_real_right_channel;           // Real part of FFT input
    std::unique_ptr<float[], PsMallocDeleter> sp_imag_right_channel;           // Imaginary part of FFT input
    std::unique_ptr<float[], PsMallocDeleter> sp_real_left_channel;            // Real part of FFT input
    std::unique_ptr<float[], PsMallocDeleter> sp_imag_left_channel;            // Imaginary part of FFT input
    std::unique_ptr<float[], PsMallocDeleter> sp_magnitudes_right_channel;     // FFT magnitudes
    std::unique_ptr<float[], PsMallocDeleter> sp_magnitudes_left_channel;      // FFT magnitudes
    size_t m_samplesSinceLastFFT;           // Counter for samples pushed since the last FFT
    UBaseType_t m_uxPriority;
    BaseType_t m_xCoreID;

    TaskHandle_t m_fft_Data_Getter_TaskHandle = nullptr;    // Task handle for the FFT Data Getter
    TaskHandle_t m_fft_Calculator_TaskHandle = nullptr;     // Task handle for the FFT Data Getter
    QueueHandle_t m_FFT_Data_Input_QueueHandle = nullptr;
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
        , m_samplesSinceLastFFT(0)
        , m_magnitudeSize(m_fftSize/2) {}
    FFT_Computer(int fftSize, int hopSize, float f_s, DataWidth_t dataWidth, UBaseType_t uxPriority, BaseType_t xCoreID)
        : m_fftSize(fftSize)
        , m_hopSize(hopSize)
        , m_f_s(f_s)
        , m_dataWidth(dataWidth)
        , m_uxPriority(uxPriority)
        , m_xCoreID(xCoreID)
        , m_isInitialized(false)
        , m_IsMultithreaded(true)
        , m_samplesSinceLastFFT(0)
        , m_magnitudeSize(m_fftSize/2) {}

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
            if(m_fft_Data_Getter_TaskHandle)
            {
                ESP_LOGD("~FFT_Computer", "m_fft_Data_Getter_TaskHandle");
                vTaskDelete(m_fft_Data_Getter_TaskHandle);
                m_fft_Data_Getter_TaskHandle= nullptr;
            }
        }
    }
    
    void TackOnSomeMultithreadedDelay(int msDelay)
    {
        if(m_IsMultithreaded)
        {
            vTaskDelay(pdMS_TO_TICKS(msDelay));
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
        mp_ringBuffer = new ShocksRingBuffer(m_fftSize * 2);

        sp_real_right_channel = std::unique_ptr<float[], PsMallocDeleter>(static_cast<float*>(ps_malloc(m_fftSize * sizeof(float))));
        sp_imag_right_channel = std::unique_ptr<float[], PsMallocDeleter>(static_cast<float*>(ps_malloc(m_fftSize * sizeof(float))));
        sp_real_left_channel = std::unique_ptr<float[], PsMallocDeleter>(static_cast<float*>(ps_malloc(m_fftSize * sizeof(float))));
        sp_imag_left_channel = std::unique_ptr<float[], PsMallocDeleter>(static_cast<float*>(ps_malloc(m_fftSize * sizeof(float))));
        sp_magnitudes_right_channel = std::unique_ptr<float[], PsMallocDeleter>(static_cast<float*>(ps_malloc(m_magnitudeSize * sizeof(float))));
        sp_magnitudes_left_channel = std::unique_ptr<float[], PsMallocDeleter>(static_cast<float*>(ps_malloc(m_magnitudeSize * sizeof(float))));

        if(m_IsMultithreaded)
        {
            m_FFT_Data_Input_QueueHandle = xQueueCreate(20, sizeof(Frame_t*) );
            if(m_FFT_Data_Input_QueueHandle) ESP_LOGD("Setup", "FFT Data Input Queue Created.");
            else ESP_LOGE("Setup", "ERROR! Error creating FFT Data Input Queue.");
            
            xTaskCreate(Static_Process_FFT_Task, "FFT Task", 5000, this, m_uxPriority, &m_fft_Calculator_TaskHandle);
            if(m_fft_Calculator_TaskHandle) ESP_LOGD("Setup", "FFT Calculator Task Started.");
            else ESP_LOGE("Setup", "ERROR! Error creating FFT Calculator Task.");            
        }
        else
        {
            m_FFT_Data_Input_QueueHandle = xQueueCreate(1, sizeof(std::vector<Frame_t>*) );
            if(m_FFT_Data_Input_QueueHandle) ESP_LOGD("Setup", "FFT Data Input Queue Created.");
            else ESP_LOGE("Setup", "ERROR! Error creating FFT Data Input Queue.");
        }
        m_isInitialized = true;
    }

    void PushFrames(Frame_t *frames, size_t count)
    {
        static LogWithRateLimit_Average<size_t> Push_Frames_RLL(1000, ESP_LOG_INFO);
        static LogWithRateLimit Push_Frames_Not_Initialized_RLL(1000, ESP_LOG_WARN);
        static LogWithRateLimit Push_Frames_Null_Pointer_RLL(1000, ESP_LOG_WARN);
        static LogWithRateLimit Push_Frames_Busy_RLL(1000, ESP_LOG_WARN);
        static LogWithRateLimit Push_Frames_Dropped_RLL(1000, ESP_LOG_WARN);
        static LogWithRateLimit Push_Frames_Threaded_RLL(1000, ESP_LOG_INFO);
        static LogWithRateLimit Push_Frames_Unthreaded_RLL(1000, ESP_LOG_INFO);

        if (!m_isInitialized)
        {
            Push_Frames_Not_Initialized_RLL.Log(ESP_LOG_WARN, "PushFrames", "WARNING! FFT class not initialized. Please call setup() first.");
            return;
        }

        if(!frames)
        {
            Push_Frames_Null_Pointer_RLL.Log(ESP_LOG_WARN, "PushFrames", "WARNING! NULL Pointers.");
            return;
        }
        m_samplesSinceLastFFT += mp_ringBuffer->push(frames, count, pdMS_TO_TICKS(0));
        Push_Frames_RLL.LogWithValue(ESP_LOG_INFO, "PushFrames", "Push Frames: \"" + std::to_string(m_samplesSinceLastFFT) + "\"", m_samplesSinceLastFFT);
        if (m_samplesSinceLastFFT >= m_hopSize)
        {
            ESP_LOGD("PushFrames", "Hop Length Met");
            if(m_IsMultithreaded)
            {
                Push_Frames_Threaded_RLL.Log(ESP_LOG_INFO, "PushFrames", "Perform FFT Threaded");
                Get_FFT_Data();
            }
            else
            {
                Push_Frames_Unthreaded_RLL.Log(ESP_LOG_INFO, "PushFrames", "Perform FFT UnThreaded");
                Get_FFT_Data();
                ProcessFFT();
            }
            m_samplesSinceLastFFT = 0;
        }
    }

    float* GetMagnitudes() {
        return sp_magnitudes_right_channel.get();
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

    void Get_FFT_Data()
    {
        static LogWithRateLimit PerformFFT_Reading_Buffers_RLL(1000, ESP_LOG_DEBUG);
        static LogWithRateLimit PerformFFT_Done_Reading_Buffers_RLL(1000, ESP_LOG_DEBUG);
        static LogWithRateLimit PerformFFT_Queued_Success_RLL(1000, ESP_LOG_INFO);
        static LogWithRateLimit PerformFFT_Queued_Fail_RLL(1000, ESP_LOG_WARN);
        static LogWithRateLimit PerformFFT_Read_Failure_RLL(1000, ESP_LOG_WARN);
        if(m_FFT_Data_Input_QueueHandle)
        {
            PerformFFT_Reading_Buffers_RLL.Log(ESP_LOG_DEBUG, "Get_FFT_Data", ("Requesting " + std::to_string(m_fftSize) + " Frames for FFT Processing.").c_str());
            std::unique_ptr<Frame_t[], PsMallocDeleter> sp_frames = std::unique_ptr<Frame_t[], PsMallocDeleter>(static_cast<Frame_t*>(ps_malloc(sizeof(Frame_t) * m_fftSize)));
            size_t receivedFrameCount = mp_ringBuffer->get(sp_frames.get(), m_fftSize, pdMS_TO_TICKS(0));
            PerformFFT_Done_Reading_Buffers_RLL.Log(ESP_LOG_DEBUG, "Get_FFT_Data", ("Received " + std::to_string(receivedFrameCount) + " Frames for FFT Processing:").c_str());
            if(receivedFrameCount == m_fftSize)
            {   
                Frame_t *temp_frames = sp_frames.release();
                if(xQueueSend(m_FFT_Data_Input_QueueHandle, &temp_frames, pdMS_TO_TICKS(0)) != pdTRUE)
                {
                    PerformFFT_Queued_Fail_RLL.Log(ESP_LOG_INFO, "Get_FFT_Data", ("Unable to Queue " + std::to_string(m_fftSize) + " Frames.").c_str());
                }
                else
                {
                    PerformFFT_Queued_Success_RLL.Log(ESP_LOG_INFO, "Get_FFT_Data", ("Queued " + std::to_string(m_fftSize) + " Frames.").c_str());
                }
            }
            else
            {
                PerformFFT_Read_Failure_RLL.Log(ESP_LOG_WARN, "Get_FFT_Data", "Unexpected buffer size read.");
            }
        }
        else
        {
            ESP_LOGW("Get_FFT_Data", "WARNING! No task Handle.");
        }
    }

    static void Static_Process_FFT_Task(void* pvParameters)
    {
        FFT_Computer* fft = (FFT_Computer*)pvParameters;
        fft->Process_FFT_Task();  
    }

    void Process_FFT_Task() 
    {
        static LogWithRateLimit PerformFFTTask_RLL(1000, ESP_LOG_INFO);
        while(true)
        {
            PerformFFTTask_RLL.Log(ESP_LOG_INFO, "Process_FFT_Task", "Process FFT Task Called");
            ProcessFFT();
        }
    }

    void ProcessFFT()
    {
        static LogWithRateLimit ProcessFFT_FFT_Started_RLL(1000, ESP_LOG_INFO);
        static LogWithRateLimit ProcessFFT_FFT_DeQueue_Fail_RLL(1000, ESP_LOG_WARN);
        static LogWithRateLimit ProcessFFT_FFT_Complete_RLL(1000, ESP_LOG_INFO);
        static LogWithRateLimit ProcessFFT_Calling_Callbacks_RLL(1000, ESP_LOG_INFO);
        static LogWithRateLimit ProcessFFT_Null_Pointers_RLL(1000, ESP_LOG_ERROR);
        
        if( sp_real_right_channel &&
            sp_real_left_channel &&
            sp_imag_right_channel &&
            sp_imag_left_channel &&
            mp_CallBack &&
            m_FFT_Data_Input_QueueHandle )
        {
            size_t messages = uxQueueMessagesWaiting(m_FFT_Data_Input_QueueHandle);
            for(int i = 0; i < messages; ++i)
            {
                if(i == 0) ProcessFFT_FFT_Started_RLL.Log(ESP_LOG_INFO, "ProcessFFT", "Process FFT Started");

                Frame_t* p_frames = nullptr;
                if(xQueueReceive(m_FFT_Data_Input_QueueHandle, &p_frames, pdMS_TO_TICKS(0)) == pdTRUE )
                {
                    if(p_frames)
                    {
                        for (int j = 0; j < m_fftSize; j++)
                        {
                            sp_real_left_channel[j] = p_frames[j].channel1;
                            sp_imag_left_channel[j] = 0.0f;
                            sp_real_right_channel[j] = p_frames[j].channel2;
                            sp_imag_right_channel[j] = 0.0f;
                        }
                        free(p_frames);
                    }
                }
                else
                {
                    ProcessFFT_FFT_DeQueue_Fail_RLL.Log(ESP_LOG_WARN, "ProcessFFT", "WARNING! Failed to DeQueue frames.");
                    TackOnSomeMultithreadedDelay(10);
                    return;
                }

                unsigned long startTime = millis();
                ComputeFFT(sp_real_left_channel.get(), sp_imag_left_channel.get(), m_fftSize);
                ComputeFFT(sp_real_right_channel.get(), sp_imag_right_channel.get(), m_fftSize);
                unsigned long stopTime = millis();

                ProcessFFT_FFT_Complete_RLL.Log(ESP_LOG_INFO, "ProcessFFT", "FFT Calculation Completed");

                float maxMagnitude = GetMaxMagnitude();
                

                std::unique_ptr<FFT_Bin_Data_t[], PsMallocDeleter> sp_freqMags_left = std::unique_ptr<FFT_Bin_Data_t[], PsMallocDeleter>(static_cast<FFT_Bin_Data_t*>(ps_malloc(m_magnitudeSize * sizeof(FFT_Bin_Data_t))));
                std::unique_ptr<FFT_Bin_Data_t[], PsMallocDeleter> sp_freqMags_right = std::unique_ptr<FFT_Bin_Data_t[], PsMallocDeleter>(static_cast<FFT_Bin_Data_t*>(ps_malloc(m_magnitudeSize * sizeof(FFT_Bin_Data_t))));
                
                size_t maxBin_Left = 0;
                size_t maxBin_Right = 0;
                
                for (int j = 0; j < m_magnitudeSize; ++j)
                {
                    sp_magnitudes_right_channel[j] = sqrtf(sp_real_right_channel[j] * sp_real_right_channel[j] +sp_imag_right_channel[j] * sp_imag_right_channel[j]);
                    sp_magnitudes_left_channel[j] = sqrtf(sp_real_left_channel[j] * sp_real_left_channel[j] + sp_imag_left_channel[j] * sp_imag_left_channel[j]);
                    if(sp_magnitudes_left_channel[j] > sp_freqMags_left[maxBin_Left].Magnitude)
                    {
                        maxBin_Left = j;
                    }
                    if(sp_magnitudes_right_channel[j] > sp_freqMags_right[maxBin_Right].Magnitude)
                    {
                        maxBin_Right = j;
                    }

                    sp_freqMags_right[j].Frequency = binToFrequency(j);
                    sp_freqMags_right[j].Magnitude = sp_magnitudes_right_channel[j];
                    sp_freqMags_right[j].NormalizedMagnitude = sp_magnitudes_right_channel[j] / maxMagnitude;

                    sp_freqMags_left[j].Frequency = binToFrequency(j);
                    sp_freqMags_left[j].Magnitude = sp_magnitudes_left_channel[j];
                    sp_freqMags_left[j].NormalizedMagnitude = sp_magnitudes_left_channel[j] / maxMagnitude;
                }
                
                ProcessFFT_Calling_Callbacks_RLL.Log(ESP_LOG_INFO, "ProcessFFT", "Calling Callback");
                std::unique_ptr<FFT_Bin_Data_Set_t> sp_FFT_Bin_Data_Set = std::make_unique<FFT_Bin_Data_Set_t>(std::move(sp_freqMags_left), std::move(sp_freqMags_right), maxBin_Left, maxBin_Right, m_magnitudeSize);
                mp_CallBack(std::move(sp_FFT_Bin_Data_Set), mp_CallBackArgs);
                TackOnSomeMultithreadedDelay(5);
            }
            TackOnSomeMultithreadedDelay(10);
        }
        else
        {
            ProcessFFT_Null_Pointers_RLL.Log(ESP_LOG_ERROR, "ProcessFFT", "ERROR! Null Pointers");
            TackOnSomeMultithreadedDelay(10);
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
        TackOnSomeMultithreadedDelay(1);
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