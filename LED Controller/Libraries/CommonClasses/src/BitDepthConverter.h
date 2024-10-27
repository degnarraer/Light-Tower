#include <cstdint>
#include <vector>
#include <esp_types.h>
#include "driver/i2s.h"

class BitDepthConverter {
public:
    static std::vector<uint8_t> ConvertBitDepth(const uint8_t* inputBuffer, size_t inputSize, i2s_bits_per_sample_t inputBits, i2s_bits_per_sample_t outputBits);
    
    // Convert 8-bit data to other bit depths
    static std::vector<int16_t> Convert8To16(const std::vector<int8_t>& input);
    static std::vector<int32_t> Convert8To24(const std::vector<int8_t>& input);
    static std::vector<int32_t> Convert8To32(const std::vector<int8_t>& input);

    // Convert 16-bit data to other bit depths
    static std::vector<int8_t> Convert16To8(const std::vector<int16_t>& input);
    static std::vector<int32_t> Convert16To24(const std::vector<int16_t>& input);
    static std::vector<int32_t> Convert16To32(const std::vector<int16_t>& input);

    // Convert 24-bit data to other bit depths
    static std::vector<int8_t> Convert24To8(const std::vector<int32_t>& input);
    static std::vector<int16_t> Convert24To16(const std::vector<int32_t>& input);
    static std::vector<int32_t> Convert24To32(const std::vector<int32_t>& input);

    // Convert 32-bit data to other bit depths
    static std::vector<int8_t> Convert32To8(const std::vector<int32_t>& input);
    static std::vector<int16_t> Convert32To16(const std::vector<int32_t>& input);
    static std::vector<int32_t> Convert32To24(const std::vector<int32_t>& input);
private:
    static int32_t ConvertSample(int32_t sample, int inputBits, int outputBits);
};
