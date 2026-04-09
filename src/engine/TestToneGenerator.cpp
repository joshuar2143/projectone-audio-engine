#include "engine/TestToneGenerator.h"

#include <cmath>

namespace projectone::engine {

void TestToneGenerator::prepare(double sampleRate, float frequency) {
    m_sampleRate = sampleRate;
    m_frequency = frequency;
}

float TestToneGenerator::nextSample() {
    constexpr float twoPi = 6.28318530718f;
    const float sample = std::sin(m_phase);
    m_phase += twoPi * m_frequency / static_cast<float>(m_sampleRate);
    if (m_phase > twoPi) {
        m_phase -= twoPi;
    }
    return sample;
}

void TestToneGenerator::reset() { m_phase = 0.0f; }

} // namespace projectone::engine
