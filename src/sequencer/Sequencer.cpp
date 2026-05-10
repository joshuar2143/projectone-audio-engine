#include "sequencer/Sequencer.h"

#include <algorithm>
#include <cmath>
#include <ostream>
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

void Sequencer::resetTransport() {
    m_sampleCursor = 0;
}

double Sequencer::samplesPerSixteenthNote() const {
    return (60.0 / m_bpm) * m_sampleRate / 4.0;
}

std::vector<projectone::synth::MidiEvent> Sequencer::buildMidiForBlock(std::size_t frames) {
    std::vector<projectone::synth::MidiEvent> out;
    if (frames == 0 || m_totalSteps == 0 || m_notes.empty()) {
        m_sampleCursor += frames;
        return out;
    }

    const double stepSamples = samplesPerSixteenthNote();
    if (stepSamples <= 0.0) {
        m_sampleCursor += frames;
        return out;
    }

    const std::size_t blockStart = m_sampleCursor;
    const std::size_t blockEnd = m_sampleCursor + frames;
    const std::size_t firstStepIdx = static_cast<std::size_t>(std::ceil(static_cast<double>(blockStart) / stepSamples));

    // Calculate the maximum number of step boundaries that will be needed for the current block.
    const std::size_t maxStepBoundaries =
        static_cast<std::size_t>(std::ceil(static_cast<double>(frames) / stepSamples)) + 1;
    out.reserve(maxStepBoundaries * m_notes.size() * 2); // reserve space for the worst case 
    // This is a common optimization technique in C++ to avoid unnecessary memory allocations during the loop.

    for (std::size_t stepIdx = firstStepIdx;; ++stepIdx) {
        // Convert the step index to a sample index, which of the sample in the block that the step corresponds to.
        const std::size_t sampleOfStep = static_cast<std::size_t>(std::llround(stepIdx * stepSamples));
        if (sampleOfStep >= blockEnd) {
            break;
        }

        const std::size_t step = stepIdx % m_totalSteps;
        const std::size_t frameOffset = sampleOfStep - blockStart;
        for (const auto& n : m_notes) {
            // check if the note starts at the current step, 4 == 4 means the note starts at the current step.
            if (n.startStep == step) {
                out.push_back(projectone::synth::MidiEvent {frameOffset, n.note, n.velocity, true});
            }
            // check if the note ends at the current step, 4 + 4 == 8 means the note ends at the 8th step, 
            // if note ends aftere the total steps, the % m_totalSteps will wrap around to 0.
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

bool Sequencer::verifyMidiTiming(std::size_t totalFrames, std::size_t blockSize, std::ostream& out) {
    resetTransport();
    const double stepSamples = samplesPerSixteenthNote();

    out << "MIDI timing check (16th-note grid)\n";
    out << "  sampleRate=" << m_sampleRate << " bpm=" << m_bpm << " samplesPer16th=" << stepSamples;
    out << " (16th duration " << (60.0 / m_bpm / 4.0) << " s)\n";
    out << "  totalFrames=" << totalFrames << " blockSize=" << blockSize << " patternSteps=" << m_totalSteps << "\n";

    if (totalFrames == 0 || blockSize == 0 || m_bpm <= 0.0 || stepSamples <= 0.0) {
        out << "  abort: invalid parameters\n";
        return false;
    }

    bool allOk = true;
    std::size_t blockStart = 0;
    while (blockStart < totalFrames) {
        const std::size_t n = std::min(blockSize, totalFrames - blockStart);
        const std::vector<projectone::synth::MidiEvent> events = buildMidiForBlock(n);
        for (const auto& evt : events) {
            const std::size_t absSample = blockStart + evt.frameOffset;
            const auto stepIdx =
                static_cast<std::size_t>(std::llround(static_cast<double>(absSample) / stepSamples));
            const auto predicted =
                static_cast<std::size_t>(std::llround(static_cast<double>(stepIdx) * stepSamples));
            const bool gridOk = (predicted == absSample);
            const std::size_t stepInPattern = stepIdx % m_totalSteps;

            bool patternOk = false;
            if (evt.noteOn) {
                for (const auto& pn : m_notes) {
                    if (pn.note == evt.note && pn.startStep == stepInPattern) {
                        patternOk = true;
                        break;
                    }
                }
            } else {
                for (const auto& pn : m_notes) {
                    if (pn.note == evt.note
                        && (pn.startStep + pn.lengthSteps) % m_totalSteps == stepInPattern) {
                        patternOk = true;
                        break;
                    }
                }
            }

            const bool lineOk = gridOk && patternOk;
            allOk = allOk && lineOk;

            out << "  absSample=" << absSample << " frameOff=" << evt.frameOffset << " block0=" << blockStart
                << (evt.noteOn ? " NOTE_ON " : " NOTE_OFF") << " note=" << evt.note << " vel=" << evt.velocity
                << " stepIdx=" << stepIdx << " stepInPat=" << stepInPattern << " grid=" << (gridOk ? "ok" : "BAD")
                << " pattern=" << (patternOk ? "ok" : "BAD") << "\n";
        }
        blockStart += n;
    }

    out << (allOk ? "All events aligned.\n" : "MISMATCH detected.\n");
    return allOk;
}

} // namespace projectone::sequencer
