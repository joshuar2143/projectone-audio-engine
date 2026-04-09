#pragma once

#include "effects/EffectsChain.h"
#include "engine/AudioBuffer.h"
#include "engine/AudioMetrics.h"
#include "sequencer/Sequencer.h"
#include "synth/SynthVoice.h"

#include <cstddef>

namespace projectone::engine {

class AudioEngine {
public:
    void prepare(double sampleRate, std::size_t blockSize, std::size_t channels);
    void process(AudioBuffer& buffer);
    bool renderOfflineWav(const char* path, std::size_t frames);

    AudioMetrics& metrics() { return m_metrics; }
    projectone::synth::SynthVoiceEngine& synth() { return m_synth; }
    projectone::sequencer::Sequencer& sequencer() { return m_sequencer; }

private:
    double m_sampleRate {48000.0};
    std::size_t m_blockSize {512};
    std::size_t m_channels {2};
    AudioMetrics m_metrics {};
    projectone::synth::SynthVoiceEngine m_synth {};
    projectone::effects::EffectsChain m_fx {};
    projectone::sequencer::Sequencer m_sequencer {};
};

} // namespace projectone::engine
