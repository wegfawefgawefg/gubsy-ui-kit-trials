#include "view_builder.hpp"

namespace {

void setting_tabs(ViewBuilder& ui, std::string_view content, const TrialModel& model) {
    ui.container(content, "setting-tabs", glayout::ContainerKind::Row,
                 {glayout::LengthKind::Fill, 1.0f}, {glayout::LengthKind::Pixels, 44.0f}, 4.0f);
    for (const char* tab : {"Display", "Audio", "Accessibility", "Gameplay"})
        ui.button("setting-tabs", std::string("setting-tab-") + tab, tab,
                  std::string("settings-tab:") + tab, "setting-tabs", 44.0f,
                  gview::ActivationPolicy::OnFocus);
    ui.spec(std::string("setting-tab-") + model.settings_tab).selected = true;
    ui.focus_group("setting-tabs", std::string("setting-tab-") + model.settings_tab,
                   "nav-Settings");
}

void display(ViewBuilder& ui) {
    ui.toggle("setting-list", "setting-fullscreen", "Fullscreen", "fullscreen", "settings-content");
    ui.select("setting-list", "setting-resolution", "Display resolution", "resolution",
              {"1280 × 720", "1920 × 1080", "2560 × 1440", "3840 × 2160"}, "settings-content");
    ui.slider("setting-list", "setting-render-scale", "Render scale", "render-scale",
              "settings-content", 0.5, 2.0, 0.01);
    ui.select("setting-list", "setting-frame-cap", "Frame cap", "frame-cap",
              {"60 FPS", "120 FPS", "144 FPS", "Unlimited"}, "settings-content");
    ui.slider("setting-list", "setting-brightness", "Brightness", "brightness", "settings-content",
              0.0, 1.0, 0.01);
    ui.select("setting-list", "setting-vsync", "Vertical synchronization", "vsync",
              {"Off", "On", "Adaptive"}, "settings-content");
}

void audio(ViewBuilder& ui) {
    ui.slider("setting-list", "setting-master", "Master volume", "master-volume",
              "settings-content", 0, 1, 0.01);
    ui.slider("setting-list", "setting-music", "Music volume", "music-volume", "settings-content",
              0, 1, 0.01);
    ui.slider("setting-list", "setting-effects", "Effects volume", "effects-volume",
              "settings-content", 0, 1, 0.01);
    ui.slider("setting-list", "setting-dialogue", "Dialogue volume", "dialogue-volume",
              "settings-content", 0, 1, 0.01);
    ui.select("setting-list", "setting-range", "Dynamic range", "dynamic-range",
              {"Night", "Medium", "Wide"}, "settings-content");
    ui.select("setting-list", "setting-output", "Output device", "output-device",
              {"Default", "Headphones", "HDMI"}, "settings-content");
}

void accessibility(ViewBuilder& ui) {
    ui.select("setting-list", "setting-text-scale", "Text scale", "text-scale",
              {"100%", "112%", "125%", "150%"}, "settings-content");
    ui.toggle("setting-list", "setting-contrast", "High contrast focus", "high-contrast",
              "settings-content");
    ui.toggle("setting-list", "setting-motion", "Reduce motion", "reduce-motion",
              "settings-content");
    ui.select("setting-list", "setting-filter", "Color filter", "color-filter",
              {"Off", "Deuteranopia", "Protanopia", "Tritanopia"}, "settings-content");
    ui.toggle("setting-list", "setting-hold", "Hold assists", "hold-assists", "settings-content");
    ui.toggle("setting-list", "setting-narration", "Menu narration", "narration",
              "settings-content");
}

void gameplay(ViewBuilder& ui) {
    ui.select("setting-list", "setting-tutorials", "Context tutorials", "tutorials",
              {"Off", "Smart", "Always"}, "settings-content");
    ui.slider("setting-list", "setting-camera-shake", "Camera shake", "camera-shake",
              "settings-content", 0, 1, 0.01);
    ui.toggle("setting-list", "setting-auto-pause", "Pause when unfocused", "auto-pause",
              "settings-content");
    ui.toggle("setting-list", "setting-checkpoints", "Checkpoint hints", "checkpoint-hints",
              "settings-content");
    ui.toggle("setting-list", "setting-timer", "Speedrun timer", "speedrun-timer",
              "settings-content");
    ui.toggle("setting-list", "setting-telemetry", "Anonymous telemetry", "telemetry",
              "settings-content");
}

} // namespace

void build_settings(ViewBuilder& ui, const TrialModel& model, std::string_view content) {
    setting_tabs(ui, content, model);
    ui.container(content, "settings-workspace", ui.split(), {glayout::LengthKind::Fill, 1.0f},
                 {glayout::LengthKind::Fill, 1.0f}, 12.0f);
    ui.container("settings-workspace", "setting-list", glayout::ContainerKind::Column,
                 {glayout::LengthKind::Fill, 1.0f}, {glayout::LengthKind::Fill, 1.0f}, 6.0f,
                 {12.0f, 10.0f, 12.0f, 10.0f});
    ui.layout("setting-list").clip = true;
    ui.spec("setting-list").control = gview::ControlKind::ScrollArea;
    if (model.settings_tab == "Audio") audio(ui);
    else if (model.settings_tab == "Accessibility") accessibility(ui);
    else if (model.settings_tab == "Gameplay") gameplay(ui);
    else display(ui);
    ui.container("settings-workspace", "setting-detail", glayout::ContainerKind::Column,
                 {glayout::LengthKind::Pixels, 430.0f}, {glayout::LengthKind::Fill, 1.0f}, 8.0f,
                 {14.0f, 12.0f, 14.0f, 12.0f});
    ui.scrolling("setting-detail");
    ui.label("setting-detail", "setting-detail-kicker", "SELECTED SETTING", 24.0f, 11.0f);
    ui.label("setting-detail", "setting-detail-title", model.settings_tab + " settings", 52.0f,
             28.0f);
    ui.label("setting-detail", "setting-detail-copy",
             "Changes use typed game-provided schemas. Previewable values apply "
             "immediately; guarded display changes commit explicitly.",
             92.0f, 14.0f);
    ui.label("setting-detail", "setting-context",
             "SETTING CONTEXT\nScope        Current machine\nProfile      "
             "Vega\nRestart      Not required",
             120.0f, 13.0f);
    ui.button("setting-detail", "apply-settings", "Apply settings", "toast:Settings applied",
              "settings-content");
    ui.button("setting-detail", "reset-category", "Reset category", "toast:Category reset",
              "settings-content");
    ui.focus_group("settings-content", {}, std::string("setting-tab-") + model.settings_tab);
    ui.edge("nav-Settings", gview::NavAction::Right,
            std::string("setting-tab-") + model.settings_tab);
    ui.group_edge("setting-tabs", gview::NavAction::Down, "settings-content");
    ui.group_edge("settings-content", gview::NavAction::Up, "setting-tabs");
}
