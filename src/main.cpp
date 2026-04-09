#include "engine/AudioEngine.h"
#include "persistence/ProjectSerializer.h"

#include <iostream>

int main() {
    projectone::engine::AudioEngine engine;
    engine.prepare(12000.0, 512, 2);

    if (!engine.renderOfflineWav("demo_render.wav", 12000 * 12)) {
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
