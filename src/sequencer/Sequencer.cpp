#include "sequencer/Sequencer.h"

#include <utility>

namespace projectone::sequencer {

void Sequencer::prepare(double sampleRate, double bpm, std::size_t ppq) {
    m_sampleRate = sampleRate;
    m_bpm = bpm;
    m_ppq = ppq;
    m_sampleCursor = 0;
}

void Sequencer::setPattern(std::vector<PatternNote> notes, std::size_t totalSteps) {
    m_notes = std::move(notes);
    m_totalSteps = totalSteps;
}

std::vector<projectone::synth::MidiEvent> Sequencer::buildMidiForBlock(std::size_t frames) {
    std::vector<projectone::synth::MidiEvent> out;
    const double beatSamples = (60.0 / m_bpm) * m_sampleRate;
    const double stepSamples = beatSamples / 4.0; // 16th grid

    for (std::size_t i = 0; i < frames; ++i) {
        const std::size_t step = static_cast<std::size_t>((m_sampleCursor + i) / stepSamples) % m_totalSteps;
        for (const auto& n : m_notes) {
            if (n.startStep == step) {
                out.push_back(projectone::synth::MidiEvent {i, n.note, n.velocity, true});
            }
            if ((n.startStep + n.lengthSteps) % m_totalSteps == step) {
                out.push_back(projectone::synth::MidiEvent {i, n.note, 0.0f, false});
            }
        }
    }
    m_sampleCursor += frames;
    return out;
}

} // namespace projectone::sequencer
