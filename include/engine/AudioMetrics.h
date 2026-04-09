#pragma once

#include <atomic>
#include <cstdint>

namespace projectone::engine {

class AudioMetrics {
public:
    void setLastCallbackMs(double ms);
    void incrementXrun();
    void addRenderedFrames(std::uint64_t frames);

    double lastCallbackMs() const;
    std::uint64_t xruns() const;
    std::uint64_t renderedFrames() const;

private:
    std::atomic<double> m_lastCallbackMs {0.0};
    std::atomic<std::uint64_t> m_xruns {0};
    std::atomic<std::uint64_t> m_renderedFrames {0};
};

} // namespace projectone::engine
