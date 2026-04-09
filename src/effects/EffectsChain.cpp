#include "effects/EffectsChain.h"

#include <algorithm>

namespace projectone::effects {

void EffectsChain::prepare(double sampleRate, std::size_t) {
    const auto delaySamples = static_cast<std::size_t>(sampleRate * 0.27);
    m_delayL.assign(delaySamples, 0.0f);
    m_delayR.assign(delaySamples, 0.0f);
    m_writeIdx = 0;
}

void EffectsChain::process(float* left, float* right, std::size_t frames) {
    if (m_delayL.empty()) {
        return;
    }
    for (std::size_t i = 0; i < frames; ++i) {
        const float dl = m_delayL[m_writeIdx];
        const float dr = m_delayR[m_writeIdx];
        const float inL = left[i];
        const float inR = right[i];

        m_delayL[m_writeIdx] = inL + (dl * m_feedback);
        m_delayR[m_writeIdx] = inR + (dr * m_feedback);

        left[i] = (1.0f - m_mix) * inL + (m_mix * dl);
        right[i] = (1.0f - m_mix) * inR + (m_mix * dr);
        m_writeIdx = (m_writeIdx + 1) % m_delayL.size();
    }
}

} // namespace projectone::effects
