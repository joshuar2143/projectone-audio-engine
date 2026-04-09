#include "engine/AudioMetrics.h"

namespace projectone::engine {

void AudioMetrics::setLastCallbackMs(double ms) { m_lastCallbackMs.store(ms); }
void AudioMetrics::incrementXrun() { m_xruns.fetch_add(1); }
void AudioMetrics::addRenderedFrames(std::uint64_t frames) { m_renderedFrames.fetch_add(frames); }

double AudioMetrics::lastCallbackMs() const { return m_lastCallbackMs.load(); }
std::uint64_t AudioMetrics::xruns() const { return m_xruns.load(); }
std::uint64_t AudioMetrics::renderedFrames() const { return m_renderedFrames.load(); }

} // namespace projectone::engine
