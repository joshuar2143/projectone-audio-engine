#include "engine/AudioEngine.h"
#include "engine/WavWriter.h"

#include <chrono>
#include <vector>

namespace projectone::engine {

void AudioEngine::prepare(double sampleRate, std::size_t blockSize, std::size_t channels) {
    m_sampleRate = sampleRate;
    m_blockSize = blockSize;
    m_channels = channels;

    m_synth.prepare(sampleRate, 16);
    m_fx.prepare(sampleRate, blockSize);
    m_sequencer.prepare(sampleRate, 50.0, 96);
    m_sequencer.setPattern({{90, 0.8f, 0, 4}, {60, 0.7f, 4, 4}, {70, 0.7f, 8, 4}, {65, 0.9f, 12, 4}, {62, 0.9f, 4, 8}, {52, 0.9f, 8, 8}}, 16);
}

void AudioEngine::process(AudioBuffer& buffer) {
    const auto start = std::chrono::high_resolution_clock::now();
    buffer.clear();
    auto midi = m_sequencer.buildMidiForBlock(buffer.samples());
    m_synth.render(buffer.channelData(0), buffer.channelData(1), buffer.samples(), midi);
    m_fx.process(buffer.channelData(0), buffer.channelData(1), buffer.samples());
    m_metrics.addRenderedFrames(buffer.samples());

    const auto end = std::chrono::high_resolution_clock::now();
    const std::chrono::duration<double, std::milli> elapsed = end - start;
    m_metrics.setLastCallbackMs(elapsed.count());
}

bool AudioEngine::renderOfflineWav(const char* path, std::size_t frames) {
    AudioBuffer buffer(m_channels, frames);
    process(buffer);
    return WavWriter::writePcm16(path, buffer, static_cast<int>(m_sampleRate));
}

} // namespace projectone::engine
