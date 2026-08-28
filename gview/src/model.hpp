#pragma once

#include <gview/gview.hpp>

#include <string>
#include <unordered_map>
#include <vector>

enum class Destination { Play, Players, Settings, Controls, Progress, Mods };
enum class PlayPage { Lobby, Quest, Rules, SessionMods };

struct TrialModel {
    Destination destination = Destination::Play;
    PlayPage play_page = PlayPage::Lobby;
    std::string players_tab = "Local players";
    std::string settings_tab = "Display";
    std::string controls_tab = "Bindings";
    std::string mods_tab = "Installed";
    std::string selected = "The Violet Reach";
    std::string provider_state = "Populated";
    std::unordered_map<std::string, gview::Value> values;
    std::string toast;
    std::string modal;
    std::string pending_focus;
    bool game_ui = false;
    bool rebuild = true;
    std::uint64_t revision = 1;

    TrialModel();
    gview::Value read(std::string_view key) const;
    std::string text(std::string_view key, std::string fallback = {}) const;
    bool flag(std::string_view key, bool fallback = false) const;
    void write(std::string_view key, const gview::Value& value);
};

std::string destination_name(Destination destination);
