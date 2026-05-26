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

const std::vector<projectone::synth::MidiEvent>& Sequencer::buildMidiForBlock(std::size_t frames) {
    m_midiEvents.clear();
    if (frames == 0 || m_totalSteps == 0 || m_notes.empty()) {
        m_sampleCursor += frames;
        return m_midiEvents;
    }

    const double stepSamples = samplesPerSixteenthNote();
    if (stepSamples <= 0.0) {
        m_sampleCursor += frames;
        return m_midiEvents;
    }

    const std::size_t blockStart = m_sampleCursor;
    const std::size_t blockEnd = m_sampleCursor + frames;
    const std::size_t firstStepIdx = static_cast<std::size_t>(std::ceil(static_cast<double>(blockStart) / stepSamples));

    const std::size_t maxStepBoundaries =
        static_cast<std::size_t>(std::ceil(static_cast<double>(frames) / stepSamples)) + 1;
    const std::size_t need = maxStepBoundaries * m_notes.size() * 2;
    if (m_midiEvents.capacity() < need) {
        m_midiEvents.reserve(need);
    }
    // generate the midi events for the block
    for (std::size_t stepIdx = firstStepIdx;; ++stepIdx) {
        const std::size_t sampleOfStep = static_cast<std::size_t>(std::llround(stepIdx * stepSamples));
        // if the sample of the step is greater than the block end, break
        if (sampleOfStep >= blockEnd) {
            break;
        }

        // get the step index
        const std::size_t step = stepIdx % m_totalSteps;
        // get the frame offset
        const std::size_t frameOffset = sampleOfStep - blockStart;
        // generate the midi events for the notes
        for (const auto& n : m_notes) {
            // if the start step of the note is the same as the step, add a note on event
            if (n.startStep == step) {
                m_midiEvents.push_back(projectone::synth::MidiEvent {frameOffset, n.note, n.velocity, true});
            }
            // if the end step of the note is the same as the step, add a note off event
            if ((n.startStep + n.lengthSteps) % m_totalSteps == step) {
                m_midiEvents.push_back(projectone::synth::MidiEvent {frameOffset, n.note, 0.0f, false});
            }
        }
    }

    std::sort(m_midiEvents.begin(), m_midiEvents.end(), [](const auto& a, const auto& b) {
        if (a.frameOffset != b.frameOffset) {
            return a.frameOffset < b.frameOffset;
        }
        if (a.noteOn != b.noteOn) {
            return a.noteOn < b.noteOn;
        }
        return a.note < b.note;
    });

    m_sampleCursor += frames;
    return m_midiEvents;
}

namespace {

void printMidiEventLine(std::ostream& out, const projectone::synth::MidiEvent& evt, std::size_t absSample,
    std::size_t stepIdx, std::size_t stepInPattern, bool gridOk, bool patternOk) {
    out << "    absSample=" << absSample << (evt.noteOn ? " ON " : " OFF") << " note=" << evt.note << " stepIdx="
        << stepIdx << " patStep=" << stepInPattern << " grid=" << (gridOk ? "ok" : "BAD") << " pattern="
        << (patternOk ? "ok" : "BAD") << "\n";
}

} // namespace

bool Sequencer::verifyMidiTiming(std::size_t totalFrames, std::size_t blockSize, std::ostream& out, bool verbose) {
    resetTransport();
    const double stepSamples = samplesPerSixteenthNote();

    if (totalFrames == 0 || blockSize == 0 || m_bpm <= 0.0 || stepSamples <= 0.0) {
        out << "  abort: invalid parameters\n";
        return false;
    }

    if (verbose) {
        out << "MIDI timing check (16th-note grid)\n";
        out << "  sampleRate=" << m_sampleRate << " bpm=" << m_bpm << " samplesPer16th=" << stepSamples;
        out << " (16th duration " << (60.0 / m_bpm / 4.0) << " s)\n";
        out << "  totalFrames=" << totalFrames << " blockSize=" << blockSize << " patternSteps=" << m_totalSteps
            << "\n";
    }

    bool allOk = true;
    std::size_t eventCount = 0;
    std::size_t okSamplesPrinted = 0;
    constexpr std::size_t kMaxOkSamples = 3;
    std::size_t blockStart = 0;

    while (blockStart < totalFrames) {
        const std::size_t n = std::min(blockSize, totalFrames - blockStart);
        const std::vector<projectone::synth::MidiEvent>& events = buildMidiForBlock(n);
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
            ++eventCount;

            if (verbose || !lineOk) {
                printMidiEventLine(out, evt, absSample, stepIdx, stepInPattern, gridOk, patternOk);
            } else if (okSamplesPrinted < kMaxOkSamples) {
                printMidiEventLine(out, evt, absSample, stepIdx, stepInPattern, gridOk, patternOk);
                ++okSamplesPrinted;
            }
        }
        blockStart += n;
    }

    if (!verbose) {
        out << "  blockSize=" << blockSize << ": " << eventCount << " events, samplesPer16th=" << stepSamples;
        if (okSamplesPrinted > 0 && okSamplesPrinted < eventCount && allOk) {
            out << " (showing " << okSamplesPrinted << " sample lines)";
        }
        out << " -> " << (allOk ? "all aligned" : "MISMATCH") << "\n";
    } else {
        out << (allOk ? "All events aligned.\n" : "MISMATCH detected.\n");
    }
    return allOk;
}

} // namespace projectone::sequencer
