
#include "BitDepthConverter.h"

std::vector<uint8_t> BitDepthConverter::convertBitDepth(const uint8_t* inputBuffer, size_t inputSize, i2s_bits_per_sample_t inputBits, i2s_bits_per_sample_t outputBits){
    int inputBitDepth = static_cast<int>(inputBits);
    int outputBitDepth = static_cast<int>(outputBits);

    // If the input and output bit depths are the same, return a copy of the input buffer
    if (inputBitDepth == outputBitDepth) {
        return std::vector<uint8_t>(inputBuffer, inputBuffer + inputSize);
    }

    size_t inputBytesPerSample = inputBitDepth / 8;
    size_t outputBytesPerSample = outputBitDepth / 8;
    size_t outputSize = (inputSize / inputBytesPerSample) * outputBytesPerSample;

    std::vector<uint8_t> outputBuffer(outputSize);
    size_t inputIndex = 0;
    size_t outputIndex = 0;

    while (inputIndex < inputSize) {
        int32_t sample = 0;

        // Read sample based on input bit depth
        switch (inputBitDepth) {
            case 8:
                sample = static_cast<int8_t>(inputBuffer[inputIndex]);
                break;
            case 16:
                sample = static_cast<int16_t>(inputBuffer[inputIndex] | (inputBuffer[inputIndex + 1] << 8));
                break;
            case 24:
                sample = (static_cast<int32_t>(inputBuffer[inputIndex]) |
                          (static_cast<int32_t>(inputBuffer[inputIndex + 1]) << 8) |
                          (static_cast<int32_t>(inputBuffer[inputIndex + 2]) << 16));
                // Adjust sign for 24-bit sample
                if (sample & 0x800000) sample |= 0xFF000000;
                break;
            case 32:
                sample = static_cast<int32_t>(inputBuffer[inputIndex] |
                                              (inputBuffer[inputIndex + 1] << 8) |
                                              (inputBuffer[inputIndex + 2] << 16) |
                                              (inputBuffer[inputIndex + 3] << 24));
                break;
            default:
                return {}; // Unsupported bit depth
        }

        sample = convertSample(sample, inputBitDepth, outputBitDepth);

        // Write sample based on output bit depth
        switch (outputBitDepth) {
            case 8:
                outputBuffer[outputIndex] = static_cast<uint8_t>(sample & 0xFF);
                break;
            case 16:
                outputBuffer[outputIndex] = static_cast<uint8_t>(sample & 0xFF);
                outputBuffer[outputIndex + 1] = static_cast<uint8_t>((sample >> 8) & 0xFF);
                break;
            case 24:
                outputBuffer[outputIndex] = static_cast<uint8_t>(sample & 0xFF);
                outputBuffer[outputIndex + 1] = static_cast<uint8_t>((sample >> 8) & 0xFF);
                outputBuffer[outputIndex + 2] = static_cast<uint8_t>((sample >> 16) & 0xFF);
                break;
            case 32:
                outputBuffer[outputIndex] = static_cast<uint8_t>(sample & 0xFF);
                outputBuffer[outputIndex + 1] = static_cast<uint8_t>((sample >> 8) & 0xFF);
                outputBuffer[outputIndex + 2] = static_cast<uint8_t>((sample >> 16) & 0xFF);
                outputBuffer[outputIndex + 3] = static_cast<uint8_t>((sample >> 24) & 0xFF);
                break;
            default:
                return {}; // Unsupported bit depth
        }

        inputIndex += inputBytesPerSample;
        outputIndex += outputBytesPerSample;
    }

    return outputBuffer;
}

// Helper function to convert a single sample between bit depths
int32_t BitDepthConverter::convertSample(int32_t sample, int inputBits, int outputBits) {
    int32_t convertedSample = sample;

    if (inputBits < outputBits) {
        // Shift up if converting to a higher bit depth
        convertedSample <<= (outputBits - inputBits);
    } else if (inputBits > outputBits) {
        // Shift down if converting to a lower bit depth, apply sign extension if necessary
        convertedSample >>= (inputBits - outputBits);
    }

    return convertedSample;
}

// Implementation of conversion methods

