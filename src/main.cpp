#include "engine/AudioEngine.h"
#include "persistence/ProjectSerializer.h"
#include "sequencer/Sequencer.h"

#include <cmath>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>

namespace {

void printBanner(const char* title) {
    std::cout << "\n========================================\n";
    std::cout << title << "\n";
    std::cout << "========================================\n";
}

void printPass(const std::string& msg) { std::cout << "[PASS] " << msg << "\n"; }
void printFail(const std::string& msg) { std::cerr << "[FAIL] " << msg << "\n"; }

projectone::sequencer::Sequencer makeVerificationSequencer() {
    projectone::sequencer::Sequencer seq;
    seq.prepare(48000.0, 50.0, 96);
    seq.setPattern(
        {
            {90, 0.8f, 0, 4},
            {60, 0.7f, 4, 4},
            {70, 0.7f, 8, 4},
            {65, 0.9f, 12, 4},
            {62, 0.9f, 4, 8},
            {52, 0.9f, 8, 8},
        },
        16);
    return seq;
}

bool runMidiTimingVerification(bool verbose) {
    printBanner("CHECK 2: MIDI @ 50 BPM (events vs step grid)");
    std::cout << "  stepSamples = (60 / BPM) * sampleRate / 4  =>  "
              << ((60.0 / 50.0) * 48000.0 / 4.0) << " @ 48 kHz, 50 BPM\n";
    if (!verbose) {
        std::cout << "  (sample events below; use --verify-midi for full listing)\n";
    }

    auto seq = makeVerificationSequencer();
    const double sp16 = seq.samplesPerSixteenthNote();
    const std::size_t totalFrames = static_cast<std::size_t>(std::llround(sp16 * 48));
    const std::size_t blockSizes[] = {128, 256, 512};

    for (const std::size_t bs : blockSizes) {
        if (!seq.verifyMidiTiming(totalFrames, bs, std::cout, verbose)) {
            printFail("MIDI timing mismatch for blockSize=" + std::to_string(bs));
            return false;
        }
        if (verbose) {
            printPass("blockSize=" + std::to_string(bs) + ": All events aligned");
        }
    }
    if (!verbose) {
        printPass("128 / 256 / 512 block sizes: all events aligned to grid and pattern");
    }
    return true;
}

bool runBufferSizeTests() {
    printBanner("CHECK 1: Execute across buffer sizes 128, 256, 512, full offline frame count");
    constexpr std::size_t fullFrames = 48000 * 20;
    const std::size_t blockSizes[] = {128, 256, 512, fullFrames};

    for (const std::size_t bs : blockSizes) {
        projectone::engine::AudioEngine engine;
        engine.prepare(48000.0, bs, 2);
        const std::string path = "test_buffer_" + std::to_string(bs) + ".wav";
        if (!engine.renderOfflineWav(path.c_str(), fullFrames)) {
            printFail("renderOfflineWav failed for blockSize=" + std::to_string(bs));
            return false;
        }
        std::cout << "  blockSize=" << bs << " -> " << path
                  << " (last process() ms=" << engine.metrics().lastCallbackMs() << ")\n";
        printPass("blockSize=" + std::to_string(bs) + " render OK");
    }
    return true;
}

struct WavSummary {
    bool ok {false};
    std::uint16_t channels {};
    std::uint32_t sampleRate {};
    std::uint16_t bitsPerSample {};
    std::uint32_t framesPerChannel {};
    double durationSec {};
};

WavSummary inspectPcm16Wav(const std::string& path) {
    WavSummary s;
    std::ifstream f(path, std::ios::binary);
    if (!f) {
        return s;
    }

    char riff[4];
    std::uint32_t riffSize {};
    char wave[4];
    if (!f.read(riff, 4) || std::strncmp(riff, "RIFF", 4) != 0 || !f.read(reinterpret_cast<char*>(&riffSize), 4)
        || !f.read(wave, 4) || std::strncmp(wave, "WAVE", 4) != 0) {
        return s;
    }

    char chunkId[4];
    std::uint32_t chunkSize {};
    bool gotFmt = false;
    bool gotData = false;

    while (f.read(chunkId, 4) && f.read(reinterpret_cast<char*>(&chunkSize), 4)) {
        if (std::strncmp(chunkId, "fmt ", 4) == 0 && chunkSize >= 16) {
            std::uint16_t audioFormat {};
            f.read(reinterpret_cast<char*>(&audioFormat), 2);
            f.read(reinterpret_cast<char*>(&s.channels), 2);
            f.read(reinterpret_cast<char*>(&s.sampleRate), 4);
            f.ignore(4); // byteRate
            f.ignore(2); // blockAlign
            f.read(reinterpret_cast<char*>(&s.bitsPerSample), 2);
            if (chunkSize > 16) {
                f.ignore(static_cast<std::streamoff>(chunkSize - 16));
            }
            gotFmt = (audioFormat == 1);
        } else if (std::strncmp(chunkId, "data", 4) == 0) {
            const std::uint32_t bytesPerFrame = s.channels * (s.bitsPerSample / 8);
            if (bytesPerFrame == 0) {
                return s;
            }
            s.framesPerChannel = chunkSize / bytesPerFrame;
            gotData = true;
            break;
        } else {
            f.ignore(static_cast<std::streamoff>(chunkSize));
        }
    }

    if (gotFmt && gotData && s.sampleRate > 0) {
        s.durationSec = static_cast<double>(s.framesPerChannel) / static_cast<double>(s.sampleRate);
        s.ok = true;
    }
    return s;
}

bool runDemoWavCheck() {
    printBanner("CHECK 3: Twenty-second stereo WAV (demo_render.wav)");
    constexpr std::size_t kFrames = 48000 * 20;
    constexpr int kSampleRate = 48000;

    projectone::engine::AudioEngine engine;
    engine.prepare(static_cast<double>(kSampleRate), 512, 2);

    const char* path = "demo_render.wav";
    if (!engine.renderOfflineWav(path, kFrames)) {
        printFail("Could not write demo_render.wav");
        return false;
    }

    const WavSummary wav = inspectPcm16Wav(path);
    if (!wav.ok) {
        printFail("demo_render.wav is missing or not a valid PCM WAV");
        return false;
    }

    std::cout << "  File: " << path << "\n";
    std::cout << "  Sample rate: " << wav.sampleRate << " Hz\n";
    std::cout << "  Channels: " << wav.channels << "\n";
    std::cout << "  Bits: " << wav.bitsPerSample << "\n";
    std::cout << "  Frames/channel: " << wav.framesPerChannel << "\n";
    std::cout << "  Duration: " << wav.durationSec << " s\n";
    std::cout << "  Last process() ms: " << engine.metrics().lastCallbackMs() << "\n";

    bool ok = true;
    if (wav.sampleRate != static_cast<std::uint32_t>(kSampleRate)) {
        printFail("Expected sample rate 48000");
        ok = false;
    }
    if (wav.channels != 2) {
        printFail("Expected stereo (2 channels)");
        ok = false;
    }
    if (wav.bitsPerSample != 16) {
        printFail("Expected PCM-16");
        ok = false;
    }
    if (wav.framesPerChannel != kFrames) {
        printFail("Expected " + std::to_string(kFrames) + " frames per channel");
        ok = false;
    }
    if (std::fabs(wav.durationSec - 20.0) > 0.01) {
        printFail("Expected duration ~20 s");
        ok = false;
    }

    if (ok) {
        printPass("Valid 20 s stereo PCM-16 WAV at 48 kHz — open in any audio app to confirm playback");
    }
    return ok;
}

bool runAllAssessmentChecks() {
    std::cout << "\n";
    printBanner("PROJECT ONE — ASSESSMENT SELF-CHECK");
    std::cout << "Running all criteria checks...\n";

    const bool buffersOk = runBufferSizeTests();
    const bool midiOk = runMidiTimingVerification(false);
    const bool wavOk = runDemoWavCheck();

    printBanner("SUMMARY");
    std::cout << (buffersOk ? "[PASS] " : "[FAIL] ") << "Buffer sizes 128 / 256 / 512 / full offline\n";
    std::cout << (midiOk ? "[PASS] " : "[FAIL] ") << "MIDI sample offsets @ 50 BPM\n";
    std::cout << (wavOk ? "[PASS] " : "[FAIL] ") << "20 s stereo WAV export\n";

    if (buffersOk && midiOk && wavOk) {
        printBanner("ALL CHECKS PASSED");
        return true;
    }
    printBanner("SOME CHECKS FAILED");
    return false;
}

} // namespace

int main(int argc, char** argv) {
    if (argc >= 2 && std::strcmp(argv[1], "--verify-midi") == 0) {
        return runMidiTimingVerification(true) ? 0 : 1;
    }
    if (argc >= 2 && std::strcmp(argv[1], "--test-buffers") == 0) {
        return runBufferSizeTests() ? 0 : 1;
    }
    if (argc >= 2 && std::strcmp(argv[1], "--demo-only") == 0) {
        return runDemoWavCheck() ? 0 : 1;
    }

    if (!runAllAssessmentChecks()) {
        return 1;
    }

    projectone::persistence::ProjectState project;
    project.notes = {{60, 0.8f, 0, 4}, {67, 0.8f, 8, 4}};
    if (!projectone::persistence::ProjectSerializer::saveJson("demo_project.json", project)) {
        printFail("demo_project.json save failed");
        return 1;
    }
    printPass("demo_project.json saved");
    return 0;
}
