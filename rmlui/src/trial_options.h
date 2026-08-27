#pragma once

#include <array>
#include <string>

struct Resolution {
  const char *label;
  int width;
  int height;
};

inline constexpr std::array<Resolution, 6> kResolutions{{
    {"1920 x 1080", 1920, 1080},
    {"1280 x 720", 1280, 720},
    {"Tablet 1024 x 768", 1024, 768},
    {"Phone landscape 915 x 412", 915, 412},
    {"Phone landscape 844 x 390", 844, 390},
    {"Phone portrait 412 x 915", 412, 915},
}};

inline constexpr std::array<const char *, 17> kTargetScreens{{
    "Play lobby",               "Quest picker",
    "Session settings",         "Session mods",
    "Players / Local",          "Players / Profiles",
    "Players / Devices",        "Settings / Display",
    "Settings / Audio",         "Settings / Accessibility",
    "Settings / Gameplay",      "Controls / Bindings",
    "Controls / Devices",       "Controls / Input tuning",
    "Progress",                 "Mods / Installed",
    "Mods / Catalog",
}};

inline constexpr std::array<const char *, 4> kProviderStates{{
    "Populated", "Empty", "Loading", "Error",
}};

struct TrialOptions {
  int window_width = 1280;
  int window_height = 720;
  int frame_limit = 0;
  int initial_screen = 0;
  int initial_provider_state = 0;
  bool hidden = false;
  bool tools_visible = true;
  bool benchmark = false;
  bool self_test = false;
  std::string capture_path;
};

enum class ParseResult { Run, ExitSuccess, ExitFailure };

ParseResult parse_trial_options(int argc, char **argv, TrialOptions &options);