// Convert 8-bit to 16-bit
std::vector<int16_t> BitDepthConverter::convert8To16(const std::vector<int8_t>& input) {
    std::vector<int16_t> output(input.size());
    for (size_t i = 0; i < input.size(); i++) {
        output[i] = static_cast<int16_t>(input[i]) << 8; // Shift up to 16-bit
    }
    return output;
}

// Convert 8-bit to 24-bit
std::vector<int32_t> BitDepthConverter::convert8To24(const std::vector<int8_t>& input) {
    std::vector<int32_t> output(input.size());
    for (size_t i = 0; i < input.size(); i++) {
        output[i] = static_cast<int32_t>(input[i]) << 16; // Shift up to 24-bit
    }
    return output;
}

// Convert 8-bit to 32-bit
std::vector<int32_t> BitDepthConverter::convert8To32(const std::vector<int8_t>& input) {
    std::vector<int32_t> output(input.size());
    for (size_t i = 0; i < input.size(); i++) {
        output[i] = static_cast<int32_t>(input[i]) << 24; // Shift up to 32-bit
    }
    return output;
}

// Convert 16-bit to 8-bit
std::vector<int8_t> BitDepthConverter::convert16To8(const std::vector<int16_t>& input) {
    std::vector<int8_t> output(input.size());
    for (size_t i = 0; i < input.size(); i++) {
        output[i] = static_cast<int8_t>(input[i] >> 8); // Shift down to 8-bit
    }
    return output;
}

// Convert 16-bit to 24-bit
std::vector<int32_t> BitDepthConverter::convert16To24(const std::vector<int16_t>& input) {
    std::vector<int32_t> output(input.size());
    for (size_t i = 0; i < input.size(); i++) {
        output[i] = static_cast<int32_t>(input[i]) << 8; // Shift up to 24-bit
    }
    return output;
}

// Convert 16-bit to 32-bit
std::vector<int32_t> BitDepthConverter::convert16To32(const std::vector<int16_t>& input) {
    std::vector<int32_t> output(input.size());
    for (size_t i = 0; i < input.size(); i++) {
        output[i] = static_cast<int32_t>(input[i]) << 16; // Shift up to 32-bit
    }
    return output;
}

// Convert 24-bit to 8-bit
std::vector<int8_t> BitDepthConverter::convert24To8(const std::vector<int32_t>& input) {
    std::vector<int8_t> output(input.size());
    for (size_t i = 0; i < input.size(); i++) {
        output[i] = static_cast<int8_t>(input[i] >> 16); // Shift down to 8-bit
    }
    return output;
}

// Convert 24-bit to 16-bit
std::vector<int16_t> BitDepthConverter::convert24To16(const std::vector<int32_t>& input) {
    std::vector<int16_t> output(input.size());
    for (size_t i = 0; i < input.size(); i++) {
        output[i] = static_cast<int16_t>(input[i] >> 8); // Shift down to 16-bit
    }
    return output;
}

// Convert 24-bit to 32-bit
std::vector<int32_t> BitDepthConverter::convert24To32(const std::vector<int32_t>& input) {
    std::vector<int32_t> output(input.size());
    for (size_t i = 0; i < input.size(); i++) {
        output[i] = input[i] << 8; // Shift up to 32-bit
    }
    return output;
}

// Convert 32-bit to 8-bit
std::vector<int8_t> BitDepthConverter::convert32To8(const std::vector<int32_t>& input) {
    std::vector<int8_t> output(input.size());
    for (size_t i = 0; i < input.size(); i++) {
        output[i] = static_cast<int8_t>(input[i] >> 24); // Shift down to 8-bit
    }
    return output;
}

// Convert 32-bit to 16-bit
std::vector<int16_t> BitDepthConverter::convert32To16(const std::vector<int32_t>& input) {
    std::vector<int16_t> output(input.size());
    for (size_t i = 0; i < input.size(); i++) {
        output[i] = static_cast<int16_t>(input[i] >> 16); // Shift down to 16-bit
    }
    return output;
}

// Convert 32-bit to 24-bit
std::vector<int32_t> BitDepthConverter::convert32To24(const std::vector<int32_t>& input) {
    std::vector<int32_t> output(input.size());
    for (size_t i = 0; i < input.size(); i++) {
        output[i] = input[i] >> 8; // Shift down to 24-bit
    }
    return output;
}
