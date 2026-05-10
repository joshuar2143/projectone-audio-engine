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
    std::vector<projectone::synth::MidiEvent> buildMidiForBlock(std::size_t frames);

    void resetTransport();
    double samplesPerSixteenthNote() const;

    /// Walks the transport in fixed-size blocks, prints each MIDI event, and checks that absolute
    /// sample times land on the 16th-note grid implied by BPM. Also checks events against the
    /// current pattern (note-on at startStep, note-off at startStep+length wrapped).
    bool verifyMidiTiming(std::size_t totalFrames, std::size_t blockSize, std::ostream& out);

private:
    std::size_t m_sampleCursor {0};
    double m_sampleRate {48000.0};
    double m_bpm {120.0};
    std::size_t m_ppq {96};
    std::size_t m_totalSteps {16};
    std::vector<PatternNote> m_notes {};
};

} // namespace projectone::sequencer
