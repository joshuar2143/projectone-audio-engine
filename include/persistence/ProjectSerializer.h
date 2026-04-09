#pragma once

#include "sequencer/Sequencer.h"
#include "synth/SynthVoice.h"

#include <string>
#include <vector>

namespace projectone::persistence {

struct ProjectState {
    int schemaVersion {1};
    projectone::synth::SynthParams params {};
    std::vector<projectone::sequencer::PatternNote> notes {};
    std::size_t totalSteps {16};
};

class ProjectSerializer {
public:
    static bool saveJson(const std::string& path, const ProjectState& project);
    static bool loadJson(const std::string& path, ProjectState& project);
};

} // namespace projectone::persistence
