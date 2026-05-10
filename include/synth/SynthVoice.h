#pragma once

#include <array>
#include <cstddef>
#include <vector>

namespace projectone::synth {

struct SynthParams {
    // The base cutoff frequency of the voice. higher -> higher cutoff frequency
    float cutoffHz {1800.0f};
    // The resonance of the voice. higher -> higher resonance
    float resonance {0.2f};
    // How long to attack the voice after the note is pressed higher -> longer attack time
    float attackSec {0.01f};
    // How long to decay the voice after the note is pressed higher -> longer decay time
    float decaySec {0.2f};
    // The sustain leve' of the voice. 0.0f -> off, 1.0f -> full volume
    float sustain {0.75f};
    // How long to release the voice after the note is released higher -> longer release time
    float releaseSec {5.3f};
    float lfoRateHz {4.0f};
    // How much to modulate the cutoff by the lfo. higher -> higher cutoff modulation
    float lfoToCutoff {350.0f};
    // How much to modulate the pitch by the lfo. higher -> higher pitch modulation
    float lfoToPitch {0.1f};
    // The master gain of the voice. higher -> higher volume
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
    void applyMidiEvent(const MidiEvent& evt);
    float renderVoice(Voice& voice, float lfo);
    float lowPassTick(float in, float cutoffHz);

    double m_sampleRate {48000.0};
    std::vector<Voice> m_voices {};
    SynthParams m_params {};
    float m_lfoPhase {0.0f};
    float m_filterState {0.0f};
    float m_cutoffBase {0.0f};
};

} // namespace projectone::synth
