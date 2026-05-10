#include "engine/AudioEngine.h"
#include "persistence/ProjectSerializer.h"
#include "sequencer/Sequencer.h"

#include <cmath>
#include <cstring>
#include <iostream>

static int runMidiTimingVerification() {
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

    const double sp16 = seq.samplesPerSixteenthNote();
    const std::size_t totalFrames = static_cast<std::size_t>(std::llround(sp16 * 48));
    const bool ok = seq.verifyMidiTiming(totalFrames, 512, std::cout);
    return ok ? 0 : 1;
}

int main(int argc, char** argv) {
    if (argc >= 2 && std::strcmp(argv[1], "--verify-midi") == 0) {
        return runMidiTimingVerification();
    }

    projectone::engine::AudioEngine engine;
    engine.prepare(48000.0, 512, 2);

    if (!engine.renderOfflineWav("demo_render.wav", 48000 * 20)) {
        std::cerr << "Failed to render demo wav\n";
        return 1;
    }

    projectone::persistence::ProjectState project;
    project.notes = {{60, 0.8f, 0, 4}, {67, 0.8f, 8, 4}};
    if (!projectone::persistence::ProjectSerializer::saveJson("demo_project.json", project)) {
        std::cerr << "Failed to save project\n";
        return 1;
    }

    std::cout << "Render complete. Last callback ms: " << engine.metrics().lastCallbackMs() << "\n";
    return 0;
}
