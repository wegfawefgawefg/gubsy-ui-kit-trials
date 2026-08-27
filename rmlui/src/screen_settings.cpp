#include "app.h"
#include "ui_markup.h"

#include <sstream>
#include <vector>

// Schema-like game and system settings surfaces.

std::string GubsyApp::BuildSettings() const {
  std::ostringstream out;
  const char *tabs[]{"Display", "Audio", "Accessibility", "Gameplay"};
  out << R"(<div class="local-tabs">)";
  for (const char *tab : tabs)
    out << R"(<button data-focus data-action="settings-tab-)" << tab
        << R"(" class=")" << markup::selected_class(state_.settings_tab == tab)
        << R"(">)" << tab << R"(</button>)";
  out << R"(<span>LB / RB change category</span></div><div class="master-detail settings-workspace"><section class="panel master-list scroll-list"><div class="scroll-body"><div class="section-title"><span><small>)"
      << state_.settings_tab << R"(</small><strong>)";
  struct Setting {
    const char *id;
    const char *name;
    const char *note;
    const char *value;
  };
  std::vector<Setting> settings;
  if (state_.settings_tab == "Display")
    settings = {
        {"fullscreen", "Fullscreen", "Use the entire selected display", "ON"},
        {"resolution", "Display resolution",
         "Output resolution for this display", "1920 × 1080⌄"},
        {"render-scale", "Render scale", "Internal 3D resolution", "100%⌄"},
        {"frame-cap", "Frame cap", "Maximum simulation frames per second",
         "144 FPS⌄"},
        {"brightness", "Brightness", "Fine-tune scene visibility", "64%"},
        {"vsync", "Vertical synchronization", "Avoid tearing when supported",
         "Adaptive⌄"}};
  else if (state_.settings_tab == "Audio")
    settings = {
        {"master-volume", "Master volume", "Final output gain", "82%"},
        {"music-volume", "Music volume", "Dynamic score and menus", "64%"},
        {"effects-volume", "Effects volume", "World and combat effects", "90%"},
        {"dialogue-volume", "Dialogue volume", "Spoken character audio",
         "100%"},
        {"dynamic-range", "Dynamic range", "Balance quiet and loud sounds",
         "Night⌄"},
        {"output-device", "Output device", "Active system audio sink",
         "Default⌄"}};
  else if (state_.settings_tab == "Accessibility")
    settings = {
        {"text-scale", "Text scale", "Interface and subtitle text", "125%⌄"},
        {"high-contrast", "High contrast focus",
         "Thicker focus and state markers", "ON"},
        {"reduce-motion", "Reduce motion", "Replace movement with fades",
         "OFF"},
        {"color-filter", "Color vision filter",
         "Transform critical color pairs", "Deuteranopia⌄"},
        {"hold-assists", "Hold input assists",
         "Convert repeated holds to toggles", "ON"},
        {"narration", "Menu narration", "Read focus changes and descriptions",
         "OFF"}};
  else
    settings = {{"tutorials", "Context tutorials",
                 "Show relevant mechanic reminders", "Smart⌄"},
                {"camera-shake", "Camera shake",
                 "Impact-driven camera movement", "35%"},
                {"auto-pause", "Pause when disconnected",
                 "Pause local sessions on device loss", "ON"},
                {"checkpoint-hints", "Checkpoint hints",
                 "Show route and recovery guidance", "ON"},
                {"speedrun-timer", "Speedrun timer",
                 "Persistent run split overlay", "OFF"},
                {"telemetry", "Anonymous telemetry",
                 "Share performance-only diagnostics", "OFF"}};
  out << settings.size()
      << R"( options</strong></span><b>SAVED LOCALLY</b></div>)";
  auto value_for = [&](const std::string &id) {
    const auto it = state_.setting_values.find(id);
    return it == state_.setting_values.end() ? std::string{} : it->second;
  };
  auto is_toggle = [](const std::string &id) {
    return id == "fullscreen" || id == "high-contrast" ||
           id == "reduce-motion" || id == "hold-assists" || id == "narration" ||
           id == "auto-pause" || id == "checkpoint-hints" ||
           id == "speedrun-timer" || id == "telemetry";
  };
  auto is_range = [](const std::string &id) {
    return id == "render-scale" || id == "brightness" ||
           id == "master-volume" || id == "music-volume" ||
           id == "effects-volume" || id == "dialogue-volume" ||
           id == "camera-shake";
  };
  auto options_for = [](const std::string &id) -> std::vector<std::string> {
    if (id == "resolution")
      return {"1280 × 720", "1920 × 1080", "2560 × 1440", "3840 × 2160"};
    if (id == "frame-cap")
      return {"60 FPS", "120 FPS", "144 FPS", "Unlimited"};
    if (id == "vsync")
      return {"Off", "On", "Adaptive"};
    if (id == "dynamic-range")
      return {"Night", "Medium", "Wide"};
    if (id == "output-device")
      return {"Default", "HDMI", "USB Headset"};
    if (id == "text-scale")
      return {"100%", "125%", "150%", "200%"};
    if (id == "color-filter")
      return {"Off", "Deuteranopia", "Protanopia", "Tritanopia"};
    if (id == "tutorials")
      return {"Off", "Smart", "Always"};
    return {};
  };
  std::string selected_id = "fullscreen";
  for (const Setting &setting : settings) {
    const std::string id = setting.id;
    const std::string value = value_for(id);
    if (state_.selected_setting == setting.name)
      selected_id = id;
    out << R"(<div id="setting-row-)" << id << R"(" data-action="setting-)"
        << setting.name << R"(" class=")"
        << markup::selected_class(state_.selected_setting == setting.name,
                          "setting-row")
        << R"("><span><strong>)" << setting.name << R"(</strong><small>)"
        << setting.note << R"(</small></span><div class="setting-control">)";
    if (is_toggle(id)) {
      out << R"(<input data-focus data-action="setting-value-)" << id
          << R"(" data-setting-name=")" << setting.name
          << R"(" class="setting-toggle" type="checkbox" value="true")";
      if (value == "true")
        out << R"( checked="checked")";
      out << R"(/><b id="value-)" << id << R"(">)"
          << markup::format_setting_value(id, value) << R"(</b>)";
    } else if (is_range(id)) {
      const int minimum = id == "render-scale" ? 50 : 0;
      const int maximum = id == "render-scale" ? 150 : 100;
      out << R"(<input data-focus data-action="setting-value-)" << id
          << R"(" data-setting-name=")" << setting.name
          << R"(" class="setting-range" type="range" min=")" << minimum
          << R"(" max=")" << maximum << R"(" step="1" value=")"
          << markup::escape_attribute(value) << R"("/><b id="value-)" << id << R"(">)"
          << markup::format_setting_value(id, value) << R"(</b>)";
    } else {
      out << R"(<select data-focus data-action="setting-value-)" << id
          << R"(" data-setting-name=")" << setting.name
          << R"(" class="setting-select">)";
      for (const std::string &option : options_for(id)) {
        out << R"(<option value=")" << markup::escape_attribute(option) << R"(")";
        if (option == value)
          out << R"( selected="selected")";
        out << R"(>)" << option << R"(</option>)";
      }
      out << R"(</select>)";
    }
    out << R"(</div></div>)";
  }
  const std::string selected_value = value_for(selected_id);
  out << R"(</div></section><aside class="panel detail setting-detail"><small>SELECTED SETTING</small><h2 id="setting-detail-title">)"
      << state_.selected_setting
      << R"(</h2><p>Change the native control directly. Values stay in local prototype state and the game-facing provider would validate and commit them.</p><div class="value-box setting-current"><small>CURRENT VALUE</small><strong id="setting-detail-current">)"
      << markup::format_setting_value(selected_id, selected_value)
      << R"(</strong></div><div class="impact"><h3>SETTING CONTEXT</h3><div class="kv"><span>Scope</span><b>Current machine</b></div><div class="kv"><span>Applies</span><b>Immediately</b></div><div class="kv"><span>Profile</span><b>Vega</b></div><div class="kv"><span>Restart</span><b>Not required</b></div></div><div class="detail-actions"><button data-focus data-action="apply-setting" class="button primary">Apply )"
      << state_.settings_tab
      << R"( settings</button><button data-focus data-action="reset-category" class="button">Reset category</button></div></aside></div>)";
  return out.str();
}
