#pragma once

#include <cstddef>
#include <vector>

namespace projectone::effects {

class EffectsChain {
public:
    void prepare(double sampleRate, std::size_t blockSize);
    void process(float* left, float* right, std::size_t frames);

private:
    std::vector<float> m_delayL {};
    std::vector<float> m_delayR {};
    std::size_t m_writeIdx {0};
    float m_feedback {0.28f};
    float m_mix {0.22f};
};

} // namespace projectone::effects
