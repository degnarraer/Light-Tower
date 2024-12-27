#include <Arduino.h>
#include <math.h>
#include <vector>
#include <memory>
#include "xtensa/core-macros.h"  // For ESP32 low-level operations
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "Helpers.h"
#include "CircularVector.h"

struct FFT_Bin_Data
{
    float Frequency = 0.0f;
    float Magnitude = 0.0f;
    float NormalizedMagnitude = 0.0f;
};
typedef FFT_Bin_Data FFT_Bin_Data_t;

struct FFT_Bin_Data_Set
{
    std::vector<FFT_Bin_Data_t>* Left_Channel;
    std::vector<FFT_Bin_Data_t>* Right_Channel;
    size_t MaxRightBin = 0;
    size_t MaxLeftBin = 0;
    FFT_Bin_Data_Set(){};
    FFT_Bin_Data_Set( std::vector<FFT_Bin_Data_t>* Left_Channel_in
                    , std::vector<FFT_Bin_Data_t>* Right_Channel_in
                    , size_t MaxRightBin_in
                    , size_t MaxLeftBin_in )
                    {
                        Left_Channel = Left_Channel_in;
                        Right_Channel = Right_Channel_in;
                        MaxRightBin = MaxRightBin_in;
                        MaxLeftBin = MaxLeftBin_in;
                    }
    virtual ~FFT_Bin_Data_Set()
    {
        ESP_LOGD("~FFT_Bin_Data_Set", "Deleting Data Set");
        delete Left_Channel;
        delete Right_Channel;
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
    typedef void FFT_Results_Callback(FFT_Bin_Data_Set_t *FFT_Bin_Data, void* args);
private:
    FFT_Results_Callback* mp_CallBack = nullptr;
    bool m_IsMultithreaded = false;
    void* mp_CallBackArgs = nullptr;
    size_t m_fftSize;                                       // FFT size (e.g., 8192)
    size_t m_magnitudeSize; 
    size_t m_hopSize;                                       // Hop size (number of samples between FFTs)
    float m_f_s;                                            // Sample Rate
    ShocksRingBuffer* mp_ringBuffer;                        // Ring Buffer
    std::vector<Frame_t> m_frames;                          // frame Buffer
    std::vector<float> m_real_right_channel;                // Real part of FFT input
    std::vector<float> m_imag_right_channel;                // Imaginary part of FFT input
    std::vector<float> m_real_left_channel;                 // Real part of FFT input
    std::vector<float> m_imag_left_channel;                 // Imaginary part of FFT input
    std::vector<float> m_magnitudes_right_channel;          // FFT magnitudes
    std::vector<float> m_magnitudes_left_channel;           // FFT magnitudes
    size_t m_samplesSinceLastFFT;                           // Counter for samples pushed since the last FFT
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

    void Setup(FFT_Results_Callback* callback, void* callBackArgs) {
        mp_CallBack = callback;
        mp_CallBackArgs = callBackArgs;
        if (m_isInitialized)
            return;
        mp_ringBuffer = new ShocksRingBuffer(m_fftSize * 2);
        m_real_right_channel.resize(m_fftSize, 0.0f);
        m_imag_right_channel.resize(m_fftSize, 0.0f);
        m_real_left_channel.resize(m_fftSize, 0.0f);
        m_imag_left_channel.resize(m_fftSize, 0.0f);
        m_magnitudes_right_channel.resize(m_magnitudeSize, 0.0f);
        m_magnitudes_left_channel.resize(m_magnitudeSize, 0.0f);

        if(m_IsMultithreaded)
        {
            m_FFT_Data_Input_QueueHandle = xQueueCreate(20, sizeof(std::vector<Frame_t>*) );
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
        static LogWithRateLimit Push_Frames_Not_Initialized_RLL(1000, ESP_LOG_WARN);
        static LogWithRateLimit Push_Frames_Null_Pointer_RLL(1000, ESP_LOG_WARN);
        static LogWithRateLimit Push_Frames_Busy_RLL(1000, ESP_LOG_WARN);
        static LogWithRateLimit Push_Frames_Dropped_RLL(1000, ESP_LOG_WARN);
        static LogWithRateLimit Push_Frames_Threaded_RLL(1000, ESP_LOG_INFO);
        static LogWithRateLimit Push_Frames_Unthreaded_RLL(1000, ESP_LOG_DEBUG);

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
        size_t actualPushCount = mp_ringBuffer->push(frames, count, pdMS_TO_TICKS(0));
        m_samplesSinceLastFFT += actualPushCount;
        ESP_LOGD("PushFrames", "Samples Since Last FFT: \"%i\" Hop Length: \"%i\"", m_samplesSinceLastFFT, m_hopSize);
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
                Push_Frames_Unthreaded_RLL.Log(ESP_LOG_DEBUG, "PushFrames", "Perform FFT UnThreaded");
                Get_FFT_Data();
                ProcessFFT();
            }
            m_samplesSinceLastFFT = 0;
        }
    }

