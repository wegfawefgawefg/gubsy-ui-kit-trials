#pragma once

#include <string>

struct TrialModel {
    // active shell and local sections
    std::string destination{"play"};
    std::string controller_status{"KEYBOARD / POINTER"};
    std::string play_view{"lobby"};
    std::string play_activity{"Continue expedition"};
    std::string play_access{"Friends can join"};
    std::string play_host{"Automatic"};
    std::string players_tab{"local"};
    std::string settings_tab{"display"};
    std::string controls_tab{"bindings"};
    std::string mods_tab{"catalog"};

    // mutable native settings
    std::string resolution{"1920 x 1080"};
    std::string frame_cap{"144 FPS"};
    bool fullscreen{true};
    bool vsync{true};
    bool subtitles{true};
    bool compatible_only{false};
    double render_scale{100.0};
    double brightness{64.0};
    double master_volume{80.0};
    double look_sensitivity{45.0};
    double stick_deadzone{12.0};
    double vibration{80.0};
    double shared_lives{4.0};
    double starting_health{4.0};
    double ghost_arrival{180.0};
    std::string response_curve{"Smooth"};
    std::string expedition_difficulty{"Standard"};
    std::string stage_variation{"Quest-authored"};
    std::string shop_frequency{"Normal"};
    bool discovered_shortcuts{true};
    bool shared_treasury{true};
    bool friendly_fire{false};

    // selected authored records
    std::string selected_profile{"Moss"};
    std::string selected_campaign{"The Glass Caverns"};
    std::string selected_mod{"Mycelium Below"};
    std::string mod_action_status{"Ready"};
    std::string selected_action{"Menu Up"};
    std::string catalog_filter;
    bool player_ready{true};
    bool capture_binding{false};
};
