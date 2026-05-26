#pragma once

#include "synth/SynthVoice.h"

#include <cstddef>
#include <iosfwd>
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
    /// Fills internal buffer; no heap allocation after capacity stabilizes.
    const std::vector<projectone::synth::MidiEvent>& buildMidiForBlock(std::size_t frames);

    void resetTransport();
    double samplesPerSixteenthNote() const;

    /// Verifies absolute sample times on the 16th-note grid and against the pattern.
    /// If verbose is false, prints a short summary plus a few sample events (and any failures).
    bool verifyMidiTiming(std::size_t totalFrames, std::size_t blockSize, std::ostream& out, bool verbose = false);

private:
    std::size_t m_sampleCursor {0};
    double m_sampleRate {48000.0};
    double m_bpm {120.0};
    std::size_t m_ppq {96};
    std::size_t m_totalSteps {16};
    std::vector<PatternNote> m_notes {};
    std::vector<projectone::synth::MidiEvent> m_midiEvents {};
};

} // namespace projectone::sequencer