    float* GetMagnitudes() {
        return m_magnitudes_right_channel.data();
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
        static LogWithRateLimit PerformFFT_Read_Success_RLL(1000, ESP_LOG_INFO);
        static LogWithRateLimit PerformFFT_Read_Failure_RLL(1000, ESP_LOG_WARN);
        if(m_FFT_Data_Input_QueueHandle)
        {
            PerformFFT_Reading_Buffers_RLL.Log(ESP_LOG_DEBUG, "Get_FFT_Data", ("Requesting " + std::to_string(m_fftSize) + " Frames for FFT Processing.").c_str());

            //Ownership of p_Frame is given to the queue and should be deleted by the recepiant.
            std::vector<Frame_t>* p_Frames = new std::vector<Frame_t>(m_fftSize);
            size_t receivedFrameCount = mp_ringBuffer->get((*p_Frames), m_fftSize, pdMS_TO_TICKS(0));
            PerformFFT_Done_Reading_Buffers_RLL.Log(ESP_LOG_DEBUG, "Get_FFT_Data", ("Received " + std::to_string(receivedFrameCount) + " Frames for FFT Processing:").c_str());
            if(receivedFrameCount == m_fftSize)
            {
                PerformFFT_Read_Success_RLL.Log(ESP_LOG_INFO, "Get_FFT_Data", ("Queueing " + std::to_string(m_fftSize) + " Frames.").c_str());
                if(xQueueSend(m_FFT_Data_Input_QueueHandle, &p_Frames, pdMS_TO_TICKS(0)) != pdTRUE)
                {
                    delete p_Frames;
                }
            }
            else
            {
                PerformFFT_Read_Failure_RLL.Log(ESP_LOG_WARN, "Get_FFT_Data", "Unexpected buffer size read.");
                delete p_Frames;
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
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }

    void ProcessFFT()
    {
        static LogWithRateLimit ProcessFFT_FFT_Started_RLL(1000, ESP_LOG_INFO);
        static LogWithRateLimit ProcessFFT_FFT_Complete_RLL(1000, ESP_LOG_DEBUG);
        static LogWithRateLimit ProcessFFT_Calling_Callbacks_RLL(1000, ESP_LOG_DEBUG);
        static LogWithRateLimit ProcessFFT_No_Callback_RLL(1000, ESP_LOG_WARN);
        static LogWithRateLimit ProcessFFT_Buffer_Size_RLL(1000, ESP_LOG_WARN);
        
        if( m_real_right_channel.size() == m_fftSize &&
            m_real_left_channel.size() == m_fftSize &&
            m_imag_right_channel.size() == m_fftSize &&
            m_imag_left_channel.size() == m_fftSize )
        {
            if (mp_CallBack && m_FFT_Data_Input_QueueHandle)
            {
                size_t messages = uxQueueMessagesWaiting(m_FFT_Data_Input_QueueHandle);
                for(int i = 0; i < messages; ++i)
                {
                    if(i == 0) ProcessFFT_FFT_Started_RLL.Log(ESP_LOG_INFO, "ProcessFFT", "Process FFT Started");
                    std::vector<Frame_t>* p_frames;
                    if( xQueueReceive(m_FFT_Data_Input_QueueHandle, &p_frames, pdMS_TO_TICKS(0)) == pdTRUE )
                    {
                        for(int i = 0; i < m_fftSize; ++i)
                        {
                            m_real_left_channel[i] = (*p_frames)[i].channel1;
                            m_real_right_channel[i] = (*p_frames)[i].channel2;
                            m_imag_right_channel[i] = 0.0f;
                            m_imag_left_channel[i] = 0.0f;
                        }
                        delete p_frames;

                        ComputeFFT(m_real_left_channel, m_imag_left_channel);
                        ComputeFFT(m_real_right_channel, m_imag_right_channel);
                        ProcessFFT_FFT_Complete_RLL.Log(ESP_LOG_DEBUG, "ProcessFFT", "FFT Calculation Completed");

                        float maxMagnitude = GetMaxMagnitude();
                        
                        //Ownership of FFT_Bin_Data_t is given to FFT_Bin_Data_Set_t
                        std::vector<FFT_Bin_Data_t>* p_freqMags_left = new std::vector<FFT_Bin_Data_t>(m_magnitudeSize);
                        std::vector<FFT_Bin_Data_t>* p_freqMags_right = new std::vector<FFT_Bin_Data_t>(m_magnitudeSize);
                        size_t maxBin_Left = 0;
                        size_t maxBin_Right = 0;
                        
                        for (int i = 0; i < m_magnitudeSize; i++)
                        {
                            m_magnitudes_right_channel[i] = sqrtf(m_real_right_channel[i] * m_real_right_channel[i] +
                                                                    m_imag_right_channel[i] * m_imag_right_channel[i]);
                            m_magnitudes_left_channel[i] = sqrtf(m_real_left_channel[i] * m_real_left_channel[i] +
                                                                m_imag_left_channel[i] * m_imag_left_channel[i]);
                            if(m_magnitudes_left_channel[i] > (*p_freqMags_left)[maxBin_Left].Magnitude) maxBin_Left = i;
                            if(m_magnitudes_right_channel[i] > (*p_freqMags_right)[maxBin_Right].Magnitude) maxBin_Right = i;

                            (*p_freqMags_right)[i].Frequency = binToFrequency(i);
                            (*p_freqMags_right)[i].Magnitude = m_magnitudes_right_channel[i];
                            (*p_freqMags_right)[i].NormalizedMagnitude = m_magnitudes_right_channel[i] / maxMagnitude;

                            (*p_freqMags_left)[i].Frequency = binToFrequency(i);
                            (*p_freqMags_left)[i].Magnitude = m_magnitudes_left_channel[i];
                            (*p_freqMags_left)[i].NormalizedMagnitude = m_magnitudes_left_channel[i] / maxMagnitude;
                        }
                        
                        ProcessFFT_Calling_Callbacks_RLL.Log(ESP_LOG_DEBUG, "ProcessFFT", "Calling Callback");

                        //Ownership of fft_Bin_Data_Set is given to the callback
                        FFT_Bin_Data_Set_t *fft_Bin_Data_Set = new FFT_Bin_Data_Set_t( p_freqMags_left, p_freqMags_right, maxBin_Left, maxBin_Right );
                        mp_CallBack(fft_Bin_Data_Set, mp_CallBackArgs);
                        if(m_IsMultithreaded)
                        {
                            vTaskDelay(pdMS_TO_TICKS(1));
                        }
                    }
                    else
                    {
                        //Shouldn't ever get here and cannot recover if we do get here.
                    }
                }
            }
            else
            {
                ProcessFFT_No_Callback_RLL.Log(ESP_LOG_WARN, "ProcessFFT", "No Callback");
            }
        }
        else
        {
            ProcessFFT_Buffer_Size_RLL.Log(ESP_LOG_WARN, "ProcessFFT", "Buffer Size Issue");
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
        vTaskDelay(pdMS_TO_TICKS(1));
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