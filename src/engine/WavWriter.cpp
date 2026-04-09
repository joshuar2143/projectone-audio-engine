#include "engine/WavWriter.h"

#include <algorithm>
#include <cstdint>
#include <fstream>

namespace projectone::engine {

namespace {
template <typename T>
void writeValue(std::ofstream& file, T value) {
    file.write(reinterpret_cast<const char*>(&value), sizeof(T));
}

} // namespace

bool WavWriter::writePcm16(const std::string& filePath, const AudioBuffer& buffer, int sampleRate) {
    std::ofstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }

    const auto channels = static_cast<std::uint16_t>(buffer.channels());
    const auto samplesPerChannel = static_cast<std::uint32_t>(buffer.samples());
    const std::uint16_t bitsPerSample = 16;
    const std::uint16_t blockAlign = channels * (bitsPerSample / 8);
    const std::uint32_t byteRate = static_cast<std::uint32_t>(sampleRate) * blockAlign;
    const std::uint32_t dataSize = samplesPerChannel * blockAlign;
    file.write("RIFF", 4);
    writeValue<std::uint32_t>(file, 36 + dataSize);
    file.write("WAVE", 4);
    file.write("fmt ", 4);
    writeValue<std::uint32_t>(file, 16);
    writeValue<std::uint16_t>(file, 1);
    writeValue<std::uint16_t>(file, channels);
    writeValue<std::uint32_t>(file, static_cast<std::uint32_t>(sampleRate));
    writeValue<std::uint32_t>(file, byteRate);
    writeValue<std::uint16_t>(file, blockAlign);
    writeValue<std::uint16_t>(file, bitsPerSample);
    file.write("data", 4);
    writeValue<std::uint32_t>(file, dataSize);

    for (std::size_t s = 0; s < buffer.samples(); ++s) {
        for (std::size_t c = 0; c < buffer.channels(); ++c) {
            const float v = std::clamp(buffer.channelData(c)[s], -1.0f, 1.0f);
            const auto pcm = static_cast<std::int16_t>(v * 32767.0f);
            writeValue<std::int16_t>(file, pcm);
        }
    }
    return true;
}

} // namespace projectone::engine
