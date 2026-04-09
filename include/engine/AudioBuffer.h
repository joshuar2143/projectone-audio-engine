#pragma once

#include <cstddef>
#include <vector>

namespace projectone::engine {

class AudioBuffer {
public:
    AudioBuffer() = default;
    AudioBuffer(std::size_t channels, std::size_t samples);

    void resize(std::size_t channels, std::size_t samples);
    void clear();

    float* channelData(std::size_t channel);
    const float* channelData(std::size_t channel) const;

    std::size_t channels() const { return m_channels; }
    std::size_t samples() const { return m_samples; }

private:
    std::size_t m_channels {0};
    std::size_t m_samples {0};
    std::vector<float> m_data {};
};

} // namespace projectone::engine
