#include "model.hpp"

TrialModel::TrialModel() {
    values = {
        {"activity", std::string("Continue expedition")}, {"access", std::string("Friends can join")},
        {"host", std::string("Automatic")}, {"fullscreen", true},
        {"resolution", std::string("1920 × 1080")}, {"render-scale", 1.0},
        {"frame-cap", std::string("144 FPS")}, {"brightness", 0.64},
        {"vsync", std::string("Adaptive")}, {"master-volume", 0.82},
        {"music-volume", 0.64}, {"effects-volume", 0.90}, {"dialogue-volume", 1.0},
        {"dynamic-range", std::string("Night")}, {"high-contrast", true},
        {"reduce-motion", false}, {"tutorials", std::string("Smart")},
        {"camera-shake", 0.35}, {"shared-lives", 4.0}, {"starting-health", 4.0},
        {"ghost-arrival", 180.0}, {"shortcuts", true}, {"treasury", true},
        {"friendly-fire", false}, {"look-sensitivity", 0.45},
        {"stick-deadzone", 0.12}, {"vibration", 0.80}, {"trigger-deadzone", 0.05},
        {"response-curve", std::string("Smooth")}, {"invert-y", false},
        {"compatible-only", false}, {"mod-version", std::string("v1.4.0")},
        {"catalog-search", std::string("")},
    };
}

// Exposes typed fake provider state through the same contract a game uses.
gview::Value TrialModel::read(std::string_view key) const {
    const auto found = values.find(std::string(key));
    return found == values.end() ? gview::Value{} : found->second;
}

std::string TrialModel::text(std::string_view key, std::string fallback) const {
    const gview::Value value = read(key);
    if (const std::string* text_value = std::get_if<std::string>(&value)) return *text_value;
    return fallback;
}

bool TrialModel::flag(std::string_view key, bool fallback) const {
    const gview::Value value = read(key);
    if (const bool* flag_value = std::get_if<bool>(&value)) return *flag_value;
    return fallback;
}

void TrialModel::write(std::string_view key, const gview::Value& value) {
    values[std::string(key)] = value;
    ++revision;
}

std::string destination_name(Destination destination) {
    switch (destination) {
    case Destination::Play: return "Play";
    case Destination::Players: return "Players";
    case Destination::Settings: return "Settings";
    case Destination::Controls: return "Controls";
    case Destination::Progress: return "Progress";
    case Destination::Mods: return "Mods";
    }
    return "Play";
}
