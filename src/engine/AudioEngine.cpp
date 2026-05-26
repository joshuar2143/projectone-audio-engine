#include "engine/AudioEngine.h"
#include "engine/WavWriter.h"

#include <algorithm>
#include <chrono>
#include <cstring>

namespace projectone::engine {

namespace {

void copyBlockToOutput(const AudioBuffer& block, AudioBuffer& output, std::size_t destOffset, std::size_t frames) {
    for (std::size_t c = 0; c < block.channels(); ++c) {
        float* dst = output.channelData(c) + destOffset;
        const float* src = block.channelData(c);
        std::memcpy(dst, src, frames * sizeof(float));
    }
}

} // namespace

void AudioEngine::prepare(double sampleRate, std::size_t blockSize, std::size_t channels) {
    m_sampleRate = sampleRate;
    m_blockSize = blockSize;
    m_channels = channels;

    m_blockBuffer.resize(channels, blockSize);
    m_synth.prepare(sampleRate, 16);
    m_fx.prepare(sampleRate, blockSize);
    m_sequencer.prepare(sampleRate, 50.0, 96);
    m_sequencer.setPattern({{90, 0.8f, 0, 4}, {60, 0.7f, 4, 4}, {70, 0.7f, 8, 4}, {65, 0.9f, 12, 4}, {62, 0.9f, 4, 8}, {52, 0.9f, 8, 8}}, 16);
}

void AudioEngine::process(AudioBuffer& buffer) {
    const auto start = std::chrono::high_resolution_clock::now();
    buffer.clear();
    const auto& midi = m_sequencer.buildMidiForBlock(buffer.samples());
    m_synth.render(buffer.channelData(0), buffer.channelData(1), buffer.samples(), midi);
    m_fx.process(buffer.channelData(0), buffer.channelData(1), buffer.samples());
    m_metrics.addRenderedFrames(buffer.samples());

    const auto end = std::chrono::high_resolution_clock::now();
    const std::chrono::duration<double, std::milli> elapsed = end - start;
    m_metrics.setLastCallbackMs(elapsed.count());
}

bool AudioEngine::renderOfflineWav(const char* path, std::size_t frames) {
    AudioBuffer output(m_channels, frames);
    m_sequencer.resetTransport();

    std::size_t written = 0;
    while (written < frames) {
        const std::size_t n = std::min(m_blockSize, frames - written);
        if (m_blockBuffer.samples() != n || m_blockBuffer.channels() != m_channels) {
            m_blockBuffer.resize(m_channels, n);
        }
        process(m_blockBuffer);
        copyBlockToOutput(m_blockBuffer, output, written, n);
        written += n;
    }

    return WavWriter::writePcm16(path, output, static_cast<int>(m_sampleRate));
}

} // namespace projectone::engine
