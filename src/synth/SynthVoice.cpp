#include "synth/SynthVoice.h"

#include <algorithm>
#include <cmath>

namespace projectone::synth {

void SynthVoiceEngine::prepare(double sampleRate, std::size_t maxVoices) {
    m_sampleRate = sampleRate;
    m_voices.assign(maxVoices, {});
}

void SynthVoiceEngine::setParams(const SynthParams& params) {
    m_params = params;
    m_cutoffBase = params.cutoffHz;
}

float SynthVoiceEngine::midiNoteToHz(int note) const {
    return 440.0f * std::pow(2.0f, (static_cast<float>(note) - 69.0f) / 12.0f);
}

float SynthVoiceEngine::nextLfo() {
    constexpr float twoPi = 6.28318530718f;
    const float out = std::sin(m_lfoPhase);
    m_lfoPhase += twoPi * m_params.lfoRateHz / static_cast<float>(m_sampleRate);
    if (m_lfoPhase >= twoPi) {
        m_lfoPhase -= twoPi;
    }
    return out;
}

void SynthVoiceEngine::applyMidiEvent(const MidiEvent& evt) {
    if (evt.noteOn) {
        auto it = std::find_if(m_voices.begin(), m_voices.end(), [](const Voice& v) { return !v.active; });
        if (it == m_voices.end()) {
            it = m_voices.begin();
        }
        *it = Voice {true, evt.note, evt.velocity, 0.0f, 0.0f, false};
    } else {
        for (auto& v : m_voices) {
            if (v.active && v.note == evt.note) {
                v.releasing = true;
            }
        }
    }
}

float SynthVoiceEngine::lowPassTick(float in, float cutoffHz) {
    const float alpha = std::clamp(cutoffHz / static_cast<float>(m_sampleRate), 0.001f, 0.45f);
    m_filterState += alpha * (in - m_filterState);
    return m_filterState;
}

float SynthVoiceEngine::renderVoice(Voice& voice, float lfo) {
    if (!voice.active) {
        return 0.0f;
    }
    constexpr float twoPi = 6.28318530718f;
    const float freq = midiNoteToHz(voice.note + static_cast<int>(lfo * m_params.lfoToPitch));
    voice.phase += twoPi * freq / static_cast<float>(m_sampleRate);
    if (voice.phase > twoPi) {
        voice.phase -= twoPi;
    }
    const float osc = (voice.phase / 3.14159265f) - 1.0f;

    if (voice.releasing) {
        voice.env -= 1.0f / std::max(1.0f, m_params.releaseSec * static_cast<float>(m_sampleRate));
    } else {
        const float attackDelta = 1.0f / std::max(1.0f, m_params.attackSec * static_cast<float>(m_sampleRate));
        voice.env = std::min(1.0f, voice.env + attackDelta);
        if (voice.env > m_params.sustain) {
            voice.env -= (1.0f - m_params.sustain) / std::max(1.0f, m_params.decaySec * static_cast<float>(m_sampleRate));
        }
    }

    if (voice.env <= 0.0f) {
        voice.active = false;
        return 0.0f;
    }
    return osc * voice.env * voice.velocity;
}

void SynthVoiceEngine::render(float* left, float* right, std::size_t frames, const std::vector<MidiEvent>& midi) {
    std::size_t midiCursor = 0;
    for (std::size_t i = 0; i < frames; ++i) {
        while (midiCursor < midi.size() && midi[midiCursor].frameOffset == i) {
            applyMidiEvent(midi[midiCursor]);
            ++midiCursor;
        }
        const float lfo = nextLfo();
        float sum = 0.0f;
        for (auto& voice : m_voices) {
            sum += renderVoice(voice, lfo);
        }
        const float modCutoff = std::max(60.0f, m_cutoffBase + lfo * m_params.lfoToCutoff);
        const float filtered = lowPassTick(sum, modCutoff) * m_params.masterGain;
        left[i] += filtered;
        right[i] += filtered;
    }
}

} // namespace projectone::synth
