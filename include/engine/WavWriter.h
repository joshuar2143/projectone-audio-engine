#pragma once

#include "engine/AudioBuffer.h"

#include <string>

namespace projectone::engine {

class WavWriter {
public:
    static bool writePcm16(const std::string& filePath, const AudioBuffer& buffer, int sampleRate);
};

} // namespace projectone::engine
