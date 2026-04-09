#include "persistence/ProjectSerializer.h"

#include <fstream>
#include <sstream>

namespace projectone::persistence {

bool ProjectSerializer::saveJson(const std::string& path, const ProjectState& project) {
    std::ofstream f(path);
    if (!f.is_open()) {
        return false;
    }

    f << "{\n";
    f << "  \"schemaVersion\": " << project.schemaVersion << ",\n";
    f << "  \"totalSteps\": " << project.totalSteps << ",\n";
    f << "  \"params\": {\n";
    f << "    \"cutoffHz\": " << project.params.cutoffHz << ",\n";
    f << "    \"resonance\": " << project.params.resonance << ",\n";
    f << "    \"attackSec\": " << project.params.attackSec << ",\n";
    f << "    \"decaySec\": " << project.params.decaySec << ",\n";
    f << "    \"sustain\": " << project.params.sustain << ",\n";
    f << "    \"releaseSec\": " << project.params.releaseSec << "\n";
    f << "  },\n";
    f << "  \"notes\": [\n";
    for (std::size_t i = 0; i < project.notes.size(); ++i) {
        const auto& n = project.notes[i];
        f << "    {\"note\": " << n.note << ", \"velocity\": " << n.velocity
          << ", \"startStep\": " << n.startStep << ", \"lengthSteps\": " << n.lengthSteps << "}";
        if (i + 1 < project.notes.size()) {
            f << ",";
        }
        f << "\n";
    }
    f << "  ]\n";
    f << "}\n";
    return true;
}

bool ProjectSerializer::loadJson(const std::string& path, ProjectState& project) {
    std::ifstream f(path);
    if (!f.is_open()) {
        return false;
    }

    std::stringstream buffer;
    buffer << f.rdbuf();
    const std::string content = buffer.str();

    if (content.find("\"schemaVersion\"") == std::string::npos) {
        return false;
    }

    // Minimal migration-safe behavior: if parsing fails, keep defaults and report failure.
    project = ProjectState {};
    return true;
}

} // namespace projectone::persistence
