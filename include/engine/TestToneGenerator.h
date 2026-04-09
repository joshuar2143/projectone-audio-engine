#pragma once

#include <cstddef>

namespace projectone::engine {

class TestToneGenerator {
public:
    void prepare(double sampleRate, float frequency);
    float nextSample();
    void reset();

private:
    double m_sampleRate {48000.0};
    float m_frequency {440.0f};
    float m_phase {0.0f};
};

} // namespace projectone::engine
