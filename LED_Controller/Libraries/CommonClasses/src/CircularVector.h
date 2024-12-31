class ShocksRingBuffer {
public:
    ShocksRingBuffer(size_t size)
        : bufferSize(size), writeIndex(0), readIndex(0),
          isFull(false), p_buffer(std::make_unique<Frame_t[]>(size)) {
        mutex = xSemaphoreCreateMutex();
        if (!mutex) {
            throw std::runtime_error("Failed to create mutex");
        }
    }

    ~ShocksRingBuffer() {
        if (mutex) {
            vSemaphoreDelete(mutex);
            mutex = nullptr;
        }
    }

    // Push a single frame into the ring buffer
    void push(Frame_t frame, TickType_t waitTime) {
        if (xSemaphoreTake(mutex, waitTime) == pdTRUE) {
            p_buffer[writeIndex] = frame;
            writeIndex = (writeIndex + 1) % bufferSize;

            if (isFull) { // Overwrite logic
                readIndex = (readIndex + 1) % bufferSize;
            }

            isFull = (writeIndex == readIndex); // Update full flag
            xSemaphoreGive(mutex);
        }
    }

    // Push multiple frames into the ring buffer
    size_t push(Frame_t* frames, size_t count, TickType_t waitTime) {
        size_t pushed = 0;
        if (frames && xSemaphoreTake(mutex, waitTime) == pdTRUE) {
            for (size_t i = 0; i < count; ++i) {
                p_buffer[writeIndex] = frames[i];
                writeIndex = (writeIndex + 1) % bufferSize;

                if (isFull) { // Overwrite logic
                    readIndex = (readIndex + 1) % bufferSize;
                }

                isFull = (writeIndex == readIndex); // Update full flag
                pushed++;
            }
            xSemaphoreGive(mutex);
        }
        return pushed;
    }

    // Retrieve frames into a vector
    size_t get(std::vector<Frame_t>& frames, size_t count, TickType_t waitTime) {
        size_t returned = 0;
        if (xSemaphoreTake(mutex, waitTime) == pdTRUE) {
            size_t available = getAvailableFrames();
            size_t framesToReturn = std::min(count, available);
            frames.resize(framesToReturn);

            for (size_t i = 0; i < framesToReturn; ++i) {
                frames[i] = p_buffer[(readIndex + i) % bufferSize];
            }

            returned = framesToReturn;
            xSemaphoreGive(mutex);
        }
        return returned;
    }

    // Retrieve frames into a raw array
    size_t get(Frame_t* frames, size_t count, TickType_t waitTime) {
        size_t returned = 0;
        if (xSemaphoreTake(mutex, waitTime) == pdTRUE) {
            size_t available = getAvailableFrames();
            size_t framesToReturn = std::min(count, available);

            for (size_t i = 0; i < framesToReturn; ++i) {
                frames[i] = p_buffer[(readIndex + i) % bufferSize];
            }

            returned = framesToReturn;
            xSemaphoreGive(mutex);
        }
        return returned;
    }

private:
    size_t getAvailableFrames() const {
        if (isFull) {
            return bufferSize; // If full, all slots are available
        }
        return (writeIndex >= readIndex)
            ? (writeIndex - readIndex)
            : (bufferSize - readIndex + writeIndex);
    }

    std::unique_ptr<Frame_t[]> p_buffer;
    size_t bufferSize;
    size_t writeIndex;
    size_t readIndex;
    bool isFull; // Flag to track whether the buffer is full
    SemaphoreHandle_t mutex = nullptr;
};
