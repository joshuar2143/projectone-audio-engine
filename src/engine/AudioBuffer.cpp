#include "engine/AudioBuffer.h"

#include <algorithm>
#include <stdexcept>

namespace projectone::engine {

AudioBuffer::AudioBuffer(std::size_t channels, std::size_t samples) {
    resize(channels, samples);
}

void AudioBuffer::resize(std::size_t channels, std::size_t samples) {
    m_channels = channels;
    m_samples = samples;
    m_data.assign(m_channels * m_samples, 0.0f);
}

void AudioBuffer::clear() {
    std::fill(m_data.begin(), m_data.end(), 0.0f);
}

float* AudioBuffer::channelData(std::size_t channel) {
    if (channel >= m_channels) {
        throw std::out_of_range("AudioBuffer channel out of range");
    }
    return m_data.data() + (channel * m_samples);
}

const float* AudioBuffer::channelData(std::size_t channel) const {
    if (channel >= m_channels) {
        throw std::out_of_range("AudioBuffer channel out of range");
    }
    return m_data.data() + (channel * m_samples);
}

} // namespace projectone::engine
