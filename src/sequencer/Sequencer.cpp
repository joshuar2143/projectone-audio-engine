#include "sequencer/Sequencer.h"

#include <algorithm>
#include <cmath>
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
    if (frames == 0 || m_totalSteps == 0 || m_notes.empty()) {
        m_sampleCursor += frames;
        return out;
    }

    const double stepSamples = (60.0 / m_bpm) * m_sampleRate / 4.0; // 16th grid
    if (stepSamples <= 0.0) {
        m_sampleCursor += frames;
        return out;
    }

    const std::size_t blockStart = m_sampleCursor;
    const std::size_t blockEnd = m_sampleCursor + frames;
    const std::size_t firstStepIdx = static_cast<std::size_t>(std::ceil(static_cast<double>(blockStart) / stepSamples));

    const std::size_t maxStepBoundaries =
        static_cast<std::size_t>(std::ceil(static_cast<double>(frames) / stepSamples)) + 1;
    out.reserve(maxStepBoundaries * m_notes.size() * 2);

    for (std::size_t stepIdx = firstStepIdx;; ++stepIdx) {
        const std::size_t sampleOfStep = static_cast<std::size_t>(std::llround(stepIdx * stepSamples));
        if (sampleOfStep >= blockEnd) {
            break;
        }

        const std::size_t step = stepIdx % m_totalSteps;
        const std::size_t frameOffset = sampleOfStep - blockStart;
        for (const auto& n : m_notes) {
            if (n.startStep == step) {
                out.push_back(projectone::synth::MidiEvent {frameOffset, n.note, n.velocity, true});
            }
            if ((n.startStep + n.lengthSteps) % m_totalSteps == step) {
                out.push_back(projectone::synth::MidiEvent {frameOffset, n.note, 0.0f, false});
            }
        }
    }

    // Keep deterministic order for same-sample events.
    std::sort(out.begin(), out.end(), [](const auto& a, const auto& b) {
        if (a.frameOffset != b.frameOffset) {
            return a.frameOffset < b.frameOffset;
        }
        if (a.noteOn != b.noteOn) {
            return a.noteOn < b.noteOn; // note-off before note-on at same frame
        }
        return a.note < b.note;
    });

    m_sampleCursor += frames;
    return out;
}

} // namespace projectone::sequencer
