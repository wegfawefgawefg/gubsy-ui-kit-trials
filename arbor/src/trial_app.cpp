#include "trial_app.hpp"

#include <utility>

TrialScreen::TrialScreen(std::filesystem::path source, std::filesystem::path components,
                         std::filesystem::path font, TrialModel& model, ActionHandler handler)
    : AppClient({.axl_path = std::move(source),
                 .components_dir = components,
                 .font_path = std::move(font)}),
      components_(std::move(components)), model_(model) {
    // bind shared scalar state
    bind_model(model_)
        .field("destination", &TrialModel::destination)
        .field("controllerStatus", &TrialModel::controller_status)
        .field("playView", &TrialModel::play_view)
        .field("playActivity", &TrialModel::play_activity)
        .field("playAccess", &TrialModel::play_access)
        .field("playHost", &TrialModel::play_host)
        .field("playersTab", &TrialModel::players_tab)
        .field("settingsTab", &TrialModel::settings_tab)
        .field("controlsTab", &TrialModel::controls_tab)
        .field("modsTab", &TrialModel::mods_tab)
        .field("resolution", &TrialModel::resolution)
        .field("frameCap", &TrialModel::frame_cap)
        .field("fullscreen", &TrialModel::fullscreen)
        .field("vsync", &TrialModel::vsync)
        .field("subtitles", &TrialModel::subtitles)
        .field("compatibleOnly", &TrialModel::compatible_only)
        .field("renderScale", &TrialModel::render_scale)
        .field("brightness", &TrialModel::brightness)
        .field("masterVolume", &TrialModel::master_volume)
        .field("lookSensitivity", &TrialModel::look_sensitivity)
        .field("stickDeadzone", &TrialModel::stick_deadzone)
        .field("vibration", &TrialModel::vibration)
        .field("sharedLives", &TrialModel::shared_lives)
        .field("startingHealth", &TrialModel::starting_health)
        .field("ghostArrival", &TrialModel::ghost_arrival)
        .field("responseCurve", &TrialModel::response_curve)
        .field("expeditionDifficulty", &TrialModel::expedition_difficulty)
        .field("stageVariation", &TrialModel::stage_variation)
        .field("shopFrequency", &TrialModel::shop_frequency)
        .field("discoveredShortcuts", &TrialModel::discovered_shortcuts)
        .field("sharedTreasury", &TrialModel::shared_treasury)
        .field("friendlyFire", &TrialModel::friendly_fire)
        .field("selectedProfile", &TrialModel::selected_profile)
        .field("selectedCampaign", &TrialModel::selected_campaign)
        .field("selectedMod", &TrialModel::selected_mod)
        .field("modActionStatus", &TrialModel::mod_action_status)
        .field("selectedAction", &TrialModel::selected_action)
        .field("catalogFilter", &TrialModel::catalog_filter)
        .field("playerReady", &TrialModel::player_ready)
        .field("captureBinding", &TrialModel::capture_binding);
    on_event(std::move(handler));
}

arbor::Result<void> TrialScreen::initialize() {
    // register shared shell and local tabs
    const auto string_type = interner().intern("string");
    auto shell = register_composite(
        components_ / "shell.axl",
        {.name = "Shell",
         .kind = arbor::runtime::WidgetKind::composite,
         .properties = {{.name = "active", .type = string_type},
                        {.name = "controller", .type = string_type}},
         .events = {{.name = "navigate", .payload_type = string_type}},
         .slots = {{.name = "default"}},
         .model = arbor::runtime::ModelContract{.property = "active", .event = "navigate"}});
    if (!shell.has_value())
        return shell;

    auto tabs = register_composite(
        components_ / "tabs.axl",
        {.name = "Tabs",
         .kind = arbor::runtime::WidgetKind::composite,
         .properties = {{.name = "active", .type = string_type}},
         .events = {{.name = "change", .payload_type = string_type}},
         .slots = {{.name = "default"}},
         .model = arbor::runtime::ModelContract{.property = "active", .event = "change"}});
    if (!tabs.has_value())
        return tabs;
    return AppClient::initialize();
}

const arbor::core::FrameProfile& TrialScreen::profile() const noexcept {
    return const_cast<TrialScreen*>(this)->app().frame_profile();
}
