#pragma once

#include "synth/SynthVoice.h"

#include <cstddef>
#include <vector>

namespace projectone::sequencer {

struct PatternNote {
    int note {60};
    float velocity {1.0f};
    std::size_t startStep {0};
    std::size_t lengthSteps {4};
};

class Sequencer {
public:
    void prepare(double sampleRate, double bpm, std::size_t ppq);
    void setPattern(std::vector<PatternNote> notes, std::size_t totalSteps);
    std::vector<projectone::synth::MidiEvent> buildMidiForBlock(std::size_t frames);

private:
    std::size_t m_sampleCursor {0};
    double m_sampleRate {48000.0};
    double m_bpm {120.0};
    std::size_t m_ppq {96};
    std::size_t m_totalSteps {16};
    std::vector<PatternNote> m_notes {};
};

} // namespace projectone::sequencer
