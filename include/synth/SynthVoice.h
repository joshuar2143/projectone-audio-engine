#pragma once

#include <array>
#include <cstddef>
#include <vector>

namespace projectone::synth {

struct SynthParams {
    float cutoffHz {1800.0f};
    float resonance {0.2f};
    float attackSec {0.01f};
    float decaySec {0.2f};
    float sustain {0.75f};
    float releaseSec {0.3f};
    float lfoRateHz {4.0f};
    float lfoToCutoff {350.0f};
    float lfoToPitch {0.2f};
    float masterGain {0.2f};
};

struct MidiEvent {
    std::size_t frameOffset {};
    int note {};
    float velocity {};
    bool noteOn {};
};

class SynthVoiceEngine {
public:
    void prepare(double sampleRate, std::size_t maxVoices);
    void setParams(const SynthParams& params);
    void render(float* left, float* right, std::size_t frames, const std::vector<MidiEvent>& midi);

private:
    struct Voice {
        bool active {false};
        int note {0};
        float velocity {0.0f};
        float phase {0.0f};
        float env {0.0f};
        bool releasing {false};
    };

    float midiNoteToHz(int note) const;
    float nextLfo();
    void processMidiAtFrame(std::size_t frame, const std::vector<MidiEvent>& midi);
    float renderVoice(Voice& voice);
    float lowPassTick(float in);

    double m_sampleRate {48000.0};
    std::vector<Voice> m_voices {};
    SynthParams m_params {};
    float m_lfoPhase {0.0f};
    float m_filterState {0.0f};
};

} // namespace projectone::synth
